#include "main_cgi.h"
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


IMPL_LOGGER(MainCgi, logger);

bool MainCgi::DerivedInit()
{
	//db

	GetConfig().GetValue("db", "db_ip", m_db_ip, "");
	GetConfig().GetValue("db", "db_user", m_db_user, "");
	GetConfig().GetValue("db", "db_passwd", m_db_pass, "");
	
	GetConfig().GetValue("db", "db_name_common", m_db_name_common, "");
	GetConfig().GetValue("db", "table_name_pic", m_table_name_pic, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_doc", m_table_name_doc, "unknown_table_name");

	GetConfig().GetValue("db", "db_name_main", m_db_name_main, "");
	GetConfig().GetValue("db", "table_name_password", m_table_name_password, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_user_info", m_table_name_user_info, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_dev_info", m_table_name_dev_info, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_oper_info", m_table_name_oper_info, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_wx_public_account", m_table_name_wx_public_account, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_wx_public_account_feedback", m_table_name_wx_public_account_feedback, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_dev_order", m_table_name_dev_order, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_oper_order", m_table_name_oper_order, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_oper_order_reverse", m_table_name_oper_order_reverse, "unknown_table_name");
	GetConfig().GetValue("db", "table_name_open_order", m_table_name_open_order, "unknown_table_name");

	if(!TableBase::g_common_mysql.Init(m_db_ip, m_db_name_common, m_db_user, m_db_pass))
	{
		LOG4CPLUS_ERROR(logger, "mysql init failed!!!, db_ip:"<<m_db_ip
				<<", db_name: "<<m_db_name_common
				<<", db_user: "<<m_db_user
				<<", db_passwd: "<<m_db_pass);

		return false;
	}

	if(!TableBase::g_main_mysql.Init(m_db_ip, m_db_name_main, m_db_user, m_db_pass))
	{
		LOG4CPLUS_ERROR(logger, "mysql init failed!!!, db_ip:"<<m_db_ip
				<<", db_name: "<<m_db_name_main
				<<", db_user: "<<m_db_user
				<<", db_passwd: "<<m_db_pass);

		return false;
	}
	
	//cookie cfg
	GetConfig().GetValue("cookie", "name_uin", m_cookie_name_uin, "hoosho_uin");
	GetConfig().GetValue("cookie", "name_key", m_cookie_name_key, "hoosho_key");
	GetConfig().GetValue("cookie", "path", m_cookie_path, "/");
	GetConfig().GetValue("cookie", "domain", m_cookie_domain, "120.25.124.72");

	//memcache
	GetConfig().GetValue("memcached", "cache_ip", m_cache_ip, "127.0.0.1");
	GetConfig().GetValue("memcached", "cache_port_session", m_cache_port_session, 39001);
	GetConfig().GetValue("memcached", "cache_port_vc", m_cache_port_vc, 39002);

	//session life
	GetConfig().GetValue("session", "life", m_session_life, 7200);

	//mail api
	GetConfig().GetValue("mail_api", "api_host", m_mail_api_host, "sendcloud.sohu.com");
	GetConfig().GetValue("mail_api", "api_ip", m_mail_api_ip, "123.125.122.12");
	GetConfig().GetValue("mail_api", "api_port", m_mail_api_port, 80);
	GetConfig().GetValue("mail_api", "api_path", m_mail_api_path, "/webapi/mail.send.json");
	GetConfig().GetValue("mail_api", "api_user", m_mail_api_user, "hoosho_test_nGrwjx");
	GetConfig().GetValue("mail_api", "api_key", m_mail_api_key, "5ht1oG5W0pqqoAbM");
	GetConfig().GetValue("mail_api", "mail_from", m_mail_from, "manager@hoosho.com");
	GetConfig().GetValue("mail_api", "mail_fromname", m_mail_from_name, "??Ц??");
	GetConfig().GetValue("mail_api", "mail_subject", m_mail_subject, "????Ц??????????");
	GetConfig().GetValue("mail_api", "mail_html", m_mail_html, "");
	
	return true;
}

bool MainCgi::Process()
{
	LOG4CPLUS_DEBUG(logger, "\n\nbegin cgi ----------------- "<<GetCgiName()<<"----------------- ");

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
		
		UserInfo stUserInfo(m_table_name_user_info);
		stUserInfo.m_uin = strCookieUin;
		if(stUserInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_FATAL(logger, "init userinfo failed, uin="<<strCookieUin<<", errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		
		m_user_info = stUserInfo;
		LOG4CPLUS_DEBUG(logger, "one req, userinfo="<<m_user_info.ToString());
	}

	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();
	
	LOG4CPLUS_DEBUG(logger, "end cgi ----------------- "<<GetCgiName()<<"----------------- ");

	return true;
}

int MainCgi::MakeCookie(const string& strUin, string& strCookieKeyValue, string& strErrMsg)
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

int MainCgi::MakeAddReplyCookie(const string& strUin,lce::cgi::CHttpHeader& stHttpRspHeader, string& strErrMsg)
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
bool MainCgi::SessionAdd(const string& strUin, const string& strKey, string& strErrMsg)
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

bool MainCgi::SessionCheck(const string& strUin, const string& strKey, string& strErrMsg)
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

bool MainCgi::VerifyCodeAdd(const string& strUin, const string& strVC, string& strErrMsg)
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

bool MainCgi::VerifyCodeCheck(const string& strUin, const string& strVC, string& strErrMsg)
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

bool MainCgi::VerifyCodeQuery(const string& strUin, string& strVC, string& strErrMsg)
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

int MainCgi::SendRegisterMail(const string& strToUin, const string& strVC, string& strErrMsg)
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
    oss<<m_mail_api_path
    	<<"?api_user="<<m_mail_api_user
    	<<"&api_key="<<m_mail_api_key
    	<<"&from="<<m_mail_from
    	<<"&fromname="<<m_mail_from_name
    	<<"&subject="<<m_mail_subject
    	<<"&html="<<aczHtmlBuffer
        <<"&to="<<strToUin;
 /*   if(!oHttp.Execute("sandboxapp.cloopen.com",8883, "", lce::cgi::CHTTPConnector::METHOD_POST, 5000))
    {
        strErrMsg = "get error. msg=";
        strErrMsg += oHttp.GetErrMsg();
        return -1;
    }*/

    char hostTempBuf[8*1024];
	struct hostent stHostent;
	struct hostent *pHostent = NULL;
	int iHErrNo = 0;
	int iRet = gethostbyname_r(m_mail_api_host.c_str(), &stHostent, hostTempBuf, sizeof(hostTempBuf), &pHostent, &iHErrNo);
	if(iRet != 0)
	{
		strErrMsg = "DNSParse failed, gethostbyname_r error, ";
		strErrMsg += strerror(iHErrNo);
		return -1;
	}

	if(!pHostent)
	{
		strErrMsg = "DNSParse failed, gethostbyname_r return 0, but pHostent is NULL, maybe invalid host=" + m_mail_api_host;
		return -1;
	}

	struct in_addr* pInAddr = (struct in_addr *) pHostent->h_addr;
	if(!pInAddr)
	{
		strErrMsg = "DNSParse failed, gethostbyname_r return 0, but pInAddr is NULL, maybe invalid host=" + m_mail_api_host;
        return -1;
	}

	string api_host_ip = inet_ntoa (*pInAddr);	
    
    if(!oHttp.Execute(api_host_ip, m_mail_api_port, oss.str(), lce::cgi::CHTTPConnector::METHOD_GET, 5000))
    {
        strErrMsg = "get error. msg=";
        strErrMsg += oHttp.GetErrMsg();
        return -1;
    }
    if(200 != oHttp.GetStatusCode())
    {
        strErrMsg = "http status error status=" + int_2_str(oHttp.GetStatusCode());
        return -1;
    }

    string strRespBody = oHttp.GetBody();
    LOG4CPLUS_DEBUG(logger, "SendRegisterMail, strRespBody="<<strRespBody);
	
	Json::Value oJson;
    Json::Reader reader;
    if(!reader.parse(strRespBody, oJson, false))
    {
        strErrMsg = "parse json error, json=" + strRespBody;
        return -1;
    }

	if(!oJson.hasKey("message"))
    {
        strErrMsg = "msg api return invalid, json=" + strRespBody;
        return -1;
    }

	string strRetMsg = oJson["message"].asString();
    if("success" != strRetMsg)
    {
        strErrMsg = "api return :" + strRespBody;
        return -1;
    }

	return 0;
}

string MainCgi::GenerateTransID(int iIDType)
{
    string strPrefix = "";
    switch(iIDType)
    {
        case ID_TYPE_PIC: strPrefix="pic"; break;
        case ID_TYPE_DOC: strPrefix="doc"; break;
        case ID_TYPE_USER: strPrefix="user"; break;
        case ID_TYPE_DEV: strPrefix="dev"; break;
        case ID_TYPE_OPER: strPrefix="oper"; break;
        case ID_TYPE_WX_PUBLIC_ACCNOUT_FEEDBACK: strPrefix="wpafb"; break;
        case ID_TYPE_ORDER_DEV: strPrefix="orderdev"; break;
        case ID_TYPE_ORDER_OPER: strPrefix="orderoper"; break;
        case ID_TYPE_ORDER_OPEN: strPrefix="orderopen"; break;
        default: break;
    }

    if("" == strPrefix)
    {
        return "";
    }

    time_t now = time(0);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm* t = ::localtime(&now);
    static char aczIDBuffer[256];
    bzero(aczIDBuffer, 256);
    snprintf(aczIDBuffer, 256, "%s%04d%02d%02d%02d%02d%02d%06d"
                        , strPrefix.c_str()
                        , t->tm_year+1900, t->tm_mon+1, t->tm_mday
                        , t->tm_hour, t->tm_min, t->tm_sec
                        , (int)tv.tv_usec);

    string strID = aczIDBuffer;
    LOG4CPLUS_DEBUG(logger, "GenerateTransID: id_type="<<iIDType<<", id="<<strID);
    
    return strID;
}

uint32_t MainCgi::GetRandNum(uint32_t dwInitSeed)
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

uint32_t MainCgi::GetRandNum(uint32_t dwFrom, uint32_t dwTo)
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
string MainCgi::GetRandNumLetter(char* key, int len)
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



