#include "usermark_fsm.h"
#include "global_var.h"
#include "usermark_fsm_state.h"


IMPL_LOGGER(UserMarkFsm, logger)

UserMarkFsm::UserMarkFsm()
{
    _id = 0;
    _conn_id = -1;
    _state = NULL;
    _timer_id = -1;
    set_state(UserMarkState::state_init);
}

UserMarkFsm::~UserMarkFsm()
{

}

void UserMarkFsm::set_state(UserMarkState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void UserMarkFsm::client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	_state->client_req_event(*this, conn, stMsg);
}

void UserMarkFsm::db_reply_event(ExecutorThreadResponseElement& element)
{
	_state->db_reply_event(*this, element);
}

void UserMarkFsm::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void UserMarkFsm::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void UserMarkFsm::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "UserMarkFsm::reset_timer fail, TimerContainer::schedule error");
    }
}

void UserMarkFsm::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int UserMarkFsm::reply_fail(int iErrCode)
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

		CASE_CMD_REQ_RES(::hoosho::msg::QUERY_USER_MARK_REQ, ::hoosho::msg::QUERY_USER_MARK_RES);
		CASE_CMD_REQ_RES(::hoosho::msg::ADD_USER_MARK_REQ, ::hoosho::msg::ADD_USER_MARK_RES);
		CASE_CMD_REQ_RES(::hoosho::msg::DELETE_USER_MARK_REQ, ::hoosho::msg::DELETE_USER_MARK_RES);

#undef CASE_CMD_REQ_RES

	default:
		LOG4CPLUS_ERROR(logger, "UserMarkFsm::reply_time, err, unknown cmd:"<<_msg.head().cmd());
		return -1;
    }
    
    header->set_result(iErrCode);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}


