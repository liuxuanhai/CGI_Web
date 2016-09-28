#include "user_info_fsm.h"
#include "global_var.h"
#include "user_info_fsm_state.h"


IMPL_LOGGER(UserInfoFsm, logger)

UserInfoFsm::UserInfoFsm()
{
    _id = 0;
    _conn_id = -1;
    _req_type = -1;
    _req_seq_id = -1;
    _state = NULL;
    _timer_id = -1;
    set_state(UserInfoFsmState::state_init);
}

UserInfoFsm::~UserInfoFsm()
{

}

void UserInfoFsm::set_state(UserInfoFsmState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void UserInfoFsm::client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	_state->client_req_event(*this, conn, stMsg);
}

void UserInfoFsm::db_reply_event(const ExecutorThreadResponseElement& element)
{
	_state->db_reply_event(*this, element);
}

void UserInfoFsm::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void UserInfoFsm::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void UserInfoFsm::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "UserInfoFsm::reset_timer fail, TimerContainer::schedule error");
    }
}

void UserInfoFsm::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int UserInfoFsm::reply_timeout()
{
	hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_req_seq_id);
    if(hoosho::msg::QUERY_USER_INFO_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::QUERY_USER_INFO_RES);
    }
    else if(hoosho::msg::ADD_USER_INFO_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::ADD_USER_INFO_RES);
    }
    else if(hoosho::msg::UPDATE_USER_INFO_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::UPDATE_USER_INFO_RES);
    }
    else if(hoosho::msg::DELETE_USER_INFO_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::DELETE_USER_INFO_RES);
    }
    
    header->set_result(-1);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}


