#ifndef _CGI_WX_BASE_H_
#define _CGI_WX_BASE_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "proto_io_tcp_client.h"
#include "msg.pb.h"
#include "../cgi_common_util.h"

using namespace std;
using namespace lce::cgi;

class CgiWXBase: public Cgi
{
public:
	CgiWXBase(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile, int iCheckLogin) :
			Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{
		m_check_login = iCheckLogin;
	}

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess()=0;

public:
	string m_token_server_ip;
	int m_token_server_port;

public:
	int PAGetAccessToken(const std::string& strAppid, std::string& strPAAccessToken);

protected:
	int m_check_login;

protected:
	DECL_LOGGER(logger)
	;
};

#endif
