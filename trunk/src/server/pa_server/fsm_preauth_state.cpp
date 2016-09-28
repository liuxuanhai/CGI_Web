#include "fsm_preauth_state.h"
#include "global_var.h"

    /*******************************************************************************************************/
    /*					                       FsmPreAuthState						       					   */
    /*******************************************************************************************************/
    IMPL_LOGGER(FsmPreAuthState, logger);

#define IMPL_FSM_STATE(classname, name) classname FsmPreAuthState::name(#classname)
    IMPL_FSM_STATE(FsmPreAuthStateInit, state_init);
    IMPL_FSM_STATE(FsmPreAuthStateWaitThreadPreAuth, state_wait_thread_pre_auth);
    IMPL_FSM_STATE(FsmPreAuthStateEnd, state_end);
#undef IMPL_FSM_STATE

    void FsmPreAuthState::enter(FsmPreAuth& fsm)
    {

    }

    void FsmPreAuthState::exit(FsmPreAuth& fsm)
    {

    }

    void FsmPreAuthState::req_preauth_event(FsmPreAuth & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        LOG4CPLUS_DEBUG(logger, "default req_preauth_event, state: "<<this->name());
    }

	void FsmPreAuthState::preauth_reply_event(FsmPreAuth& fsm, const ExecutorThreadResponseElement& element)
    {
		LOG4CPLUS_DEBUG(logger, "default preauth_reply_event, state: "<<this->name());
    }

    void FsmPreAuthState::timeout_event(FsmPreAuth& fsm, void* param)
    {
		LOG4CPLUS_DEBUG(logger, "default timeout_event, state: "<<this->name());
    }

    /*******************************************************************************************************/
    /*                                                                                  FsmPreAuthStateInit                                                                                         */
    /*******************************************************************************************************/
    void FsmPreAuthStateInit::req_preauth_event(FsmPreAuth& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();
        fsm._req_seq_id = stMsg.head().seq();
        fsm._req_preauth_code = stMsg.parse_pre_auth_code_req().pre_auth_code();

		//if not exsts in cache, query db
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PREAUTH, FsmContainer<int>::FSM_TYPE_PREAUTH, fsm._id);
		req.need_reply();
		req.m_pre_auth_code = fsm._req_preauth_code;
		req.m_pa_appid = HX_PA_APPID;
		req.m_pa_appsecret = HX_PA_APPSECRET;
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmPreAuthState::state_wait_thread_pre_auth);
        return;		
    }


    /*******************************************************************************************************/
    /*                                                                           FsmPreAuthStateWaitThreadPreAuth                                                                         */
    /*******************************************************************************************************/
    void FsmPreAuthStateWaitThreadPreAuth::enter(FsmPreAuth& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmPreAuthStateWaitThreadPreAuth::exit(FsmPreAuth& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmPreAuthStateWaitThreadPreAuth::preauth_reply_event(FsmPreAuth& fsm, const ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		//resp msg
		::hoosho::msg::Msg stRespMsg;
        ::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_cmd(hoosho::msg::PARSE_PRE_AUTH_CODE_RES);
        header->set_seq(fsm._req_seq_id);
        header->set_result(::hoosho::msg::E_OK);

		if(reply.m_result_code != ExecutorThreadRequestType::E_OK)
		{
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmPreAuthState::state_end);
			return;
		}

		const common::wxapi::WXUserInfo& stWxUserInfo = reply.m_wx_user_info;
		::hoosho::pa::ParsePreAuthCodeRes* pParsePreAuthCodeRes = stRespMsg.mutable_parse_pre_auth_code_res();
		::hoosho::commstruct::UserDetailInfo* pUserDetailInfo = pParsePreAuthCodeRes->mutable_user_detail_info();
		FsmPreAuth::wxuserinfo_to_pbuserinfo(stWxUserInfo, pUserDetailInfo);
		g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(FsmPreAuthState::state_end);
		
		return;
    }

    void FsmPreAuthStateWaitThreadPreAuth::timeout_event(FsmPreAuth& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmPreAuthStateWaitThreadPreAuth time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmPreAuthState::state_end);
        return;
    }


    /*******************************************************************************************************/
    /*                                                                                     FsmPreAuthStateEnd                                                                                     */
    /*******************************************************************************************************/
    void FsmPreAuthStateEnd::enter(FsmPreAuth& fsm)
    {
        g_fsm_preauth_container->del_fsm(&fsm);
    }

