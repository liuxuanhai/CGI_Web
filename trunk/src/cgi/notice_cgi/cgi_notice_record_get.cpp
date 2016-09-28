#include "notice_server_cgi.h"
#include "proto_io_tcp_client.h"

class CgiNoticeRecordGet: public NoticeServerCgi
{
public:
	CgiNoticeRecordGet() :
			NoticeServerCgi(0, "config.ini", "logger.properties")
	{

	}
	int FetchCommentInfo(set<uint64_t>& setFollowId, 							
							uint64_t iPaAppidMd5, 
							uint64_t iOpenidMd5)
	{
		set<uint64_t> setOpenidMd5;
		set<uint64_t> setFeedId;
		
		::hoosho::msg::Msg stRequestFeedServerMsg;
		::hoosho::msg::Msg stResponseFeedServerMsg;
		::hoosho::msg::MsgHead* stRequestFeedServerMsgHead = stRequestFeedServerMsg.mutable_head();
		stRequestFeedServerMsgHead->set_cmd(hoosho::msg::QUERY_FOLLOW_DETAIL_REQ);
		stRequestFeedServerMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFollowDetailReq* stQueryFollowDetailReq = stRequestFeedServerMsg.mutable_query_follow_detail_req();
		stQueryFollowDetailReq->set_pa_appid_md5(iPaAppidMd5);
		stQueryFollowDetailReq->set_openid_md5(iOpenidMd5);

		for (set<uint64_t>::iterator iter = setFollowId.begin(); iter != setFollowId.end(); iter++)
		{
			stQueryFollowDetailReq->add_follow_id_list(*iter);
		}

		//3.send to server, and recv responseMsg protobuf
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);

		std::string strErrMsg = "";
		int iRet = ioclient.io(stRequestFeedServerMsg, stResponseFeedServerMsg, strErrMsg);
		if (iRet != 0)
		{
			LOG4CPLUS_ERROR(logger, "FetchCommentInfo IO failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return -1;
		}
		//	LOG4CPLUS_TRACE(logger, "resp = " << stResponseFeedServerMsg.Utf8DebugString());
		iRet = stResponseFeedServerMsg.head().result();

		if (iRet != hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "QUERY_FOLLOW_DETAIL_RES.result != E_OK");

			return -1;
		}

		const hoosho::feeds::QueryFollowDetailRes& stQueryFollowDetailRes = stResponseFeedServerMsg.query_follow_detail_res();

		LOG4CPLUS_DEBUG(logger, "QUERY_FOLLOW_DETAIL_RES Msg: \n" << stResponseFeedServerMsg.Utf8DebugString());	

		for (int i = 0; i < stQueryFollowDetailRes.follow_list_size(); i++)
		{
			setOpenidMd5.insert(stQueryFollowDetailRes.follow_list(i).openid_md5_from());
			setFeedId.insert(stQueryFollowDetailRes.follow_list(i).feed_id());
		}

		//Get FeedInfo
		if(FetchFeedsInfo(setFeedId, iPaAppidMd5, iOpenidMd5) < 0)
			return -1;
		
		//Get UserInfo
		if(FetchUserInfo(setOpenidMd5) < 0)
			return -1;	


		//Gen FollowId set & FollowInfo list
		setFollowId.clear();
		lce::cgi::CAnyValue avFollowList;
		for (int i = stQueryFollowDetailRes.follow_list_size()-1; i > 0; i--)
		{
			uint64_t tmp_follow_feed_id = stQueryFollowDetailRes.follow_list(i).feed_id();
			uint64_t tmp_follow_openid_md5 = stQueryFollowDetailRes.follow_list(i).openid_md5_from();

			if(setFeedId.find(tmp_follow_feed_id) == setFeedId.end() || setOpenidMd5.find(tmp_follow_openid_md5) == setOpenidMd5.end())
				continue;

			setFollowId.insert(stQueryFollowDetailRes.follow_list(i).follow_id());	
			avFollowList.push_back(ToAnyValue(stQueryFollowDetailRes.follow_list(i)));
		}
		
