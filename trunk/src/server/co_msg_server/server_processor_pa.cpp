#include "server_processor_pa.h"
#include "global_var.h"

IMPL_LOGGER(ServerProcessorPA, logger);

ServerProcessorPA::ServerProcessorPA()
{
}

ServerProcessorPA::~ServerProcessorPA()
{
	
}

void ServerProcessorPA::accept(lce::net::ConnectionInfo & conn)
{
    ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter == _conn_map.end())
	{
		_conn_map.insert(std::make_pair(conn.get_id(), &conn));
	}
}

void ServerProcessorPA::remove(lce::net::ConnectionInfo & conn)
{
	ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter != _conn_map.end())
	{
		_conn_map.erase(iter);
	}
}

void ServerProcessorPA::process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram )
{
	lce::net::DatagramStringBuffer& stDatagramStringBuffer = dynamic_cast<lce::net::DatagramStringBuffer&>(ogram);
	::hoosho::msg::Msg stMsg;
	if(!stMsg.ParseFromString(stDatagramStringBuffer._strbuffer))
	{
		LOG4CPLUS_ERROR(logger, "ServerProcessorPA::process_input fail"
											<<", ParseFromString failed, data.size="<<stDatagramStringBuffer._strbuffer.size());
		conn.close();
		return;
	}

	LOG4CPLUS_DEBUG(logger, "ServerProcessorPA::process_input, one msg:"<<stMsg.Utf8DebugString());
		
	::hoosho::msg::MsgHead stMsgHead = stMsg.head();
	uint32_t dwCmd = stMsgHead.cmd();
	switch(dwCmd)
	{
#define CLIENT_PROCESSOR_CMD_HANDLER(cmd, handler) \
			case cmd: \
				handler(conn, stMsg); \
				break;

		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::PARSE_PRE_AUTH_CODE_RES, process_pre_auth_reply);
		
#undef CLIENT_PROCESSOR_CMD_HANDLER

		default:
			LOG4CPLUS_ERROR(logger, "ServerProcessorPA::process_input fail, unknown cmd="<<dwCmd);
			conn.close();
			break;
	}


	return;
}

void ServerProcessorPA::process_pre_auth_reply(lce::net::ConnectionInfo& conn, ::hoosho::msg::Msg& msg)
{
	//get respon from pa server, swapcontext from main -> coroutine
	
	int64_t coroutine_id = msg.head().seq();
	if(coroutine_id == 0)
		return;
		
	CoroutineModuleBase<TimerModule>& co_module = Singleton<CoroutineModuleBase<TimerModule>>::Instance();
	co_module.ReadLock();
	co_module.Resume(coroutine_id, &msg);
	co_module.ReleaseLock();
	
}

int ServerProcessorPA::send_datagram(const hoosho::msg::Msg& stMsg)
{
	if(_conn_map.empty())
	{
		LOG4CPLUS_DEBUG(logger, "ServerProcessorPA::send_datagram failed, connection not found");
		return -1;
	}
	
	ConnMapIterator iter = _conn_map.begin();
	lce::net::DatagramStringBuffer datagram;
	if(!stMsg.SerializeToString(&datagram._strbuffer))
	{
		LOG4CPLUS_DEBUG(logger, "ServerProcessorPA::send_datagram failed, msg  SerializeToString failed");
		return -1;
	}
	
	LOG4CPLUS_DEBUG(logger, "ServerProcessorPA::send_datagram, one msg:"<<stMsg.Utf8DebugString());
	iter->second->write(datagram);
	
	return 0;
}


void ServerProcessorPA::process_output()
{

}


