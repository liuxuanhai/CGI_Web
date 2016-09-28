#include "cgi_feeds_base.h"

IMPL_LOGGER(CgiFeedsBase, logger);

bool CgiFeedsBase::DerivedInit()
{
	GetConfig().GetValue("z_server", "ip", m_z_server_ip, "");
	GetConfig().GetValue("z_server", "port", m_z_server_port, 0);

	return true;
}

bool CgiFeedsBase::Process()
{
	LOG4CPLUS_INFO(logger, "BEGIN CGI ----------------- "<<GetCgiName()<<"----------------- ");

	if(strcmp(GetInput().GetQueryString(), ""))
	{
		LOG4CPLUS_INFO(logger, "query_string: " << GetInput().GetQueryString());
	}
	if(strcmp(GetInput().GetPostData(), ""))
	{
		LOG4CPLUS_INFO(logger, "post_data: " << GetInput().GetPostData());
	}

	//logic
	InnerProcess();

	LOG4CPLUS_INFO(logger, "END CGI \n");
	return true;
}
