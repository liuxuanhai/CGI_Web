#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiFollowDetail: public CgiFeedsServer
{
public:
	CgiFollowDetail():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		string strFollowIdList = (string) GetInput().GetValue("follow_id_list");

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		EMPTY_STR_RETURN(strFollowIdList);

			
		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_FOLLOW_DETAIL_REQ);		
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFollowDetailReq* stQueryFollowDetailReq = stRequestMsg.mutable_query_follow_detail_req();
		stQueryFollowDetailReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFollowDetailReq->set_openid_md5(qwOpenidMd5);

		vector<string> vecFollowId;
		lce::cgi::Split(strFollowIdList, "|", vecFollowId);
		for(size_t i=0; i != vecFollowId.size(); ++i)
		{
			stQueryFollowDetailReq->add_follow_id_list( strtoul( vecFollowId[i].c_str(), NULL, 10) );
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


		const hoosho::feeds::QueryFollowDetailRes& stQueryFollowDetailRes = stResponseMsg.query_follow_detail_res();
		lce::cgi::CAnyValue avFollowList;

		//-0- init openid_md5 & feed_id set
		set<uint64_t> setOpenidMd5;
		set<uint64_t> setFeedId;
		LOG4CPLUS_TRACE(logger, "list size = " << stQueryFollowDetailRes.follow_list_size());
		for(int i = 0; i < stQueryFollowDetailRes.follow_list_size(); i++)
		{
			avFollowList.push_back( ToAnyValue( stQueryFollowDetailRes.follow_list(i) ) );

			setOpenidMd5.insert( stQueryFollowDetailRes.follow_list(i).openid_md5_from() );
			setFeedId.insert( stQueryFollowDetailRes.follow_list(i).feed_id() );
		}


		//4.Build strResponse Json from responseMsg protobuf
		GetAnyValue()["follow_list"] = avFollowList;


		//-1- get feed_info
		::hoosho::msg::Msg stFeedDetailRequestMsg;
		
		::hoosho::msg::MsgHead* stFeedDetailRequestMsgHead = stFeedDetailRequestMsg.mutable_head();
		stFeedDetailRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_DETAIL_REQ);		
		stFeedDetailRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFeedDetailReq* stQueryFeedDetailReq = stFeedDetailRequestMsg.mutable_query_feed_detail_req();
		stQueryFeedDetailReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFeedDetailReq->set_openid_md5(qwOpenidMd5);

		for(set<uint64_t>::iterator iter = setFeedId.begin(); iter != setFeedId.end(); iter++)
		{
			stQueryFeedDetailReq->add_feed_id_list( *iter );
		}
		iRet = ioclient.io(stFeedDetailRequestMsg, stResponseMsg, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return true;	
		}
		//parse response
		if(stResponseMsg.head().cmd() != ::hoosho::msg::QUERY_FEED_DETAIL_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stResponseMsg.head().cmd()<<", unknown, fuck!!!");
			return true;	
		}

		if(stResponseMsg.head().result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stResponseMsg.head().result());
			return true;	
		}
		LOG4CPLUS_DEBUG(logger, "size = " << stResponseMsg.query_feed_detail_res().feed_list_size());
		for(int i=0; i<stResponseMsg.query_feed_detail_res().feed_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::commstruct::FeedInfo& stFeedInfo = stResponseMsg.query_feed_detail_res().feed_list(i);
			LOG4CPLUS_DEBUG(logger, "one feedinfo="<<stFeedInfo.Utf8DebugString());
			
			GetAnyValue()["feed_info"][ int_2_str(stFeedInfo.feed_id()) ] = ToAnyValue(stFeedInfo);	
		}
		
		//-2- get user_info		
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

		DoReply(CGI_RET_CODE_OK);
		return true;		
	}
		
};

int main()
{
	CgiFollowDetail cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
