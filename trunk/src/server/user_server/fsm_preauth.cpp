#include "fsm_preauth.h"
#include "global_var.h"
#include "fsm_preauth_state.h"
#include "util/lce_util.h"


IMPL_LOGGER(FsmPreAuth, logger)

FsmPreAuth::FsmPreAuth()
{
    _id = 0;
    _conn_id = -1;
    _req_seq_id = -1;
    _state = NULL;
    _timer_id = -1;

	_req_seq_id = -1;
	_req_preauth_code = "";
	
    set_state(FsmPreAuthState::state_init);
}

FsmPreAuth::~FsmPreAuth()
{

}

void FsmPreAuth::set_state(FsmPreAuthState& state)
{
    if(_state)
    {
        _state->exit(*this);
    }
    _state = &state;
    _state->enter(*this);
}

void FsmPreAuth::req_preauth_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
	_state->req_preauth_event(*this, conn, stMsg);
}

void FsmPreAuth::preauth_reply_event(const ::hoosho::msg::Msg& stMsg)
{
	_state->preauth_reply_event(*this, stMsg);
}

void FsmPreAuth::user_info_get_reply_event(ExecutorThreadResponseElement& element)
{
	_state->user_info_get_reply_event(*this, element);
}

void FsmPreAuth::apply_hoosho_no_reply_event(const ::hoosho::msg::Msg& stMsg)
{
	_state->apply_hoosho_no_reply_event(*this, stMsg);
}

void FsmPreAuth::user_info_update_reply_event(const ExecutorThreadResponseElement& element)
{
	_state->user_info_update_reply_event(*this, element);
}

void FsmPreAuth::timeout_event(void* param)
{
	_state->timeout_event(*this, param);
}

void FsmPreAuth::cancel_timer()
{
    if(_timer_id >= 0)
    {
        g_timer_container->cancel(_timer_id);
        _timer_id = -1;
    }
}

void FsmPreAuth::reset_timer(int wait_time)
{
    cancel_timer();
    _timer_id = g_timer_container->schedule(*this, wait_time, 0, NULL);

    if(_timer_id < 0)
    {
        LOG4CPLUS_FATAL(logger, "FsmPreAuth::reset_timer fail, TimerContainer::schedule error");
    }
}

void FsmPreAuth::handle_timeout(void* param)
{
    this->timeout_event(param);
}

int FsmPreAuth::reply_fail(int iErrCode)
{
	hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_req_seq_id);
    header->set_cmd(::hoosho::msg::USER_PARSE_PRE_AUTH_CODE_RES);
    header->set_result(iErrCode);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}

int FsmPreAuth::reply_ok()
{
	hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(_req_seq_id);
    header->set_cmd(::hoosho::msg::USER_PARSE_PRE_AUTH_CODE_RES);
    header->set_result(::hoosho::msg::E_OK);
    ::hoosho::user::UserParsePreAuthCodeRes* pUserParsePreAuthCodeRes = stRespMsg.mutable_user_parse_pre_auth_code_res();
    ::hoosho::commstruct::UserDetailInfo* pUserDetailInfo = pUserParsePreAuthCodeRes->mutable_user_detail_info();
    pUserDetailInfo->CopyFrom(_user_detail_info);

    return g_client_processor->send_datagram(_conn_id, stRespMsg);
}



