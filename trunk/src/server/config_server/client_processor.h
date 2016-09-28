#ifndef _HOOSHO_CONFIG_SERVER_CLIENT_MESSAGE_PROCESSOR_H_
#define _HOOSHO_CONFIG_SERVER_CLIENT_MESSAGE_PROCESSOR_H_

#include "util/logger.h"
#include <ext/hash_map>
#include "net/connectioninfo.h"
#include "net/imessageprocessor.h"
#include "msg.pb.h"
#include "app/timer.h"

using namespace lce;

class ClientProcessor : public lce::net::IMessageProcessor, public lce::app::TimerHandler
{
public:
	ClientProcessor();
	~ClientProcessor();

	void accept(lce::net::ConnectionInfo& conn);
	void remove(lce::net::ConnectionInfo& conn);

	void process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram);
	void process_hoosho_no_apply(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	
	virtual void handle_timeout(void * param);
	
	int send_datagram(uint32_t conn_id, const hoosho::msg::Msg& stMsg);
	void process_output();

	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;

	ConnMap _conn_map;

private:
	DECL_LOGGER(logger);
};

#endif

