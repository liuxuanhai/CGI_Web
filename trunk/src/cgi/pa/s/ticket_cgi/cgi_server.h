#ifndef _USER_CGI_Z_SERVER_H_
#define _USER_CGI_Z_SERVER_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "../cgi_common_util.h"

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
	int GetUserFeedContendIdList(common::protoio::ProtoIOTcpClient &ioclient
								, const std::string &strCode, uint64_t qwUserid, const std::vector<uint64_t> &vecFeedidList
								, std::vector<hoosho::msg::s::FeedContendInfo> &vecFeedContendInfo);
	int GetUserInfoList(common::protoio::ProtoIOTcpClient &ioclient
								, const std::string &strCode, const std::set<uint64_t> &setUserid
								, std::vector<hoosho::msg::s::UserInfo> &vecUserInfo);

public:
	

public:
	std::string SServerIP;
	int SServerPort;	

protected:
	DECL_LOGGER(logger)	;
};

#endif
