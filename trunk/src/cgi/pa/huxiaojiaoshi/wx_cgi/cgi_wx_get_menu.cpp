#ifndef _HOOSHO_CGI_WX_GET_MENU_H_
#define _HOOSHO_CGI_WX_GET_MENU_H_

#include "cgi_wx_base.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>
#include "wx_api_menu.h"

class CgiWXGetMenu: public CgiWXBase
{
public:
	 CgiWXGetMenu(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:CgiWXBase(qwFlag, strCgiConfigFile, strLogConfigFile, false)
	 {
	 	
	 }

	virtual bool InnerProcess()
	{
		//std::string strAppid = "wx528b586255278787";
		std::string strAppid = GetInput().GetValue("appid");
		std::string strPAAccessToken = GetInput().GetValue("access_token");
		EMPTY_STR_RETURN(strAppid);
		EMPTY_STR_RETURN(strPAAccessToken);
		std::string strMenuContent;
		int iErrcode = 0;

		int iRet;

		::common::wxapi::WXAPIMenu stWXAPIMenu;
		iRet = stWXAPIMenu.GetMenu(strPAAccessToken, strMenuContent, iErrcode);
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "WXAPI GetMenu failed");
			if(iErrcode != stWXAPIMenu.WX_RETURN_ERRCODE_SUCC)
			{
				DoReply(iErrcode);
				return true;
			}
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		GetAnyValue()["menu_content"] = strMenuContent;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main(int argc, char** argv)
{
	CgiWXGetMenu cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

