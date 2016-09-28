#ifndef _HOOSHO_CGI_USER_LOGIN_H_
#define _HOOSHO_CGI_USER_LOGIN_H_

#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"
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
		GetConfig().GetValue("DB", "ip", m_db_ip, "");
		GetConfig().GetValue("DB", "db_name", m_db_name, "");
		GetConfig().GetValue("DB", "user", m_db_user, "");
		GetConfig().GetValue("DB", "passwd", m_db_passwd, "");		
		return true;
	}

	int UserPowerFentch(uint64_t qwOpenidMd5, uint64_t qwAppidMd5, std::vector<std::string>& vecPowerList)
	{
			vecPowerList.clear();

			::hoosho::msg::Msg stRequest;
			::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
			pHead->set_cmd(::hoosho::msg::QUERY_USER_POWER_REQ);
			pHead->set_seq(time(0));
			::hoosho::user::QueryUserPowerReq* pQueryUserPowerReq = stRequest.mutable_query_user_power_req();
			pQueryUserPowerReq->set_openid_md5(qwOpenidMd5);
			pQueryUserPowerReq->set_appid_md5(qwAppidMd5);

			std::string strErrMsg = "";
			::hoosho::msg::Msg stResponse;
			::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
			int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
			if(iRet < 0)
			{
					LOG4CPLUS_ERROR(logger, "UserPowerFentch failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
					return -1;
			}

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

	void PaUVStatistics(uint64_t paAppid_md5, uint64_t Openid_md5)
	{
		lce::cgi::CMysql m_mysql_helper;
		if(!m_mysql_helper.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd))
		{
			LOG4CPLUS_ERROR(logger, "PaUVStatistics CMysql init failed!");
			return;
		}

		time_t m_time;
		struct tm *p;
		time(&m_time);
		time_t m_bj_time = m_time + 8*60*60; 	//转化成北京时间
		p=gmtime(&m_bj_time);
		int year = 1900+p->tm_year;
		int month = 1+p->tm_mon;		

		char mon_str[16];
		sprintf(mon_str, "%d%02d", year, month);		

		std::ostringstream ossSql;
		ossSql.str("");

		ossSql<<"INSERT INTO t_pa_uv_"<<mon_str
			<<" SET pa_appid_md5="<<paAppid_md5
			<<", openid_md5="<<Openid_md5
			<<", create_ts="<<m_time
			<<" ON DUPLICATE KEY UPDATE"
			<<" create_ts="<<m_time;

		
		LOG4CPLUS_DEBUG(logger, ossSql.str());

		if(!m_mysql_helper.Query(ossSql.str()))
		{
			LOG4CPLUS_ERROR(logger, "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg());
		}


		return;		
		
	}

	virtual bool Process()
	{
		std::string strPreAuthCode = (std::string)GetInput().GetValue("pre_auth_code");
		std::string strPAAppIdMd5 = (std::string)GetInput().GetValue("appid_md5");
		if(strPreAuthCode.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strPreAuthCode is empty");
			return true;	
		}

		LOG4CPLUS_DEBUG(logger, "strPreAuthCode="<<strPreAuthCode<<", strPAAppIdMd5="<<strPAAppIdMd5);

		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::USER_PARSE_PRE_AUTH_CODE_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::UserParsePreAuthCodeReq* pUserParsePreAuthCodeReq = stRequest.mutable_user_parse_pre_auth_code_req();
		pUserParsePreAuthCodeReq->set_pre_auth_code(strPreAuthCode);

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
		if(stHead.cmd() != ::hoosho::msg::USER_PARSE_PRE_AUTH_CODE_RES)
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
		
		const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.user_parse_pre_auth_code_res().user_detail_info();
		lce::cgi::CAnyValue stAnyValue;
		UserInfoPB2Any(stUserDetailInfo, stAnyValue);
		GetAnyValue()["user_info"] = stAnyValue;
		std::vector<std::string> vecPowerList;
		if(0 == UserPowerFentch(stUserDetailInfo.openid_md5()
								, strtoul(strPAAppIdMd5.c_str(), NULL, 10)
								, vecPowerList))
		{
				for(size_t i=0; i!=vecPowerList.size(); ++i)
				{
						GetAnyValue()["user_power_list"].push_back(vecPowerList[i]);
				}
		}

		DoReply(CGI_RET_CODE_OK);

		//pa UV statistics
		PaUVStatistics(strtoul(strPAAppIdMd5.c_str(), NULL, 10), stUserDetailInfo.openid_md5());
		return true;
	}

protected:
	string m_user_server_ip;
	int m_user_server_port;
	string m_db_ip;
	string m_db_name;
	string m_db_user;
	string m_db_passwd;
	
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

