#ifndef _HOOSHO_CGI_USER_ZOMBIE_LOGIN_H_
#define _HOOSHO_CGI_USER_ZOMBIE_LOGIN_H_

#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"
#include <sstream>
#include <stdio.h>


class CgiUserZombieLogin: public lce::cgi::Cgi
{
public:
	 CgiUserZombieLogin(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{		
		GetConfig().GetValue("zombie", "url", zombie_url, "");
		return true;
	}
	
	virtual bool Process()
	{
		/*
		std::string strUin = (std::string)GetInput().GetValue("uin");
		if(strUin.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strUin is empty");
			return true;	
		}

		LOG4CPLUS_DEBUG(logger, "strUin="<<strUin);
		*/

		

		GetAnyValue()["url"] = zombie_url;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	string zombie_url;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUserZombieLogin, logger);

int main(int argc, char** argv)
{
	CgiUserZombieLogin cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

