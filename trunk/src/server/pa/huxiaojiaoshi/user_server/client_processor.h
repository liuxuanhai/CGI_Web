#ifndef _HOOSHO_J_USER_SERVER_CLIENT_MESSAGE_PROCESSOR_H_
#define _HOOSHO_J_USER_SERVER_CLIENT_MESSAGE_PROCESSOR_H_

#include "util/logger.h"
#include <ext/hash_map>
#include "net/connectioninfo.h"
#include "net/imessageprocessor.h"
#include "msg.pb.h"

using namespace lce;

class ClientProcessor : public lce::net::IMessageProcessor
{
public:
	ClientProcessor();
	~ClientProcessor();

	void accept(lce::net::ConnectionInfo& conn);
	void remove(lce::net::ConnectionInfo& conn);
	
	void process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram);
	
	void process_user_login(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_check_login(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_detail_info_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_detail_info_update(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_get_jsapi_ticket(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	
	int send_datagram(uint32_t conn_id, const hoosho::msg::Msg& stMsg);
	void process_output();

	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;

	ConnMap _conn_map;

public:
	class LoginSessionInfo
	{
	public:
		std::string _openid;
		std::string _session;
		uint64_t _expire_ts;

		LoginSessionInfo():_openid(""), _session(""), _expire_ts(0){}
		LoginSessionInfo(const std::string& strOpenid, const std::string& strSession, uint64_t qwExpireTS)
				:_openid(strOpenid), _session(strSession), _expire_ts(qwExpireTS)
		{

		}
	};

	struct OpenidHash
	{
	    size_t __stl_hash_string(const char* __s) const
	    {
	        unsigned long __h = 0;
	        for(; *__s; ++__s)
	            __h = 5*__h + *__s;

	        return size_t(__h);
	    }
		
	    size_t operator()(const std::string& str) const
	    {
	        return __stl_hash_string(str.c_str());
	    }
	};
	
	typedef __gnu_cxx::hash_map<std::string, LoginSessionInfo, OpenidHash> LoginSessionInfoMap;
	typedef __gnu_cxx::hash_map<std::string, LoginSessionInfo, OpenidHash>::iterator LoginSessionInfoMapIterator;
	LoginSessionInfoMap _login_session_map;

	void add_login_session(const std::string& strOpenid, LoginSessionInfo& stLoginSessionInfo);
	bool check_login_session(const std::string& strOpenid, const std::string strSession);
	

private:
	DECL_LOGGER(logger);
};

#endif

