#include "cgi/cgi.h"
#include "util/lce_util.h"
#include "../cgi_common_util.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>


class CgiQixiSharePageStatistics: public lce::cgi::Cgi
{
public:
	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_table_qixi_share_page_statistics_name;

public:
	CgiQixiSharePageStatistics(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
		:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{		
	}

	virtual bool DerivedInit()
	{
		GetConfig().GetValue("DB", "db_ip", m_db_ip, "");
		GetConfig().GetValue("DB", "db_user", m_db_user, "");
		GetConfig().GetValue("DB", "db_passwd", m_db_passwd, "");
		GetConfig().GetValue("DB", "db_name", m_db_name, "");
		GetConfig().GetValue("DB", "table_qixi_share_page_statistics_name", m_table_qixi_share_page_statistics_name, "");

		LOG4CPLUS_INFO(logger, "db_ip: " << m_db_ip
							 << ", db_user: " << m_db_user
							 << ", db_passwd: " << m_db_passwd
							 << ", db_name: " << m_db_name
							 << ", table_order_info_name: " << m_table_qixi_share_page_statistics_name);
		return true;
	}
	
	virtual bool Process()
	{
		LOG4CPLUS_INFO(logger, "BEGIN CGI ----------------- "<<GetCgiName()<<"----------------- ");
		
		// count
		lce::cgi::CMysql mysql;
		if(!mysql.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd))
		{
			LOG4CPLUS_ERROR(logger, "msyql init failed");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		
		uint64_t curTime = time(NULL);
		
		std::ostringstream oss;

		oss.str("");
		oss << "INSERT INTO " << m_table_qixi_share_page_statistics_name 
		    << " SET create_ts = " << ((curTime + 8 * 60 * 60) / (24 * 60 * 60))
		    << " , share_time = 1"
		    << " ON DUPLICATE KEY UPDATE "
      		    << " share_time = share_time + 1";

		LOG4CPLUS_DEBUG(logger, oss.str());
		if(!mysql.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "sql query failed, msg = " << mysql.GetErrMsg());
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		DoReply(CGI_RET_CODE_OK);

		LOG4CPLUS_INFO(logger, "END CGI \n");
		return true;
	}

protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiQixiSharePageStatistics, logger);

int main(int argc, char** argv)
{
	CgiQixiSharePageStatistics cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

