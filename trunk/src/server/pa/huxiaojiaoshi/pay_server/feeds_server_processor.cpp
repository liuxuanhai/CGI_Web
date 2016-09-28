#include "feeds_server_processor.h"
#include "global_var.h"

IMPL_LOGGER(FeedsServerProcessor, logger);

FeedsServerProcessor::FeedsServerProcessor()
{
}

FeedsServerProcessor::~FeedsServerProcessor()
{
	
}

void FeedsServerProcessor::accept(lce::net::ConnectionInfo & conn)
{
    ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter == _conn_map.end())
	{
		_conn_map.insert(std::make_pair(conn.get_id(), &conn));
	}
}

void FeedsServerProcessor::remove(lce::net::ConnectionInfo & conn)
{
	ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter != _conn_map.end())
	{
		_conn_map.erase(iter);
	}
}

void FeedsServerProcessor::process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram )
{
	lce::net::DatagramStringBuffer& stDatagramStringBuffer = dynamic_cast<lce::net::DatagramStringBuffer&>(ogram);
	::hoosho::msg::Msg stMsg;
	if(!stMsg.ParseFromString(stDatagramStringBuffer._strbuffer))
	{
		LOG4CPLUS_ERROR(logger, "FeedsServerProcessor::process_input fail"
											<<", ParseFromString failed, data.size="<<stDatagramStringBuffer._strbuffer.size());
		conn.close();
		return;
	}

	LOG4CPLUS_DEBUG(logger, "FeedsServerProcessor::process_input, one msg:"<<stMsg.Utf8DebugString());
		
	::hoosho::msg::MsgHead stMsgHead = stMsg.head();
	uint32_t dwCmd = stMsgHead.cmd();
	switch(dwCmd)
	{
#define CLIENT_PROCESSOR_CMD_HANDLER(cmd, handler) \
			case cmd: \
				handler(conn, stMsg); \
				break;

		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::J_SET_OUT_TRADE_NO_RES, process_feeds_server_reply);
		
#undef CLIENT_PROCESSOR_CMD_HANDLER

		default:
			LOG4CPLUS_ERROR(logger, "FeedsServerProcessor::process_input fail, unknown cmd="<<dwCmd);
			conn.close();
			break;
	}


	return;
}

void FeedsServerProcessor::process_feeds_server_reply(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	if(msg.head().result() != ::hoosho::msg::E_OK)
	{
		LOG4CPLUS_FATAL(logger, "FeedsServerProcessor reply ERROR CODE: "<<msg.head().result());
		return;
	}

	LOG4CPLUS_DEBUG(logger, "FeedsServerProcessor reply E_OK");
}

int FeedsServerProcessor::send_datagram(const hoosho::msg::Msg& stMsg)
{
	if(_conn_map.empty())
	{
		LOG4CPLUS_DEBUG(logger, "FeedsServerProcessor::send_datagram failed, connection not found");
		return -1;
	}
	
	ConnMapIterator iter = _conn_map.begin();
	lce::net::DatagramStringBuffer datagram;
	if(!stMsg.SerializeToString(&datagram._strbuffer))
	{
		LOG4CPLUS_DEBUG(logger, "FeedsServerProcessor::send_datagram failed, msg  SerializeToString failed");
		return -1;
	}
	
	LOG4CPLUS_DEBUG(logger, "FeedsServerProcessor::send_datagram, one msg:"<<stMsg.Utf8DebugString());
	iter->second->write(datagram);
	
	return 0;
}


void FeedsServerProcessor::process_output()
{

}


