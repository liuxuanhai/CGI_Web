#ifndef _WEB_HX_CGI_CARD_H_
#define _WEB_HX_CGI_CARD_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "comm_def_card.h"
#include "comm_struct_card.h"
#include "comm_struct_relation_card.h"
using namespace std;
using namespace lce::cgi;

class HooshoCgiCard: public Cgi
{
public:
	HooshoCgiCard(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile, int iCheckLogin) :
			Cgi(qwFlag, strCgiConfigFile, strLogConfigFile), m_user_info("user_info")
	{
		m_check_login = iCheckLogin;
	}

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess()=0;

public:
	//cookie and vc func
	int MakeCookie(const string& strUin, string& strCookieKeyValue, string& strErrMsg);
	int MakeAddReplyCookie(const int& itype, const string& strUin,string& strErrMsg);
	bool SessionAdd(const string& strUin, const string& strKey, string& strErrMsg);
	bool SessionCheck(const string& strUin, const string& strKey, string& strErrMsg);
	bool VerifyCodeAdd(const string& strUin, const string& strVC, string& strErrMsg);
	bool VerifyCodeCheck(const string& strUin, const string& strVC, string& strErrMsg);
	bool VerifyCodeQuery(const string& strUin, string& strVC, string& strErrMsg);
	static bool Memcachedadd(const string& strUin, string& strKey, uint32_t iLife, string& strErrMsg);
	int SendRegisterMail(const string& strToUin, const string& strVC, string& strErrMsg);
	int SendRegisterSMS(const string& strToUin, const string& strVC, string& strErrMsg);
	// captcha
	bool CaptchaAdd(const string& key, const string& data, const size_t& data_size, string& strErrMsg);
	int CaptchaCheck(const string& key, const string& data, string& strErrMsg);
	bool MemcachedDelete(const string& key, string& strErrMsg);
	int CaptchaExist(const string& key, string& strErrMsg);
	bool ResetPasswordVCAdd(const string& prefixUin, const string& strVC, string& strErrMsg);
	int CookieExtend(const uint32_t iType,const string& strCacheNameValue, const string& strCacheKeyValue , string& strErrMsg);

	//id func
	string GenerateTransID(int iIDType);

public:
	static uint32_t GetRandNum(uint32_t dwInitSeed);
	static uint32_t GetRandNum(uint32_t dwFrom, uint32_t dwTo);
	static string GetRandNumLetter(char* key, int len);

protected:

	//DB
	string m_db_ip;
	string m_db_name;
	string m_db_user;
	string m_db_pass;
	string m_table_name_card_user_password;
	string m_table_name_card_user_info;

	string m_table_name_card_picture;
	string m_table_name_apply;
	string m_table_name_developer_info;
	string m_table_name_operator_info;
	string m_table_name_wx_account_auth_info;
	string m_table_name_wb_account_auth_info;
	string m_table_name_options_info;

	string m_table_name_landing_page_info;
	string m_table_name_entity_shop_info;
	string m_table_name_wx_entity_shop_info;

	string m_table_name_card_info;
	string m_table_name_submerchant_info;

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

	//entityshop
	uint32_t m_entityshop_life;
	uint32_t m_entityshop_limit;
	uint32_t m_cardlist_life;
	uint32_t m_submerchantlist_life;

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
	//sms api
	string m_sms_api_host;
	string m_sms_api_ip;
	uint16_t m_sms_api_port;
	string m_sms_api_account;
	string m_sms_api_accountpwd;
	string m_sms_api_appid;
	string m_sms_api_version;
	string m_sms_templateid;
	string m_sms_subject;
	string m_sms_html;

	// font path
	string m_font_path;

	//�Ƿ���֤��¼̬
	int m_check_login;
	//login user
	CardUserInfo m_user_info;

protected:
	DECL_LOGGER(logger)
	;
};

#endif
