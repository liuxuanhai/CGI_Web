
#ifndef _HP_MANAGER_CGI_H_
#define _HP_MANAGER_CGI_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "comm_def.h"
#include "comm_struct.h"
#include "../cgi_ret_code_def.h"
#include "proto_io_tcp_client.h"
#include "msg.pb.h"
#include "manager_cgi_util.h"

using namespace std;
using namespace lce::cgi;

class ManagerCgi: public Cgi
{
public:
	 ManagerCgi(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile, int iCheckLogin)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	m_check_login = iCheckLogin;
	 }

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess()=0;

public:
	//cookie and vc func
	int MakeCookie(const string& strUin,  string& strCookieKeyValue, string& strErrMsg);
	int MakeAddReplyCookie(const string& strUin,lce::cgi::CHttpHeader& stHttpRspHeader, string& strErrMsg);
	bool SessionAdd(const string& strUin, const string& strKey, string& strErrMsg);
	bool SessionCheck(const string& strUin, const string& strKey, string& strErrMsg);
	bool VerifyCodeAdd(const string& strUin, const string& strVC, string& strErrMsg);
	bool VerifyCodeCheck(const string& strUin, const string& strVC, string& strErrMsg);
	bool VerifyCodeQuery(const string& strUin, string& strVC, string& strErrMsg);

public:
	static uint32_t GetRandNum(uint32_t dwInitSeed);
	static uint32_t GetRandNum(uint32_t dwFrom, uint32_t dwTo);
	static string GetRandNumLetter(char* key, int len);

protected:

	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;

	string m_db_name_user;
	string m_table_name_manager;

	string m_db_name_feeds;
	string m_table_name_user_forbid;
	string m_table_name_feed_report;
	string m_table_name_feed_info;
	string m_table_name_feed_index_on_appid;

	string m_db_name_pa;
	string m_table_name_pa_info;

	//server
	string m_feeds_server_ip;
	int m_feeds_server_port;

	string m_user_server_ip;
	int m_user_server_port;

	string m_statistic_server_ip;
	int m_statistic_server_port;
	
	uint32_t m_feeds_limit_length;
	uint32_t m_feeds_limit_num;

	//cookie
	string m_cookie_name_uin;
	string m_cookie_name_key;
	string m_cookie_domain;
	string m_cookie_path;

	// memcache
	string m_cache_ip;
	uint16_t m_cache_port_session;
	uint16_t m_cache_port_vc;

	//session life
	uint32_t m_session_life;

	//是否需要校验登陆态
	int m_check_login;

	//login manager
	ManagerInfo m_manager_info;
protected:
	DECL_LOGGER(logger);
};

#endif
