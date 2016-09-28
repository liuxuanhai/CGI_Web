#ifndef _CGI_FEEDS_SERVER_H_
#define _CGI_FEEDS_SERVER_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "proto_io_tcp_client.h"
#include "msg.pb.h"
#include "../cgi_ret_code_def.h"
#include "feeds_cgi_util.h"

using namespace std;
using namespace lce::cgi;

class CgiFeedsServer: public Cgi
{
public:
	CgiFeedsServer(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile, int iCheckLogin) :
			Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{
		m_check_login = iCheckLogin;
	}

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess()=0;

public:
	string m_feeds_server_ip;
	int m_feeds_server_port;

	string m_user_server_ip;
	int m_user_server_port;

	uint32_t m_feeds_limit_length;
	uint32_t m_follow_limit_length;
	uint32_t m_default_show_topic_feed_num;
	uint32_t m_default_show_banner_feed_num;
	uint32_t m_default_show_comment_num;
	uint32_t m_default_show_favorite_num;

	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;
	string m_db_name_pa;
	string m_table_name_pa_info;

	lce::cgi::CMysql m_pa_mysql;
	
public:
	

protected:
	int m_check_login;

protected:
	DECL_LOGGER(logger)
	;
};

#endif
