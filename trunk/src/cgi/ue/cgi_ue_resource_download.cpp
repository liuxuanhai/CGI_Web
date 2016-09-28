#ifndef _HOOSHO_CGI_UE_RESOURCE_DOWNLOAD_H_
#define _HOOSHO_CGI_UE_RESOURCE_DOWNLOAD_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "common_util.h"
#include <sstream>
#include <stdio.h>
#include "cgi_ue_common.h"

class CgiUEResourceDownload: public lce::cgi::Cgi
{
public:
	 CgiUEResourceDownload(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
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
		GetConfig().GetValue("db", "table_name_resource_prefix", m_table_resource_name_prefix, "");

		m_mysql.Init(m_db_ip, m_db_name_ue, m_db_user, m_db_pass);
		return true;
	}

	void Return404()
    { 
    	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
    	stHttpRspHeader.AddHeader("Status: 404");
    	stHttpRspHeader.Output();
    }

    void Return403()
    { 
    	//no previledges
    	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
    	stHttpRspHeader.AddHeader("Status: 403");
    	stHttpRspHeader.Output();
    }

	void Return200(const string& strResourceData, uint32_t dwResourceType)
	{
		GetHeader().AddHeader("Cache-Control: max-age=864000");
		if(UE_RESOURCE_TYPE_IMAGE == dwResourceType)
		{
			GetHeader().SetContentType("image/png");
			
		}
		else if(UE_RESOURCE_TYPE_VIDEO == dwResourceType)
		{
			GetHeader().SetContentType("video/mpg");
		}

		
		//GetHeader().SetContentType("image/png");

		GetHeader().Output();
		if(!std::cout.good())
		{
			std::cout.clear();
			std::cout.flush();
		}

		std::cout<<strResourceData;

		return;
	}
	
	virtual bool Process()
	{
		std::string strResourceId = GetInput().GetValue("resource_id");
		uint64_t qwResourceId = strtoul(strResourceId.c_str(),  NULL, 10);
	
		//need check openid_md5 first
		//ignore first, for add more platform data
		char aczTableName[32] = {0};
		::snprintf(aczTableName, 32, "%s%02x", m_table_resource_name_prefix.c_str(), (uint8_t)(qwResourceId % 256));		
		std::ostringstream ossSql;
		ossSql.str("");
		ossSql<<"select * from "<<aczTableName
					<<" where resource_id="<<qwResourceId;
		if(!m_mysql.Query(ossSql.str()))
		{
			Return404();
			LOG4CPLUS_ERROR(logger, "mysql query error, sql="<<ossSql<<", msg="<<m_mysql.GetErrMsg());
			return true;
		}

		if(m_mysql.GetRowCount() && m_mysql.Next())
		{
			/*
			+---------------+---------------------+------+-----+---------+-------+
			| Field         | Type                | Null | Key | Default | Extra |
			+---------------+---------------------+------+-----+---------+-------+
			| resource_id   | bigint(20) unsigned | NO   | PRI | 0       |       |
			| resource_type | int(10) unsigned    | NO   |     | 0       |       |
			| data          | mediumblob          | NO   |     | NULL    |       |
			| name          | blob                | NO   |     | NULL    |       |
			| create_ts     | bigint(20) unsigned | NO   |     | 0       |       |
			+---------------+---------------------+------+-----+---------+-------+	
			*/
			uint32_t dwResourceType = atol(m_mysql.GetRow(1));
			std::string strResourceName = m_mysql.GetRow(3);
			std::string strResourceData = "";
			strResourceData.assign(m_mysql.GetRow(2), m_mysql.GetRowLength(2));
			Return200(strResourceData, dwResourceType);
			LOG4CPLUS_DEBUG(logger, "resource_size="<<strResourceData.size()
						<<", resource_id="<<qwResourceId
						<<", resource_type="<<dwResourceType
						<<", resource_name="<<strResourceName);
			return true;
		}

		Return404();
		LOG4CPLUS_ERROR(logger, "resource_id="<<qwResourceId <<", not found in db, return http 404");
		
		return true;
	}

protected:
	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;	
	string m_db_name_ue;
	string m_table_resource_name_prefix;

	lce::cgi::CMysql m_mysql;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUEResourceDownload, logger);

int main(int argc, char** argv)
{
	CgiUEResourceDownload cgi(0, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

