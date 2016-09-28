#include "cgi_manager_base.h"
#include "common_util.h"

class CgiManagerGetBannerList:public CgiManagerBase
{
public:
	CgiManagerGetBannerList():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		uint64_t qwBeginBannerid = strtoul(((string)GetInput().GetValue("begin_banner_id")).c_str(), NULL, 10); 
		uint32_t dwLimit = (uint32_t)GetInput().GetValue("limit");

		ZERO_INT_RETURN(dwLimit);

		if(0 == qwBeginBannerid)
			qwBeginBannerid = 0xFFFFFFFFFFFFFFFF;
		
		std::ostringstream oss;
		oss.str("");
		oss << "SELECT * FROM t_banner_info WHERE banner_id<" << qwBeginBannerid
			<< " AND del_status=0"
       		<< " ORDER BY banner_id DESC LIMIT " << dwLimit;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		if(m_mysql_helper.GetRowCount())
		{
			while(m_mysql_helper.Next())
			{
				lce::cgi::CAnyValue item;
				BannerInfoDB2Any(m_mysql_helper, item);
				GetAnyValue()["banner_info_list"].push_back(item);
			}
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiManagerGetBannerList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
