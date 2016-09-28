#ifndef _PA_Z_CGI_ORDER_BASE_H_
#define _PA_Z_CGI_ORDER_BASE_H_

#include "cgi/cgi.h"
#include "../cgi_common_util.h"

using namespace lce::cgi;

class CgiOrderBase: public Cgi
{
public:
	CgiOrderBase(uint64_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
		:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{

	}

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess() = 0;

public:
	std::string m_z_server_ip;
	uint32_t m_z_server_port;

protected:
	DECL_LOGGER(logger);

};

#endif
