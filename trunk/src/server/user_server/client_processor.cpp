#include "client_processor.h"
#include "global_var.h"
#include "util/lce_util.h"
#include "executor_thread_queue.h"

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

		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ, process_user_detail_info_get);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::UPDATE_USER_DETAIL_INFO_REQ, process_user_detail_info_update);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::QUERY_USER_FANS_REQ, process_user_fans_get);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::QUERY_USER_FANS_NUM_REQ, process_user_fans_num_get);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::QUERY_USER_FOLLOWS_REQ, process_user_follows_get);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::USER_FOLLOW_REQ, process_user_follow);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::QUERY_USER_POWER_REQ, process_user_power_get);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::USER_PARSE_PRE_AUTH_CODE_REQ, process_pre_auth_code);
		
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::QUERY_USER_ZOMBIE_REQ, process_user_zombies_get);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::QUERY_USER_ZOMANAGER_REQ, process_user_zomanager_get);
		
		
#undef CLIENT_PROCESSOR_CMD_HANDLER

		default:
			LOG4CPLUS_ERROR(logger, "ClientProcessor::process_input fail, unknown cmd="<<dwCmd);
			conn.close();
			break;
	}


	return;
}

void ClientProcessor::process_pre_auth_code(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmPreAuth* fsm = g_fsm_preauth_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_pre_auth_code fail, new FsmPreAuth failed");
		conn.close();
		return;
	}

	fsm->req_preauth_event(conn, msg);
    return;
}




void ClientProcessor::process_user_detail_info_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_detail_info_get fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_user_power_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_power_get fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}


void ClientProcessor::process_user_detail_info_update(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_detail_info_update fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_user_follow(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_follow fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_user_fans_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_fans_get fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_user_fans_num_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_fans_num_get fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_user_follows_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_follows_get fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_user_zombies_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_follows_get fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_user_zomanager_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_follows_get fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
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


