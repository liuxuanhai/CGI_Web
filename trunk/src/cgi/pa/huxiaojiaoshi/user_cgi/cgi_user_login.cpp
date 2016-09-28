#ifndef _HOOSHO_CGI_J_USER_LOGIN_H_
#define _HOOSHO_CGI_J_USER_LOGIN_H_

#include "cgi/cgi.h"
#include "../cgi_common_util.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>

class CgiUserLogin: public lce::cgi::Cgi
{
public:
	 CgiUserLogin(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("user_server", "ip", m_user_server_ip, "");
		GetConfig().GetValue("user_server", "port", m_user_server_port, 0);
		GetConfig().GetValue("cookie", "name_key", m_cookie_name_key, "");
		GetConfig().GetValue("cookie", "name_value", m_cookie_name_value, "");
		GetConfig().GetValue("cookie", "domain", m_cookie_domain, "");
		GetConfig().GetValue("cookie", "path", m_cookie_path, "");
		
		return true;
	}
	
	virtual bool Process()
	{
		std::string strSNSApiBasePreAuthCode = (std::string)GetInput().GetValue("pre_auth_code");
		LOG4CPLUS_DEBUG(logger, "strSNSApiBasePreAuthCode="<<strSNSApiBasePreAuthCode);
		
		if(strSNSApiBasePreAuthCode.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strSNSApiBasePreAuthCode is empty!!!!!!");
			return true;	
		}

		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::J_USER_LOGIN_REQ);
		pHead->set_seq(time(0));
		::hoosho::j::user::UserLoginReq* pUserLoginReq = stRequest.mutable_user_login_req();
		pUserLoginReq->set_pre_auth_code(strSNSApiBasePreAuthCode);

		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return true;	
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::J_USER_LOGIN_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;	
		}

		if(stHead.seq() != pHead->seq())
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.seq="<<stHead.seq()<<", while request.seq="<<(pHead->seq()));
			return true;	
		}

		if(stHead.result() == ::hoosho::msg::E_NOT_PA_FANS)
		{
			DoReply(CGI_RET_CODE_NOT_PA_FANS);
			LOG4CPLUS_ERROR(logger, "fuck, u are not pa fans, response.result="<<stHead.result());
			return true;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;	
		}

		//const hoosho::j::commstruct::UserInfo& stUserInfo = stResponse.user_login_res().userinfo();
		string strCookieKey = stResponse.user_login_res().cookie_key();
		string strCookieValue = stResponse.user_login_res().cookie_value();
		uint64_t qwSessionLife = stResponse.user_login_res().cookie_life();

		//send cookie & reply
	    lce::cgi::CHttpHeader& stHttpRspHeader =  GetHeader();
    	stHttpRspHeader.SetCookie(m_cookie_name_key, strCookieKey, m_cookie_domain, qwSessionLife, m_cookie_path, false, false);
    	stHttpRspHeader.SetCookie(m_cookie_name_value, strCookieValue, m_cookie_domain, qwSessionLife, m_cookie_path, false, false);
    	
        DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	string m_user_server_ip;
	int m_user_server_port;
	string m_cookie_name_key;
	string m_cookie_name_value;
	string m_cookie_domain;
	string m_cookie_path;
	
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUserLogin, logger);

int main(int argc, char** argv)
{
	CgiUserLogin cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

