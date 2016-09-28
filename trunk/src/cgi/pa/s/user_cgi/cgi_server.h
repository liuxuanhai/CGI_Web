#ifndef _USER_CGI_Z_SERVER_H_
#define _USER_CGI_Z_SERVER_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"

using namespace std;
using namespace lce::cgi;

class CgiServer: public Cgi
{
public:
	CgiServer(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile):
			Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{		
	}
	
public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess()=0;

public:
	

public:
	std::string SServerIP;
	int SServerPort;	

protected:
	DECL_LOGGER(logger)	;
};

#endif
