#include "fsm_follow.h"
#include "global_var.h"
#include "fsm_follow_state.h"


IMPL_LOGGER(FsmFollow, logger)

FsmFollow::FsmFollow()
{
    _id = 0;
    _conn_id = -1;
    _req_type = -1;
    _req_seq_id = -1;
    _state = NULL;
    _timer_id = -1;
    set_state(FsmFollowState::state_init);
}

FsmFollow::~FsmFollow()
{

}

void FsmFollow::set_state(FsmFollowState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void FsmFollow::client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	_state->client_req_event(*this, conn, stMsg);
}

void FsmFollow::db_reply_event(const ExecutorThreadResponseElement& element)
{
	_state->db_reply_event(*this, element);
}

void FsmFollow::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void FsmFollow::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void FsmFollow::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "FsmFeed::reset_timer fail, TimerContainer::schedule error");
    }
}

void FsmFollow::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int FsmFollow::reply_timeout()
{
	hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_req_seq_id);
    if(hoosho::msg::QUERY_FOLLOW_COMMENT_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::QUERY_FOLLOW_COMMENT_RES);
    }
	else if(hoosho::msg::QUERY_FOLLOW_DETAIL_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::QUERY_FOLLOW_DETAIL_REQ);
    }
	else if(hoosho::msg::QUERY_FOLLOW_LIST_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::QUERY_FOLLOW_LIST_REQ);
    }
	else if(hoosho::msg::ADD_FOLLOW_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::ADD_FOLLOW_RES);
    }
//    else if(hoosho::msg::UPDATE_USER_INFO_REQ == _req_type)
//    {
//		header->set_cmd(hoosho::msg::UPDATE_USER_INFO_RES);
//    }
    else if(hoosho::msg::DELETE_FOLLOW_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::DELETE_FOLLOW_RES);
    }
    
    header->set_result(hoosho::msg::E_SERVER_TIMEOUT);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}

