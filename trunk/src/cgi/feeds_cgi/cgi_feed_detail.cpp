#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiFeedDetail: public CgiFeedsServer
{
public:
	CgiFeedDetail():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	int FetchCommentList(const uint64_t& qwFeedId, vector<hoosho::commstruct::FollowCommentInfo>& vecFollowComment
		, uint64_t& qwTotal
		, const uint64_t& qwPaAppidMd5, const uint64_t& qwOpenidMd5, const uint64_t& qwLimit)
	{
		::hoosho::msg::Msg stCommentListRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		::hoosho::msg::MsgHead* stCommentListRequestMsgHead = stCommentListRequestMsg.mutable_head();
		stCommentListRequestMsgHead->set_cmd(hoosho::msg::QUERY_FOLLOW_COMMENT_REQ);
		stCommentListRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFollowCommentReq* stQueryFollowCommentReq = stCommentListRequestMsg.mutable_query_follow_comment_req();
		stQueryFollowCommentReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFollowCommentReq->set_openid_md5(qwOpenidMd5);
		stQueryFollowCommentReq->set_begin_comment_id( 0 );
		stQueryFollowCommentReq->set_limit( qwLimit );
		stQueryFollowCommentReq->set_feed_id( qwFeedId );

		string strErrMsg;
		int iRet;
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);

		iRet = ioclient.io(stCommentListRequestMsg, stResponseMsg, strErrMsg);
		if(iRet != 0 || stResponseMsg.head().result() != hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "iRet = " << iRet << ", result = " << stResponseMsg.head().result() );
			return CGI_RET_CODE_SERVER_BUSY;
		}

		::hoosho::feeds::QueryFollowCommentRes stQueryFollowCommentRes = stResponseMsg.query_follow_comment_res();

		LOG4CPLUS_DEBUG(logger, "size = " << stQueryFollowCommentRes.comment_list_size() );
		vecFollowComment.clear();
		for(int i = 0; i < stQueryFollowCommentRes.comment_list_size(); i++)
		{
			vecFollowComment.push_back( stQueryFollowCommentRes.comment_list(i) );
		}
		qwTotal = stQueryFollowCommentRes.total();

		return CGI_RET_CODE_OK;
	}

	int FetchCollectCheck(vector<uint64_t>& qwVecFeedId, vector<uint64_t>& vecCollect,
			uint64_t qwPaAppidMd5, uint64_t qwOpenidMd5)
	{
		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_LIST_COLLECT_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFeedListCollectReq* stQueryFeedListCollectReq = stRequestMsg.mutable_query_feed_list_collect_req();
		stQueryFeedListCollectReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFeedListCollectReq->set_openid_md5(qwOpenidMd5);


		for(size_t i=0; i != qwVecFeedId.size(); ++i)
		{
			stQueryFeedListCollectReq->add_feed_id_list( qwVecFeedId[i] );
		}


		//3.send to server, and recv responseMsg protobuf
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
		if(iRet != 0)
		{
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
//			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return CGI_RET_CODE_SERVER_BUSY;
		}
		LOG4CPLUS_TRACE(logger, "resp = " << stResponseMsg.Utf8DebugString());
		iRet = stResponseMsg.head().result();
		SERVER_NOT_OK_RETURN(iRet);

		const hoosho::feeds::QueryFeedListCollectRes& stQueryFeedListCollectRes = stResponseMsg.query_feed_list_collect_res();

		vecCollect.clear();
		for(int i = 0; i < stQueryFeedListCollectRes.collect_list_size(); i++)
		{
			vecCollect.push_back( stQueryFeedListCollectRes.collect_list(i) );
		}

		return CGI_RET_CODE_OK;
	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		string strFeedIdList = (string) GetInput().GetValue("feed_id_list");

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		EMPTY_STR_RETURN(strFeedIdList);


		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_DETAIL_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFeedDetailReq* stQueryFeedDetailReq = stRequestMsg.mutable_query_feed_detail_req();
		stQueryFeedDetailReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFeedDetailReq->set_openid_md5(qwOpenidMd5);

		vector<string> vecFeedId;
		lce::cgi::Split(strFeedIdList, "|", vecFeedId);

		vector<uint64_t> qwVecFeedId;
		for(size_t i=0; i != vecFeedId.size(); ++i)
		{
			qwVecFeedId.push_back(strtoul( vecFeedId[i].c_str(), NULL, 10));
			stQueryFeedDetailReq->add_feed_id_list( qwVecFeedId[i] );
		}


		//3.send to server, and recv responseMsg protobuf
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
		if(iRet != 0)
		{
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		LOG4CPLUS_TRACE(logger, "resp = " << stResponseMsg.Utf8DebugString());
		iRet = stResponseMsg.head().result();

		SERVER_NOT_OK_RETURN(iRet);

		const hoosho::feeds::QueryFeedDetailRes& stQueryFeedDetailRes = stResponseMsg.query_feed_detail_res();

		lce::cgi::CAnyValue avFeedList;

		for(int i = 0; i < stQueryFeedDetailRes.feed_list_size(); i++)
		{
			if( stQueryFeedDetailRes.feed_list(i).del_ts() )
			{
				LOG4CPLUS_ERROR(logger, "feed_id = " << stQueryFeedDetailRes.feed_list(i).feed_id() << " has deleted");
				DoReply(CGI_RET_CODE_FEED_DELETED);
				return true;
			}
			avFeedList.push_back(ToAnyValue( stQueryFeedDetailRes.feed_list(i) ));
		}

		//4.Build strResponse Json from responseMsg protobuf
		GetAnyValue()["feed_list"] = avFeedList;

		//-0- init openid_md5 set
		set<uint64_t> setOpenidMd5;
		for(int i = 0; i < stQueryFeedDetailRes.feed_list_size(); i++)
		{
			setOpenidMd5.insert( stQueryFeedDetailRes.feed_list(i).openid_md5() );
		}


		//-1- get comment_list
		for(size_t x = 0; x < qwVecFeedId.size(); x++)
		{
			uint64_t qwTotal;
			vector<hoosho::commstruct::FollowCommentInfo> vecFollowComment;

			iRet = FetchCommentList(qwVecFeedId[x], vecFollowComment, qwTotal, qwPaAppidMd5, qwOpenidMd5, m_default_show_comment_num);
			if(iRet != CGI_RET_CODE_OK)
			{
				DoReply(iRet);
				return true;
			}
			lce::cgi::CAnyValue avCommentList;
			for(size_t i = 0; i < vecFollowComment.size(); i++)
			{
				lce::cgi::CAnyValue avCommentInfo;
				avCommentInfo["comment"] = ToAnyValue( vecFollowComment[i].comment());
				setOpenidMd5.insert( vecFollowComment[i].comment().openid_md5_from() );
				LOG4CPLUS_DEBUG(logger, "follow_id = " << vecFollowComment[i].comment().follow_id());

				for(int j = 0; j < vecFollowComment[i].reply_list_size(); j++)
				{
					avCommentInfo["reply_list"].push_back( ToAnyValue(vecFollowComment[i].reply_list(j) ) );
					setOpenidMd5.insert( vecFollowComment[i].reply_list(j).openid_md5_from() );
				}
				avCommentList.push_back(avCommentInfo);
			}
			GetAnyValue()["comment_list"][ vecFeedId[x] ]["total"] = qwTotal;
			GetAnyValue()["comment_list"][ vecFeedId[x] ]["comment_list"] = avCommentList;
		}

		//-2- get favor_list
		::hoosho::msg::Msg stFavoriteListRequestMsg;

		::hoosho::msg::MsgHead* stFavoriteListRequestMsgHead = stFavoriteListRequestMsg.mutable_head();
		stFavoriteListRequestMsgHead->set_cmd(hoosho::msg::QUERY_FAVORITE_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFavoriteReq* stQueryFavoriteReq = stFavoriteListRequestMsg.mutable_query_favorite_req();
		stQueryFavoriteReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFavoriteReq->set_openid_md5(qwOpenidMd5);
		stQueryFavoriteReq->set_begin_create_ts( 0 );
		stQueryFavoriteReq->set_limit( m_default_show_favorite_num );

		for(size_t x = 0; x != vecFeedId.size(); ++x)
		{
			stQueryFavoriteReq->set_feed_id( qwVecFeedId[x] );
			iRet = ioclient.io(stFavoriteListRequestMsg, stResponseMsg, strErrMsg);
			if(iRet != 0 || stResponseMsg.head().result() != hoosho::msg::E_OK)
			{
				LOG4CPLUS_ERROR(logger, "iRet = " << iRet << ", result = " << stResponseMsg.head().result() );
				continue;
			}

			::hoosho::feeds::QueryFavoriteRes stQueryFavoriteRes = stResponseMsg.query_favorite_res();
			lce::cgi::CAnyValue avFavoriteList;

			LOG4CPLUS_DEBUG(logger, "size = " << stQueryFavoriteRes.openid_md5_list_size());
			for(int i = 0; i < stQueryFavoriteRes.openid_md5_list_size(); i++)
			{
				avFavoriteList.push_back( int_2_str(stQueryFavoriteRes.openid_md5_list(i)) );
				setOpenidMd5.insert( stQueryFavoriteRes.openid_md5_list(i) );
			}

			GetAnyValue()["favorite_list"][ vecFeedId[x] ] = avFavoriteList;
		}


		//-3- get favor_check
		::hoosho::msg::Msg stFeedListFavoriteRequestMsg;

		::hoosho::msg::MsgHead* stFeedListFavoriteRequestMsgHead = stFeedListFavoriteRequestMsg.mutable_head();
		stFeedListFavoriteRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_LIST_FAVORITE_REQ);
		stFeedListFavoriteRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFeedListFavoriteReq* stQueryFeedListFavoriteReq = stFeedListFavoriteRequestMsg.mutable_query_feed_list_favorite_req();
		stQueryFeedListFavoriteReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFeedListFavoriteReq->set_openid_md5(qwOpenidMd5);

		for(size_t i = 0; i < vecFeedId.size(); i++)
		{
			stQueryFeedListFavoriteReq->add_feed_id_list( qwVecFeedId[i] );
		}
		do{
			iRet = ioclient.io(stFeedListFavoriteRequestMsg, stResponseMsg, strErrMsg);
			if(iRet != 0 || stResponseMsg.head().result() != hoosho::msg::E_OK)
			{
				LOG4CPLUS_ERROR(logger, "iRet = " << iRet << ", result = " << stResponseMsg.head().result() );
				break;
			}

			::hoosho::feeds::QueryFeedListFavoriteRes stQueryFeedListFavoriteRes = stResponseMsg.query_feed_list_favorite_res();

			LOG4CPLUS_DEBUG(logger, "size = " << stQueryFeedListFavoriteRes.favorite_list_size());
			for(int i = 0; i < stQueryFeedListFavoriteRes.favorite_list_size(); i++)
			{
				GetAnyValue()["favorite_check"][ vecFeedId[i] ] = stQueryFeedListFavoriteRes.favorite_list(i);
			}

		}while(0);


		//-4- get user_info

		::hoosho::msg::Msg stUserInfoRequest;
		::hoosho::msg::MsgHead* pHead = stUserInfoRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stUserInfoRequest.mutable_query_user_detail_info_req();
		for(std::set<uint64_t>::iterator iter=setOpenidMd5.begin(); iter!=setOpenidMd5.end(); ++iter)
		{
			pQueryUserDetailInfoReq->add_openid_md5_list(*iter);
		}

		//io
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		iRet = stProtoIOTcpClient.io(stUserInfoRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return true;
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_DETAIL_INFO_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "size = " << stResponse.query_user_detail_info_res().user_detail_info_list_size());
		for(int i=0; i<stResponse.query_user_detail_info_res().user_detail_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
			LOG4CPLUS_DEBUG(logger, "one detailuserinfo="<<stUserDetailInfo.Utf8DebugString());
			UserInfoPB2Any(stUserDetailInfo, stAnyValue);
			GetAnyValue()["user_info"][ int_2_str(stUserDetailInfo.openid_md5()) ] = stAnyValue;
		}

		//-5- get collect_check
		vector < uint64_t > vecCollect;
		iRet = FetchCollectCheck(qwVecFeedId, vecCollect, qwPaAppidMd5, qwOpenidMd5);
		if(iRet != CGI_RET_CODE_OK)
		{
			DoReply(iRet);
			return true;
		}
		for(size_t i = 0; i < qwVecFeedId.size(); i++)
		{
			GetAnyValue()["collect_check"][ int_2_str(qwVecFeedId[i]) ] = vecCollect[i];
		}


		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiFeedDetail cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
