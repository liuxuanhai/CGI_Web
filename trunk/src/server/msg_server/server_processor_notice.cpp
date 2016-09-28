#include "server_processor_notice.h"
#include "global_var.h"

IMPL_LOGGER(ServerProcessorNotice, logger);

ServerProcessorNotice::ServerProcessorNotice()
{
}

ServerProcessorNotice::~ServerProcessorNotice()
{
	
}

void ServerProcessorNotice::accept(lce::net::ConnectionInfo & conn)
{
    ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter == _conn_map.end())
	{
		_conn_map.insert(std::make_pair(conn.get_id(), &conn));
	}
}

void ServerProcessorNotice::remove(lce::net::ConnectionInfo & conn)
{
	ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter != _conn_map.end())
	{
		_conn_map.erase(iter);
	}
}

void ServerProcessorNotice::process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram )
{
	lce::net::DatagramStringBuffer& stDatagramStringBuffer = dynamic_cast<lce::net::DatagramStringBuffer&>(ogram);
	::hoosho::msg::Msg stMsg;
	if(!stMsg.ParseFromString(stDatagramStringBuffer._strbuffer))
	{
		LOG4CPLUS_ERROR(logger, "ServerProcessorNotice::process_input fail"
											<<", ParseFromString failed, data.size="<<stDatagramStringBuffer._strbuffer.size());
		conn.close();
		return;
	}

	LOG4CPLUS_DEBUG(logger, "ServerProcessorNotice::process_input, one msg:"<<stMsg.Utf8DebugString());
		
	::hoosho::msg::MsgHead stMsgHead = stMsg.head();
	uint32_t dwCmd = stMsgHead.cmd();
	switch(dwCmd)
	{
#define CLIENT_PROCESSOR_CMD_HANDLER(cmd, handler) \
			case cmd: \
				handler(conn, stMsg); \
				break;

		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::ADD_NOTICE_RECORD_RES, process_add_notice_reply);
		
#undef CLIENT_PROCESSOR_CMD_HANDLER

		default:
			LOG4CPLUS_ERROR(logger, "ServerProcessorNotice::process_input fail, unknown cmd="<<dwCmd);
			conn.close();
			break;
	}


	return;
}

void ServerProcessorNotice::process_add_notice_reply(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	LOG4CPLUS_DEBUG(logger, "ServerProcessorNotice::process_add_notice_reply, not care !!!");
}

int ServerProcessorNotice::send_datagram(const hoosho::msg::Msg& stMsg)
{
	if(_conn_map.empty())
	{
		LOG4CPLUS_DEBUG(logger, "ServerProcessorNotice::send_datagram failed, connection not found");
		return -1;
	}
	
	ConnMapIterator iter = _conn_map.begin();
	lce::net::DatagramStringBuffer datagram;
	if(!stMsg.SerializeToString(&datagram._strbuffer))
	{
		LOG4CPLUS_DEBUG(logger, "ServerProcessorNotice::send_datagram failed, msg  SerializeToString failed");
		return -1;
	}
	
	LOG4CPLUS_DEBUG(logger, "ServerProcessorNotice::send_datagram, one msg:"<<stMsg.Utf8DebugString());
	iter->second->write(datagram);
	
	return 0;
}


void ServerProcessorNotice::process_output()
{

}


