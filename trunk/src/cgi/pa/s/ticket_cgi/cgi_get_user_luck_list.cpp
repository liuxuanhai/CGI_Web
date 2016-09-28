#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiGetUserLuckList: public CgiServer
{
public:
	CgiGetUserLuckList(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		uint64_t qwBeginFeedid = strtoul(((std::string)GetInput().GetValue("begin_feed_id")).c_str(), NULL, 10);
		uint64_t qwUserid = strtoul(((std::string)GetInput().GetValue("user_id")).c_str(), NULL, 10);
		uint32_t dwLimit = (uint32_t)GetInput().GetValue("limit");

		EMPTY_STR_RETURN(strCode);
		ZERO_INT_RETURN(dwLimit);

		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::S_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::s::MsgReq* pSMsgReq = stRequestMsg.mutable_s_msg_req();
		pSMsgReq->set_sub_cmd(::hoosho::msg::s::GET_USER_LUCK_LIST_REQ);
		pSMsgReq->set_code(strCode);

		::hoosho::msg::s::GetUserLuckListReq *pGetUserLuckListReq = pSMsgReq->mutable_get_user_luck_list_req();
		pGetUserLuckListReq->set_begin_feed_id(qwBeginFeedid);
		pGetUserLuckListReq->set_limit(dwLimit);
		pGetUserLuckListReq->set_user_id(qwUserid);

		//3.Send to SServer, and recv responseMsg
		common::protoio::ProtoIOTcpClient ioclient(SServerIP, SServerPort);

		std::string strErrMsg = "";
		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

		if(iRet != 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = "<<strErrMsg);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "response Msg: \n"<<stResponseMsg.Utf8DebugString());

		//4.parse responseMsg
		const ::hoosho::msg::MsgHead& stHead = stResponseMsg.head();
		if(stHead.cmd() != ::hoosho::msg::S_PROJECT_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;
		}
		SERVER_NOT_OK_RETURN(stHead.result());
		
		const ::hoosho::msg::s::MsgRes& stSMsgRes = stResponseMsg.s_msg_res();
		if(stSMsgRes.sub_cmd() != ::hoosho::msg::s::GET_USER_LUCK_LIST_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stSMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//5.feedback		
		const ::hoosho::msg::s::GetUserLuckListRes stGetUserLuckListRes = stSMsgRes.get_user_luck_list_res();

		for(int i = 0; i < stGetUserLuckListRes.feed_info_list_size(); i++)
		{
			GetAnyValue()["feed_info_list"].push_back(FeedInfoPB2Any(stGetUserLuckListRes.feed_info_list(i)));	
		}
		for(int i = 0; i < stGetUserLuckListRes.good_info_list_size(); i++)
		{
			GetAnyValue()["good_info_list"][int_2_str(stGetUserLuckListRes.good_info_list(i).good_id())]
					= GoodInfoPB2Any(stGetUserLuckListRes.good_info_list(i));
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main(int argc, char **argv)
{
	CgiGetUserLuckList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

