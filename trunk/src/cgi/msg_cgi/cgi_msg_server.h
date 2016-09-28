#ifndef _CGI_MSG_SERVER_H_
#define _CGI_MSG_SERVER_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "../cgi_ret_code_def.h"

using namespace std;
using namespace lce::cgi;

class CgiMsgServer: public Cgi
{
public:
	CgiMsgServer(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile):
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
	std::string msgServerIP;
	int msgServerPort;
	std::string userServerIP;
	int userServerPort;
	std::string feedServerIP;
	int feedServerPort;
	uint32_t interval;
	uint32_t lenLimit;
	uint32_t getMsgAmountLimit;

protected:
	

protected:
	DECL_LOGGER(logger)
	;
};

#endif
