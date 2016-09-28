#ifndef _CGI_WX_CALLBACK_H_
#define _CGI_WX_CALLBACK_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "proto_io_tcp_client.h"
#include "msg.pb.h"
#include "../cgi_common_util.h"

using namespace std;
using namespace lce::cgi;

class CgiWXCallback: public Cgi
{
public:
	CgiWXCallback(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile, int iCheckLogin) :
			Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{
		m_check_login = iCheckLogin;
	}

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess()=0;

protected:
	int m_check_login;

public:
	std::string m_mp_token;
	std::string m_encoding_aes_key;
	std::string m_appid;

protected:
	DECL_LOGGER(logger)
	;
};

#endif
