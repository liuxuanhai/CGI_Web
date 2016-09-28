#include "fsm_resource.h"
#include "global_var.h"
#include "fsm_resource_state.h"
#include "util/lce_util.h"


IMPL_LOGGER(FsmResource, logger)

FsmResource::FsmResource()
{
    _id = 0;
    _conn_id = -1;
    _state = NULL;
    _timer_id = -1;

	_req_msg.Clear();
	
    set_state(FsmResourceState::state_init);
}

FsmResource::~FsmResource()
{

}

void FsmResource::set_state(FsmResourceState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void FsmResource::download_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	_state->download_req_event(*this, conn, stMsg);
}

void FsmResource::download_reply_event(const ExecutorThreadResponseElement& element)
{
	_state->download_reply_event(*this, element);
}

void FsmResource::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void FsmResource::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void FsmResource::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "FsmResource::reset_timer fail, TimerContainer::schedule error");
    }
}

void FsmResource::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int FsmResource::reply_timeout()
{
	hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_req_msg.head().seq());

	if(::hoosho::msg::DOWNLOAD_PIC_RESOURCE_REQ == _req_msg.head().cmd())
	{
		header->set_cmd(::hoosho::msg::DOWNLOAD_PIC_RESOURCE_RES);
	}
	else
	{
		LOG4CPLUS_FATAL(logger, "FsmResource::reply_timeout failed, unknown req_cmd="<<_req_msg.head().cmd());
		return -1;
	}
    
    header->set_result(::hoosho::msg::E_SERVER_TIMEOUT);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}


