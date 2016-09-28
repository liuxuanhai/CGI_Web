#ifndef _HOOSHO_J_USER_SERVER_CLIENT_MESSAGE_PROCESSOR_H_
#define _HOOSHO_J_USER_SERVER_CLIENT_MESSAGE_PROCESSOR_H_

#include "util/logger.h"
#include <ext/hash_map>
#include "net/connectioninfo.h"
#include "net/imessageprocessor.h"
#include "msg.pb.h"
#include "cgi/cgi.h"

using namespace lce;

class ClientProcessor : public lce::net::IMessageProcessor
{
public:
	ClientProcessor();
	~ClientProcessor();

	int init();
	int load_zombie();

	void accept(lce::net::ConnectionInfo& conn);
	void remove(lce::net::ConnectionInfo& conn);
	
	void process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram);
	void process_get_jsapi_ticket(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_get_access_token(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_wx_pay_callback(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_login(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_input_authenticate(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);	
	
	int send_datagram(uint32_t conn_id, const hoosho::msg::Msg& stMsg);
	void process_output();

	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;

	ConnMap _conn_map;

public:
	class LoginSessionInfo
	{
	public:
		hoosho::msg::z::UserInfo _user_info;
		uint64_t _expire_ts;

		LoginSessionInfo():_expire_ts(0)
		{
			_user_info.Clear();
		}
		
		LoginSessionInfo(const hoosho::msg::z::UserInfo& stUserInfo, uint64_t qwExpireTS)
				:_expire_ts(qwExpireTS)
		{
			_user_info.CopyFrom(stUserInfo);
		}
	};

	class PhoneVCSessionInfo
	{
	public:
		std::string _vc;		
		uint64_t _expire_ts;

		PhoneVCSessionInfo():_vc(""), _expire_ts(0){}
		PhoneVCSessionInfo(const std::string& strVC, uint64_t qwExpireTS)
				:_vc(strVC), _expire_ts(qwExpireTS)
		{

		}
	};

	struct strHash
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
	
	typedef __gnu_cxx::hash_map<std::string, LoginSessionInfo, strHash> LoginSessionInfoMap;
	typedef __gnu_cxx::hash_map<std::string, LoginSessionInfo, strHash>::iterator LoginSessionInfoMapIterator;
	LoginSessionInfoMap _login_session_map;

	typedef __gnu_cxx::hash_map<std::string, std::string, strHash> Openid2AuthCodeMap;
	typedef __gnu_cxx::hash_map<std::string, std::string, strHash>::iterator Openid2AuthCodeMapIterator;
	Openid2AuthCodeMap _openid_2_authcode_map;

	typedef __gnu_cxx::hash_map<std::string, PhoneVCSessionInfo, strHash> Phone2VerifyCodeMap;
	typedef __gnu_cxx::hash_map<std::string, PhoneVCSessionInfo, strHash>::iterator Phone2VerifyCodeMapIterator;
	Phone2VerifyCodeMap _phone_2_verifycod_map;
	

	void add_login_session(const std::string& strAuthCode, const hoosho::msg::z::UserInfo& stUserInfo, uint64_t qwExpireTs = 0);
	int check_login_session(const std::string& strAuthCode, hoosho::msg::z::UserInfo& stUserInfo);

	void add_phonevc_session(const std::string& strPhone, const std::string& strPhoneVC);
	void delete_phonevc_session(const std::string& strPhone);
	bool check_phonevc_session(const std::string& strPhone, const std::string& strPhoneVC);

public:
	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_db_table_user_info_name;	
	lce::cgi::CMysql m_mysql_helper;	

private:
	DECL_LOGGER(logger);
};

#endif

