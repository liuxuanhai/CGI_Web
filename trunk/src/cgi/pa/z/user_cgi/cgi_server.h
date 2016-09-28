#ifndef _CGI_PAY_SERVER_H_
#define _CGI_PAY_SERVER_H_

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
	std::string ZServerIP;
	int ZServerPort;	

protected:
	DECL_LOGGER(logger)	;
};

#endif
