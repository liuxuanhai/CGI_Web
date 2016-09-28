#ifndef _HOOSHO_TOKEN_SERVER_PA_INFO_H_
#define _HOOSHO_TOKEN_SERVER_PA_INFO_H_

#include "util/logger.h"
#include <string>
#include <ext/hash_map>
#include "app/timer.h"

using namespace lce;

class PAInfo
{
public:
	PAInfo():strAppid(""), strAppSecret(""), strBaseAccessToken(""), strJSAPITicket(""), qwLastUpdateTs(0)
	{
	}
	
	PAInfo(const std::string& appid, const std::string appsecret):
			  strAppid(appid), strAppSecret(appsecret)
			, strBaseAccessToken(""), strJSAPITicket("")
			, qwLastUpdateTs(0)
	{
	}
	
	~PAInfo(){}

public:
	std::string ToString() const;
	void update_token(const std::string& strAccessToekn, const std::string& strTicketJSAPI);
	
public:
	std::string strAppid;
	std::string strAppSecret;

	std::string strBaseAccessToken; //base access token	
	std::string strJSAPITicket;	//jsapi ticket

	//update ts
	uint64_t qwLastUpdateTs; 
	

private:
	DECL_LOGGER(logger);
};

class PAInfoManager : public lce::app::TimerHandler
{
public:
	struct AppidHash
	{
	    size_t __stl_hash_string(const char* __s) const
	    {
	        unsigned long __h = 0;
	        for(; *__s; ++__s)
	            __h = 5*__h + *__s;

	        return size_t(__h);
	    }
		
	    size_t operator()(const std::string& appid) const
	    {
	        return __stl_hash_string(appid.c_str());
	    }
	};
	
	typedef __gnu_cxx::hash_map<std::string, PAInfo, AppidHash> PAInfoMap;  //appid->painfo
	PAInfoMap mapPAInfo;
	
	PAInfoManager()
	{
		mapPAInfo.clear();
	}
	
	~PAInfoManager()
	{
	}

public:	
	int init();
	virtual void handle_timeout(void * param);	
	const PAInfoMap& get_pa_info_map(){return mapPAInfo;};
	void update_pa_info(const std::string& strAppid, const std::string& strAccessToekn, const std::string& strTicketJSAPI);

private:
	DECL_LOGGER(logger);
};

#endif

