#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiUserRecvAddrInfoAdd: public CgiServer
{
public:
	CgiUserRecvAddrInfoAdd(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		std::string strRecvName = (std::string)GetInput().GetValue("recv_name");
		std::string strIdentityNum = (std::string)GetInput().GetValue("identity_num");
		std::string strPhone = (std::string)GetInput().GetValue("phone");
		std::string strPostNum = (std::string)GetInput().GetValue("post_num");
		std::string strAddrProvince = (std::string)GetInput().GetValue("addr_province");
		std::string strAddrCity = (std::string)GetInput().GetValue("addr_city");
		std::string strAddrDistinct = (std::string)GetInput().GetValue("addr_distinct");
		std::string strAddrDetail = (std::string)GetInput().GetValue("addr_detail");
		uint32_t dwDefaultAddr = (uint32_t)GetInput().GetValue("default_addr");

		EMPTY_STR_RETURN(strCode);
		EMPTY_STR_RETURN(strRecvName);
		EMPTY_STR_RETURN(strIdentityNum);
		EMPTY_STR_RETURN(strPhone);
		EMPTY_STR_RETURN(strPostNum);
		EMPTY_STR_RETURN(strAddrProvince);
		EMPTY_STR_RETURN(strAddrCity);
		EMPTY_STR_RETURN(strAddrDistinct);
		EMPTY_STR_RETURN(strAddrDetail);


		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::S_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::s::MsgReq* pSMsgReq = stRequestMsg.mutable_s_msg_req();
		pSMsgReq->set_sub_cmd(::hoosho::msg::s::ADD_USER_RECV_ADDR_INFO_REQ);
		pSMsgReq->set_code(strCode);

		::hoosho::msg::s::AddUserRecvAddrInfoReq* pAddUserRecvAddrInfoReq = pSMsgReq->mutable_add_user_recv_addr_info_req();
		pAddUserRecvAddrInfoReq->set_set_default_addr(dwDefaultAddr);

		::hoosho::msg::s::UserRecvAddrInfo *pUserRecvAddrInfo = pAddUserRecvAddrInfoReq->mutable_user_recv_addr_info();

		pUserRecvAddrInfo->set_recv_name(strRecvName);
		pUserRecvAddrInfo->set_identity_num(strIdentityNum);
		pUserRecvAddrInfo->set_phone(strPhone);
		pUserRecvAddrInfo->set_post_num(strPostNum);
		pUserRecvAddrInfo->set_addr_province(strAddrProvince);
		pUserRecvAddrInfo->set_addr_city(strAddrCity);
		pUserRecvAddrInfo->set_addr_district(strAddrDistinct);
		pUserRecvAddrInfo->set_addr_detail(strAddrDetail);

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
		if(stSMsgRes.sub_cmd() != ::hoosho::msg::s::ADD_USER_RECV_ADDR_INFO_RES)
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
	CgiUserRecvAddrInfoAdd cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

