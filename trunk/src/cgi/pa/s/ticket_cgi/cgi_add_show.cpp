#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiAddShow: public CgiServer
{
public:
	CgiAddShow(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		uint64_t qwFeedid = strtoul(((std::string)GetInput().GetValue("feed_id")).c_str(), NULL, 10);
		std::string strTitle = (std::string)GetInput().GetValue("title");
		std::string strDesc = (std::string)GetInput().GetValue("desc");
		std::string strPics = (std::string)GetInput().GetValue("show_pics");

		EMPTY_STR_RETURN(strCode);
		ZERO_INT_RETURN(qwFeedid);
		EMPTY_STR_RETURN(strTitle);
		EMPTY_STR_RETURN(strDesc);
		EMPTY_STR_RETURN(strPics);

		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::S_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::s::MsgReq* pSMsgReq = stRequestMsg.mutable_s_msg_req();
		pSMsgReq->set_sub_cmd(::hoosho::msg::s::ADD_SHOW_REQ);
		pSMsgReq->set_code(strCode);

		::hoosho::msg::s::AddShowReq* pAddShowReq = pSMsgReq->mutable_add_show_req();
		::hoosho::msg::s::ShowInfo* pShowInfo = pAddShowReq->mutable_show_info();
		pShowInfo->set_feed_id(qwFeedid);
		pShowInfo->set_title(strTitle);
		pShowInfo->set_show_desc(strDesc);
		pShowInfo->set_show_pics(strPics);

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
		if(stSMsgRes.sub_cmd() != ::hoosho::msg::s::ADD_SHOW_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stSMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}
		

		//5.feedback
		GetAnyValue()["show_id"] = int_2_str(stSMsgRes.add_show_res().show_id());

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main(int argc, char **argv)
{
	CgiAddShow cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
