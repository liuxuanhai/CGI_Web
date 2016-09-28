#include "cgi_server.h"
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

IMPL_LOGGER(CgiServer, logger);

bool CgiServer::DerivedInit()
{
	GetConfig().GetValue("s_server", "ip", SServerIP, "");
	GetConfig().GetValue("s_server", "port", SServerPort, 0);
	
	return true;
}

bool CgiServer::Process()
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

