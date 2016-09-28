#include "cgi_feeds_server.h"
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



IMPL_LOGGER(CgiFeedsServer, logger);

bool CgiFeedsServer::DerivedInit()
{
	//db

	//GetConfig().GetValue("db", "db_ip", m_db_ip, "");
	

	GetConfig().GetValue("feeds_server", "ip", m_feeds_server_ip, "");
	GetConfig().GetValue("feeds_server", "port", m_feeds_server_port, 0);

	GetConfig().GetValue("user_server", "ip", m_user_server_ip, "");
	GetConfig().GetValue("user_server", "port", m_user_server_port, 0);
	
	GetConfig().GetValue("feeds", "feeds_limit_length", m_feeds_limit_length, 0);
	GetConfig().GetValue("feeds", "follow_limit_length", m_follow_limit_length, 0);
	GetConfig().GetValue("feeds", "default_show_topic_feed_num", m_default_show_topic_feed_num , 0);
	GetConfig().GetValue("feeds", "default_show_banner_feed_num", m_default_show_banner_feed_num, 0);
	GetConfig().GetValue("feeds", "default_show_comment_num", m_default_show_comment_num , 0);
	GetConfig().GetValue("feeds", "default_show_favorite_num", m_default_show_favorite_num , 0);

	GetConfig().GetValue("db", "db_ip", m_db_ip, "");
	GetConfig().GetValue("db", "db_user", m_db_user, "");
	GetConfig().GetValue("db", "db_passwd", m_db_pass, "");
	GetConfig().GetValue("db", "db_name_pa", m_db_name_pa, "");
	GetConfig().GetValue("db", "table_name_pa_info", m_table_name_pa_info, "");

	m_pa_mysql.Init(m_db_ip, m_db_name_pa, m_db_user, m_db_pass);

	LOG4CPLUS_DEBUG(logger, "CONFIG MSG:");
	LOG4CPLUS_DEBUG(logger, "feeds_server: ip = " << m_feeds_server_ip << ", port = " << m_feeds_server_port << endl
							<< "user_server: ip = " << m_user_server_ip << ", port = " << m_user_server_port << endl
							<< "feeds_limit_length = " << m_feeds_limit_length << endl
							<< "follow_limit_length = " << m_follow_limit_length << endl
							<< "default_show_topic_feed_num = " << m_default_show_topic_feed_num << endl
							<< "default_show_banner_feed_num = " << m_default_show_banner_feed_num << endl
							<< "default_show_comment_num = " << m_default_show_comment_num << endl
							<< "default_show_favorite_num = " << m_default_show_favorite_num << endl
							);
	return true;
}

bool CgiFeedsServer::Process()
{
	LOG4CPLUS_DEBUG(logger, "begin cgi ----------------- "<<GetCgiName()<<"----------------- ");

	LOG4CPLUS_DEBUG(logger, "query_string="<<(GetInput().GetQueryString()));
	LOG4CPLUS_DEBUG(logger, "post_data="<<(GetInput().GetPostData()));

	

	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();

	LOG4CPLUS_DEBUG(logger, "end cgi ----------------- "<<GetCgiName()<<"----------------- \n\n\n");

	return true;
}

