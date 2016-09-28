#include "cgi_manager_base.h"
#include "common_util.h"

class CgiManagerDelBanner:public CgiManagerBase
{
public:
	CgiManagerDelBanner():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		uint64_t qwBannerid = strtoul(((string) GetInput().GetValue("banner_id")).c_str(), NULL, 10);

		ZERO_INT_RETURN(qwBannerid);

		std::ostringstream oss;
		oss.str("");
		oss<<"UPDATE t_banner_info SET del_status=1 WHERE banner_id="<<qwBannerid;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiManagerDelBanner cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
