#ifndef _HOOSHO_FEEDS_SERVER_PROCESSOR_H_
#define _HOOSHO_FEEDS_SERVER_PROCESSOR_H_

#include "util/logger.h"
#include <ext/hash_map>
#include "net/connectioninfo.h"
#include "net/imessageprocessor.h"
#include "msg.pb.h"

using namespace lce;

class FeedsServerProcessor : public lce::net::IMessageProcessor
{
public:
	FeedsServerProcessor();
	~FeedsServerProcessor();

	void accept(lce::net::ConnectionInfo& conn);
	void remove(lce::net::ConnectionInfo& conn);

	void process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram);
	void process_feeds_server_reply(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	
	int send_datagram(const hoosho::msg::Msg& stMsg);
	void process_output();

	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;

	ConnMap _conn_map;

private:
	DECL_LOGGER(logger);
};

#endif

