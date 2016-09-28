#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiUserActivityInfoUpdate: public CgiServer
{
public:
	CgiUserActivityInfoUpdate(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		std::string strInfoId = (std::string)GetInput().GetValue("info_id");
		uint32_t dwActivityType = (uint32_t)GetInput().GetValue("activity_type");
		std::string strActivityInfo = (std::string)GetInput().GetValue("activity_info");

		EMPTY_STR_RETURN(strCode);
		EMPTY_STR_RETURN(strInfoId);
		ZERO_INT_RETURN(dwActivityType);
		EMPTY_STR_RETURN(strActivityInfo);

		if(strInfoId.length() > 120)
		{
			LOG4CPLUS_ERROR(logger, "length=" << strInfoId.length() << ", too large");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stRequestMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::UPDATE_USER_ACTIVITY_INFO_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::UpdateUserActivityInfoReq* pUpdateUserActivityInfoReq = pZMsgReq->mutable_update_user_activity_info_req();
		::hoosho::msg::z::UserActivityInfo *pUserActivityInfo = pUpdateUserActivityInfoReq->mutable_user_activity_info();
		pUserActivityInfo->set_info_id(strInfoId);
		pUserActivityInfo->set_activity_type(dwActivityType);
		pUserActivityInfo->set_activity_info(strActivityInfo);

		//3.Send to ZServer, and recv responseMsg
		common::protoio::ProtoIOTcpClient ioclient(ZServerIP, ZServerPort);

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
		if(stHead.cmd() != ::hoosho::msg::Z_PROJECT_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;
		}
		SERVER_NOT_OK_RETURN(stHead.result());
		
		
		const ::hoosho::msg::z::MsgRes& stZMsgRes = stResponseMsg.z_msg_res();
		if(stZMsgRes.sub_cmd() != ::hoosho::msg::z::UPDATE_USER_ACTIVITY_INFO_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stZMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//5.feedback

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main(int argc, char **argv)
{
	CgiUserActivityInfoUpdate cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

