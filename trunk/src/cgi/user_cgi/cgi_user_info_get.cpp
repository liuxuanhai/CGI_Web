#ifndef _HOOSHO_CGI_USER_LOGIN_H_
#define _HOOSHO_CGI_USER_LOGIN_H_

#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"
#include <sstream>
#include <stdio.h>

class CgiUserInfoGet: public lce::cgi::Cgi
{
public:
	 CgiUserInfoGet(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("user_server", "ip", m_user_server_ip, "");
		GetConfig().GetValue("user_server", "port", m_user_server_port, 0);
		GetConfig().GetValue("pa_server", "ip", m_pa_server_ip, "");
		GetConfig().GetValue("pa_server", "port", m_pa_server_port, 0);
		return true;
	}

	int UserPowerFentch(uint64_t qwOpenidMd5, uint64_t qwAppidMd5, std::vector<std::string>& vecPowerList)
	{
		vecPowerList.clear();
		
		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_POWER_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserPowerReq* pQueryUserPowerReq = stRequest.mutable_query_user_power_req();
		pQueryUserPowerReq->set_openid_md5(qwOpenidMd5);
		pQueryUserPowerReq->set_appid_md5(qwAppidMd5);

		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "UserPowerFentch failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return -1;
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_POWER_RES)
		{
			LOG4CPLUS_ERROR(logger, "UserPowerFentch failed, response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return -1;
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "UserPowerFentch failed, response.result="<<stHead.result());
			return -1;
		}

		for(int i=0; i<stResponse.query_user_power_res().power_list_size(); ++i)
		{
			vecPowerList.push_back(int_2_str(stResponse.query_user_power_res().power_list(i)));	
		}

		return 0;
	}
	
	virtual bool Process()
	{
		std::string strOpenidMd5List = (std::string)GetInput().GetValue("openid_md5_list");
		std::string strAppidMd5 = (std::string)GetInput().GetValue("appid_md5");
		if(strOpenidMd5List.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strOpenidMd5List is empty");
			return true;	
		}

		LOG4CPLUS_DEBUG(logger, "strOpenidMd5List="<<strOpenidMd5List<<", strAppidMd5="<<strAppidMd5);

		std::vector<std::string> vecOpenidMd5;
		lce::cgi::Split(strOpenidMd5List, "|", vecOpenidMd5);
		/*
		std::set<uint64_t> setOpenidMd5;
		for(size_t i=0; i<vecOpenidMd5.size(); ++i)
		{
			setOpenidMd5.insert(strtoul(vecOpenidMd5[i].c_str(), NULL, 10));
		}*/
		
		if(vecOpenidMd5.size() > 10)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strOpenidMd5List.size > 10, too much, refused");
			return true;
		}

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
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return true;	
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_DETAIL_INFO_RES)
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

		for(int i=0; i<stResponse.query_user_detail_info_res().user_detail_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
			LOG4CPLUS_DEBUG(logger, "one detailuserinfo="<<stUserDetailInfo.Utf8DebugString());
			UserInfoPB2Any(stUserDetailInfo, stAnyValue);	
			GetAnyValue()["user_info_list"].push_back(stAnyValue);	
		}

		//fentch power, first person only
		if(!strAppidMd5.empty() && !vecOpenidMd5.empty())
		{
			//power
			GetAnyValue()["user_power_list"].clear();	
			std::vector<std::string> vecPowerList;
			if(0 == UserPowerFentch(strtoul(vecOpenidMd5[0].c_str(), NULL, 10)
						, strtoul(strAppidMd5.c_str(), NULL, 10)
						, vecPowerList))
			{
				for(size_t i=0; i!=vecPowerList.size(); ++i)
				{
					GetAnyValue()["user_power_list"].push_back(vecPowerList[i]);	
				}
			}
			
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	string m_user_server_ip;
	int m_user_server_port;
	string m_pa_server_ip;
	int m_pa_server_port;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUserInfoGet, logger);

int main(int argc, char** argv)
{
	CgiUserInfoGet cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

