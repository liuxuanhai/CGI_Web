#include "cgi/cgi.h"
#include "../cgi_common_util.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>

class CgiPicDownload:public lce::cgi::Cgi 
{
public:
	CgiPicDownload():Cgi(0, "config.ini", "logger.properties")
	{

	}

    //image/png
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
		GetHeader().AddHeader("Cache-Control: max-age=0, must-revalidate");
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

	virtual bool DerivedInit()
	{
		GetConfig().GetValue("DB", "db_ip", m_db_ip, "");
		GetConfig().GetValue("DB", "db_user", m_db_user, "");
		GetConfig().GetValue("DB", "db_passwd", m_db_passwd, "");
		GetConfig().GetValue("DB", "db_name", m_db_name, "");
		GetConfig().GetValue("DB", "table_name_pic_info", m_table_name_pic_info, "");
		return true;
	}

	virtual bool Process()
	{
		lce::cgi::CMysql mysql;
		if(!mysql.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd))
		{
			LOG4CPLUS_ERROR(logger, "msyql init failed");
			Return404();
			return true;
		}

		uint64_t qwPicId = strtoul(((string)GetInput().GetValue("pic_id")).c_str(), NULL, 10);

		if(qwPicId == 0)
		{
			Return404();
			LOG4CPLUS_ERROR(logger, "zero pic_ic");
			return true;
		}

		std::ostringstream oss;
		std::string strPicData;
		uint32_t dwPicType = 0;
		
		oss.str("");
		oss << "SELECT content, type FROM " << m_table_name_pic_info
			<< " WHERE pic_id = " << qwPicId;
		SQL_QUERY_FAIL_REPLY(mysql, oss, logger);
		if(mysql.GetRowCount() && mysql.Next())
		{
			strPicData.assign(mysql.GetRow(0), mysql.GetRowLength(0));
			dwPicType = atoi(mysql.GetRow(1));
			if(dwPicType == hoosho::msg::s::PIC_TYPE_PRIVATE)
			{
				LOG4CPLUS_DEBUG(logger, "pic_id=" << qwPicId << " is private");
				Return403();
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "read pic.size="<<strPicData.size());
			Return200(strPicData);
			return true;
		}

		Return404();
		LOG4CPLUS_ERROR(logger, "pic_id=" << qwPicId << " not found");
		return true;
	}

protected:
	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_table_name_pic_info;

protected:
	DECL_LOGGER(logger);
};
IMPL_LOGGER(CgiPicDownload, logger);

int main()
{
	CgiPicDownload cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
