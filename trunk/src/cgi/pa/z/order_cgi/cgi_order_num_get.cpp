#include "cgi/cgi.h"
#include "util/lce_util.h"
#include "../cgi_common_util.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>


class CgiOrderNumGet: public lce::cgi::Cgi
{
public:
	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_table_order_info_name;

public:
	CgiOrderNumGet(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
		:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{		
	}

	virtual bool DerivedInit()
	{
		GetConfig().GetValue("DB", "db_ip", m_db_ip, "");
		GetConfig().GetValue("DB", "db_user", m_db_user, "");
		GetConfig().GetValue("DB", "db_passwd", m_db_passwd, "");
		GetConfig().GetValue("DB", "db_name", m_db_name, "");
		GetConfig().GetValue("DB", "table_order_info_name", m_table_order_info_name, "");

		LOG4CPLUS_INFO(logger, "db_ip: " << m_db_ip
							 << ", db_user: " << m_db_user
							 << ", db_passwd: " << m_db_passwd
							 << ", db_name: " << m_db_name
							 << ", table_order_info_name: " << m_table_order_info_name);
		return true;
	}
	
	virtual bool Process()
	{
		LOG4CPLUS_INFO(logger, "BEGIN CGI ----------------- "<<GetCgiName()<<"----------------- ");
		if(strcmp(GetInput().GetQueryString(), ""))
		{
			LOG4CPLUS_INFO(logger, "query_string: " << GetInput().GetQueryString());
		}
		if(strcmp(GetInput().GetPostData(), ""))
		{
			LOG4CPLUS_INFO(logger, "post_data: " << GetInput().GetPostData());
		}

		// smaple authentication
		string strCode = GetInput().GetValue("code");
		EMPTY_STR_RETURN(strCode);
		
		// count
		lce::cgi::CMysql mysql;
		if(!mysql.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd))
		{
			LOG4CPLUS_ERROR(logger, "msyql init failed");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		std::ostringstream oss;
		uint32_t dwWaitingNum;
		uint32_t dwFetchedNum;

		oss.str("");
		oss << "SELECT COUNT(*) FROM " << m_table_order_info_name << " WHERE order_status = 1";
		if(!mysql.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "sql query failed, msg = " << mysql.GetErrMsg());
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		if(mysql.GetRowCount() && mysql.Next())
		{
			dwWaitingNum = atoi(mysql.GetRow(0));
		}
		else 
		{
			LOG4CPLUS_ERROR(logger, "sql query failed, msg = " << mysql.GetErrMsg());
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		oss.str("");
		oss << "SELECT COUNT(*) FROM " << m_table_order_info_name << " WHERE order_status = 2 OR order_status = 3";
		if(!mysql.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "sql query failed, msg = " << mysql.GetErrMsg());
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		if(mysql.GetRowCount() && mysql.Next())
		{
			dwFetchedNum = atoi(mysql.GetRow(0));
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "sql query failed, msg = " << mysql.GetErrMsg());
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
			


		// return
		GetAnyValue()["waiting"] = dwWaitingNum + 3124;
		GetAnyValue()["fetched"] = dwFetchedNum + 3412;


		DoReply(CGI_RET_CODE_OK);

		LOG4CPLUS_INFO(logger, "END CGI \n");
		return true;
	}

protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiOrderNumGet, logger);

int main(int argc, char** argv)
{
	CgiOrderNumGet cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

