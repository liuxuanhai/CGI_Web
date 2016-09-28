#include "cgi_manager_base.h"

class CgiManagerPicDownload:public CgiManagerBase
{
public:
	CgiManagerPicDownload():CgiManagerBase(0, "config.ini", "logger.properties", true)
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

	bool InnerProcess()
	{
		uint64_t qwPicId = strtoul(((string)GetInput().GetValue("pic_id")).c_str(), NULL, 10);

		if(qwPicId == 0)
		{
			Return404();
			LOG4CPLUS_ERROR(logger, "zero pic_ic");
			return true;
		}

		std::ostringstream oss;
		std::string strPicData;
		
		oss.str("");
		oss << "SELECT content FROM " << m_table_name_pic_info
			<< " WHERE pic_id = " << qwPicId;
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			strPicData.assign(m_mysql_helper.GetRow(0), m_mysql_helper.GetRowLength(0));
			LOG4CPLUS_DEBUG(logger, "read pic.size="<<strPicData.size());
			Return200(strPicData);
			return true;
		}

		Return404();
		LOG4CPLUS_ERROR(logger, "pic_id=" << qwPicId << " not found");
		return true;
	}
};

int main()
{
	CgiManagerPicDownload cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
