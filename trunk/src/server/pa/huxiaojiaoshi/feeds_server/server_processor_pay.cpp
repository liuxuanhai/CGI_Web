#include "server_processor_pay.h"
#include "global_var.h"

IMPL_LOGGER(ServerProcessorPay, logger);

ServerProcessorPay::ServerProcessorPay()
{
}

ServerProcessorPay::~ServerProcessorPay()
{

}

void ServerProcessorPay::accept(lce::net::ConnectionInfo & conn)
{
    ConnMapIterator iter = _conn_map.find(conn.get_id());
    if(iter == _conn_map.end())
    {
        _conn_map.insert(std::make_pair(conn.get_id(), &conn));
    }
}

void ServerProcessorPay::remove(lce::net::ConnectionInfo & conn)
{
    ConnMapIterator iter = _conn_map.find(conn.get_id());
    if(iter != _conn_map.end())
    {
        _conn_map.erase(iter);
    }
}

void ServerProcessorPay::process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram )
{
    lce::net::DatagramStringBuffer& stDatagramStringBuffer = dynamic_cast<lce::net::DatagramStringBuffer&>(ogram);
    ::hoosho::msg::Msg stMsg;
    if(!stMsg.ParseFromString(stDatagramStringBuffer._strbuffer))
    {
        LOG4CPLUS_ERROR(logger, "ServerProcessorPay::process_input fail"
                        <<", ParseFromString failed, data.size="<<stDatagramStringBuffer._strbuffer.size());
        conn.close();
        return;
    }

    LOG4CPLUS_DEBUG(logger, "ServerProcessorPay::process_input, one msg:"<<stMsg.Utf8DebugString());


    ::hoosho::msg::MsgHead stMsgHead = stMsg.head();
    uint32_t dwCmd = stMsgHead.cmd();

    switch(dwCmd)
    {
#define CLIENT_PROCESSOR_CMD_HANDLER(cmd, handler) \
			case cmd: \
				handler(conn, stMsg); \
				break;

        CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::J_PAY_UNIFIEDORDER_QUERY_RES, process_unifiedorder_query_reply);

        CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::J_PAY_LISTEN_INCOME_RES, process_listen_income_reply);

#undef CLIENT_PROCESSOR_CMD_HANDLER

    default:
        LOG4CPLUS_ERROR(logger, "ServerProcessorPay::process_input fail, unknown cmd="<<dwCmd);
        conn.close();
        break;
    }


    return;
}

void ServerProcessorPay::process_unifiedorder_query_reply(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
    LOG4CPLUS_DEBUG(logger, "ServerProcessorPay::process_unifiedorder_query_reply");

    uint32_t dwMsgSeq = msg.head().seq();
    FsmFollow* fsm = g_fsm_follow_container->find_fsm(dwMsgSeq);
    if(!fsm)
    {
        LOG4CPLUS_ERROR(logger, "ServerProcessorPay::process_unifiedorder_query_reply fail, fsm not found for seq="<<dwMsgSeq);
        return;
    }

    fsm->check_out_trade_no_valid_reply_event(msg);
}

void ServerProcessorPay::process_listen_income_reply(lce::net::ConnectionInfo &conn, const ::hoosho::msg::Msg &msg)
{
    LOG4CPLUS_DEBUG(logger, "ServerProcessorPay::process_listen_income_reply, not care!");
    return;
}

int ServerProcessorPay::send_datagram(const hoosho::msg::Msg& stMsg)
{
    if(_conn_map.empty())
    {
        LOG4CPLUS_DEBUG(logger, "ServerProcessorPay::send_datagram failed, connection not found");
        return -1;
    }

    ConnMapIterator iter = _conn_map.begin();
    lce::net::DatagramStringBuffer datagram;
    if(!stMsg.SerializeToString(&datagram._strbuffer))
    {
        LOG4CPLUS_DEBUG(logger, "ServerProcessorPay::send_datagram failed, msg  SerializeToString failed");
        return -1;
    }

    LOG4CPLUS_DEBUG(logger, "ServerProcessorPay::send_datagram, one msg:"<<stMsg.Utf8DebugString());
    iter->second->write(datagram);

    return 0;
}


void ServerProcessorPay::process_output()
{

}


