#ifndef _HOOSHO_USER_SERVER_SERVER_MESSAGE_PROCESSOR_NOTICE_H_
#define _HOOSHO_USER_SERVER_SERVER_MESSAGE_PROCESSOR_NOTICE_H_

#include "util/logger.h"
#include <ext/hash_map>
#include "net/connectioninfo.h"
#include "net/imessageprocessor.h"
#include "msg.pb.h"

using namespace lce;

class ServerProcessorMsg : public lce::net::IMessageProcessor
{
public:
	ServerProcessorMsg();
	~ServerProcessorMsg();

	void accept(lce::net::ConnectionInfo& conn);
	void remove(lce::net::ConnectionInfo& conn);

	void process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram);
	void process_add_notice_reply(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
	
	int send_datagram(const hoosho::msg::Msg& stMsg);
	void process_output();

	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;

	ConnMap _conn_map;

private:
	DECL_LOGGER(logger);
};

#endif

