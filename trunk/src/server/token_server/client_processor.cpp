#include "client_processor.h"
#include "global_var.h"
#include "util/lce_util.h"
#include "executor_thread_queue.h"
#include <openssl/sha.h>

IMPL_LOGGER(ClientProcessor, logger);

ClientProcessor::ClientProcessor()
{
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
		Notify(conn.get_id());
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
	LOG4CPLUS_FATAL(logger, "ClientProcessor::process_input does not support any Receive Msg, but only Notify out is Supported!@!");

	return;
}

void ClientProcessor::Notify()
{
	for(ConnMapIterator iter = _conn_map.begin(); iter!=_conn_map.end(); ++iter)
	{
		Notify(iter->first);
	}
	
	return;
}

void ClientProcessor::Notify(uint32_t dwConnid)
{
	::hoosho::msg::Msg stMsg;

	::hoosho::msg::MsgHead* pMsgHead = stMsg.mutable_head();
	pMsgHead->set_cmd(::hoosho::msg::TOKEN_PA_TOKEN_INFO_NOTIFY);
	pMsgHead->set_seq(time(0));
	pMsgHead->set_result(::hoosho::msg::E_OK);

	::hoosho::token::PATokenInfoNotify* pMsgBody = stMsg.mutable_token_info_notify();
	const PAInfoManager::PAInfoMap& mapPAInfo = g_pa_info_manager->get_pa_info_map();
	for(PAInfoManager::PAInfoMap::const_iterator iter = mapPAInfo.begin(); iter!=mapPAInfo.end(); ++iter)
	{
		const PAInfo& stPAInfo = iter->second;
		::hoosho::token::PATokenInfo* pPATokenInfo = pMsgBody->add_pa_token_info_list();
		pPATokenInfo->set_pa_appid(stPAInfo.strAppid);
		pPATokenInfo->set_pa_access_token(stPAInfo.strBaseAccessToken);
		pPATokenInfo->set_pa_jsapi_ticket(stPAInfo.strJSAPITicket);
	}

	send_datagram(dwConnid, stMsg);
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



