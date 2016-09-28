#include "cgi_manager_base.h"
#include "libmemcached/memcached.h"
#include "jsoncpp/json.h"
#include "wx_api_message.h"

IMPL_LOGGER(CgiManagerBase, logger);

inline std::string time_2_format_str(long qwTs)
{
	long dwTs = qwTs + 60 * 60 * 8;
	struct tm *p;
    p = gmtime(&dwTs);
    char bufTs[80];
    strftime(bufTs, 80, "%Y年%m月%d日 %H:%M", p);
	return string(bufTs);
}

bool CgiManagerBase::DerivedInit()
{
	GetConfig().GetValue("manager", "size", m_manager_phone_size, 0);
	m_manager_phone_list.clear();
	m_manager_passwd_list.clear();
	std::string strPhone;
	md5 m_md5;
	for(uint32_t i = 0; i < m_manager_phone_size; i++)
	{
			std::string strPhone = "phone_" + int_2_str(i);
			GetConfig().GetValue("manager", strPhone.c_str(), strPhone, "");
			LOG4CPLUS_INFO(logger, "phone_" << i << ": " << strPhone);
			
			std::string strPasswd = "passwd_" + int_2_str(i);
			GetConfig().GetValue("manager", strPasswd.c_str(), strPasswd, "");
			LOG4CPLUS_INFO(logger, "passwd_" << i << ": " << strPasswd);
			strPasswd = m_md5.hash(strPasswd);
			LOG4CPLUS_INFO(logger, "passwd_" << i << ": " << strPasswd);

			m_manager_phone_list.push_back(strPhone);
			m_manager_passwd_list.push_back(strPasswd);
	}

	GetConfig().GetValue("cookie", "name_uin", m_cookie_name_uin, "");
	GetConfig().GetValue("cookie", "name_key", m_cookie_name_key, "");
	GetConfig().GetValue("cookie", "domain", m_cookie_domain, "");
	GetConfig().GetValue("cookie", "path", m_cookie_path, "");

	GetConfig().GetValue("memcached", "cache_ip", m_memcached_cache_ip, "");
	GetConfig().GetValue("memcached", "cache_port_session", m_memcached_cache_port_session, 0);
	GetConfig().GetValue("memcached", "cache_port_vc", m_memcached_cache_port_vc, 0);

	GetConfig().GetValue("session", "life", m_session_life, 0);

	GetConfig().GetValue("DB", "db_ip", m_db_ip, "");
	GetConfig().GetValue("DB", "db_user", m_db_user, "");
	GetConfig().GetValue("DB", "db_passwd", m_db_passwd, "");
	GetConfig().GetValue("DB", "db_name", m_db_name, "");
	GetConfig().GetValue("DB", "table_name_user_info", m_table_name_user_info, "");
	GetConfig().GetValue("DB", "table_name_user_identity_info", m_table_name_user_identity_info, "");
	GetConfig().GetValue("DB", "table_name_good_info", m_table_name_good_info, "");
	GetConfig().GetValue("DB", "table_name_good_type", m_table_name_good_type, "");
	GetConfig().GetValue("DB", "table_name_consume_type", m_table_name_consume_type, "");
	GetConfig().GetValue("DB", "table_name_feed_info", m_table_name_feed_info, "");
	GetConfig().GetValue("DB", "table_name_ticket_info", m_table_name_ticket_info, "");
	GetConfig().GetValue("DB", "table_name_pic_info", m_table_name_pic_info, "");

	assert(m_mysql_helper.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd));

	GetConfig().GetValue("sms_api", "sms_api_host", m_sms_api_host, "");
	GetConfig().GetValue("sms_api", "sms_api_ip", m_sms_api_ip, "");
	GetConfig().GetValue("sms_api", "sms_api_port", m_sms_api_port, 0);
	GetConfig().GetValue("sms_api", "sms_api_account", m_sms_api_account, "");
	GetConfig().GetValue("sms_api", "sms_api_accountpwd", m_sms_api_accountpwd, "");
	GetConfig().GetValue("sms_api", "sms_api_appid", m_sms_api_appid, "");
	GetConfig().GetValue("sms_api", "sms_api_version", m_sms_api_version, "");
	GetConfig().GetValue("sms_api", "sms_templateid", m_sms_templateid, "");
	GetConfig().GetValue("sms_api", "sms_subject", m_sms_subject, "");
	GetConfig().GetValue("sms_api", "sms_html", m_sms_html, "");

	GetConfig().GetValue("s_server", "ip", m_s_server_ip, "");
	GetConfig().GetValue("s_server", "port", m_s_server_port, 0);

	GetConfig().GetValue("PA", "appid", m_pa_appid, "");

	GetConfig().GetValue("TEMPLATE_INVOICE_PASS", "template_id", m_invoice_pass_template_id, "");
	GetConfig().GetValue("TEMPLATE_INVOICE_PASS", "color", m_invoice_pass_color, "");
    GetConfig().GetValue("TEMPLATE_INVOICE_PASS", "url", m_invoice_pass_url, "");
    GetConfig().GetValue("TEMPLATE_INVOICE_PASS", "first", m_invoice_pass_first, "");
    GetConfig().GetValue("TEMPLATE_INVOICE_PASS", "keyword1", m_invoice_pass_keyword1, "");
	GetConfig().GetValue("TEMPLATE_INVOICE_PASS", "keyword2", m_invoice_pass_keyword2, "");
    GetConfig().GetValue("TEMPLATE_INVOICE_PASS", "keyword3", m_invoice_pass_keyword3, "");
    GetConfig().GetValue("TEMPLATE_INVOICE_PASS", "remark", m_invoice_pass_remark, "");

	return true;
}

