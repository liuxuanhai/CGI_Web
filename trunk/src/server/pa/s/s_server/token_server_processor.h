#ifndef _HOOSHO_J_USER_SERVER_TOKEN_SERVER_MESSAGE_PROCESSOR_H_
#define _HOOSHO_J_USER_SERVER_TOKEN_SERVER_MESSAGE_PROCESSOR_H_

#include "util/logger.h"
#include <ext/hash_map>
#include "net/connectioninfo.h"
#include "net/imessageprocessor.h"
#include "msg.pb.h"

using namespace lce;

class TokenServerProcessor : public lce::net::IMessageProcessor
{
public:
	TokenServerProcessor();
	~TokenServerProcessor();

	void accept(lce::net::ConnectionInfo& conn);
	void remove(lce::net::ConnectionInfo& conn);
	
	void process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram);
	void process_pa_token_notify(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	
	void process_output();

	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;

	ConnMap _conn_map;

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
    
    typedef __gnu_cxx::hash_map<std::string, std::string, AppidHash> PAAppid2BaseAccessTokenMap;  //appid->base_access_token
    typedef __gnu_cxx::hash_map<std::string, std::string, AppidHash> PAAppid2JSAPITiketMap;  //appid->jsapiTiket
	PAAppid2BaseAccessTokenMap _base_access_token_map;
	PAAppid2JSAPITiketMap _jsapi_ticket_map;

	void set_pa_info(const std::string& strAppid, const std::string& strBaseAcessTokenn, const std::string& strJSAPITicket);
	int get_pa_info(const std::string& strAppid, std::string& strBaseAcessTokenn, std::string& strJSAPITicket);

private:
	
	DECL_LOGGER(logger);
};

#endif

