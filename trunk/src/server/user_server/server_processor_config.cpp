#include "server_processor_config.h"
#include "global_var.h"

IMPL_LOGGER(ServerProcessorConfig, logger);

ServerProcessorConfig::ServerProcessorConfig()
{
}

ServerProcessorConfig::~ServerProcessorConfig()
{
	
}

void ServerProcessorConfig::accept(lce::net::ConnectionInfo & conn)
{
    ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter == _conn_map.end())
	{
		_conn_map.insert(std::make_pair(conn.get_id(), &conn));
	}
}

void ServerProcessorConfig::remove(lce::net::ConnectionInfo & conn)
{
	ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter != _conn_map.end())
	{
		_conn_map.erase(iter);
	}
}

void ServerProcessorConfig::process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram )
{
	lce::net::DatagramStringBuffer& stDatagramStringBuffer = dynamic_cast<lce::net::DatagramStringBuffer&>(ogram);
	::hoosho::msg::Msg stMsg;
	if(!stMsg.ParseFromString(stDatagramStringBuffer._strbuffer))
	{
		LOG4CPLUS_ERROR(logger, "ServerProcessorConfig::process_input fail"
											<<", ParseFromString failed, data.size="<<stDatagramStringBuffer._strbuffer.size());
		conn.close();
		return;
	}

	LOG4CPLUS_DEBUG(logger, "ServerProcessorConfig::process_input, one msg:"<<stMsg.Utf8DebugString());
		
	::hoosho::msg::MsgHead stMsgHead = stMsg.head();
	uint32_t dwCmd = stMsgHead.cmd();
	switch(dwCmd)
	{
#define CLIENT_PROCESSOR_CMD_HANDLER(cmd, handler) \
			case cmd: \
				handler(conn, stMsg); \
				break;

		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::HOOSHO_NO_APPLY_RES, process_hoosho_no_apply_reply);
		
#undef CLIENT_PROCESSOR_CMD_HANDLER

		default:
			LOG4CPLUS_ERROR(logger, "ServerProcessorConfig::process_input fail, unknown cmd="<<dwCmd);
			conn.close();
			break;
	}


	return;
}

void ServerProcessorConfig::process_hoosho_no_apply_reply(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	uint32_t dwMsgSeq = msg.head().seq();
	FsmPreAuth* fsm = g_fsm_preauth_container->find_fsm(dwMsgSeq);
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ServerProcessorConfig::process_pre_auth_reply fail, fsm not found for seq="<<dwMsgSeq);
		return;
	}

	fsm->apply_hoosho_no_reply_event(msg);
}

int ServerProcessorConfig::send_datagram(const hoosho::msg::Msg& stMsg)
{
	if(_conn_map.empty())
	{
		LOG4CPLUS_DEBUG(logger, "ServerProcessorConfig::send_datagram failed, connection not found");
		return -1;
	}
	
	ConnMapIterator iter = _conn_map.begin();
	lce::net::DatagramStringBuffer datagram;
	if(!stMsg.SerializeToString(&datagram._strbuffer))
	{
		LOG4CPLUS_DEBUG(logger, "ServerProcessorConfig::send_datagram failed, msg  SerializeToString failed");
		return -1;
	}
	
	LOG4CPLUS_DEBUG(logger, "ServerProcessorConfig::send_datagram, one msg:"<<stMsg.Utf8DebugString());
	iter->second->write(datagram);
	
	return 0;
}


void ServerProcessorConfig::process_output()
{

}


