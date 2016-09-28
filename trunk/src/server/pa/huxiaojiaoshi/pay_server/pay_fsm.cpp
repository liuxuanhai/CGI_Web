#include "pay_fsm.h"
#include "global_var.h"
#include "pay_fsm_state.h"


IMPL_LOGGER(PayFsm, logger)

PayFsm::PayFsm()
{
    _id = 0;
    _conn_id = -1;
    _state = NULL;
    _timer_id = -1;
    set_state(PayState::state_init);
}

PayFsm::~PayFsm()
{

}

void PayFsm::set_state(PayState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void PayFsm::client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	_state->client_req_event(*this, conn, stMsg);
}

void PayFsm::db_reply_event(ExecutorThreadResponseElement& element)
{
	_state->db_reply_event(*this, element);
}

void PayFsm::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void PayFsm::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void PayFsm::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "PayFsm::reset_timer fail, TimerContainer::schedule error");
    }
}

void PayFsm::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int PayFsm::reply_fail(int iErrCode)
{
	hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_seq);
	switch(_cmd)
    {
#define CASE_CMD_REQ_RES(reqcmd, rescmd) \
	case reqcmd: \
		header->set_cmd(rescmd); \
		break;

 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_UNIFIEDORDER_REQ, ::hoosho::msg::J_PAY_UNIFIEDORDER_RES);
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_TRANSFER_REQ, ::hoosho::msg::J_PAY_TRANSFER_RES);
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_GET_USER_CASH_REQ, ::hoosho::msg::J_PAY_GET_USER_CASH_RES);
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_QUERY_CASH_FLOW_REQ, ::hoosho::msg::J_PAY_QUERY_CASH_FLOW_RES);
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_UNIFIEDORDER_CALLBACK_REQ, ::hoosho::msg::J_PAY_UNIFIEDORDER_CALLBACK_RES);
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_UNIFIEDORDER_QUERY_REQ, ::hoosho::msg::J_PAY_UNIFIEDORDER_QUERY_RES);
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_LISTEN_INCOME_REQ, ::hoosho::msg::J_PAY_LISTEN_INCOME_RES);
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_SENDREDPACK_REQ, ::hoosho::msg::J_PAY_SENDREDPACK_RES); 
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_OUTCOME_QUERY_REQ, ::hoosho::msg::J_PAY_OUTCOME_QUERY_RES);
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_OUTCOME_COMMIT_REQ, ::hoosho::msg::J_PAY_OUTCOME_COMMIT_RES);
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_OUTCOME_CHECK_REQ, ::hoosho::msg::J_PAY_OUTCOME_CHECK_RES);
 		CASE_CMD_REQ_RES(::hoosho::msg::J_PAY_OUTCOME_UPDATE_REQ, ::hoosho::msg::J_PAY_OUTCOME_UPDATE_RES);

#undef CASE_CMD_REQ_RES

	default:
		LOG4CPLUS_ERROR(logger, "PayFsm::reply_time, err, unknown cmd:"<<_cmd);
		return -1;
    }
    
    header->set_result(iErrCode);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}


