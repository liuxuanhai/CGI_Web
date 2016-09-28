#include "hoosho_cgi_card.h"
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
#include "comm_def_card.h"
#include "comm_struct_card.h"
#include "libmemcached/memcached.h"

#define SA      struct sockaddr
#define MAXLINE 4096
#define MAXSUB  2000
#define MAXPARAM 2048

#define LISTENQ         1024

extern int h_errno;
int sockfd;
const char hname[40] = "yunpian.com";
const char send_sms_json[40] = "/v1/sms/send.json";
const char get_user_json[40] = "/v1/user/get.json";

/**
 * 查账户信息
 */
/*ssize_t get_user(char *apikey)
 {
 char params[MAXPARAM + 1];
 char *cp = params;
 sprintf(cp,"apikey=%s", apikey);
 return http_post(get_user_json, cp);
 }*/

IMPL_LOGGER(HooshoCgiCard, logger);

bool HooshoCgiCard::DerivedInit()
{
	//db

	GetConfig().GetValue("db", "db_ip", m_db_ip, "");
	GetConfig().GetValue("db", "db_name", m_db_name, "");
	GetConfig().GetValue("db", "db_user", m_db_user, "");
	GetConfig().GetValue("db", "db_passwd", m_db_pass, "");
	GetConfig().GetValue("db", "table_name_card_user_password", m_table_name_card_user_password, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_card_user_info", m_table_name_card_user_info, "unknown_table_name");

	GetConfig().GetValue("db", "table_name_card_picture", m_table_name_card_picture, "unknown_table_name");

	GetConfig().GetValue("db", "table_name_wx_account_auth_info", m_table_name_wx_account_auth_info, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_wb_account_auth_info", m_table_name_wb_account_auth_info, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_options_info", m_table_name_options_info, "unknown_table_name");

	GetConfig().GetValue("db", "table_name_entity_shop_info", m_table_name_entity_shop_info, "unknown_table_name");

	GetConfig().GetValue("db", "table_name_wx_entity_shop_info", m_table_name_wx_entity_shop_info, "unknown_table_name");

	GetConfig().GetValue("db", "table_name_landing_page_info", m_table_name_landing_page_info, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_card_info", m_table_name_card_info, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_submerchant_info", m_table_name_submerchant_info, "unknown_table_name");

	if (!TableBaseCard::g_hx_mysql.Init(m_db_ip, m_db_name, m_db_user, m_db_pass))
	{
		LOG4CPLUS_ERROR(logger,
				"mysql init failed!!!, db_ip:"<<m_db_ip <<", db_name: "<<m_db_name <<", db_user: "<<m_db_user <<", db_passwd: "<<m_db_pass);

		return false;
	}
	if (!TableRelationBaseCard::g_hx_mysql.Init(m_db_ip, m_db_name, m_db_user, m_db_pass))
	{
		LOG4CPLUS_ERROR(logger,
				"mysql relation init failed!!!, db_ip:"<<m_db_ip <<", db_name: "<<m_db_name <<", db_user: "<<m_db_user <<", db_passwd: "<<m_db_pass);

		return false;
	}

	//cookie cfg
	GetConfig().GetValue("cookie", "name_uin", m_cookie_name_uin, "hoosho_card_uin");
	GetConfig().GetValue("cookie", "name_key", m_cookie_name_key, "hoosho_card_key");

	GetConfig().GetValue("cookie", "path", m_cookie_path, "/");
	GetConfig().GetValue("cookie", "domain", m_cookie_domain, "120.25.124.72");

	//memcache
	GetConfig().GetValue("memcached", "cache_ip", m_cache_ip, "127.0.0.1");
	GetConfig().GetValue("memcached", "cache_port_session", m_cache_port_session, 49001);
	GetConfig().GetValue("memcached", "cache_port_vc", m_cache_port_vc, 49002);

	//session life
	GetConfig().GetValue("session", "life", m_session_life, 43200);

	//list
	GetConfig().GetValue("list", "entityshoplistlife", m_entityshop_life, 3600);
	GetConfig().GetValue("list", "entityshoplistlimit", m_entityshop_limit, 50);
	GetConfig().GetValue("list", "cardlistlife", m_cardlist_life, 3600);
	GetConfig().GetValue("list", "submerchantlistlife", m_submerchantlist_life, 3600);

	//mail api
	GetConfig().GetValue("mail_api", "api_host", m_mail_api_host, "sendcloud.sohu.com");
	GetConfig().GetValue("mail_api", "api_ip", m_mail_api_ip, "123.125.122.12");
	GetConfig().GetValue("mail_api", "api_port", m_mail_api_port, 80);
	GetConfig().GetValue("mail_api", "api_path", m_mail_api_path, "/webapi/mail.send.json");
	GetConfig().GetValue("mail_api", "api_user", m_mail_api_user, "hoosho_test_nGrwjx");
	GetConfig().GetValue("mail_api", "api_key", m_mail_api_key, "5ht1oG5W0pqqoAbM");
	GetConfig().GetValue("mail_api", "mail_from", m_mail_from, "manager@hoosho.com");
	GetConfig().GetValue("mail_api", "mail_fromname", m_mail_from_name, "��Ц��");
	GetConfig().GetValue("mail_api", "mail_subject", m_mail_subject, "����Ц��ע����֤��");
	GetConfig().GetValue("mail_api", "mail_html", m_mail_html, "");
	//sms api
	GetConfig().GetValue("sms_api", "sms_api_host", m_sms_api_host, "sandboxapp.cloopen.com");
	GetConfig().GetValue("sms_api", "sms_api_ip", m_sms_api_ip, "42.121.254.126");
	GetConfig().GetValue("sms_api", "sms_api_port", m_sms_api_port, 8883);
	GetConfig().GetValue("sms_api", "sms_api_account", m_sms_api_account, "aaf98f894f402f15014f43a63e000227");
	GetConfig().GetValue("sms_api", "sms_api_accountpwd", m_sms_api_accountpwd, "444a2b08cdac4a25983fc80f02eaaa6f");
	GetConfig().GetValue("sms_api", "sms_api_appid", m_sms_api_appid, "aaf98f894f402f15014f43a9c147022d");
	GetConfig().GetValue("sms_api", "sms_api_version", m_sms_api_version, "/2013-12-26");
	GetConfig().GetValue("sms_api", "sms_templateid", m_sms_templateid, "1");
	GetConfig().GetValue("sms_api", "sms_subject", m_sms_subject, "【虎笑网】注册验证码");
	GetConfig().GetValue("sms_api", "sms_html", m_sms_html, "欢迎注册虎笑网,您本次的注册码为%s,请妥善保管,感谢您的支持!");

	//font path
	GetConfig().GetValue("captcha", "font_path", m_font_path, "./cmr10.ttf");

	return true;
}

bool HooshoCgiCard::Process()
{
	LOG4CPLUS_DEBUG(logger, "\n\nbegin cgi ----------------- "<<GetCgiName()<<"----------------- ");

	LOG4CPLUS_DEBUG(logger, "query_string="<<(GetInput().GetQueryString()));
	LOG4CPLUS_DEBUG(logger, "post_data="<<(GetInput().GetPostData()));

	std::string strErrMsg = "";
	LOG4CPLUS_DEBUG(logger, " LoginStatus="<<m_check_login<<"(0:No login,1:need login,2:anyway)");
	if (m_check_login == NO_LOGIN)
	{
		LOG4CPLUS_DEBUG(logger, "no need check login");
	}
	else if (m_check_login > NO_LOGIN)
	{
		string strCookieUin;
		string strUin;
		string strCookieKey;

		if (m_check_login == USER_LOGIN)
		{
			strCookieUin = ((string) GetInput().GetCookie(m_cookie_name_uin));

			if (strCookieUin.length() <= 9)
			{
				LOG4CPLUS_ERROR(logger, "no invalid cookie found, strCookieUin="<<strCookieUin);
				DoReply(CGI_RET_CODE_NO_LOGIN);
				return true;
			}

			strUin = strCookieUin.substr(9); // prefix="0_0_CARD"
			strCookieKey = (string) GetInput().GetCookie(m_cookie_name_key);
			string strCacheName = "NOCARD" + strUin;
			if (!SessionCheck(strCacheName, strCookieKey, strErrMsg))
			{
				LOG4CPLUS_ERROR(logger, "CheckCookie failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_NO_LOGIN);
				return true;
			}
			CardUserInfo stCardUserInfo(m_table_name_card_user_info);
			stCardUserInfo.m_uin = strUin;
			if (stCardUserInfo.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_FATAL(logger, "init CardUserInfo failed, uin="<<strCookieUin<<", errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			m_user_info = stCardUserInfo;
			if (-1 == CookieExtend(USER_LOGIN, strCacheName, strCookieKey, strErrMsg))
			{
				LOG4CPLUS_ERROR(logger, "Cookie extend fail,errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "one req, CardUserInfo="<<m_user_info.ToString());

		}
		else if (m_check_login == USER_LOGIN_NOLOGIN)
		{

			strCookieUin = ((string) GetInput().GetCookie(m_cookie_name_uin));
			m_user_info.m_uin.clear();
			while (1)
			{
				if (strCookieUin.length() <= 9)
				{
					LOG4CPLUS_ERROR(logger, "no invalid cookie found, LOGIN_NOLOGIN = NOLOGIN");
					break;
				}
				strCookieUin = strCookieUin.substr(9); // prefix="0_0_"
				strCookieKey = (string) GetInput().GetCookie(m_cookie_name_key);
				if (!SessionCheck("NOCARD" + strCookieUin, strCookieKey, strErrMsg))
				{
					LOG4CPLUS_ERROR(logger, "CheckCookie failed,LOGIN_NOLOGIN = NOLOGIN");
					break;
				}
				CardUserInfo stCardUserInfo(m_table_name_card_user_info);
				stCardUserInfo.m_uin = strCookieUin;
				if (stCardUserInfo.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
				{
					LOG4CPLUS_FATAL(logger, "init CardUserInfo failed,NOLOGIN_NOLOGIN,uin=" << strCookieUin<< ", errmsg=" << strErrMsg);
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}
				m_user_info = stCardUserInfo;
				LOG4CPLUS_DEBUG(logger, "one req, CardUserInfo=" << m_user_info.ToString());
				break;
			}

		}

	}

	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();

	LOG4CPLUS_DEBUG(logger, "end cgi ----------------- "<<GetCgiName()<<"----------------- ");

	return true;
}

/*int HooshoCgiCard::MakeCookie(const int& itype,const string& strUin, string& strCookieUinValue, string& strCookieKeyValue, string& strErrMsg)
 {
 if(strUin.empty())
 {
 strErrMsg = "uin is empty";
 return -1;
 }
 if(itype<1 || itype>3)
 {
 strErrMsg = "itype error";
 return -1;
 }
 //targetstring = md5(uin + time(0) + rand)
 timeval tv;
 gettimeofday(&tv, NULL);
 srand(tv.tv_usec ^ 77);
 uint32_t r = rand();
 string strTargetString = strUin + int_2_str(time(0)) + int_2_str(r);
 md5 md;
 uint64_t qwMd5Sum =  md.hash64(strTargetString.c_str(), strTargetString.size());
 uint16_t dwHigh16 = (uint16_t)(qwMd5Sum >> 48);
 uint32_t dwLow16 = (uint16_t)(qwMd5Sum & 0xFFFF);
 uint32_t dwFinal = ((dwHigh16<<16) | dwLow16);
 if(itype==1)
 {
 strCookieUinValue = "0_0_" + strUin;
 }
 else if(itype==2)
 {
 strCookieUinValue = "0_0_O_P_E_" + strUin;
 }
 else if(itype == FACILITATOR_LOGIN)
 {
 strCookieUinValue = "0_0_F_A_C_" + strUin;
 }
 else
 {
 strErrMsg = "itype error,second";
 return -1;
 }
 strCookieKeyValue = int_2_str(dwFinal);

 return 0;
 }*/
int HooshoCgiCard::MakeCookie(const string& strUin, string& strCookieKeyValue, string& strErrMsg)
{
	if (strUin.empty())
	{
		strErrMsg = "uin is empty";
		return -1;
	}

	//targetstring = md5(uin + time(0) + rand)
	timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec ^ 77);
	uint32_t r = rand();
	string strTargetString = strUin + int_2_str(time(0)) + int_2_str(r);
	md5 md;
	uint64_t qwMd5Sum = md.hash64(strTargetString.c_str(), strTargetString.size());
	uint16_t dwHigh16 = (uint16_t) (qwMd5Sum >> 48);
	uint32_t dwLow16 = (uint16_t) (qwMd5Sum & 0xFFFF);
	uint32_t dwFinal = ((dwHigh16 << 16) | dwLow16);

	//strCookieUinValue = "0_0_" + strUin;

	strCookieKeyValue = int_2_str(dwFinal);

	return 0;
}

int HooshoCgiCard::CookieExtend(const uint32_t iType, const string& strCacheNameValue, const string& strCacheKeyValue, string& strErrMsg)
{
	if (strCacheNameValue.empty() || strCacheKeyValue.empty())
	{
		strErrMsg = "cachename or cachekeyvalue  is empty";
		return -1;
	}

	if (iType == USER_LOGIN)
	{
		LOG4CPLUS_ERROR(logger, "cookie extend, strCacheNameValue="<<strCacheNameValue<<",strCacheKeyValue="<<strCacheKeyValue<<",m_session_life="<<m_session_life);
		if (!SessionAdd(strCacheNameValue, strCacheKeyValue, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "session add failed, errmsg="<<strErrMsg);
			return -1;
		}
	}
	else
	{
		strErrMsg = "invalid cookietype";
		LOG4CPLUS_ERROR(logger, strErrMsg<<",type="<<iType);
		return -1;
	}

	return true;
}
int HooshoCgiCard::MakeAddReplyCookie(const int& itype, const string& strUin, string& strErrMsg)
{
	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
	if (strUin.empty())
	{
		strErrMsg = "uin is empty";
		return -1;
	}
	if (itype < 1 || itype > 3)
	{
		strErrMsg = "itype error";
		return -1;
	}
	string strCookieUinValue = "";
	string strCookieKeyValue = "";

	string strUinCache = "";
	if (MakeCookie(strUin, strCookieKeyValue, strErrMsg) < 0)
	{
		DoReply(CGI_RET_CODE_SERVER_BUSY);
		LOG4CPLUS_ERROR(logger, "make cookie failed, errmsg="<<strUin);
		return true;
	}
	if (itype == USER_LOGIN)
	{
		strCookieUinValue = "0_0_Card_" + strUin;
		strUinCache = "NOCARD" + strUin;
		stHttpRspHeader.SetCookie(m_cookie_name_uin, strCookieUinValue, m_cookie_domain, m_session_life, m_cookie_path, false, false);
		stHttpRspHeader.SetCookie(m_cookie_name_key, strCookieKeyValue, m_cookie_domain, m_session_life, m_cookie_path, false, false);
	}
	else
	{
		strErrMsg = "itype error,second";
		stHttpRspHeader.Reset();
		return -1;
	}

	if (!SessionAdd(strUinCache, strCookieKeyValue, strErrMsg))
	{
		stHttpRspHeader.Reset();
		DoReply(CGI_RET_CODE_SERVER_BUSY);
		LOG4CPLUS_ERROR(logger, "session add failed, errmsg="<<strErrMsg);
		return -1;
	}

	return true;
}
bool HooshoCgiCard::SessionAdd(const string& strUin, const string& strKey, string& strErrMsg)
{
	if (strUin.empty() || strKey.empty())
	{
		strErrMsg = "empty uin or key!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_session, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//set
	rc = memcached_set(memc, strUin.c_str(), strUin.size(), strKey.c_str(), strKey.size(), m_session_life/6, 0);
	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_set failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	memcached_free(memc);
	return true;
}

bool HooshoCgiCard::SessionCheck(const string& strUin, const string& strKey, string& strErrMsg)
{
	if (strUin.empty())
	{
		strErrMsg = "empty uin!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_session, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//get
	uint32_t dwFlags = 0;
	size_t szValueLength;
	char* cpValue = memcached_get(memc, strUin.c_str(), strUin.size(), &szValueLength, &dwFlags, &rc);
	if (MEMCACHED_NOTFOUND == rc)
	{
		strErrMsg = "memcached_get, session not found for key=" + strUin;
		memcached_free(memc);
		return false;
	}

	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_get failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	//check
	string strResult(cpValue, szValueLength);
	if (strResult != strKey)
	{
		strErrMsg = "not equal, req_key=" + strKey + ", cache_key=" + strResult;
		memcached_free(memc);
		return false;
	}

	memcached_free(memc);
	return true;
}

bool HooshoCgiCard::VerifyCodeAdd(const string& strUin, const string& strVC, string& strErrMsg)
{
	if (strUin.empty() || strVC.empty())
	{
		strErrMsg = "empty uin or key!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//set
	rc = memcached_set(memc, strUin.c_str(), strUin.size(), strVC.c_str(), strVC.size(), 600, 0);
	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_set failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	memcached_free(memc);
	return true;
}

bool HooshoCgiCard::VerifyCodeCheck(const string& strUin, const string& strVC, string& strErrMsg)
{
	if (strUin.empty() || strVC.empty())
	{
		strErrMsg = "uin=" + strUin + ", vc=" + strVC + ", empty!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//get
	uint32_t dwFlags = 0;
	size_t szValueLength;
	char* cpValue = memcached_get(memc, strUin.c_str(), strUin.size(), &szValueLength, &dwFlags, &rc);
	if (MEMCACHED_NOTFOUND == rc)
	{
		strErrMsg = "memcached_get, vc not found for key=" + strUin;
		memcached_free(memc);
		return false;
	}

	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_get failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	//check
	string strResult(cpValue, szValueLength);
	if (strResult != strVC)
	{
		strErrMsg = "not equal, req_vc=" + strVC + ", cache_vc=" + strResult;
		memcached_free(memc);
		return false;
	}

	//delete
	memcached_delete(memc, strUin.c_str(), strUin.size(), 0);
	memcached_free(memc);
	return true;
}

bool HooshoCgiCard::VerifyCodeQuery(const string& strUin, string& strVC, string& strErrMsg)
{
	if (strUin.empty())
	{
		strErrMsg = "empty uin!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//get
	uint32_t dwFlags = 0;
	size_t szValueLength;
	char* cpValue = memcached_get(memc, strUin.c_str(), strUin.size(), &szValueLength, &dwFlags, &rc);
	if (MEMCACHED_NOTFOUND == rc)
	{
		strVC = "";
		memcached_free(memc);
		return true;
	}

	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_get failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	//check
	strVC = string(cpValue, szValueLength);
	memcached_free(memc);
	return true;
}
int HooshoCgiCard::SendRegisterSMS(const string& strToUin, const string& strVC, string& strErrMsg)
{
	LOG4CPLUS_DEBUG(logger, "SendRegisterSMS, strToUin"<<strToUin<<", strVC"<<strVC);

	struct sockaddr_in servaddr;
	char **pptr;
	char str[50];
	struct hostent *hptr;
	if ((hptr = gethostbyname(hname)) == NULL)
	{
		fprintf(stderr, "通过域名获取IP失败: %s: %s", hname, hstrerror(h_errno));
		exit(1);
	}
	printf("域名: %s\n", hptr->h_name);
	if (hptr->h_addrtype == AF_INET && (pptr = hptr->h_addr_list) != NULL)
	{
		printf("IP: %s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
	}
	else
	{
		fprintf(stderr, "Error call inet_ntop \n");
		exit(1);
	}

	//建立socket连接
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(80);
	inet_pton(AF_INET, str, &servaddr.sin_addr);

	connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	string apikey = "1933dc1657e18b61e6d7b9f92a41f3cb";
	//修改为您要发送的手机号
	//设置您要发送的内容
	string phone = strToUin;

	string strtext = "【虎笑网络】欢迎使用虎笑科技，您的手机验证码是" + strVC + "。本条信息无需回复";

	/**************** 查账户信息调用示例 *****************/
	//get_user(apikey);
	/**************** 使用通用接口发短信 *****************/
	char params[MAXPARAM + 1];
	char *cp = params;
	sprintf(cp, "apikey=%s&mobile=%s&text=%s", apikey.c_str(), phone.c_str(), strtext.c_str());
	char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
	ssize_t n;
	snprintf(sendline, MAXSUB, "POST %s HTTP/1.0\r\n"
			"Host: %s\r\n"
			"Content-type: application/x-www-form-urlencoded\r\n"
			"Content-length: %zu\r\n\r\n"
			"%s", send_sms_json, hname, strlen(cp), cp);

	write(sockfd, sendline, strlen(sendline));
	string strread = "";
	while ((n = read(sockfd, recvline, MAXLINE)) > 0)
	{
		recvline[n] = '\0';
		strread += recvline;

	}
	close(sockfd);

	vector<string> vecReturn;
	string strReturncode;
	lce::cgi::Split(strread, "\r\n", vecReturn);

	if ("HTTP/1.1 200 OK" != vecReturn[0])
	{
		strErrMsg = "SMS status error status,it return:\r\n" + strread;
		return -1;
	}

	return 0;
}

int HooshoCgiCard::SendRegisterMail(const string& strToUin, const string& strVC, string& strErrMsg)
{
	LOG4CPLUS_DEBUG(logger, "SendRegisterMail, strToUin"<<strToUin<<", strVC"<<strVC);

	//http req
	lce::cgi::CHTTPConnector oHttp;
	oHttp.SetHead("Host", m_mail_api_host);

	static char aczHtmlBuffer[2048];
	bzero(aczHtmlBuffer, 2048);
	snprintf(aczHtmlBuffer, 2048, m_mail_html.c_str(), strVC.c_str());

	std::ostringstream oss;
	oss.str("");
	/*http://sendcloud.sohu.com/webapi/mail.send.json?
	 api_user=***
	 &api_key=***
	 &from=test@test.com
	 &fromname=name
	 &subject=test_subject
	 &html=test_html
	 &to=ben@ifaxin.com*/
	oss << m_mail_api_path << "?api_user=" << m_mail_api_user << "&api_key=" << m_mail_api_key << "&from=" << m_mail_from << "&fromname="
			<< m_mail_from_name << "&subject=" << m_mail_subject << "&html=" << aczHtmlBuffer << "&to=" << strToUin;
	/*   if(!oHttp.Execute("sandboxapp.cloopen.com",8883, "", lce::cgi::CHTTPConnector::METHOD_POST, 5000))
	 {
	 strErrMsg = "get error. msg=";
	 strErrMsg += oHttp.GetErrMsg();
	 return -1;
	 }*/
	if (!oHttp.Execute(m_mail_api_ip, m_mail_api_port, oss.str(), lce::cgi::CHTTPConnector::METHOD_GET, 5000))
	{
		strErrMsg = "get error. msg=";
		strErrMsg += oHttp.GetErrMsg();
		return -1;
	}
	if (200 != oHttp.GetStatusCode())
	{
		strErrMsg = "http status error status=" + int_2_str(oHttp.GetStatusCode());
		return -1;
	}

	string strRespBody = oHttp.GetBody();
	LOG4CPLUS_DEBUG(logger, "SendRegisterMail, strRespBody="<<strRespBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRespBody, oJson, false))
	{
		strErrMsg = "parse json error, json=" + strRespBody;
		return -1;
	}

	if (!oJson.hasKey("message"))
	{
		strErrMsg = "msg api return invalid, json=" + strRespBody;
		return -1;
	}

	string strRetMsg = oJson["message"].asString();
	if ("success" != strRetMsg)
	{
		strErrMsg = "api return :" + strRespBody;
		return -1;
	}

	return 0;
}

string HooshoCgiCard::GenerateTransID(int iIDType)
{
	string strPrefix = "";
	switch (iIDType)
	{

	case ID_TYPE_PICTURE:
		strPrefix = "pic";
		break;
	case ID_TYPE_ENTITY_SHOP:
		strPrefix = "entityshop";
		break;

	default:
		break;
	}

	if ("" == strPrefix)
	{
		return "";
	}

	time_t now = time(0);
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* t = ::localtime(&now);
	static char aczIDBuffer[256];
	bzero(aczIDBuffer, 256);
	snprintf(aczIDBuffer, 256, "%s%04d%02d%02d%02d%02d%02d%06d", strPrefix.c_str(), t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour,
			t->tm_min, t->tm_sec, (int) tv.tv_usec);

	string strID = aczIDBuffer;
	LOG4CPLUS_DEBUG(logger, "GenerateTransID: id_type="<<iIDType<<", id="<<strID);

	return strID;
}
bool HooshoCgiCard::CaptchaAdd(const string& key, const string& data, const size_t& data_size, string& strErrMsg)
{

	if (key.empty() || data.empty())
	{
		strErrMsg = "key=" + key + ", data=" + data + ", empty!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//set
	rc = memcached_set(memc, key.c_str(), key.size(), data.c_str(), data_size, 1800, 0);
	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_set failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	memcached_free(memc);
	return true;
}

int HooshoCgiCard::CaptchaCheck(const string& key, const string& data, string& strErrMsg)
{
	if (key.empty() || data.empty())
	{
		strErrMsg = "key=" + key + ", data=" + data + ", empty!!";
		return -1;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return -1;
	}
	memcached_server_list_free(server);

	//get
	uint32_t dwFlags = 0;
	size_t szValueLength;
	char* cpValue = memcached_get(memc, key.c_str(), key.size(), &szValueLength, &dwFlags, &rc);
	if (MEMCACHED_NOTFOUND == rc)
	{
		strErrMsg = "memcached_get, vc not found for key=" + key;
		memcached_free(memc);
		return -1;
	}

	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_get failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return -1;
	}

	//check
	string strResult(cpValue, szValueLength);
	if (strResult != data)
	{
		memcached_free(memc);
		LOG4CPLUS_DEBUG(logger, "memcached_get + check complete! key != data, key= "<< key << " data = "<<data);
		return 0;
	}

	memcached_free(memc);
	return 1;
}

int HooshoCgiCard::CaptchaExist(const string& key, string& strErrMsg)
{
	if (key.empty())
	{
		strErrMsg = "empty key!!";
		return -1;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return -1;
	}
	memcached_server_list_free(server);

	// check exist
	uint32_t dwFlags = 0;
	size_t szValueLength;
	char* cpValue = memcached_get(memc, key.c_str(), key.size(), &szValueLength, &dwFlags, &rc);

	if (rc != MEMCACHED_SUCCESS)
	{
		memcached_free(memc);

		return 0;
	}

	//check
	string strResult(cpValue, szValueLength);

	memcached_free(memc);

	return 1;
}

bool HooshoCgiCard::MemcachedDelete(const string& key, string& strErrMsg)
{
	if (key.empty())
	{
		strErrMsg = "empty key!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//delete
	memcached_delete(memc, key.c_str(), key.size(), 0);

	memcached_free(memc);
	return true;
}
uint32_t HooshoCgiCard::GetRandNum(uint32_t dwInitSeed)
{
	if (0 == dwInitSeed)
	{
		timeval tv;
		gettimeofday(&tv, NULL);
		srand(tv.tv_usec ^ 77);
	}
	else
	{
		srand(dwInitSeed ^ 77);
	}

	return rand();
}

uint32_t HooshoCgiCard::GetRandNum(uint32_t dwFrom, uint32_t dwTo)
{
	if (dwFrom > dwTo)
	{
		uint32_t dwTmp = dwFrom;
		dwFrom = dwTo;
		dwTo = dwTmp;
	}

	uint32_t r = GetRandNum(0);
	return dwFrom + (r % (dwTo - dwFrom + 1));
}
string HooshoCgiCard::GetRandNumLetter(char* key, int len)
{
	char symbols[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz1234567890";
	srand((unsigned) time(0));
	for (int i = 0; i < len; ++i)
	{
		key[i] = symbols[rand() % (sizeof(symbols) - 1)];
	}
	key[len - 1] = '\0';
	string ret = (string) key;
	return ret;
}
bool HooshoCgiCard::ResetPasswordVCAdd(const string& prefixUin, const string& strVC, string& strErrMsg)
{
	if (prefixUin.empty() || strVC.empty())
	{
		strErrMsg = "empty prefixUin or key!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	rc = memcached_set(memc, prefixUin.c_str(), prefixUin.size(), strVC.c_str(), strVC.size(), 43200, 0);
	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_set failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	memcached_free(memc);
	return true;
}

