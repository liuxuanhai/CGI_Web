#ifndef _WEB_HX_CGI_CARD_H_
#define _WEB_HX_CGI_CARD_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "ns_comm_def.h"
using namespace std;
using namespace lce::cgi;

class NoticeServerCgi: public Cgi
{
public:
	NoticeServerCgi(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile) :
			Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{

	}

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess()=0;

public:
	//cookie and vc func
	string m_notice_server_ip;
	int m_notice_server_port;
	
	string m_feeds_server_ip;
	int m_feeds_server_port;

	string m_user_server_ip;
	int m_user_server_port;

	string m_statistic_server_ip;
	int m_statistic_server_port;

public:

protected:



protected:
	DECL_LOGGER(logger)
	;
};

#endif