bool CgiManagerBase::Process()
{
	LOG4CPLUS_INFO(logger, "BEGIN CGI ----------------- "<<GetCgiName()<<"----------------- ");

	if(strcmp(GetInput().GetQueryString(), ""))
	{
		LOG4CPLUS_INFO(logger, "query_string: " << GetInput().GetQueryString());
	}
	if(strcmp(GetInput().GetPostData(), ""))
	{
		LOG4CPLUS_INFO(logger, "post_data: " << GetInput().GetPostData());
	}

	if(m_check_login)
	{
		std::string strCookieUin = ((string)GetInput().GetCookie(m_cookie_name_uin));
		std::string strCookieKey = (string) GetInput().GetCookie(m_cookie_name_key);
		std::string strErrMsg;

		if(strCookieUin.length() <= 4)
		{
			LOG4CPLUS_ERROR(logger, "no invalid cookie found, strCookieUin="<<strCookieUin);
			DoReply(CGI_RET_CODE_NO_LOGIN);
			return true;
		}

		strCookieUin = strCookieUin.substr(4); // prefix="0_0_"
		if(!SessionCheck(strCookieUin, strCookieKey, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "CheckCookie failed, errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_NO_LOGIN);
			return true;
		}

		//add another session life
		if(!SessionAdd(strCookieUin, strCookieKey, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "SessionAdd failed, errmsg="<<strErrMsg);
		}

		m_login_phone = strCookieUin;
		LOG4CPLUS_INFO(logger, "current phone=" << m_login_phone); 
		GetAnyValue()["phone"] = m_login_phone;
	}
	else
	{
		LOG4CPLUS_DEBUG(logger, "not need check login");
	}

	//logic
	InnerProcess();

	LOG4CPLUS_INFO(logger, "END CGI \n");
	return true;
}

bool CgiManagerBase::IsValidPhone(const std::string& strPhone)
{
	for(size_t i = 0; i < m_manager_phone_list.size(); i++)
	{
		if(m_manager_phone_list[i] == strPhone)
		{
			return true;
		}
	}
	return false;
}

bool CgiManagerBase::CheckPasswd(const std::string& strPhone, const std::string strPasswd, std::string& strErrMsg)
{
	for(size_t i = 0; i < m_manager_phone_list.size(); i++)
	{
		if(m_manager_phone_list[i] == strPhone)
		{
			if(m_manager_passwd_list[i] == strPasswd)
			{
				return true;
			}
			else
			{
				strErrMsg = "phone=" + strPhone + ", passwd=" + m_manager_passwd_list[i] + ", req_passwd=" + strPasswd + ", not equal";
				return false;
			}
		}
	}
	strErrMsg = "phone=" + strPhone + " not found";
	return false;
}

bool CgiManagerBase::VerifyCodeAdd(const string& strPhone, const string& strVC, string& strErrMsg)
{
	if (strPhone.empty() || strVC.empty())
	{
		strErrMsg = "phone=" + strPhone + ", vc=" + strVC + ", empty!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_memcached_cache_ip.c_str(), m_memcached_cache_port_vc, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//set
	rc = memcached_set(memc, strPhone.c_str(), strPhone.size(), strVC.c_str(), strVC.size(), 600, 0);
	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_set failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	memcached_free(memc);
	return true;
}

bool CgiManagerBase::VerifyCodeQuery(const string& strPhone, string& strVC, string& strErrMsg)
{
	if (strPhone.empty())
	{
		strErrMsg = "empty uin!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_memcached_cache_ip.c_str(), m_memcached_cache_port_vc, &rc);
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
	char* cpValue = memcached_get(memc, strPhone.c_str(), strPhone.size(), &szValueLength, &dwFlags, &rc);
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

bool CgiManagerBase::VerifyCodeCheck(const string& strPhone, const string& strVC, string& strErrMsg)
{
	if (strPhone.empty() || strVC.empty())
	{
		strErrMsg = "phone=" + strPhone + ", vc=" + strVC + ", empty!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_memcached_cache_ip.c_str(), m_memcached_cache_port_vc, &rc);
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
	char* cpValue = memcached_get(memc, strPhone.c_str(), strPhone.size(), &szValueLength, &dwFlags, &rc);
	if (MEMCACHED_NOTFOUND == rc)
	{
		strErrMsg = "memcached_get, vc not found for key=" + strPhone;
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
	memcached_delete(memc, strPhone.c_str(), strPhone.size(), 0);
	memcached_free(memc);
	return true;
}

bool CgiManagerBase::SessionAdd(const string& strPhone, const string& strKey, string& strErrMsg)
{
	if (strPhone.empty() || strKey.empty())
	{
		strErrMsg = "uin=" + strPhone + ", key=" + strKey + ", empty!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_memcached_cache_ip.c_str(), m_memcached_cache_port_session, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//set
	rc = memcached_set(memc, strPhone.c_str(), strPhone.size(), strKey.c_str(), strKey.size(), m_session_life, 0);
	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_set failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	memcached_free(memc);
	return true;
}

bool CgiManagerBase::SessionCheck(const string& strPhone, const string& strKey, string& strErrMsg)
{
	if (strPhone.empty() || strKey.empty())
	{
		strErrMsg = "uin=" + strPhone + ", key=" + strKey + ", empty!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, m_memcached_cache_ip.c_str(), m_memcached_cache_port_session, &rc);
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
	char* cpValue = memcached_get(memc, strPhone.c_str(), strPhone.size(), &szValueLength, &dwFlags, &rc);
	if (MEMCACHED_NOTFOUND == rc)
	{
		strErrMsg = "memcached_get, session not found for key=" + strPhone;
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

int CgiManagerBase::MakeCookie(const string& strPhone, string& strCookieKeyValue, string& strErrMsg)
{
	if(strPhone.empty())
	{
		strErrMsg = "uin is empty";
		return -1;
	}

	//targetstring = md5(uin + time(0) + rand)
	timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec ^ 77);
	uint32_t r = rand();
	string strTargetString = strPhone + int_2_str(time(0)) + int_2_str(r);
	md5 md;
	uint64_t qwMd5Sum =  md.hash64(strTargetString.c_str(), strTargetString.size());
	uint16_t dwHigh16 = (uint16_t)(qwMd5Sum >> 48);
	uint32_t dwLow16 = (uint16_t)(qwMd5Sum & 0xFFFF);
	uint32_t dwFinal = ((dwHigh16<<16) | dwLow16);

	//strCookieUinValue = "0_0_" + strPhone;

	strCookieKeyValue = int_2_str(dwFinal);

	return 0;
}

int CgiManagerBase::MakeAddReplyCookie(const string& strPhone,lce::cgi::CHttpHeader& stHttpRspHeader, string& strErrMsg)
{
	if(strPhone.empty())
	{
		strErrMsg = "uin is empty";
		return -1;
	}

	string strCookieUinValue = "0_0_" + strPhone;
	string strCookieKeyValue = "";
	if(MakeCookie(strPhone, strCookieKeyValue, strErrMsg) < 0)
	{
	     DoReply(CGI_RET_CODE_SERVER_BUSY);
		 LOG4CPLUS_ERROR(logger, "make cookie failed, errmsg="<<strPhone);
		 return -1;
	}

	//set-cookie
	stHttpRspHeader.SetCookie(m_cookie_name_uin, strCookieUinValue, m_cookie_domain, m_session_life, m_cookie_path, false, false);
	stHttpRspHeader.SetCookie(m_cookie_name_key, strCookieKeyValue, m_cookie_domain, m_session_life, m_cookie_path, false, false);

	//add seesion to memcache
	if(!SessionAdd(strPhone, strCookieKeyValue,strErrMsg))
	{
		 DoReply(CGI_RET_CODE_SERVER_BUSY);
		 LOG4CPLUS_ERROR(logger, "session add failed, errmsg="<<strErrMsg);
		 return -1;
	}

	return 0;
}

int CgiManagerBase::SendTemplateMessageOfInvoicePass(const hoosho::msg::z::OrderInfo &stOrderInfo)
{
		/*
	LOG4CPLUS_DEBUG(logger, "send template message of invoice pass");

	std::string strCheckRet = "";
	if(stOrderInfo.order_status() == ORDER_STATUS_INVOICE_CHECK_SUCC)
	{
		strCheckRet = "通过";
	}
	else if(stOrderInfo.order_status() == ORDER_STATUS_INVOICE_CHECK_FAIL)
	{
		LOG4CPLUS_ERROR(logger, "order_status = " << stOrderInfo.order_status() << ", check faile, don't need to notify");
		return -1;
		strCheckRet = "不通过";
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "unknow order_status = " << stOrderInfo.order_status());
		return -1;
	}

    Json::Value any;
    any["touser"] = stOrderInfo.openid_master();
    any["template_id"] = m_invoice_pass_template_id;
    any["url"] = m_invoice_pass_url;
    any["data"]["first"]["value"] = m_invoice_pass_first;
    any["data"]["first"]["color"] = m_invoice_pass_color;
    any["data"][m_invoice_pass_keyword1]["value"] = int_2_str(stOrderInfo.order_id());
    any["data"][m_invoice_pass_keyword1]["color"] = m_invoice_pass_color;
    any["data"][m_invoice_pass_keyword2]["value"] = time_2_format_str(stOrderInfo.create_ts());
    any["data"][m_invoice_pass_keyword2]["color"] = m_invoice_pass_color;
    any["data"][m_invoice_pass_keyword3]["value"] = strCheckRet;
    any["data"][m_invoice_pass_keyword3]["color"] = m_invoice_pass_color;
    any["data"]["remark"]["value"] = m_invoice_pass_remark;
    any["data"]["remark"]["color"] = m_invoice_pass_color;

    std::string strPostData = any.toJsonString();
    LOG4CPLUS_DEBUG(logger, "postdata = " << strPostData);

    //wx api
    common::wxapi::WXAPIMessage wxapiMessage;
    std::string strBaseAccessToken;
	std::string strErrMsg;
	if(GetPAAccessToken(m_z_server_ip, m_z_server_port, m_pa_appid, strBaseAccessToken, strErrMsg) < 0)
	{
        LOG4CPLUS_ERROR(logger, "GetPAAccessToken failed, errmsg = " << strErrMsg);
		return -1;
	}

    int iRet = 0;

    if(wxapiMessage.SendTemplateMessage(strBaseAccessToken, strPostData, iRet) < 0)
    {
        LOG4CPLUS_ERROR(logger, "WXAPI.SendTemplateMessage failed, errcode = " << iRet);
        return -1;
    }
	*/
	return 0;
}


