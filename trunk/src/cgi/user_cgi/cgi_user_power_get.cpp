#ifndef _HOOSHO_CGI_USER_POWER_GET_H_
#define _HOOSHO_CGI_USER_POWER_GET_H_

#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"
#include <sstream>
#include <stdio.h>

class CgiUserPowerGet: public lce::cgi::Cgi
{
public:
	 CgiUserPowerGet(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
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
		std::string strOpenidMd5 = (std::string)GetInput().GetValue("openid_md5");
		std::string strPAAppidMd5 = (std::string)GetInput().GetValue("pa_appid_md5");
		LOG4CPLUS_DEBUG(logger, "strOpenidMd5="<<strOpenidMd5<<", strPAAppidMd5="<<strPAAppidMd5);
		
		if(strOpenidMd5.empty() || strPAAppidMd5.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strOpenidMd5List or strPAAppidMd5 is empty");
			return true;	
		}

		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_POWER_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserPowerReq* pQueryUserPowerReq = stRequest.mutable_query_user_power_req();
		pQueryUserPowerReq->set_openid_md5(strtoul(strOpenidMd5.c_str(), NULL, 10));
		pQueryUserPowerReq->set_appid_md5(strtoul(strPAAppidMd5.c_str(), NULL, 10));

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
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_POWER_RES)
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

		for(int i=0; i<stResponse.query_user_power_res().power_list_size(); ++i)
		{
			GetAnyValue()["power_list"].push_back(int_2_str(stResponse.query_user_power_res().power_list(i)));	
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

IMPL_LOGGER(CgiUserPowerGet, logger);

int main(int argc, char** argv)
{
	CgiUserPowerGet cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

