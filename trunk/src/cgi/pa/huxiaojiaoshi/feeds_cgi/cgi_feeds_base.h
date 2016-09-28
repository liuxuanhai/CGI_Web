#ifndef _CGI_FEEDS_BASE_H_
#define _CGI_FEEDS_BASE_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "proto_io_tcp_client.h"
#include "msg.pb.h"
#include "../cgi_common_util.h"

//#define LOCAL_TEST

using namespace std;
using namespace lce::cgi;

class CgiFeedsBase: public Cgi
{
public:
	CgiFeedsBase(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile, int iCheckLogin) :
			Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{
#ifdef LOCAL_TEST
        m_check_login = false;
#else
		m_check_login = iCheckLogin;
#endif
	}

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess()=0;

public:
	//

protected:
	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;
	string m_db_name;
	string m_table_name_user_info;
	lce::cgi::CMysql m_mysql;

	//cookie
	string m_cookie_name_key;
	string m_cookie_name_val;
	string m_cookie_value_key;
	string m_cookie_value_val;

	//feeds
	uint64_t m_default_show_follow_num;
	uint64_t m_feed_length_limit;

	//res
	string m_res_path;

	//server
	string m_feeds_server_ip;
	int m_feeds_server_port;

	string m_user_server_ip;
	int m_user_server_port;

	//
	int m_check_login;

	//login user
	::hoosho::j::commstruct::UserInfo m_user_info;

protected:
	DECL_LOGGER(logger)
	;
};

#endif
