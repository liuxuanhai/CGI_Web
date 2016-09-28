#ifndef _HOOSHO_CGI_J_USER_INFO_GET_H_
#define _HOOSHO_CGI_J_USER_INFO_GET_H_

#include "cgi/cgi.h"
#include "util/lce_util.h"
#include "../cgi_common_util.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>

class CgiUserInfoGet: public lce::cgi::Cgi
{
public:
	 CgiUserInfoGet(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("user_server", "ip", m_user_server_ip, "");
		GetConfig().GetValue("user_server", "port", m_user_server_port, 0);
		GetConfig().GetValue("cookie", "name_key", m_cookie_name_key, "");
		GetConfig().GetValue("cookie", "name_value", m_cookie_name_value, "");
		
		return true;
	}
	
	virtual bool Process()
	{
		//1.check login first
		std::string strErrMsg = "";
		if(!CheckLogin(GetInput(), m_cookie_name_key, m_cookie_name_value, m_user_server_ip, m_user_server_port, strErrMsg))
		{
			DoReply(CGI_RET_CODE_NO_LOGIN);
			LOG4CPLUS_ERROR(logger, "CheckLogin failed, strErrMsg="<<strErrMsg);
			return true;
		}

		//get all openid
		std::string strOpenidSelf =  (std::string)GetInput().GetCookie(m_cookie_name_key); //cookie. self
		std::string strOpenidList = (std::string)GetInput().GetValue("openid_list");
		std::vector<std::string> vecOpenidList;
		lce::util::StringOP::Split(strOpenidList, "|", vecOpenidList);
		vecOpenidList.push_back(strOpenidSelf);

		std::set<std::string> setOpenid;
		for(size_t i=0; i!=vecOpenidList.size(); ++i)
		{
			setOpenid.insert(vecOpenidList[i]);
		}

		//get all user info
		std::map<std::string, lce::cgi::CAnyValue> mapResult;
		if(FetchUserInfo(setOpenid, mapResult, m_user_server_ip, m_user_server_port, strErrMsg)  < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "FentchUserInfo failed, strErrMsg="<<strErrMsg);
			return true;
		}

		for(std::map<std::string, lce::cgi::CAnyValue>::iterator iter = mapResult.begin()
			; iter != mapResult.end()
			; ++iter)
		{
			const std::string& strOpenid = iter->first;
			const lce::cgi::CAnyValue& stUserInfo = iter->second;
			GetAnyValue()[strOpenid] = stUserInfo;
		}

        DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	string m_user_server_ip;
	int m_user_server_port;
	string m_cookie_name_key;
	string m_cookie_name_value;
	
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUserInfoGet, logger);

int main(int argc, char** argv)
{
	CgiUserInfoGet cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

