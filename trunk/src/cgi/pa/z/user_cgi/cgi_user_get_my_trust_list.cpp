#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiUserGetMyTrustList: public CgiServer
{
public:
	CgiUserGetMyTrustList(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		std::string strOpenid = (std::string)GetInput().GetValue("openid");
		std::string strOffsetOpenid = (std::string)GetInput().GetValue("offset_openid");
		uint32_t iLimit = (int)GetInput().GetValue("limit");
		
		if(strCode.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "code empty!");
			return true;
		}

		if(iLimit < 0 || iLimit > 50)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "available param: 0 < limit <= 50");
			return true;
		}		

		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stRequestMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_MY_TRUST_LIST_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetMyTrustListReq* pGetMyTrustListReq = pZMsgReq->mutable_get_my_trust_list_req();
		pGetMyTrustListReq->set_openid(strOpenid);
		pGetMyTrustListReq->set_offset_openid(strOffsetOpenid);
		pGetMyTrustListReq->set_limit(iLimit);

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
		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			if(stHead.result() == ::hoosho::msg::E_LOGIN_INVALID)
				DoReply(CGI_RET_CODE_NO_LOGIN);			
			else
				DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;
		}
		
		const ::hoosho::msg::z::MsgRes& stZMsgRes = stResponseMsg.z_msg_res();
		if(stZMsgRes.sub_cmd() != ::hoosho::msg::z::GET_MY_TRUST_LIST_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stZMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//5.feedback
		for(int i=0; i<stZMsgRes.user_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::msg::z::UserInfo& stUserInfo = stZMsgRes.user_info_list(i);
			UserInfoPB2Any(stUserInfo, stAnyValue);	
			GetAnyValue()["user_info_list"].push_back(stAnyValue);						
		}		

		const ::hoosho::msg::z::GetMyTrustListRes& stGetMyTrustListRes = stZMsgRes.get_my_trust_list_res();
		for(int i=0; i<stGetMyTrustListRes.user_extra_info_list_size(); i++)
		{
			lce::cgi::CAnyValue item, stAnyValue;
			const ::hoosho::msg::z::UserExtraInfo& stUserExtraInfo = stGetMyTrustListRes.user_extra_info_list(i);
			UserExtraInfoPB2Any(stUserExtraInfo, stAnyValue);
			item[stUserExtraInfo.openid()] = stAnyValue;					
			GetAnyValue()["user_extra_info_list"].push_back(item);		
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main(int argc, char **argv)
{
	CgiUserGetMyTrustList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

