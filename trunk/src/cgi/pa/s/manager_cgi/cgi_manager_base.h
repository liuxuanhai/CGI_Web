#ifndef _PA_Z_CGI_ORDER_BASE_H_
#define _PA_Z_CGI_ORDER_BASE_H_

#include "cgi/cgi.h"
#include "../cgi_common_util.h"
#include "common_util.h"

using namespace lce::cgi;

class CgiManagerBase: public Cgi
{
public:
	CgiManagerBase(uint64_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile, bool bCheckLogin)
		:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile), m_check_login(bCheckLogin)
	{

	}

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess() = 0;

public:
	bool IsValidPhone(const std::string& strPhone);
	bool CheckPasswd(const std::string& strPhone, const std::string strPasswd, std::string& strErrMsg);

	bool VerifyCodeAdd(const std::string& strPhone, const std::string& strVC, std::string& strErrMsg);
	bool VerifyCodeQuery(const std::string& strPhone, std::string& strVC, std::string& strErrMsg);
	bool VerifyCodeCheck(const std::string& strPhone, const std::string& strVC, std::string& strErrMsg);

	bool SessionAdd(const std::string& strUin, const std::string& strVC, std::string& strErrMsg);
	bool SessionCheck(const std::string& strUin, const std::string& strVC, std::string& strErrMsg);

	int MakeCookie(const std::string& strUin, std::string& strCookieKeyValue, std::string& strErrMsg);
	int MakeAddReplyCookie(const string& strUin,lce::cgi::CHttpHeader& stHttpRspHeader, string& strErrMsg);
	
	int SendTemplateMessageOfInvoicePass(const hoosho::msg::z::OrderInfo &stOrderInfo);

public:
	bool m_check_login;
	std::string m_login_phone;

	uint32_t m_manager_phone_size;
	std::vector<std::string> m_manager_phone_list;
	std::vector<std::string> m_manager_passwd_list;

	std::string m_cookie_name_uin;
	std::string m_cookie_name_key;
	std::string m_cookie_domain;
	std::string m_cookie_path;

	std::string m_memcached_cache_ip;
	uint32_t m_memcached_cache_port_session;
	uint32_t m_memcached_cache_port_vc;

	uint32_t m_session_life;

	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_table_name_user_info;
	std::string m_table_name_user_identity_info;
	std::string m_table_name_good_info;
	std::string m_table_name_good_type;
	std::string m_table_name_consume_type;
	std::string m_table_name_feed_info;
	std::string m_table_name_ticket_info;
	std::string m_table_name_pic_info;
	lce::cgi::CMysql m_mysql_helper;

	std::string m_sms_api_host;
	std::string m_sms_api_ip;
	uint16_t m_sms_api_port;
	std::string m_sms_api_account;
	std::string m_sms_api_accountpwd;
	std::string m_sms_api_appid;
	std::string m_sms_api_version;
	std::string m_sms_templateid;
	std::string m_sms_subject;
	std::string m_sms_html;

	std::string m_s_server_ip;
	uint16_t m_s_server_port;

	std::string m_pa_appid;

	std::string m_invoice_pass_template_id;
	std::string m_invoice_pass_color;
	std::string m_invoice_pass_url;
	std::string m_invoice_pass_first;
	std::string m_invoice_pass_keyword1;
	std::string m_invoice_pass_keyword2;
	std::string m_invoice_pass_keyword3;
	std::string m_invoice_pass_remark;
	
protected:
	DECL_LOGGER(logger);

};






#endif
