#ifndef _HOOSHO_CGI_UE_RESOURCE_DOWNLOAD_H_
#define _HOOSHO_CGI_UE_RESOURCE_DOWNLOAD_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "common_util.h"
#include <sstream>
#include <stdio.h>
#include "cgi_ue_common.h"

class CgiUEContentDownload: public lce::cgi::Cgi
{
public:
	 CgiUEContentDownload(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("db", "db_ip", m_db_ip, "");
		GetConfig().GetValue("db", "db_user", m_db_user, "");
		GetConfig().GetValue("db", "db_passwd", m_db_pass, "");		
		GetConfig().GetValue("db", "db_name_ue", m_db_name_ue, "");
		GetConfig().GetValue("db", "table_name_content_prefix", m_table_content_name_prefix, "");

		m_mysql.Init(m_db_ip, m_db_name_ue, m_db_user, m_db_pass);
		return true;
	}
	
	virtual bool Process()
	{
		std::string strContentId = GetInput().GetValue("content_id");
		uint64_t qwContentId = strtoul(strContentId.c_str(),  NULL, 10);
	
		//need check openid_md5 first
		//ignore first, for add more platform data
		char aczTableName[32] = {0};
		::snprintf(aczTableName, 32, "%s%02x", m_table_content_name_prefix.c_str(), (uint8_t)(qwContentId % 256));		
		std::ostringstream ossSql;
		ossSql.str("");
		ossSql<<"select * from "<<aczTableName
					<<" where content_id="<<qwContentId;
		if(!m_mysql.Query(ossSql.str()))
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "mysql query error, sql="<<ossSql<<", msg="<<m_mysql.GetErrMsg());
			return true;
		}

		if(m_mysql.GetRowCount() && m_mysql.Next())
		{
			/*
			Field   Type    Null    Key     Default Extra
			content_id      bigint(20) unsigned     NO      PRI     0
			data    mediumblob      NO              NULL
			create_ts       bigint(20) unsigned     NO              0 
			*/
			std::string strContentData = "";
			strContentData.assign(m_mysql.GetRow(1), m_mysql.GetRowLength(1));
			LOG4CPLUS_DEBUG(logger, "contnet_size="<<strContentData.size()<<", content_id="<<qwContentId);
			GetAnyValue()["content"] = strContentData;
			DoReply(CGI_RET_CODE_SUCC);
			return true;
		}

		LOG4CPLUS_ERROR(logger, "content_id="<<qwContentId<<", not found in db");
		DoReply(CGI_RET_CODE_CONTENT_NOTFOUND);
		return true;
	}

protected:
	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;	
	string m_db_name_ue;
	string m_table_content_name_prefix;

	lce::cgi::CMysql m_mysql;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUEContentDownload, logger);

int main(int argc, char** argv)
{
	CgiUEContentDownload cgi(0, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

