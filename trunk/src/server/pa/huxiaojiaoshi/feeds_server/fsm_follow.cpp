#include "fsm_follow.h"
#include "global_var.h"
#include "fsm_follow_state.h"


IMPL_LOGGER(FsmFollow, logger);

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

void FsmFollow::query_db_reply_event(const ExecutorThreadResponseElement &element)
{
    _state->query_db_reply_event(*this, element);
}

void FsmFollow::check_out_trade_no_valid_reply_event(const hoosho::msg::Msg &stMsg)
{
    _state->check_out_trade_no_valid_reply_event(*this, stMsg);
}

void FsmFollow::check_out_trade_no_unique_reply_event(const ExecutorThreadResponseElement &element)
{
    _state->check_out_trade_no_unique_reply_event(*this, element);
}

void FsmFollow::transcode_reply_event(const hoosho::msg::Msg &stMsg)
{
    _state->transcode_reply_event(*this, stMsg);
}

void FsmFollow::insert_into_db_reply_event(const ExecutorThreadResponseElement& element)
{
    _state->insert_into_db_reply_event(*this, element);
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
        LOG4CPLUS_FATAL(logger, "FsmFollow::reset_timer fail, TimerContainer::schedule error");
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
    if(hoosho::msg::J_ADD_FOLLOW_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_ADD_FOLLOW_RES);
    }
    else if(hoosho::msg::J_ADD_LISTEN_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_ADD_LISTEN_RES);
    }
    else if(hoosho::msg::J_GET_LISTEN_DETAIL_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_GET_COMMENT_FOLLOW_RES);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "unknow cmd, or unfinish code, cmd = " << _req_type);
    }


    header->set_result(hoosho::msg::E_SERVER_TIMEOUT);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}

int FsmFollow::reply_fail(uint32_t dwErrcode)
{
    hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_req_seq_id);
    if(hoosho::msg::J_ADD_FOLLOW_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_ADD_FOLLOW_RES);
    }
    else if(hoosho::msg::J_ADD_LISTEN_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_ADD_LISTEN_RES);
    }

    header->set_result(dwErrcode);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}

int FsmFollow::reply_ok()
{
    LOG4CPLUS_DEBUG(logger, "FsmFollow reply_ok");
    hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_req_seq_id);
    if(hoosho::msg::J_ADD_FOLLOW_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_ADD_FOLLOW_RES);
    }
    else if(hoosho::msg::J_ADD_LISTEN_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_ADD_LISTEN_RES);
    }
    else if(hoosho::msg::J_GET_LISTEN_DETAIL_REQ == _req_type)
    {
        header->set_cmd(hoosho::msg::J_GET_LISTEN_DETAIL_RES);
        hoosho::j::feeds::GetListenDetailRes* pGetListenDetailRes = stRespMsg.mutable_j_feeds_get_listen_detail_res();
        hoosho::j::commstruct::ListenInfo* pListenInfo = pGetListenDetailRes->mutable_listen_info();
        hoosho::j::commstruct::FollowInfo* pFollowInfo = pGetListenDetailRes->mutable_follow_info();

        pListenInfo->CopyFrom(_listen_info);
        pFollowInfo->CopyFrom(_follow_info);
    }

    header->set_result(hoosho::msg::E_OK);

    LOG4CPLUS_DEBUG(logger, "conid = " << _conn_id);
    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}


