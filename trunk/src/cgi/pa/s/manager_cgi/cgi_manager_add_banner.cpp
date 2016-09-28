#include "cgi_manager_base.h"
#include "common_util.h"

class CgiManagerAddBanner:public CgiManagerBase
{
public:
	CgiManagerAddBanner():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		uint64_t qwPicid = strtoul(((string)GetInput().GetValue("pic_id")).c_str(), NULL, 10);
		uint32_t iContentType = (uint32_t)GetInput().GetValue("content_type");
		string strContent = (string)GetInput().GetValue("content_type");

		EMPTY_STR_RETURN(strContent);
		ZERO_INT_RETURN(iContentType);

		uint64_t qwBannerid = ::common::util::generate_unique_id();
		std::ostringstream oss;
		oss.str("");
		oss << "INSERT INTO t_banner_info "
			<< " SET banner_id=" << qwBannerid
			<< ", pic_id=" << qwPicid
			<< ", content_type=" << iContentType
			<< ", content='" << sql_escape(strContent) << "'"
			<< ", create_ts=unix_timestamp()";
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiManagerAddBanner cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
