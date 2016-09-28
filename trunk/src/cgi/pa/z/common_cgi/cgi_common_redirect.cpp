#ifndef _HOOSHO_CGI_COMMON_JSAPI_SIGNATURE_H_
#define _HOOSHO_CGI_COMMON_JSAPI_SIGNATURE_H_

#include "cgi/cgi.h"
#include "../cgi_common_util.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>

class CgiCommonRedirect: public lce::cgi::Cgi
{
public:
	 CgiCommonRedirect(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		return true;
	}
	
	virtual bool Process()
	{
		lce::cgi::CHttpHeader& stRespHeader = GetHeader();

		stRespHeader.AddHeader("Status: 302");
		//stRespHeader.AddHeader("Location: http://www.baidu.com");
		//stRespHeader.AddHeader("Location: https://open.weixin.qq.com/connect/oauth2/authorize?appid=wx41c62067410a82f6&redirect_uri=http%3A%2F%2Fwww.huxiao.com%2Fziga%2Findex.html&response_type=code&scope=snsapi_base&state=z&connect_redirect=1#wechat_redirect");
		stRespHeader.AddHeader("Location: https://open.weixin.qq.com/connect/oauth2/authorize?appid=wx41c62067410a82f6&redirect_uri=http%3A%2F%2Fwww.huxiao.com%2Fziga%2Findex.html%3Forderid%3D96381914926563265%23qixi&response_type=code&scope=snsapi_userinfo&state=z&connect_redirect=1#wechat_redirect");


		DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiCommonRedirect, logger);

int main(int argc, char** argv)
{
	CgiCommonRedirect cgi(0, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

