#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiUserActivityInfoGet: public CgiServer
{
public:
	CgiUserActivityInfoGet(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		std::string strInfoId = (std::string)GetInput().GetValue("info_id");
		uint32_t dwActivityType = (uint32_t)GetInput().GetValue("activity_type");

		EMPTY_STR_RETURN(strCode);
		EMPTY_STR_RETURN(strInfoId);
		ZERO_INT_RETURN(dwActivityType);


		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stRequestMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_USER_ACTIVITY_INFO_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetUserActivityInfoReq* pGetUserActivityInfoReq = pZMsgReq->mutable_get_user_activity_info_req();
		pGetUserActivityInfoReq->set_info_id(strInfoId);
		pGetUserActivityInfoReq->set_activity_type(dwActivityType);

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
		if(stZMsgRes.sub_cmd() != ::hoosho::msg::z::GET_USER_ACTIVITY_INFO_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stZMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//5.feedback		
		GetAnyValue()["activity_info"] = stZMsgRes.get_user_activity_info_res().user_activity_info().activity_info();
		

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main(int argc, char **argv)
{
	CgiUserActivityInfoGet cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

