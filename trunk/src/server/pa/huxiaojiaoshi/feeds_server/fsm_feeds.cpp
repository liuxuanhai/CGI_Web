#include "fsm_feeds.h"
#include "global_var.h"
#include "fsm_feeds_state.h"


IMPL_LOGGER(FsmFeeds, logger);

FsmFeeds::FsmFeeds()
{
    _id = 0;
    _conn_id = -1;
    _req_type = -1;
    _req_seq_id = -1;
    _state = NULL;
    _timer_id = -1;
    set_state(FsmFeedsState::state_init);
}

FsmFeeds::~FsmFeeds()
{

}

void FsmFeeds::set_state(FsmFeedsState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void FsmFeeds::client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
    _state->client_req_event(*this, conn, stMsg);
}

void FsmFeeds::db_reply_event(const ExecutorThreadResponseElement& element)
{
    _state->db_reply_event(*this, element);
}

void FsmFeeds::timeout_event(void* param)
{
    _state->timeout_event(*this, param);
}

void FsmFeeds::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void FsmFeeds::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "FsmFeeds::reset_timer fail, TimerContainer::schedule error");
    }
}

void FsmFeeds::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int FsmFeeds::reply_timeout()
{
    hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_req_seq_id);
    if(hoosho::msg::J_GET_FEED_LIST_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_GET_FEED_LIST_RES);
    }
    else if(hoosho::msg::J_GET_FEED_DETAIL_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_GET_FEED_DETAIL_RES);
    }
    else if(hoosho::msg::J_ADD_FEED_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_ADD_FEED_RES);
    }
    else if(hoosho::msg::J_GET_FOLLOW_LIST_REQ == _req_type)
    {
    	header->set_cmd(hoosho::msg::J_GET_FOLLOW_LIST_RES);
    }
    else if(hoosho::msg::J_GET_FOLLOW_DETAIL_REQ == _req_type)
    {
    	header->set_cmd(hoosho::msg::J_GET_FOLLOW_DETAIL_RES);
    }
    else if(hoosho::msg::J_ADD_FOLLOW_REQ == _req_type)
    {
    	header->set_cmd(hoosho::msg::J_ADD_FOLLOW_RES);
    }
    else if(hoosho::msg::J_GET_LISTEN_LIST_REQ == _req_type)
    {
    	header->set_cmd(hoosho::msg::J_GET_LISTEN_LIST_RES);
	}
	else if(hoosho::msg::J_GET_LISTEN_DETAIL_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::J_GET_LISTEN_DETAIL_RES);
	}
	else if(hoosho::msg::J_ADD_LISTEN_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::J_ADD_LISTEN_RES);
	}
	else if(hoosho::msg::J_GET_HISTORY_LIST_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::J_GET_HISTORY_LIST_RES);
	}
	else if(hoosho::msg::J_CHECK_LISTEN_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::J_CHECK_LISTEN_RES);
	}
	else if(hoosho::msg::J_GET_COMMENT_FOLLOW_REQ== _req_type)
	{
		header->set_cmd(hoosho::msg::J_GET_COMMENT_FOLLOW_RES);
	}
	else if(hoosho::msg::J_COMMENT_FOLLOW_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::J_COMMENT_FOLLOW_RES);
	}
	else if(hoosho::msg::J_SET_OUT_TRADE_NO_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::J_CHECK_BUSINESS_ID_VALID_RES);
	}
	else if(hoosho::msg::J_CHECK_BUSINESS_ID_VALID_REQ == _req_type)
	{
		header->set_cmd(hoosho::msg::J_CHECK_BUSINESS_ID_VALID_RES);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "unknow cmd, or unfinish code, cmd = " << _req_type);
	}

	
    header->set_result(hoosho::msg::E_SERVER_TIMEOUT);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}


