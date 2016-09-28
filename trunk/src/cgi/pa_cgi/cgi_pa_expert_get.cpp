#include "cgi_pa_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"

class CgiPaExpertGet: public CgiPaServer
{
public:
	CgiPaExpertGet():
		CgiPaServer(0, "config.ini", "logger.properties")
	{		
	}

	int FetchUserInfo(::hoosho::msg::Msg& stResponseMsg)
	{
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(NULL));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stRequest.mutable_query_user_detail_info_req();

		for(int i=0; i<stResponseMsg.pa_expert_query_res().openid_md5_list_size(); i++)
		{
			uint64_t openid_md5 =  stResponseMsg.pa_expert_query_res().openid_md5_list(i);
			pQueryUserDetailInfoReq->add_openid_md5_list(openid_md5);
		}		

		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(userServerIP, userServerPort);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return -1;	
		}

		LOG4CPLUS_DEBUG(logger, "response Msg: \n" << stResponse.Utf8DebugString());		
		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_DETAIL_INFO_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return -1;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return -1;	
		}

		for(int i=0; i<stResponse.query_user_detail_info_res().user_detail_info_list_size(); i++)
		{
			lce::cgi::CAnyValue avUserDetailInfo;
			const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
			UserInfoPB2Any(stUserDetailInfo, avUserDetailInfo);

			GetAnyValue()["user_info"][ int_2_str(stUserDetailInfo.openid_md5()) ] = avUserDetailInfo;
		}		
		
		return 0;
	}

	int FetchFeedsInfo(::hoosho::msg::Msg& stResponseMsg, uint64_t pa_appid_md5)
	{
		for(int i=0; i<stResponseMsg.pa_expert_query_res().openid_md5_list_size(); i++)
		{
			::hoosho::msg::Msg stFeedsServerRequest;
			::hoosho::msg::MsgHead* pHead = stFeedsServerRequest.mutable_head();
			pHead->set_cmd(::hoosho::msg::QUERY_FEED_REQ);
			pHead->set_seq(time(NULL));
			::hoosho::feeds::QueryFeedReq* pQueryFeedReq = stFeedsServerRequest.mutable_query_feed_req();

			uint64_t openid_md5 =  stResponseMsg.pa_expert_query_res().openid_md5_list(i);

			pQueryFeedReq->set_pa_appid_md5(pa_appid_md5);
			pQueryFeedReq->set_openid_md5(openid_md5);
			pQueryFeedReq->set_begin_feed_id(0);
			pQueryFeedReq->set_limit(getFeedsLimit);
			pQueryFeedReq->set_type(2);
			pQueryFeedReq->set_feed_type(::hoosho::commenum::FEEDS_FEED_TYPE_BASE_FEED|::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED|::hoosho::commenum::FEEDS_FEED_TYPE_FEED_OF_TOPIC);
			pQueryFeedReq->set_origin_feed_id(0);
			
			//io
			std::string strErrMsg = "";
			::hoosho::msg::Msg stFeedsServerResponse;			
			::common::protoio::ProtoIOTcpClient stIOClient2FeedsServer(feedsServerIP, feedsServerPort);
			int iRet = stIOClient2FeedsServer.io(stFeedsServerRequest, stFeedsServerResponse, strErrMsg);
			if(iRet < 0)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "Process failed, stIOClient2FeedsServer.io, strErrMsg="<<strErrMsg);
				return -1;	
			}
			
			LOG4CPLUS_DEBUG(logger, "response Msg: \n" << stFeedsServerResponse.Utf8DebugString());		
			//parse response
			const ::hoosho::msg::MsgHead& stFeedsServerResponseHead = stFeedsServerResponse.head();
			if(stFeedsServerResponseHead.cmd() != ::hoosho::msg::QUERY_FEED_RES)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "response.cmd="<<stFeedsServerResponseHead.cmd()<<", unknown, fuck!!!");
				return -1;	
			}
			
			if(stFeedsServerResponseHead.result() != ::hoosho::msg::E_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "response.result="<<stFeedsServerResponseHead.result());
				return -1;	
			}

			int feedListSize = stFeedsServerResponse.query_feed_res().feed_list_size();
			if(0 == feedListSize)
			{
				GetAnyValue()["feeds_list"][ int_2_str(openid_md5) ] = "";
			}
			else
			{
				for(int j=0; j<feedListSize; j++)
				{
					lce::cgi::CAnyValue avFeedInfo;
					const ::hoosho::commstruct::FeedInfo& stFeedInfo = stFeedsServerResponse.query_feed_res().feed_list(j);
					FeedInfoPB2Any(stFeedInfo, avFeedInfo);
					
					GetAnyValue()["feeds_list"][ int_2_str(stFeedInfo.openid_md5()) ] = avFeedInfo;
				}		
			}			

			//Get feedsnum
			GetAnyValue()["feeds_num_list"][ int_2_str(openid_md5) ] = stFeedsServerResponse.query_feed_res().total();
		}		
		
		return 0;
	}
	
	int FetchFansNum(::hoosho::msg::Msg& stResponseMsg)
	{		
		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_FANS_NUM_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserFansNumReq* pQueryUserFansNumReq = stRequest.mutable_query_user_fans_num_req();
		for(int i=0; i<stResponseMsg.pa_expert_query_res().openid_md5_list_size(); i++)
		{
			uint64_t openid_md5 =  stResponseMsg.pa_expert_query_res().openid_md5_list(i);
			pQueryUserFansNumReq->add_openid_md5_list(openid_md5);
		}

		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(userServerIP, userServerPort);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "FetchFansNum failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return -1;	
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_FANS_NUM_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return -1;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return -1;	
		}

		for(int i=0; i<stResponse.query_user_fans_num_res().fans_num_list_size(); ++i)
		{
			const ::hoosho::commstruct::KeyValueIntInt& stPair = stResponse.query_user_fans_num_res().fans_num_list(i);
			GetAnyValue()["fans_num_list"][int_2_str(stPair.key())] = stPair.value();
			
			/*
			lce::cgi::CAnyValue stAnyValue;
			stAnyValue["openid_md5"] = int_2_str(stPair.key());
			stAnyValue["fans_num"] = stPair.value();
			GetAnyValue()["fans_num_list"].push_back(stAnyValue);
			*/
			
		}

		return 0;
		
	}
	
	bool InnerProcess()
	{
		//1.get HTTP params
		uint64_t pa_appid_md5 = strtoul(((string)GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t begin_create_ts = strtoul(((string)GetInput().GetValue("begin_create_ts")).c_str(), NULL, 10);
		uint64_t limit = strtoul(((string)GetInput().GetValue("limit")).c_str(), NULL, 10);

		if(0 == pa_appid_md5)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID PA APPID MD5");
			return true;
		}

		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::QUERY_PA_EXPERT_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::pa::QueryPaExpertReq* stQueryPaExpertReq = stRequestMsg.mutable_pa_expert_query_req();
		stQueryPaExpertReq->set_appid_md5(pa_appid_md5);
		stQueryPaExpertReq->set_begin_create_ts(begin_create_ts);
		stQueryPaExpertReq->set_limit(limit);

		//3.send to server, and recv responseMsg protobuf
		common::protoio::ProtoIOTcpClient ioclient(paServerIP, paServerPort);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

		if(iRet != 0)
		{
			LOG4CPLUS_DEBUG(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "response Msg: \n" << stResponseMsg.Utf8DebugString());
		//parse response
		const ::hoosho::msg::MsgHead& stMsgHead = stResponseMsg.head();
		if(stMsgHead.cmd() != ::hoosho::msg::QUERY_PA_EXPERT_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stMsgHead.cmd()<<", unknown, fuck!!!");
			return true;
		}

		if(stMsgHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stMsgHead.result());
			return true;
		}

		if(stResponseMsg.pa_expert_query_res().openid_md5_list_size() == 0)
		{
			GetAnyValue()["openid_md5_list"] = "";
			DoReply(CGI_RET_CODE_OK);
			return true;
		}

		//4.Build strResponse Json from responseMsg protobuf
		lce::cgi::CAnyValue avOpenidMd5List;
		for(int i=0; i<stResponseMsg.pa_expert_query_res().openid_md5_list_size(); i++)
		{
			uint64_t openid_md5 =  stResponseMsg.pa_expert_query_res().openid_md5_list(i);
			avOpenidMd5List.push_back(int_2_str(openid_md5));
		}

		GetAnyValue()["openid_md5_list"] = avOpenidMd5List;		

		//5.Get userinfo
		if(FetchUserInfo(stResponseMsg) < 0)
			return true;	
		
		//6.Get feedsinfolist
		if(FetchFeedsInfo(stResponseMsg, pa_appid_md5) < 0)
			return true;

		//7.Get fansnum
		if(FetchFansNum(stResponseMsg) < 0)
			return true;

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
	
};

int main(int argc, char** argv)
{
	CgiPaExpertGet cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
