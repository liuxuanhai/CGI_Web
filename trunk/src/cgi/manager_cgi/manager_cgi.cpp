#include "manager_cgi.h"
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
#include "comm_def.h"
#include "comm_struct.h"
#include "libmemcached/memcached.h"


IMPL_LOGGER(ManagerCgi, logger);

bool ManagerCgi::DerivedInit()
{
	//db

	GetConfig().GetValue("db", "db_ip", m_db_ip, "");
	GetConfig().GetValue("db", "db_user", m_db_user, "");
	GetConfig().GetValue("db", "db_passwd", m_db_pass, "");

	GetConfig().GetValue("db", "db_name_user", m_db_name_user, "");
	GetConfig().GetValue("db", "table_name_manager", m_table_name_manager, "unknown_table_name");

	GetConfig().GetValue("db", "db_name_feeds", m_db_name_feeds, "");
	GetConfig().GetValue("db", "table_name_user_forbid", m_table_name_user_forbid, "");
	GetConfig().GetValue("db", "table_name_feed_report", m_table_name_feed_report, "");
	GetConfig().GetValue("db", "table_name_feed_info", m_table_name_feed_info, "");
	GetConfig().GetValue("db", "table_name_feed_index_on_appid", m_table_name_feed_index_on_appid, "");

	GetConfig().GetValue("db", "db_name_pa", m_db_name_pa, "");
	GetConfig().GetValue("db", "table_name_pa_info", m_table_name_pa_info, "unknown_table_name");
	
	if(!TableBase::g_user_mysql.Init(m_db_ip, m_db_name_user, m_db_user, m_db_pass))
	{
		LOG4CPLUS_ERROR(logger, "mysql init failed!!!, db_ip:" << m_db_ip
				<< ", db_name: " << m_db_name_user
				<< ", db_user: " << m_db_user
				<< ", db_passwd: " << m_db_pass);

		return false;
	}
	if(!TableBase::g_feeds_mysql.Init(m_db_ip, m_db_name_feeds, m_db_user, m_db_pass))
	{
		LOG4CPLUS_ERROR(logger, "mysql init failed!!!, db_ip:" << m_db_ip
				<< ", db_name: " << m_db_name_feeds
				<< ", db_user: " << m_db_user
				<< ", db_passwd: " << m_db_pass);

		return false;
	}
	if(!TableBase::g_pa_mysql.Init(m_db_ip, m_db_name_pa, m_db_user, m_db_pass))
	{
		LOG4CPLUS_ERROR(logger, "mysql init failed!!!, db_ip:" << m_db_ip
								<< ", db_name: " << m_db_name_pa
								<< ", db_user: " << m_db_user
								<< ", db_passwd: " << m_db_pass);

		return false;
	}

	//server
	GetConfig().GetValue("feeds_server", "ip", m_feeds_server_ip, "");
	GetConfig().GetValue("feeds_server", "port", m_feeds_server_port, 0);

	GetConfig().GetValue("user_server", "ip", m_user_server_ip, "");
	GetConfig().GetValue("user_server", "port", m_user_server_port, 0);

	GetConfig().GetValue("statistic_server", "ip", m_statistic_server_ip, "");
	GetConfig().GetValue("statistic_server", "port", m_statistic_server_port, 0);

	GetConfig().GetValue("feeds", "feeds_limit_length", m_feeds_limit_length, 0);
	GetConfig().GetValue("feeds", "feeds_limit_num", m_feeds_limit_num, 50);

	//cookie cfg
	GetConfig().GetValue("cookie", "name_uin", m_cookie_name_uin, "hp_manager_uin");
	GetConfig().GetValue("cookie", "name_key", m_cookie_name_key, "hp_manager_key");
	GetConfig().GetValue("cookie", "path", m_cookie_path, "/");
	GetConfig().GetValue("cookie", "domain", m_cookie_domain, "120.25.124.72");

	//memcache
	GetConfig().GetValue("memcached", "cache_ip", m_cache_ip, "127.0.0.1");
	GetConfig().GetValue("memcached", "cache_port_session", m_cache_port_session, 39901);
	GetConfig().GetValue("memcached", "cache_port_vc", m_cache_port_vc, 39902);

	//session life
	GetConfig().GetValue("session", "life", m_session_life, 7200);

	return true;
}

