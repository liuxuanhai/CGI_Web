#include "msg_fsm.h"
#include "global_var.h"
#include "msg_fsm_state.h"


IMPL_LOGGER(MsgFsm, logger)

MsgFsm::MsgFsm()
{
    _id = 0;
    _conn_id = -1;
    _state = NULL;
    _timer_id = -1;

    _msg.Clear();
    
    set_state(MsgFsmState::state_init);
}

MsgFsm::~MsgFsm()
{

}

void MsgFsm::set_state(MsgFsmState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void MsgFsm::client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	_state->client_req_event(*this, conn, stMsg);
}

void MsgFsm::db_reply_event(ExecutorThreadResponseElement& element)
{
	_state->db_reply_event(*this, element);
}

void MsgFsm::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void MsgFsm::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void MsgFsm::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "MsgFsm::reset_timer fail, TimerContainer::schedule error");
    }
}

void MsgFsm::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int MsgFsm::reply_timeout()
{
	::hoosho::msg::Msg stRespMsg;
    ::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_msg.head().seq());
    switch(_msg.head().cmd())
    {
#define CASE_CMD_REQ_RES(reqcmd, rescmd) \
	case reqcmd: \
		header->set_cmd(rescmd); \
		break;

		CASE_CMD_REQ_RES(::hoosho::msg::QUERY_MSG_REQ, ::hoosho::msg::QUERY_MSG_RES);
		CASE_CMD_REQ_RES(::hoosho::msg::ADD_MSG_REQ, ::hoosho::msg::ADD_MSG_RES);
		CASE_CMD_REQ_RES(::hoosho::msg::DELETE_MSG_REQ, ::hoosho::msg::DELETE_MSG_RES);

#undef CASE_CMD_REQ_RES

	default:
		LOG4CPLUS_ERROR(logger, "MsgFsm::reply_time, err, unknown cmd:"<<_msg.head().cmd());
		return -1;
    }
    
    header->set_result(::hoosho::msg::E_SERVER_TIMEOUT);
    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}


