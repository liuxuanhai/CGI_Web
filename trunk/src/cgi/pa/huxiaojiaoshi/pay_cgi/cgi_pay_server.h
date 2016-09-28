#ifndef _CGI_PAY_SERVER_H_
#define _CGI_PAY_SERVER_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"

using namespace std;
using namespace lce::cgi;

class CgiPayServer: public Cgi
{
public:
	CgiPayServer(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile, int iCheckLogin):
			Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{
		m_check_login = iCheckLogin;
	}

	bool isAllSpace(const char* s);				
	void sendNotify();

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess()=0;

public:
	

public:
	std::string PayServerIP;
	int PayServerPort;	

	std::string UserServerIP;
	int UserServerPort;

	//cookie	
	std::string m_cookie_name_key;	
	std::string m_cookie_name_val;	
	std::string m_cookie_value_key;	
	std::string m_cookie_value_val;
	
	int m_check_login;

protected:
	DECL_LOGGER(logger)	;
};

#endif
