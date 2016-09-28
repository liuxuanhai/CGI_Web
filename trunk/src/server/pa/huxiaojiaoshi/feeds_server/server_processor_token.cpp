#include "global_var.h"
#include "server_processor_token.h"

IMPL_LOGGER(ServerProcessorToken, logger);

ServerProcessorToken::ServerProcessorToken()
{
}

ServerProcessorToken::~ServerProcessorToken()
{
	
}

void ServerProcessorToken::accept(lce::net::ConnectionInfo & conn)
{
    ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter == _conn_map.end())
	{
		_conn_map.insert(std::make_pair(conn.get_id(), &conn));
	}
}

void ServerProcessorToken::remove(lce::net::ConnectionInfo & conn)
{
	ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter != _conn_map.end())
	{
		_conn_map.erase(iter);
	}
}

void ServerProcessorToken::process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram )
{
	lce::net::DatagramStringBuffer& stDatagramStringBuffer = dynamic_cast<lce::net::DatagramStringBuffer&>(ogram);
	::hoosho::msg::Msg stMsg;
	if(!stMsg.ParseFromString(stDatagramStringBuffer._strbuffer))
	{
		LOG4CPLUS_ERROR(logger, "ServerProcessorToken::process_input fail"
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

		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::TOKEN_PA_TOKEN_INFO_NOTIFY, process_pa_token_notify);
		
#undef CLIENT_PROCESSOR_CMD_HANDLER

		default:
			LOG4CPLUS_ERROR(logger, "ServerProcessorToken::process_input fail, unknown cmd="<<dwCmd);
			conn.close();
			break;
	}


	return;
}

void ServerProcessorToken::process_pa_token_notify(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	const ::hoosho::token::PATokenInfoNotify& stPATokenInfoNotify = msg.token_info_notify();
	for(int i=0; i!=stPATokenInfoNotify.pa_token_info_list_size(); ++i)
	{
		const ::hoosho::token::PATokenInfo& stPATokenInfo = stPATokenInfoNotify.pa_token_info_list(i);
		set_pa_info(stPATokenInfo.pa_appid()
					, stPATokenInfo.pa_access_token()
					, stPATokenInfo.pa_jsapi_ticket());
	}
}

void ServerProcessorToken::set_pa_info(const std::string& strAppid, const std::string& strBaseAcessTokenn, const std::string& strJSAPITicket)
{
	_base_access_token_map[strAppid] = strBaseAcessTokenn;
	_jsapi_ticket_map[strAppid] = strJSAPITicket;
}

int ServerProcessorToken::get_pa_info(const std::string& strAppid, std::string& strBaseAcessTokenn, std::string& strJSAPITicket)
{
	strBaseAcessTokenn = _base_access_token_map[strAppid];
	strJSAPITicket = _jsapi_ticket_map[strAppid];

	if(strBaseAcessTokenn.empty() || strJSAPITicket.empty())
	{
		LOG4CPLUS_ERROR(logger, "ServerProcessorToken::get_pa_info failed, pa token info not exists for strAppid="<<strAppid
							<<". reason>> strBaseAcessTokenn="<<strBaseAcessTokenn
							<<", strJSAPITicket="<<strJSAPITicket);
		return -1;
	}

	return 0;
}

void ServerProcessorToken::process_output()
{

}



