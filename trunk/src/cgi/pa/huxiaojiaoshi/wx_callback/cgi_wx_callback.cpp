#include "cgi_wx_callback.h"
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


IMPL_LOGGER(CgiWXCallback, logger);

bool CgiWXCallback::DerivedInit()
{
	GetConfig().GetValue("wx", "mp_token", m_mp_token, "");
	GetConfig().GetValue("wx", "encoding_aes_key", m_encoding_aes_key, "");
	GetConfig().GetValue("wx", "appid", m_appid, "");

	LOG4CPLUS_DEBUG(logger, "mp_token = " << m_mp_token);
	LOG4CPLUS_DEBUG(logger, "encoding_aes_key = " << m_encoding_aes_key);
	LOG4CPLUS_DEBUG(logger, "appid = " << m_appid);
	return true;
}

bool CgiWXCallback::Process()
{
	LOG4CPLUS_DEBUG(logger, "begin cgi ----------------- "<<GetCgiName()<<"----------------- ");

	LOG4CPLUS_DEBUG(logger, "query_string="<<(GetInput().GetQueryString()));
	LOG4CPLUS_DEBUG(logger, "post_data="<<(GetInput().GetPostData()));


	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();

	LOG4CPLUS_DEBUG(logger, "end cgi ----------------- "<<GetCgiName()<<"----------------- \n\n\n");

	return true;
}

