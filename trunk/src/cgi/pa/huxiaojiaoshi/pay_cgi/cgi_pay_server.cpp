#include "cgi_pay_server.h"
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

#include "../cgi_common_util.h"



IMPL_LOGGER(CgiPayServer, logger);

bool CgiPayServer::DerivedInit()
{
	GetConfig().GetValue("pay_server", "ip", PayServerIP, "");
	GetConfig().GetValue("pay_server", "port", PayServerPort, 0);

	GetConfig().GetValue("user_server", "ip", UserServerIP, "");	
	GetConfig().GetValue("user_server", "port", UserServerPort, 0);

	GetConfig().GetValue("cookie", "name_key", m_cookie_name_key, "");	
	GetConfig().GetValue("cookie", "name_value", m_cookie_name_val, "");
	
	return true;
}

bool CgiPayServer::Process()
{
	LOG4CPLUS_INFO(logger, "BEGIN CGI ----------------- "<<GetCgiName()<<"----------------- ");

	if(strcmp(GetInput().GetQueryString(), "") != 0)
	{
		LOG4CPLUS_INFO(logger, "GET PARAM: \n"<<(GetInput().GetQueryString()));
	}
	if(strcmp(GetInput().GetPostData(), "") != 0)
	{
		LOG4CPLUS_INFO(logger, "POST PARAM: \n"<<(GetInput().GetPostData()));
	}	

	std::string strErrMsg = "";
	if(CGI_NEED_LOGIN_NO == m_check_login)
	{
		LOG4CPLUS_DEBUG(logger, "no need check login");
	}
	else if(CGI_NEED_LOGIN_YES == m_check_login)
	{
		m_cookie_value_key = (string)GetInput().GetCookie(m_cookie_name_key);		
		m_cookie_value_val = (string)GetInput().GetCookie(m_cookie_name_val);

		if(!CheckLogin(GetInput(), m_cookie_name_key, m_cookie_name_val, UserServerIP, UserServerPort, strErrMsg))		
		{			
			LOG4CPLUS_ERROR(logger, "SessionCheck failed, errmsg = " << strErrMsg);			
			DoReply(CGI_RET_CODE_NO_LOGIN);			
			return true;		
		}
		LOG4CPLUS_INFO(logger, "openid: "<<m_cookie_value_key);
	}

	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();

	LOG4CPLUS_INFO(logger, "END CGI \n");

	return true;
}

bool CgiPayServer::isAllSpace(const char* s)
{
	const char *start = s;
	int len = strlen(s);
	const char *end = s + len;

	while(true)
	{
		if((*start == '\0') || (*start != ' '))
			break;
		start++;
	}

	if(start < end)
		return false;

	return true;	
}


