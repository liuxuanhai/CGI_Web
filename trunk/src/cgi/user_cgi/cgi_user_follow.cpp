#ifndef _HOOSHO_CGI_USER_LOGIN_H_
#define _HOOSHO_CGI_USER_LOGIN_H_

#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"
#include <sstream>
#include <stdio.h>

class CgiUserFollow: public lce::cgi::Cgi
{
public:
	 CgiUserFollow(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("user_server", "ip", m_user_server_ip, "");
		GetConfig().GetValue("user_server", "port", m_user_server_port, 0);
		return true;
	}
	
	virtual bool Process()
	{
		std::string strOpenidMd5From = GetInput().GetValue("openid_md5_from");
		std::string strOpenidMd5To = GetInput().GetValue("openid_md5_to");
		uint64_t qwOpenidMd5From = strtoul(strOpenidMd5From.c_str(), NULL, 10);
		uint64_t qwOpenidMd5To = strtoul(strOpenidMd5To.c_str(), NULL, 10);
		uint32_t dwOP = (uint32_t)GetInput().GetValue("op");
		
		LOG4CPLUS_DEBUG(logger, "qwOpenidMd5From="<<qwOpenidMd5From
							<<", qwOpenidMd5To="<<qwOpenidMd5To
							<<", dwOP="<<dwOP);
							
		if(0 == qwOpenidMd5From || 0 == qwOpenidMd5To || (dwOP!=1 && dwOP!=2))
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "req param invalid, not found");
			return true;	
		}	

		if(qwOpenidMd5From == qwOpenidMd5To)
		{
			DoReply(CGI_RET_CODE_INVALID_OP);
			LOG4CPLUS_ERROR(logger, "qwOpenidMd5From == qwOpenidMd5To, invalid");
			return true;
		}

		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::USER_FOLLOW_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::UserFollowReq* pUserFollowReq = stRequest.mutable_user_follow_req();
		pUserFollowReq->set_openid_md5_from(qwOpenidMd5From);
		pUserFollowReq->set_openid_md5_to(qwOpenidMd5To);
		pUserFollowReq->set_relation(dwOP);

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
		if(stHead.cmd() != ::hoosho::msg::USER_FOLLOW_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;	 
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;	
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	string m_user_server_ip;
	int m_user_server_port;
	
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUserFollow, logger);

int main(int argc, char** argv)
{
	CgiUserFollow cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

