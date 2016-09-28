#ifndef _XDRIVE_SERVER_CLIENT_MESSAGE_PROCESSOR_H_
#define _XDRIVE_SERVER_CLIENT_MESSAGE_PROCESSOR_H_

#include "util/logger.h"
#include <ext/hash_map>
#include "net/connectioninfo.h"
#include "net/imessageprocessor.h"
#include "msg.pb.h"
#include "cgi/cgi_mysql.h"
#include "memory/array_object_pool.h"

using namespace lce;

typedef lce::memory::ArrayObjectPool<::hoosho::msg::Msg> HooshoMsgPool;

class ClientProcessor : public lce::net::IMessageProcessor
{
public:
	ClientProcessor();
	ClientProcessor(int32_t sec);
	~ClientProcessor();
	
	void accept(lce::net::ConnectionInfo& conn);
	void remove(lce::net::ConnectionInfo& conn);

	void process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram );
	int send_datagram(uint32_t conn_id, const hoosho::msg::Msg& stMsg);
	
	void process_output();
	static void* CoCallBack(int64_t coroutine_id, void* ptr_usr1, void* ptr_usr2);

	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
	typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;
	
private:
	ConnMap _conn_map;
	int32_t _timeout;

	DECL_LOGGER(logger);
};

#endif

