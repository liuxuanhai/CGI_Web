#include "cgi_feeds_base.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

IMPL_LOGGER(CgiFeedsBase, logger);

bool CgiFeedsBase::DerivedInit()
{
	GetConfig().GetValue("db", "ip", m_db_ip, "");
	GetConfig().GetValue("db", "name", m_db_name, "");
	GetConfig().GetValue("db", "user", m_db_user, "");
	GetConfig().GetValue("db", "passwd", m_db_pass, "");
	GetConfig().GetValue("db", "table_name_user_info", m_table_name_user_info, "");

	assert(m_mysql.Init(m_db_ip, m_db_name, m_db_user, m_db_pass));

	GetConfig().GetValue("cookie", "name_key", m_cookie_name_key, "");
	GetConfig().GetValue("cookie", "name_value", m_cookie_name_val, "");


	GetConfig().GetValue("feeds", "default_show_follow_num", m_default_show_follow_num, 0);
	GetConfig().GetValue("feeds", "feed_length_limit", m_feed_length_limit, 0);

	GetConfig().GetValue("resource", "path", m_res_path, "");

	GetConfig().GetValue("feeds_server", "ip", m_feeds_server_ip, "");
	GetConfig().GetValue("feeds_server", "port", m_feeds_server_port, 0);

	GetConfig().GetValue("user_server", "ip", m_user_server_ip, "");
	GetConfig().GetValue("user_server", "port", m_user_server_port, 0);
	
	return true;
}

bool CgiFeedsBase::Process()
{
	LOG4CPLUS_DEBUG(logger, "begin cgi ----------------- "<<GetCgiName()<<"----------------- ");

	LOG4CPLUS_DEBUG(logger, "query_string="<<(GetInput().GetQueryString()));
	LOG4CPLUS_DEBUG(logger, "post_data="<<(GetInput().GetPostData()));

	std::string strErrMsg = "";
	if(CGI_NEED_LOGIN_NO == m_check_login)
	{
		LOG4CPLUS_DEBUG(logger, "no need check login");
		m_cookie_value_key = (string)GetInput().GetValue("openid");
	}
	else if(CGI_NEED_LOGIN_YES == m_check_login)
	{
		m_cookie_value_key = (string)GetInput().GetCookie(m_cookie_name_key);
		m_cookie_value_val = (string)GetInput().GetCookie(m_cookie_name_val);

		if(!CheckLogin(GetInput(), m_cookie_name_key, m_cookie_name_val, m_user_server_ip, m_user_server_port, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "SessionCheck failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_NO_LOGIN);
			return true;
		}
	}
	
	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();

	LOG4CPLUS_DEBUG(logger, "end cgi ----------------- "<<GetCgiName()<<"----------------- \n\n\n");

	return true;
}


