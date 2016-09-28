#include "client_processor.h"
#include "global_var.h"
#include "util/lce_util.h"
#include "net/datagramstringbuffer.h"
#include "executor_thread_queue.h"

IMPL_LOGGER(ClientProcessor, logger);

ClientProcessor::ClientProcessor()
{
	
	assert(g_timer_container->schedule(*this, 5, g_server->config().get_int_param("SAVE", "time"), NULL) >= 0);
}

ClientProcessor::~ClientProcessor()
{
	
}

void ClientProcessor::accept(lce::net::ConnectionInfo & conn)
{
    ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter == _conn_map.end())
	{
		_conn_map.insert(std::make_pair(conn.get_id(), &conn));
	}
}

void ClientProcessor::remove(lce::net::ConnectionInfo & conn)
{
	ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter != _conn_map.end())
	{
		_conn_map.erase(iter);
	}
}

void ClientProcessor::process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram )
{
	lce::net::DatagramStringBuffer& stDatagramStringBuffer = dynamic_cast<lce::net::DatagramStringBuffer&>(ogram);
	::hoosho::msg::Msg stMsg;
	if(!stMsg.ParseFromString(stDatagramStringBuffer._strbuffer))
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_input fail"
											<<", ParseFromString failed, data.size="<<stDatagramStringBuffer._strbuffer.size());
		conn.close();
		return;
	}

	LOG4CPLUS_DEBUG(logger, "process_input, one msg:"<<stMsg.Utf8DebugString());
		
	::hoosho::msg::MsgHead stMsgHead = stMsg.head();
	uint32_t dwCmd = stMsgHead.cmd();
	switch(dwCmd)
	{
#define CLIENT_PROCESSOR_CMD_HANDLER(cmd, handler) \
			case cmd: \
				handler(conn, stMsg); \
				break;

		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::HOOSHO_NO_APPLY_REQ, process_hoosho_no_apply);
		
#undef CLIENT_PROCESSOR_CMD_HANDLER

		default:
			LOG4CPLUS_ERROR(logger, "ClientProcessor::process_input fail, unknown cmd="<<dwCmd);
			conn.close();
			break;
	}


	return;
}

void ClientProcessor::process_hoosho_no_apply(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	::hoosho::msg::Msg stResp;
	::hoosho::msg::MsgHead* pRespHead = stResp.mutable_head();
	pRespHead->set_cmd(::hoosho::msg::HOOSHO_NO_APPLY_RES);
	pRespHead->set_seq(msg.head().seq());
	pRespHead->set_result(::hoosho::msg::E_OK);
	if(0 == g_hoosho_no_account)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_hoosho_no_apply failed, g_hoosho_no_account=0, not inted succ!!");
		pRespHead->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		send_datagram(conn.get_id(), stResp);
		return;
	}

	::hoosho::config::HooshoNOApplyRes* pHooshoNOApplyRes = stResp.mutable_hoosho_no_apply_res();
	pHooshoNOApplyRes->set_hoosho_no(++g_hoosho_no_account);
	send_datagram(conn.get_id(), stResp);
	
	int iSavePerNum = g_server->config().get_int_param("SAVE", "num");
	if(0 == (g_hoosho_no_account % iSavePerNum))
	{
		ExecutorThreadRequestElement element(ExecutorThreadRequestType::T_SAVE_HOOSHO_NO_ACCOUNT);
		element.m_c_key = 1;
		element.m_c_value = g_hoosho_no_account;
		g_executor_thread_processor->send_request(element);
	}
}

void ClientProcessor::handle_timeout(void * param)
{
	LOG4CPLUS_DEBUG(logger, "timeout, auto save cur g_hoosho_no_account="<<g_hoosho_no_account);
	
	ExecutorThreadRequestElement element(ExecutorThreadRequestType::T_SAVE_HOOSHO_NO_ACCOUNT);
	element.m_c_key = 1;
	element.m_c_value = g_hoosho_no_account;
	g_executor_thread_processor->send_request(element);
}

int ClientProcessor::send_datagram(uint32_t conn_id, const hoosho::msg::Msg& stMsg)
{
	ConnMapIterator iter = _conn_map.find(conn_id);
	if(iter == _conn_map.end())
	{
		LOG4CPLUS_DEBUG(logger, "ClientProcessor::send_datagram failed, connection not found, conn_id="<<conn_id);
		return -1;
	}
	lce::net::ConnectionInfo* conn = iter->second;

	lce::net::DatagramStringBuffer datagram;
	if(!stMsg.SerializeToString(&datagram._strbuffer))
	{
		LOG4CPLUS_DEBUG(logger, "ClientProcessor::send_datagram failed, msg  SerializeToString failed");
		return -1;
	}
	
	LOG4CPLUS_DEBUG(logger, "send_datagram, one msg:"<<stMsg.Utf8DebugString());
	conn->write(datagram);
	
	return 0;
}


void ClientProcessor::process_output()
{

}


