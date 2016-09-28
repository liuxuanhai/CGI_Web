#ifndef _CGI_MARK_SERVER_H_
#define _CGI_MARK_SERVER_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "../cgi_ret_code_def.h"

using namespace std;
using namespace lce::cgi;

class CgiMarkServer: public Cgi
{
public:
	CgiMarkServer(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile):
			Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{
		
	}

	bool isAllSpace(const char* s);				
	void sendNotify();

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess()=0;

public:
	

public:
	std::string markServerIP;
	int markServerPort;	

protected:
	DECL_LOGGER(logger)
	;
};

#endif