		GetAnyValue()["follow_list"] = avFollowList;

		return 0;
	}
	
	int FetchFeedsInfo(set<uint64_t>& setFeedId, uint64_t iPaAppidMd5, uint64_t iOpenidMd5)
	{
		::hoosho::msg::Msg stFeedDetailRequestMsg;
		::hoosho::msg::Msg stResponseFeedServerMsg;
		::hoosho::msg::MsgHead* stFeedDetailRequestMsgHead = stFeedDetailRequestMsg.mutable_head();
		stFeedDetailRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_DETAIL_REQ);
		stFeedDetailRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFeedDetailReq* stQueryFeedDetailReq = stFeedDetailRequestMsg.mutable_query_feed_detail_req();
		stQueryFeedDetailReq->set_pa_appid_md5(iPaAppidMd5);
		stQueryFeedDetailReq->set_openid_md5(iOpenidMd5);

		for (set<uint64_t>::iterator iter = setFeedId.begin(); iter != setFeedId.end(); iter++)
		{
			stQueryFeedDetailReq->add_feed_id_list(*iter);
		}
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);

		std::string strErrMsg = "";
		int iRet = ioclient.io(stFeedDetailRequestMsg, stResponseFeedServerMsg, strErrMsg);
		if (iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "FetchFeedsInfo IO failed, strErrMsg="<<strErrMsg);
			return -1;
		}
		//parse response
		if (stResponseFeedServerMsg.head().cmd() != ::hoosho::msg::QUERY_FEED_DETAIL_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "QUERY_FEED_DETAIL_RES.cmd="<<stResponseFeedServerMsg.head().cmd()<<", unknown, fuck!!!");
			return -1;
		}

		if (stResponseFeedServerMsg.head().result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "QUERY_FEED_DETAIL_RES.result != E_OK, "<<stResponseFeedServerMsg.head().result());
			return -1;
		}

		LOG4CPLUS_DEBUG(logger, "QUERY_FEED_DETAIL_RES Msg: \n" << stResponseFeedServerMsg.Utf8DebugString());	
		
		setFeedId.clear();
		for (int i = 0; i < stResponseFeedServerMsg.query_feed_detail_res().feed_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::commstruct::FeedInfo& stFeedInfo = stResponseFeedServerMsg.query_feed_detail_res().feed_list(i);

			GetAnyValue()["feed_info"][int_2_str(stFeedInfo.feed_id())] = ToAnyValue(stFeedInfo);
			setFeedId.insert(stFeedInfo.feed_id());
		}

		return 0;
	}

	int FetchUserInfo(set<uint64_t>& setOpenidMd5)
	{
		::hoosho::msg::Msg stUserInfoRequest;
		::hoosho::msg::MsgHead* pHead = stUserInfoRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stUserInfoRequest.mutable_query_user_detail_info_req();
		for (std::set<uint64_t>::iterator iter = setOpenidMd5.begin(); iter != setOpenidMd5.end(); ++iter)
		{
			pQueryUserDetailInfoReq->add_openid_md5_list(*iter);
		}

		//io
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);

		std::string strErrMsg = "";
		int iRet = stProtoIOTcpClient.io(stUserInfoRequest, stResponse, strErrMsg);
		if (iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "FetchUserInfo IO failed, strErrMsg="<<strErrMsg);
			return -1;
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if (stHead.cmd() != ::hoosho::msg::QUERY_USER_DETAIL_INFO_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "QUERY_USER_DETAIL_INFO_RES.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return -1;
		}

		if (stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "QUERY_USER_DETAIL_INFO_RES.result != E_OK, "<<stHead.result());
			return -1;
		}

		LOG4CPLUS_DEBUG(logger, "QUERY_USER_DETAIL_INFO_RES Msg: \n" << stResponse.Utf8DebugString());

		setOpenidMd5.clear();
		for (int i = 0; i < stResponse.query_user_detail_info_res().user_detail_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
			UserInfoPB2Any(stUserDetailInfo, stAnyValue);
			GetAnyValue()["user_info"][int_2_str(stUserDetailInfo.openid_md5())] = stAnyValue;

			setOpenidMd5.insert(stUserDetailInfo.openid_md5());
		}

		return 0;
	}
	bool InnerProcess()
	{
		uint64_t iPaAppidMd5 = strtoul(((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t iOpenidMd5 = strtoul(((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		uint32_t iType = (uint32_t) GetInput().GetValue("type");
		uint64_t iLimitTs = strtoul(((string) GetInput().GetValue("limit_ts")).c_str(), NULL, 10);
		uint32_t iPagesize = (uint32_t) GetInput().GetValue("pagesize");

		ZERO_INT_RETURN(iPaAppidMd5);
		ZERO_INT_RETURN(iOpenidMd5);
		ZERO_INT_RETURN(iType);

		if (iType < 1 || iType > 3)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid iType="<<iType);
			return true;
		}
		::common::protoio::ProtoIOTcpClient client(m_notice_server_ip, m_notice_server_port);
		std::string strErrMsg = "";
		::hoosho::msg::Msg stRequestNoticeServer;
		::hoosho::msg::Msg stResponseNoticeServer;

		::hoosho::msg::MsgHead* headerNoticeServer = stRequestNoticeServer.mutable_head();
		uint32_t iSeq = 1;
		headerNoticeServer->set_seq(iSeq);
		headerNoticeServer->set_cmd(hoosho::msg::QUERY_NOTICE_RECORD_REQ);
		::hoosho::noticenotify::QueryNoticeRecordReq* req = stRequestNoticeServer.mutable_notice_record_query_req();
		req->set_pa_appid_md5(iPaAppidMd5);
		req->set_openid_md5(iOpenidMd5);
		req->set_type(iType);
		req->set_limit_ts(iLimitTs);
		req->set_pagesize(iPagesize);

		if (client.io(stRequestNoticeServer, stResponseNoticeServer, strErrMsg) < 0)
		{
			LOG4CPLUS_ERROR(logger, "InnerProcess IO failed, strErrMsg=" << strErrMsg << std::endl;);

			return -1;
		}
		std::ostringstream oss;
		if (stResponseNoticeServer.head().cmd() != hoosho::msg::QUERY_NOTICE_RECORD_RES || stResponseNoticeServer.head().seq() != iSeq)
		{
			LOG4CPLUS_ERROR(logger, "QUERY_NOTICE_RECORD_RES.cmd = "<<stResponseNoticeServer.head().cmd()<<", unknown, fuck!!!");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;

		}
		if (stResponseNoticeServer.head().result() == hoosho::msg::E_SERVER_INNER_ERROR)
		{
			LOG4CPLUS_ERROR(logger, "QUERY_NOTICE_RECORD_RES.result = E_SERVER_INNER_ERROR" << std::endl);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		if (stResponseNoticeServer.head().result() != hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "QUERY_NOTICE_RECORD_RES.result != E_OK" << std::endl);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		
		LOG4CPLUS_DEBUG(logger, "QUERY_NOTICE_RECORD_RES Msg: \n" << stResponseNoticeServer.Utf8DebugString());

		if (iType == hoosho::commenum::NOTICE_FEEDS_COMMENT)
		{					
			set<uint64_t> setFollowId;
			
			const hoosho::noticenotify::QueryNoticeRecordRes& res = stResponseNoticeServer.notice_record_query_res();
			int iSize = res.notice_record_list_size();
			for (int i = 0; i < iSize; i++)
			{
				setFollowId.insert(strtoul(res.notice_record_list(i).extra_data_2().c_str(), NULL, 10));
			}

			//Get CommentInfo
			if(FetchCommentInfo(setFollowId, iPaAppidMd5, iOpenidMd5) < 0)
				return true;			
			
			//Gen notice record list
			for (int i = 0; i < iSize; i++)
			{
				uint64_t tmp_follow_id = strtoul(res.notice_record_list(i).extra_data_2().c_str(), NULL, 10);	//follow_id

				if(setFollowId.find(tmp_follow_id) == setFollowId.end())
					continue;
			
				lce::cgi::CAnyValue stNoticeRecord;

				stNoticeRecord["pa_appid_md5"] = int_2_str(res.notice_record_list(i).pa_appid_md5());
				stNoticeRecord["openid_md5"] = int_2_str(res.notice_record_list(i).openid_md5());
				stNoticeRecord["create_ts"] = int_2_str(res.notice_record_list(i).create_ts()/1000000);
				stNoticeRecord["type"] = res.notice_record_list(i).type();
				stNoticeRecord["status"] = res.notice_record_list(i).status();
				stNoticeRecord["extra_data_0"] = res.notice_record_list(i).extra_data_0();
				stNoticeRecord["extra_data_1"] = res.notice_record_list(i).extra_data_1();
				stNoticeRecord["extra_data_2"] = res.notice_record_list(i).extra_data_2();
				
				GetAnyValue()["notice_record_list"].push_back(stNoticeRecord);
			}

		}
		else if (iType == hoosho::commenum::NOTICE_FEEDS_FAVORITE)
		{
			set<uint64_t> setOpenidMd5;
			set<uint64_t> setFeedId;			

			const hoosho::noticenotify::QueryNoticeRecordRes& res = stResponseNoticeServer.notice_record_query_res();
			int iSize = res.notice_record_list_size();
			for (int i = 0; i < iSize; i++)
			{
				setFeedId.insert(strtoul(res.notice_record_list(i).extra_data_0().c_str(), NULL, 10));			//被点赞feed id
				setOpenidMd5.insert(strtoul(res.notice_record_list(i).extra_data_1().c_str(), NULL, 10));		//点赞用户openid_m5

			}

			//Get FeedInfo
			if(FetchFeedsInfo(setFeedId, iPaAppidMd5, iOpenidMd5) < 0)
				return true;
			
			//Get UserInfo
			if(FetchUserInfo(setOpenidMd5) < 0)
				return true;
			
			//Gen notice record list
			for (int i = 0; i < iSize; i++)
			{
				uint64_t tmp_feed_id = strtoul(res.notice_record_list(i).extra_data_0().c_str(), NULL, 10);	//被点赞feed id
				uint64_t tmp_openid_md5 = strtoul(res.notice_record_list(i).extra_data_1().c_str(), NULL, 10);	//点赞用户openid_m5
				
				if((setFeedId.find(tmp_feed_id) == setFeedId.end()) || (setOpenidMd5.find(tmp_openid_md5) == setOpenidMd5.end()))
					continue;
					
				lce::cgi::CAnyValue stNoticeRecord;

				stNoticeRecord["pa_appid_md5"] = int_2_str(res.notice_record_list(i).pa_appid_md5());
				stNoticeRecord["openid_md5"] = int_2_str(res.notice_record_list(i).openid_md5());	//feed 所属用户openid_md5
				stNoticeRecord["create_ts"] = int_2_str(res.notice_record_list(i).create_ts()/1000000);
				stNoticeRecord["type"] = res.notice_record_list(i).type();
				stNoticeRecord["status"] = res.notice_record_list(i).status();
				stNoticeRecord["extra_data_0"] = res.notice_record_list(i).extra_data_0();	//被点赞feed id
				stNoticeRecord["extra_data_1"] = res.notice_record_list(i).extra_data_1();	//点赞用户openid_m5
				stNoticeRecord["extra_data_2"] = res.notice_record_list(i).extra_data_2();
				
				GetAnyValue()["notice_record_list"].push_back(stNoticeRecord);				
			}

		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiNoticeRecordGet cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
