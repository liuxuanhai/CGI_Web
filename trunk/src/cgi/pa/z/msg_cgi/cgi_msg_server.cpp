#include "cgi_msg_server.h"
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



IMPL_LOGGER(CgiMsgServer, logger);

bool CgiMsgServer::DerivedInit()
{
	GetConfig().GetValue("z_server", "ip", ZServerIP, "");
	GetConfig().GetValue("z_server", "port", ZServerPort, 0);
	GetConfig().GetValue("filter", "interval", interval, 0);
	GetConfig().GetValue("filter", "lenLimit", lenLimit, 0);
	GetConfig().GetValue("filter", "getMsgAmountLimit", getMsgAmountLimit, 0);
	return true;
}

bool CgiMsgServer::Process()
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

	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();

	LOG4CPLUS_INFO(logger, "END CGI \n");

	return true;
}

bool CgiMsgServer::isAllSpace(const char* s)
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


