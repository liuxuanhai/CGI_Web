#ifndef _HOOSHO_FEEDS_SERVER_SERVER_MESSAGE_PROCESSOR_PAY_H_
#define _HOOSHO_FEEDS_SERVER_SERVER_MESSAGE_PROCESSOR_PAY_H_

#include "util/logger.h"
#include <ext/hash_map>
#include "net/connectioninfo.h"
#include "net/imessageprocessor.h"
#include "msg.pb.h"

using namespace lce;

class ServerProcessorPay : public lce::net::IMessageProcessor
{
public:
    ServerProcessorPay();
    ~ServerProcessorPay();

    void accept(lce::net::ConnectionInfo& conn);
    void remove(lce::net::ConnectionInfo& conn);

    void process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram);
    void process_unifiedorder_query_reply(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);
    void process_listen_income_reply(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg);

    int send_datagram(const hoosho::msg::Msg& stMsg);
    void process_output();

    typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*> ConnMap;
    typedef __gnu_cxx::hash_map<uint32_t, lce::net::ConnectionInfo*>::iterator ConnMapIterator;

    ConnMap _conn_map;

private:
    DECL_LOGGER(logger);
};

#endif