bool ManagerCgi::Process()
{
	LOG4CPLUS_DEBUG(logger, "begin cgi ----------------- "<<GetCgiName()<<"----------------- ");

	LOG4CPLUS_DEBUG(logger, "query_string="<<(GetInput().GetQueryString()));
	LOG4CPLUS_DEBUG(logger, "post_data="<<(GetInput().GetPostData()));

	std::string strErrMsg = "";

	if(CGI_NEED_LOGIN_NO == m_check_login)
	{
		LOG4CPLUS_DEBUG(logger, "no need check login");
	}
	else if(CGI_NEED_LOGIN_YES == m_check_login)
	{
		string strCookieKey ;
		string strCookieUin = ((string)GetInput().GetCookie(m_cookie_name_uin));
		if(strCookieUin.length() <= 4)
		{
			LOG4CPLUS_ERROR(logger, "no invalid cookie found, strCookieUin="<<strCookieUin);
			DoReply(CGI_RET_CODE_NO_LOGIN);
			return true;
		}

		strCookieUin = strCookieUin.substr(4); // prefix="0_0_"
		strCookieKey = (string) GetInput().GetCookie(m_cookie_name_key);
		if(!SessionCheck(strCookieUin, strCookieKey, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "CheckCookie failed, errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_NO_LOGIN);
			return true;
		}

		ManagerInfo stManagerInfo(m_table_name_manager);
		stManagerInfo.m_uin = strCookieUin;
		if(stManagerInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_FATAL(logger, "init manager info failed, uin = " << strCookieUin
                                << ", errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		m_manager_info = stManagerInfo;
		LOG4CPLUS_DEBUG(logger, "one req, manager = "<< stManagerInfo.ToString());
	}

	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();

	LOG4CPLUS_DEBUG(logger, "end cgi ----------------- "<<GetCgiName()<<"-----------------\n\n");

	return true;
}

int ManagerCgi::MakeCookie(const string& strUin, string& strCookieKeyValue, string& strErrMsg)
{
	if(strUin.empty())
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
	uint64_t qwMd5Sum =  md.hash64(strTargetString.c_str(), strTargetString.size());
	uint16_t dwHigh16 = (uint16_t)(qwMd5Sum >> 48);
	uint32_t dwLow16 = (uint16_t)(qwMd5Sum & 0xFFFF);
	uint32_t dwFinal = ((dwHigh16<<16) | dwLow16);

	//strCookieUinValue = "0_0_" + strUin;

	strCookieKeyValue = int_2_str(dwFinal);

	return 0;
}

int ManagerCgi::MakeAddReplyCookie(const string& strUin,lce::cgi::CHttpHeader& stHttpRspHeader, string& strErrMsg)
{
	if(strUin.empty())
	{
		strErrMsg = "uin is empty";
		return -1;
	}

	string strCookieUinValue = "0_0_" + strUin;
	string strCookieKeyValue = "";
	if(MakeCookie(strUin, strCookieKeyValue, strErrMsg) < 0)
	{
	     DoReply(CGI_RET_CODE_SERVER_BUSY);
		 LOG4CPLUS_ERROR(logger, "make cookie failed, errmsg="<<strUin);
		 return -1;
	}

	//set-cookie
	stHttpRspHeader.SetCookie(m_cookie_name_uin, strCookieUinValue, m_cookie_domain, m_session_life, m_cookie_path, false, false);
	stHttpRspHeader.SetCookie(m_cookie_name_key, strCookieKeyValue, m_cookie_domain, m_session_life, m_cookie_path, false, false);

	//add seesion to memcache
	if(!SessionAdd(strUin, strCookieKeyValue,strErrMsg))
	{
		 DoReply(CGI_RET_CODE_SERVER_BUSY);
		 LOG4CPLUS_ERROR(logger, "session add failed, errmsg="<<strErrMsg);
		 return -1;
	}

	return 0;
}
bool ManagerCgi::SessionAdd(const string& strUin, const string& strKey, string& strErrMsg)
{
	if(strUin.empty() || strKey.empty())
	{
		strErrMsg = "empty uin or key!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_session, &rc);
    rc = memcached_server_push(memc,server);
    if(MEMCACHED_SUCCESS != rc)
    {
         strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		 memcached_server_list_free(server);
		 return false;
    }
	memcached_server_list_free(server);

	//set
	rc = memcached_set(memc, strUin.c_str(), strUin.size(), strKey.c_str(), strKey.size(), m_session_life, 0);
    if(rc != MEMCACHED_SUCCESS)
    {
  		strErrMsg = "memcached_set failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
    }

	memcached_free(memc);
	return true;
}

bool ManagerCgi::SessionCheck(const string& strUin, const string& strKey, string& strErrMsg)
{
	if(strUin.empty())
	{
		strErrMsg = "empty uin!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_session, &rc);
    rc = memcached_server_push(memc,server);
    if(MEMCACHED_SUCCESS != rc)
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
	if(MEMCACHED_NOTFOUND == rc)
    {
        strErrMsg = "memcached_get, session not found for key=" + strUin;
	    memcached_free(memc);
	    return false;
    }

    if(rc != MEMCACHED_SUCCESS)
    {
  		strErrMsg = "memcached_get failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
    }

	//check
	string strResult(cpValue, szValueLength);
	if(strResult != strKey)
	{
		strErrMsg = "not equal, req_key=" + strKey + ", cache_key=" + strResult;
		memcached_free(memc);
		return false;
	}

	memcached_free(memc);
	return true;
}

bool ManagerCgi::VerifyCodeAdd(const string& strUin, const string& strVC, string& strErrMsg)
{
	if(strUin.empty() || strVC.empty())
	{
		strErrMsg = "empty uin or key!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
    rc = memcached_server_push(memc, server);
    if(MEMCACHED_SUCCESS != rc)
    {
         strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		 memcached_server_list_free(server);
		 return false;
    }
	memcached_server_list_free(server);

	//set
	rc = memcached_set(memc, strUin.c_str(), strUin.size(), strVC.c_str(), strVC.size(), 600, 0);
    if(rc != MEMCACHED_SUCCESS)
    {
  		strErrMsg = "memcached_set failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
    }

	memcached_free(memc);
	return true;
}

bool ManagerCgi::VerifyCodeCheck(const string& strUin, const string& strVC, string& strErrMsg)
{
	if(strUin.empty() || strVC.empty())
	{
		strErrMsg = "uin="+strUin + ", vc="+strVC + ", empty!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
    rc = memcached_server_push(memc,server);
    if(MEMCACHED_SUCCESS != rc)
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
	if(MEMCACHED_NOTFOUND == rc)
    {
        strErrMsg = "memcached_get, vc not found for key=" + strUin;
	    memcached_free(memc);
	    return false;
    }

    if(rc != MEMCACHED_SUCCESS)
    {
  		strErrMsg = "memcached_get failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
    }

	//check
	string strResult(cpValue, szValueLength);
	if(strResult != strVC)
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

bool ManagerCgi::VerifyCodeQuery(const string& strUin, string& strVC, string& strErrMsg)
{
    if(strUin.empty())
	{
		strErrMsg = "empty uin!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_cache_ip.c_str(), m_cache_port_vc, &rc);
    rc = memcached_server_push(memc,server);
    if(MEMCACHED_SUCCESS != rc)
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
	if(MEMCACHED_NOTFOUND == rc)
    {
        strVC = "";
	    memcached_free(memc);
	    return true;
    }

    if(rc != MEMCACHED_SUCCESS)
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

uint32_t ManagerCgi::GetRandNum(uint32_t dwInitSeed)
{
	if(0 == dwInitSeed)
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

uint32_t ManagerCgi::GetRandNum(uint32_t dwFrom, uint32_t dwTo)
{
    if(dwFrom > dwTo)
    {
        uint32_t dwTmp = dwFrom;
        dwFrom = dwTo;
        dwTo = dwTmp;
    }
    
    uint32_t r = GetRandNum(0);
    return dwFrom + (r % (dwTo - dwFrom + 1));
}
string ManagerCgi::GetRandNumLetter(char* key, int len)
{
    char symbols[]   = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz1234567890";
    srand((unsigned)time(0));
    for (int i = 0; i < len; ++i)
    {
    	key[i] = symbols[rand() % (sizeof(symbols) -1)];
    }
    key[len-1] = '\0';
    string ret = (string)key;
    return ret;
}


