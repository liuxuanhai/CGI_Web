#ifndef _HOOSHO_CGI_UE_CONTENT_UPLOAD_H_
#define _HOOSHO_CGI_UE_CONTENT_UPLOAD_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "common_util.h"
#include "proto_io_tcp_client.h"
#include "util/lce_util.h"
#include "cgi_ue_common.h"
#include <sstream>
#include <fstream>
#include <stdio.h>


#define UE_MAX_CONTENT_LEN (16 * 1024 * 1024)

class CgiUEContentUpload: public lce::cgi::Cgi
{
public:
	 CgiUEContentUpload(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
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
		
		GetConfig().GetValue("ue", "config_json", m_ue_config_json, "");

		m_mysql.Init(m_db_ip, m_db_name_ue, m_db_user, m_db_pass);
		return true;
	}

	uint64_t generate_unique_id()
	{
		timeval tv;
		gettimeofday(&tv, NULL);
		uint64_t qwMS = (uint64_t)tv.tv_sec * 1000;
		qwMS += ((uint64_t)tv.tv_usec / 1000);
		srand(tv.tv_usec ^ 77);
		uint16_t randNum = rand() % 65536;

		//高6字节为毫秒， 低2字节为随机数
		uint64_t qwRetId = (qwMS << 16) | randNum;
		return qwRetId;	
	}

	virtual bool Process()
	{
		std::string strContentData = (std::string)GetInput().GetValue("content");
		if(strContentData.empty())
		{
			LOG4CPLUS_ERROR(logger, "empty content!!");
			DoReply(CGI_RET_CODE_CONTENT_EMPTY);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "content.len = "<<strContentData.size());
		
		if(strContentData.size() >= UE_MAX_CONTENT_LEN)
		{
			LOG4CPLUS_ERROR(logger, "too large content for max_ue_content_len="<<UE_MAX_CONTENT_LEN);
			DoReply(CGI_RET_CODE_CONTENT_OVERFLOW);
			return true; 
		}

		/*
		Field   Type    Null    Key     Default Extra
		content_id      bigint(20) unsigned     NO      PRI     0
		data    mediumblob      NO              NULL
		create_ts       bigint(20) unsigned     NO              0 
		*/ 
		uint64_t qwContentId = generate_unique_id();
		char aczTableName[32] = {0};
		::snprintf(aczTableName, 32, "%s%02x", m_table_content_name_prefix.c_str(), (uint8_t)(qwContentId % 256));		
		std::ostringstream ossSql;
		ossSql.str("");
		ossSql<<"insert into "<<aczTableName
					<<" set content_id="<<qwContentId
					<<", data='"<<lce::cgi::CMysql::MysqlEscape(strContentData)<<"'"
					<<", create_ts=unix_timestamp()";
		if(!m_mysql.Query(ossSql.str()))
		{
			LOG4CPLUS_ERROR(logger, "save content failed, mysql query error, sql="<<ossSql<<", msg="<<m_mysql.GetErrMsg());
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "save content succ, content_id="<<qwContentId);
		
		GetAnyValue()["content_id"] = qwContentId;
		DoReply(CGI_RET_CODE_SUCC);
		return true;
	}


protected:
	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;	
	string m_db_name_ue;
	string m_table_content_name_prefix;

	//ue config
	string m_ue_config_json;

	lce::cgi::CMysql m_mysql;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUEContentUpload, logger);

int main(int argc, char** argv)
{
	CgiUEContentUpload cgi(0, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

