#include "sys_notice_fsm.h"
#include "global_var.h"
#include "sys_notice_fsm_state.h"

IMPL_LOGGER(SysNoticeFsm, logger)

SysNoticeFsm::SysNoticeFsm()
{
	_id = 0;
	_conn_id = -1;
	_req_type = -1;
	_req_seq_id = -1;
	_state = NULL;
	_timer_id = -1;
	set_state(SysNoticeFsmState::state_init);
}

SysNoticeFsm::~SysNoticeFsm()
{

}

void SysNoticeFsm::set_state(SysNoticeFsmState& state)
{
	if (_state)
	{
		_state->exit(*this);
	}
	_state = &state;
	_state->enter(*this);
}

void SysNoticeFsm::client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	_state->client_req_event(*this, conn, stMsg);
}

void SysNoticeFsm::db_reply_event(const ExecutorThreadResponseElement& element)
{
	_state->db_reply_event(*this, element);
}

void SysNoticeFsm::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void SysNoticeFsm::cancel_timer()
{
	if (_timer_id >= 0)
	{
		g_timer_container->cancel(_timer_id);
		_timer_id = -1;
	}
}

void SysNoticeFsm::reset_timer(int wait_time)
{
	cancel_timer();
	_timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

	if (_timer_id < 0)
	{
		LOG4CPLUS_FATAL(logger, "SysNoticeFsm::reset_timer fail, TimerContainer::schedule error");
	}
}

void SysNoticeFsm::handle_timeout(void* param)
{
	this->timeout_event(param);
}

int SysNoticeFsm::reply_timeout()
{
	hoosho::msg::Msg stRespMsg;
	hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	header->set_seq(_req_seq_id);
	//---------------------------------------sys_notice--------------------------------------------------//
	if (hoosho::msg::QUERY_SYS_NOTICE_ALL_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::QUERY_SYS_NOTICE_ALL_REQ);
	}
	else if (hoosho::msg::QUERY_SYS_NOTICE_NEW_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::QUERY_SYS_NOTICE_NEW_REQ);
	}
	else if (hoosho::msg::UPDATE_SYS_NOTICE_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::UPDATE_SYS_NOTICE_RES);
	}
	else if (hoosho::msg::UPDATE_SYS_NOTICE_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::UPDATE_SYS_NOTICE_RES);
	}
	else if (hoosho::msg::DELETE_SYS_NOTICE_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::DELETE_SYS_NOTICE_RES);
	}
//-----------------------------------------------------notice_notify---------------------------------------------//
	else if (hoosho::msg::QUERY_NOTICE_NOTIFY_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::QUERY_NOTICE_NOTIFY_RES);

	}
	else if (hoosho::msg::QUERY_NOTICE_NOTIFY_TYPELIST_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::QUERY_NOTICE_NOTIFY_TYPELIST_RES);

	}
	//------------------------------------------------------notice_record-------------------------------------------//
	else if (hoosho::msg::QUERY_NOTICE_RECORD_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::QUERY_NOTICE_RECORD_RES);
	}
	else if (hoosho::msg::UPDATE_NOTICE_RECORD_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::UPDATE_NOTICE_RECORD_RES);
	}
	else if (hoosho::msg::ADD_NOTICE_RECORD_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::ADD_NOTICE_RECORD_RES);
	}
	else if (hoosho::msg::DELETE_NOTICE_RECORD_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::DELETE_NOTICE_RECORD_RES);
	}
	else if (hoosho::msg::READ_NOTICE_RECORD_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::READ_NOTICE_RECORD_RES);
	}

	header->set_result(::hoosho::msg::E_SERVER_TIMEOUT);

	return g_client_processor->send_datagram(_conn_id, stRespMsg);
}

