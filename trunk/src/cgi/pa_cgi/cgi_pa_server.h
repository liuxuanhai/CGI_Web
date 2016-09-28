#ifndef _CGI_MARK_SERVER_H_
#define _CGI_MARK_SERVER_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"

using namespace std;
using namespace lce::cgi;

class CgiPaServer: public Cgi
{
public:
	CgiPaServer(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile):
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
	std::string paServerIP;
	int paServerPort;	
	std::string userServerIP;
	int userServerPort;
	std::string feedsServerIP;
	int feedsServerPort;
	int getFeedsLimit;

protected:
	DECL_LOGGER(logger)
	;
};

#endif
