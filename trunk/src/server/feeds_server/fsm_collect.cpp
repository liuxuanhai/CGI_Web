#include "fsm_collect.h"
#include "global_var.h"
#include "fsm_collect_state.h"


IMPL_LOGGER(FsmCollect, logger)

FsmCollect::FsmCollect()
{
    _id = 0;
    _conn_id = -1;
    _req_type = -1;
    _req_seq_id = -1;
    _state = NULL;
    _timer_id = -1;
    set_state(FsmCollectState::state_init);
}

FsmCollect::~FsmCollect()
{

}

void FsmCollect::set_state(FsmCollectState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void FsmCollect::client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	_state->client_req_event(*this, conn, stMsg);
}

void FsmCollect::db_reply_event(const ExecutorThreadResponseElement& element)
{
	_state->db_reply_event(*this, element);
}

void FsmCollect::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void FsmCollect::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void FsmCollect::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "FsmFeed::reset_timer fail, TimerContainer::schedule error");
    }
}

void FsmCollect::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int FsmCollect::reply_timeout()
{
	hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_req_seq_id);
	if(hoosho::msg::QUERY_FEED_LIST_COLLECT_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::QUERY_FEED_LIST_COLLECT_RES);
	}
    else if(hoosho::msg::QUERY_COLLECT_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::QUERY_COLLECT_RES);
    }
    else if(hoosho::msg::ADD_COLLECT_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::ADD_COLLECT_RES);
    }
    else if(hoosho::msg::DELETE_COLLECT_REQ == _req_type)
    {
		header->set_cmd(hoosho::msg::DELETE_COLLECT_RES);
    }
    
    header->set_result(hoosho::msg::E_SERVER_TIMEOUT);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}


