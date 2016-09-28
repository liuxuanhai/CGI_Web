#include "ffmpeg_fsm.h"
#include "global_var.h"
#include "ffmpeg_fsm_state.h"


IMPL_LOGGER(FfmpegFsm, logger)

FfmpegFsm::FfmpegFsm()
{
    _id = 0;
    _conn_id = -1;
    _state = NULL;
    _timer_id = -1;
    set_state(FfmpegState::state_init);
}

FfmpegFsm::~FfmpegFsm()
{

}

void FfmpegFsm::set_state(FfmpegState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void FfmpegFsm::client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	_state->client_req_event(*this, conn, stMsg);
}

void FfmpegFsm::db_reply_event(ExecutorThreadResponseElement& element)
{
	_state->db_reply_event(*this, element);
}

void FfmpegFsm::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void FfmpegFsm::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void FfmpegFsm::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "FfmpegFsm::reset_timer fail, TimerContainer::schedule error");
    }
}

void FfmpegFsm::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int FfmpegFsm::reply_fail(int iErrCode)
{
	hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_msg.head().seq());
	switch(_msg.head().cmd())
    {
#define CASE_CMD_REQ_RES(reqcmd, rescmd) \
	case reqcmd: \
		header->set_cmd(rescmd); \
		break;

 		CASE_CMD_REQ_RES(::hoosho::msg::J_TRANSCODE_REQ, ::hoosho::msg::J_TRANSCODE_RES);

#undef CASE_CMD_REQ_RES

	default:
		LOG4CPLUS_ERROR(logger, "FfmpegFsm::reply_time, err, unknown cmd:"<<_msg.head().cmd());
		return -1;
    }
    
    header->set_result(iErrCode);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}


