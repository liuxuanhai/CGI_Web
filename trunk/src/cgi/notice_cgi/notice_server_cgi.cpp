#include "notice_server_cgi.h"
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
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <iconv.h>
#include "util/md5.h"

#define SA      struct sockaddr
#define MAXLINE 4096
#define MAXSUB  2000
#define MAXPARAM 2048

#define LISTENQ         1024

extern int h_errno;
int sockfd;

IMPL_LOGGER(NoticeServerCgi, logger);

bool NoticeServerCgi::DerivedInit()
{
	GetConfig().GetValue("notice_server", "ip", m_notice_server_ip, "");
	GetConfig().GetValue("notice_server", "port", m_notice_server_port, 0);

	GetConfig().GetValue("feeds_server", "ip", m_feeds_server_ip, "");
	GetConfig().GetValue("feeds_server", "port", m_feeds_server_port, 0);

	GetConfig().GetValue("user_server", "ip", m_user_server_ip, "");
	GetConfig().GetValue("user_server", "port", m_user_server_port, 0);

	GetConfig().GetValue("statistic_server", "ip", m_statistic_server_ip, "");
	GetConfig().GetValue("statistic_server", "port", m_statistic_server_port, 0);



	LOG4CPLUS_DEBUG(logger, "feeds_server: ip = " << m_feeds_server_ip << ", port = " << m_feeds_server_port << endl
							<< "user_server: ip = " << m_user_server_ip << ", port = " << m_user_server_port << endl
							<< "statistic_server: ip = " << m_statistic_server_ip << ", port = " << m_statistic_server_port << endl
							);
	return true;



	return true;
}

bool NoticeServerCgi::Process()
{
	LOG4CPLUS_DEBUG(logger, "\n\nbegin cgi ----------------- "<<GetCgiName()<<"----------------- ");

	LOG4CPLUS_DEBUG(logger, "query_string="<<(GetInput().GetQueryString()));
	LOG4CPLUS_DEBUG(logger, "post_data="<<(GetInput().GetPostData()));




	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();

	LOG4CPLUS_DEBUG(logger, "end cgi ----------------- "<<GetCgiName()<<"----------------- ");

	return true;
}



