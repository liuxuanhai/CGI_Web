
#ifndef _WEB_MAIN_CGI_H_
#define _WEB_MAIN_CGI_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "comm_def.h"
#include "comm_struct.h"
using namespace std;
using namespace lce::cgi;

class MainCgi: public Cgi
{
public:
	 MainCgi(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile, int iCheckLogin)
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
	int SendRegisterMail(const string& strToUin, const string& strVC, string& strErrMsg);
	//id func
	string GenerateTransID(int iIDType);

public:
	static uint32_t GetRandNum(uint32_t dwInitSeed);
	static uint32_t GetRandNum(uint32_t dwFrom, uint32_t dwTo);
	static string GetRandNumLetter(char* key, int len);

protected:

	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;
	
	string m_db_name_common;
	string m_table_name_pic;
	string m_table_name_doc;
	
	string m_db_name_main;
	string m_table_name_password;
	string m_table_name_user_info;
	string m_table_name_dev_info;
	string m_table_name_oper_info;
	string m_table_name_wx_public_account;
	string m_table_name_wx_public_account_feedback;
	string m_table_name_dev_order;
	string m_table_name_oper_order;
	string m_table_name_oper_order_reverse;
	string m_table_name_open_order;

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

	//ranklist
	uint32_t m_ranklist_life;
	uint32_t m_ranklist_size;

	//mail api
	string m_mail_api_host;
	string m_mail_api_ip;
	uint16_t m_mail_api_port;
	string m_mail_api_path;
	string m_mail_api_user;
	string m_mail_api_key;
	string m_mail_from;
	string m_mail_from_name;
	string m_mail_subject;
	string m_mail_html;

	//是否需要校验登陆态
	int m_check_login;

	//login user
	UserInfo m_user_info;
protected:
	DECL_LOGGER(logger);
};

#endif
