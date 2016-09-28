#ifndef _HOOSHO_CGI_USER_LOGIN_H_
#define _HOOSHO_CGI_USER_LOGIN_H_

#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"
#include <sstream>
#include <stdio.h>

class CgiUserFollowsGet: public lce::cgi::Cgi
{
public:
	 CgiUserFollowsGet(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
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

	void FentchUserInfo(const std::vector<std::string>& vecOpenidMd5, std::map<std::string, lce::cgi::CAnyValue>& mapResult)
	{
		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stRequest.mutable_query_user_detail_info_req();
		//for(std::set<uint64_t>::iterator iter=setOpenidMd5.begin(); iter!=setOpenidMd5.end(); ++iter)
		for(size_t i=0; i!=vecOpenidMd5.size(); ++i)
		{
			pQueryUserDetailInfoReq->add_openid_md5_list(strtoul(vecOpenidMd5[i].c_str(), NULL, 10));
		}
		
		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "FentchUserInfo failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return;
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_DETAIL_INFO_RES)
		{
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return;	
		}

		mapResult.clear();
		for(int i=0; i<stResponse.query_user_detail_info_res().user_detail_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
			UserInfoPB2Any(stUserDetailInfo, stAnyValue);	
			mapResult[int_2_str(stUserDetailInfo.openid_md5())] = stAnyValue;	
		}
	}
	
	virtual bool Process()
	{
		std::string strOpenidMd5 = GetInput().GetValue("openid_md5");
		uint64_t qwOpenidMd5 = strtoul(strOpenidMd5.c_str(), NULL, 10);
		if(0 == qwOpenidMd5)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "qwOpenidMd5 is 0");
			return true;	
		}

		LOG4CPLUS_DEBUG(logger, "qwOpenidMd5="<<qwOpenidMd5);

		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_FOLLOWS_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserFollowsReq* pQueryUserFollowsReq = stRequest.mutable_query_user_follows_req();
		pQueryUserFollowsReq->set_openid_md5(qwOpenidMd5);

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
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_FOLLOWS_RES)
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

		std::vector<std::string> vecOpenidMd5;
		for(int i=0; i<stResponse.query_user_follows_res().openid_md5_list_size(); ++i)
		{
			std::string strOpenidMd5 = int_2_str(stResponse.query_user_follows_res().openid_md5_list(i));
			GetAnyValue()["fans"].push_back(strOpenidMd5);	
			vecOpenidMd5.push_back(strOpenidMd5);
		}

		std::map<std::string, lce::cgi::CAnyValue> mapResult;
		FentchUserInfo(vecOpenidMd5, mapResult);
		for(std::map<std::string, lce::cgi::CAnyValue>::iterator iter=mapResult.begin(); iter!=mapResult.end(); ++iter)
		{
			GetAnyValue()["user_info"][iter->first] = iter->second;
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

IMPL_LOGGER(CgiUserFollowsGet, logger);

int main(int argc, char** argv)
{
	CgiUserFollowsGet cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

