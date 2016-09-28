#ifndef _HOOSHO_CGI_COMMON_PIC_DOWNLOAD_H_
#define _HOOSHO_CGI_COMMON_PIC_DOWNLOAD_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "common_util.h"
#include <sstream>
#include <stdio.h>

class CgiCommonPicDownload: public lce::cgi::Cgi
{
public:
	 CgiCommonPicDownload(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("db", "db_ip", m_db_ip, "");
		GetConfig().GetValue("db", "db_user", m_db_user, "");
		GetConfig().GetValue("db", "db_passwd", m_db_pass, "");		
		GetConfig().GetValue("db", "db_name_pic", m_db_name_pic, "");
		GetConfig().GetValue("db", "table_name_pic_prefix", m_table_pic_name_prefix, "");

		m_mysql.Init(m_db_ip, m_db_name_pic, m_db_user, m_db_pass);
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

	void Return200(const string& strPicData)
	{
		//GetHeader().AddHeader("Cache-Control: max-age=0, must-revalidate");
		GetHeader().AddHeader("Cache-Control: max-age=864000");
		GetHeader().SetContentType("image/png");

		GetHeader().Output();
		if(!std::cout.good())
		{
			std::cout.clear();
			std::cout.flush();
		}

		std::cout<<strPicData;

		return;
	}
	
	virtual bool Process()
	{
		std::string strPicId = GetInput().GetValue("pic_id");
		uint64_t qwPicId = strtoul(strPicId.c_str(),  NULL, 10);
	
		//need check openid_md5 first
		//ignore first, for add more platform data
		char aczTableName[32] = {0};
		::snprintf(aczTableName, 32, "%s%02x", m_table_pic_name_prefix.c_str(), (uint8_t)(qwPicId % 256));		
		std::ostringstream ossSql;
		ossSql.str("");
		ossSql<<"select * from "<<aczTableName
					<<" where pic_id="<<qwPicId;
		if(!m_mysql.Query(ossSql.str()))
		{
			Return404();
			LOG4CPLUS_ERROR(logger, "mysql query error, sql="<<ossSql<<", msg="<<m_mysql.GetErrMsg());
			return true;
		}

		if(m_mysql.GetRowCount() && m_mysql.Next())
		{
			std::string strPicData = "";
			strPicData.assign(m_mysql.GetRow(1), m_mysql.GetRowLength(1));
			Return200(strPicData);
			LOG4CPLUS_DEBUG(logger, "pic.size="<<strPicData.size()<<", picid="<<qwPicId);
			return true;
		}

		Return404();
		LOG4CPLUS_ERROR(logger, "picid="<<qwPicId <<", not found in db, return http 404");
		
		return true;
	}

protected:
	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;	
	string m_db_name_pic;
	string m_table_pic_name_prefix;

	lce::cgi::CMysql m_mysql;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiCommonPicDownload, logger);

int main(int argc, char** argv)
{
	CgiCommonPicDownload cgi(0, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

