#include "fsm_business.h"
#include "global_var.h"
#include "fsm_business_state.h"
#include "util/lce_util.h"


IMPL_LOGGER(FsmBusiness, logger)

FsmBusiness::FsmBusiness()
{
    _id = 0;
    _conn_id = -1;
    _state = NULL;
    _timer_id = -1;

    _msg.Clear();  	
    _self_user_info.Clear();
	
    set_state(FsmBusinessState::state_init);
}

FsmBusiness::~FsmBusiness()
{

}

void FsmBusiness::set_state(FsmBusinessState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void FsmBusiness::client_req_event(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	_state->client_req_event(*this, conn, msg);
}

void FsmBusiness::thread_reply_event(ExecutorThreadResponseElement& element)
{
	_state->thread_reply_event(*this, element);
}

void FsmBusiness::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void FsmBusiness::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void FsmBusiness::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "FsmBusiness::reset_timer fail, TimerContainer::schedule error");
    }
}

void FsmBusiness::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int FsmBusiness::reply_fail(int iErrCode)
{
	::hoosho::msg::Msg stRespMsg;
    ::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_result(iErrCode);
    header->set_seq(_msg.head().seq());
    header->set_cmd(::hoosho::msg::Z_PROJECT_RES);

    ::hoosho::msg::z::MsgRes* pZMsgRes = stRespMsg.mutable_z_msg_res();
    pZMsgRes->set_sub_cmd(_msg.z_msg_req().sub_cmd() + 1);
    
    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}



