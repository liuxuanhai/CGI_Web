#ifndef _HOOSHO_CGI_USER_LOGIN_H_
#define _HOOSHO_CGI_USER_LOGIN_H_

#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"
#include "util/lce_util.h"
#include <sstream>
#include <stdio.h>

class CgiUserFansNumGet: public lce::cgi::Cgi
{
public:
	 CgiUserFansNumGet(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
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
		std::string strOpenidMd5List = (std::string)GetInput().GetValue("openid_md5_list");
		if(strOpenidMd5List.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strOpenidMd5List is empty");
			return true;	
		}

		LOG4CPLUS_DEBUG(logger, "strOpenidMd5List="<<strOpenidMd5List);

		std::vector<std::string> vecOpenidMd5;
		lce::cgi::Split(strOpenidMd5List, "|", vecOpenidMd5);

		std::set<uint64_t> setOpenidMd5;
		for(size_t i=0; i<vecOpenidMd5.size(); ++i)
		{
			setOpenidMd5.insert(strtoul(vecOpenidMd5[i].c_str(), NULL, 10));
		}
		
		if(setOpenidMd5.size() > 10)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strOpenidMd5List.size > 10, too much, refused");
			return true;
		}
		
		
		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_FANS_NUM_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserFansNumReq* pQueryUserFansNumReq = stRequest.mutable_query_user_fans_num_req();
		for(std::set<uint64_t>::iterator iter=setOpenidMd5.begin(); iter!=setOpenidMd5.end(); ++iter)
		{
			pQueryUserFansNumReq->add_openid_md5_list(*iter);
		}

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
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_FANS_NUM_RES)
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

		for(int i=0; i<stResponse.query_user_fans_num_res().fans_num_list_size(); ++i)
		{
			const ::hoosho::commstruct::KeyValueIntInt& stPair = stResponse.query_user_fans_num_res().fans_num_list(i);
			lce::cgi::CAnyValue stAnyValue;
			stAnyValue["openid_md5"] = int_2_str(stPair.key());
			stAnyValue["fans_num"] = stPair.value();
			GetAnyValue()["fans_num_list"].push_back(stAnyValue);
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

IMPL_LOGGER(CgiUserFansNumGet, logger);

int main(int argc, char** argv)
{
	CgiUserFansNumGet cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

