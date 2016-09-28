#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiTicketAdd: public CgiServer
{
public:
	CgiTicketAdd(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		uint64_t qwFeedid = strtoul(((std::string)GetInput().GetValue("feed_id")).c_str(), NULL, 10);
		std::string strPics = (std::string)GetInput().GetValue("pics");

		EMPTY_STR_RETURN(strCode);
		ZERO_INT_RETURN(qwFeedid);
		EMPTY_STR_RETURN(strPics);

		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::S_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::s::MsgReq* pSMsgReq = stRequestMsg.mutable_s_msg_req();
		pSMsgReq->set_sub_cmd(::hoosho::msg::s::ADD_TICKET_REQ);
		pSMsgReq->set_code(strCode);

		::hoosho::msg::s::AddTicketReq* pAddTicketReq = pSMsgReq->mutable_add_ticket_req();
		::hoosho::msg::s::TicketInfo* pTicketInfo = pAddTicketReq->mutable_ticket_info();
		pTicketInfo->set_feed_id(qwFeedid);
		pTicketInfo->set_ticket_pics(strPics);
		pTicketInfo->set_user_ip(GetInput().GetClientIPStr());

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
		if(stSMsgRes.sub_cmd() != ::hoosho::msg::s::ADD_TICKET_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stSMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//5.feedback
		GetAnyValue()["ticket_id"] = int_2_str(stSMsgRes.add_ticket_res().ticket_id());

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main(int argc, char **argv)
{
	CgiTicketAdd cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
