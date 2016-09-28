#ifndef _HOOSHO_PA_SERVER_CLIENT_MESSAGE_PROCESSOR_H_
#define _HOOSHO_PA_SERVER_CLIENT_MESSAGE_PROCESSOR_H_

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
	void process_pre_auth_code(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_detail_info_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_detail_info_update(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_follow(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_fans_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_fans_num_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_follows_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_power_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_zombies_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	void process_user_zomanager_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);

	
	int send_datagram(uint32_t conn_id, const hoosho::msg::Msg& stMsg);
	void process_output();

	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;

	ConnMap _conn_map;

private:
	DECL_LOGGER(logger);
};

#endif

