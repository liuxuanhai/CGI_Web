#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiUserIdentityInfoUpdate: public CgiServer
{
public:
	CgiUserIdentityInfoUpdate(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		std::string strPhone = (std::string)GetInput().GetValue("phone");
		std::string strVC = (std::string)GetInput().GetValue("vc");
		std::string strIdentityNum = (std::string)GetInput().GetValue("identity_num");
		std::string strIdentityPic1Id = (std::string)GetInput().GetValue("identity_pic1_id");
		std::string	strIdentityPic2Id = (std::string)GetInput().GetValue("identity_pic2_id");
		std::string	strIdentityPic3Id = (std::string)GetInput().GetValue("identity_pic3_id");

		EMPTY_STR_RETURN(strCode);
		EMPTY_STR_RETURN(strPhone);
		EMPTY_STR_RETURN(strVC);
		EMPTY_STR_RETURN(strIdentityNum);
		if(strIdentityNum.length() != 18)
		{
			LOG4CPLUS_ERROR(logger, "invalid strIdentityNum length=" << strIdentityNum.length());
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		EMPTY_STR_RETURN(strIdentityPic1Id);
		EMPTY_STR_RETURN(strIdentityPic2Id);
		EMPTY_STR_RETURN(strIdentityPic3Id);


		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::S_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::s::MsgReq* pSMsgReq = stRequestMsg.mutable_s_msg_req();
		pSMsgReq->set_sub_cmd(::hoosho::msg::s::UPDATE_USER_IDENTITY_INFO_REQ);
		pSMsgReq->set_code(strCode);

		::hoosho::msg::s::UpdateUserIdentityInfoReq* pUpdateUserIdentityInfoReq = pSMsgReq->mutable_update_user_identity_info_req();
		::hoosho::msg::s::UserIdentityInfo *pUserIdentityInfo = pUpdateUserIdentityInfoReq->mutable_user_identity_info();

		pUserIdentityInfo->set_identity_status(hoosho::msg::s::USER_IDENTITY_STATUS_PENDING);
		pUserIdentityInfo->set_phone(strPhone);
		pUserIdentityInfo->set_identity_num(strIdentityNum);
		pUserIdentityInfo->set_identity_pic1_id(strIdentityPic1Id);
		pUserIdentityInfo->set_identity_pic2_id(strIdentityPic2Id);
		pUserIdentityInfo->set_identity_pic3_id(strIdentityPic3Id);

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
		if(stSMsgRes.sub_cmd() != ::hoosho::msg::s::UPDATE_USER_IDENTITY_INFO_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stSMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//5.feedback

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main(int argc, char **argv)
{
	CgiUserIdentityInfoUpdate cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

