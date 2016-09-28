#ifndef _XDRIVE_SERVER_CLIENT_MESSAGE_PROCESSOR_H_
#define _XDRIVE_SERVER_CLIENT_MESSAGE_PROCESSOR_H_

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

	void process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram );
	int send_datagram(uint32_t conn_id, const hoosho::msg::Msg& stMsg);
	
	void process_output();

	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;

	ConnMap _conn_map;

	DECL_LOGGER(logger);
};

#endif

