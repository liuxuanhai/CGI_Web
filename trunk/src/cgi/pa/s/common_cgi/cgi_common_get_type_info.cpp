#ifndef _HOOSHO_CGI_COMMON_JSAPI_SIGNATURE_H_
#define _HOOSHO_CGI_COMMON_JSAPI_SIGNATURE_H_

#include "cgi/cgi.h"
#include "../cgi_common_util.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>

class CgiCommonGetTypeInfo: public lce::cgi::Cgi
{
public:
	 CgiCommonGetTypeInfo(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("DB", "db_ip", m_db_ip, "");
		GetConfig().GetValue("DB", "db_user", m_db_user, "");
		GetConfig().GetValue("DB", "db_passwd", m_db_passwd, "");
		GetConfig().GetValue("DB", "db_name", m_db_name, "");
		GetConfig().GetValue("DB", "table_name_good_type", m_table_name_good_type, "");
		GetConfig().GetValue("DB", "table_name_consume_type", m_table_name_consume_type, "");
		return true;
	}
	
	virtual bool Process()
	{
		lce::cgi::CMysql mysql;
		if(!mysql.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd))
		{
			LOG4CPLUS_ERROR(logger, "msyql init failed");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		ostringstream oss;

		oss.str("");
		oss << "SELECT * FROM " << m_table_name_consume_type;
		SQL_QUERY_FAIL_REPLY(mysql, oss, logger);

		hoosho::msg::s::ConsumeType stConsumeType;
		while(mysql.GetRowCount() && mysql.Next())
		{
			ConsumeTypeDB2PB(mysql, stConsumeType);
			GetAnyValue()["consume_type_list"].push_back(ConsumeTypePB2Any(stConsumeType));
		}

		
		oss.str("");
		oss << "SELECT * FROM " << m_table_name_good_type;
		SQL_QUERY_FAIL_REPLY(mysql, oss, logger);

		hoosho::msg::s::GoodType stGoodType;
		while(mysql.GetRowCount() && mysql.Next())
		{
			GoodTypeDB2PB(mysql, stGoodType);
			GetAnyValue()["good_type_list"].push_back(GoodTypePB2Any(stGoodType));
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_table_name_good_type;
	std::string m_table_name_consume_type;

protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiCommonGetTypeInfo, logger);

int main(int argc, char** argv)
{
	CgiCommonGetTypeInfo cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

