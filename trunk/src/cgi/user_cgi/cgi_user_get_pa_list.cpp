#ifndef _HOOSHO_CGI_USER_GET_PA_LIST_H_
#define _HOOSHO_CGI_USER_GET_PA_LIST_H_

#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"
#include <sstream>
#include <stdio.h>

class CgiUserGetPaList: public lce::cgi::Cgi
{
public:
	 CgiUserGetPaList(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("pa_server", "ip", m_pa_server_ip, "");
		GetConfig().GetValue("pa_server", "port", m_pa_server_port, 0);
		return true;
	}
	
	virtual bool Process()
	{
		/*
		std::string strUin = (std::string)GetInput().GetValue("uin");
		if(strUin.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strUin is empty");
			return true;	
		}

		LOG4CPLUS_DEBUG(logger, "strUin="<<strUin);
		*/

		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_PA_REQ);
		pHead->set_seq(time(0));
		//::hoosho::user::QueryUserPaReq* pQueryUserPaReq = stRequest.mutable_query_user_pa_req();
		//pQueryUserPaReq->set_uin(strUin);

		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_pa_server_ip, m_pa_server_port);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return true;	
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_PA_RES)
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

		lce::cgi::CAnyValue stAnyValue;
		for(int i=0; i<stResponse.query_user_pa_res().appid_md5_list_size(); i++)
		{
			uint64_t appid_md5 =  stResponse.query_user_pa_res().appid_md5_list(i);
			stAnyValue.push_back(int_2_str(appid_md5));
		}

		GetAnyValue()["pa_appid_md5_list"] = stAnyValue;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	string m_pa_server_ip;
	int m_pa_server_port;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUserGetPaList, logger);

int main(int argc, char** argv)
{
	CgiUserGetPaList cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

