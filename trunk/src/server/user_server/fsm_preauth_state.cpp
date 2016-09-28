#include "fsm_preauth_state.h"
#include "global_var.h"

    /*******************************************************************************************************/
    /*					                       FsmPreAuthState						       					   */
    /*******************************************************************************************************/
    IMPL_LOGGER(FsmPreAuthState, logger);

#define IMPL_FSM_STATE(classname, name) classname FsmPreAuthState::name(#classname)
    IMPL_FSM_STATE(FsmPreAuthStateInit, state_init);
    IMPL_FSM_STATE(FsmPreAuthStateWaitPreauthReply, state_wait_preauth_reply);
    IMPL_FSM_STATE(FsmPreAuthStateWaitUserInfoGetReply, state_wait_user_info_get_reply);
    IMPL_FSM_STATE(FsmPreAuthStateWaitApplyHooshoNOReply, state_wait_apply_hoosho_no_reply);
    IMPL_FSM_STATE(FsmPreAuthStateWaitUserInfoUpdateReply, state_wait_user_info_update_reply);
    IMPL_FSM_STATE(FsmPreAuthStateEnd, state_end);
#undef IMPL_FSM_STATE

    void FsmPreAuthState::enter(FsmPreAuth& fsm)
    {

    }
    void FsmPreAuthState::exit(FsmPreAuth& fsm)
    {

    }
	void FsmPreAuthState::req_preauth_event(FsmPreAuth& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
	{
		LOG4CPLUS_DEBUG(logger, "default req_preauth_event, state: "<<this->name());
	}
	void FsmPreAuthState::preauth_reply_event(FsmPreAuth& fsm, const ::hoosho::msg::Msg& stMsg)
	{
		LOG4CPLUS_DEBUG(logger, "default preauth_reply_event, state: "<<this->name());
	}
	void FsmPreAuthState::user_info_get_reply_event(FsmPreAuth& fsm, ExecutorThreadResponseElement& element)
	{
		LOG4CPLUS_DEBUG(logger, "default user_info_get_reply_event, state: "<<this->name());
	}
	void FsmPreAuthState::apply_hoosho_no_reply_event(FsmPreAuth& fsm, const ::hoosho::msg::Msg& stMsg)
	{
		LOG4CPLUS_DEBUG(logger, "default apply_hoosho_no_reply_event, state: "<<this->name());
	}
	void FsmPreAuthState::user_info_update_reply_event(FsmPreAuth& fsm, const ExecutorThreadResponseElement& element)
	{
		LOG4CPLUS_DEBUG(logger, "default user_info_update_reply_event, state: "<<this->name());
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
        fsm._req_preauth_code = stMsg.user_parse_pre_auth_code_req().pre_auth_code();

		::hoosho::msg::Msg stPARequest;
		::hoosho::msg::MsgHead* pPARequestHead = stPARequest.mutable_head();
		pPARequestHead->set_cmd(::hoosho::msg::PARSE_PRE_AUTH_CODE_REQ);
		pPARequestHead->set_seq(fsm._id);
		::hoosho::pa::ParsePreAuthCodeReq* pPAParsePreAuthCodeReq = stPARequest.mutable_parse_pre_auth_code_req();
		pPAParsePreAuthCodeReq->set_pre_auth_code(fsm._req_preauth_code);
		g_server_processor_pa->send_datagram(stPARequest);

        fsm.set_state(FsmPreAuthState::state_wait_preauth_reply);
        return;		
    }

	/*******************************************************************************************************/
    /*                                                                                  FsmPreAuthStateWaitPreauthReply                                                                  */
    /*******************************************************************************************************/
	void FsmPreAuthStateWaitPreauthReply::enter(FsmPreAuth& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }
    void FsmPreAuthStateWaitPreauthReply::exit(FsmPreAuth& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmPreAuthStateWaitPreauthReply::preauth_reply_event(FsmPreAuth & fsm, const hoosho::msg::Msg & stMsg)
    {
        //cancel timer first
        fsm.cancel_timer();

        //pa return err
		if(stMsg.head().result() != ::hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "FsmPreAuthStateWaitPreauthReply::preauth_reply_event failed, recv msg.result="<<stMsg.head().result());
			fsm.reply_fail(stMsg.head().result());
			fsm.set_state(state_end);
			return;
		}

		//keep inner result
		fsm._user_detail_info.Clear();
		fsm._user_detail_info.CopyFrom(stMsg.parse_pre_auth_code_res().user_detail_info());

		//query userinfo
		ExecutorThreadRequestElement stQueryUserInfoRequest(ExecutorThreadRequestType::T_USERINFO_QUERY
														, FsmContainer<int>::FSM_TYPE_PREAUTH
														, fsm._id);
		stQueryUserInfoRequest.need_reply();
		stQueryUserInfoRequest.m_user_detail_info_from.CopyFrom(fsm._user_detail_info);
		g_executor_thread_processor->send_request(stQueryUserInfoRequest);
		fsm.set_state(state_wait_user_info_get_reply);

		return;
    }

    void FsmPreAuthStateWaitPreauthReply::timeout_event(FsmPreAuth& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmPreAuthStateWaitPreauthReply time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_fail(::hoosho::msg::E_SERVER_TIMEOUT);
        fsm.set_state(FsmPreAuthState::state_end);
        return;
    }
    /*******************************************************************************************************/
    /*                                                                                  FsmPreAuthStateWaitUserInfoGetReply                                                             */
    /*******************************************************************************************************/
	void FsmPreAuthStateWaitUserInfoGetReply::enter(FsmPreAuth& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }
    void FsmPreAuthStateWaitUserInfoGetReply::exit(FsmPreAuth& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmPreAuthStateWaitUserInfoGetReply::user_info_get_reply_event(FsmPreAuth & fsm, ExecutorThreadResponseElement & element)
    {
        //cancel timer first
        fsm.cancel_timer();

        //thread return not ok
		if(element.m_result_code != ExecutorThreadRequestType::E_OK)
		{
			//err
			if(element.m_result_code != ExecutorThreadRequestType::E_NOT_EXIST)
			{
				LOG4CPLUS_ERROR(logger, "FsmPreAuthStateWaitUserInfoGetReply::user_info_get_reply_event failed, recv thread.result="<<element.m_result_code);
				fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);
				fsm.set_state(state_end);
				return;	
			}

			//not exists, create one hoosho no
			::hoosho::msg::Msg stConfigRequest;
			::hoosho::msg::MsgHead* pConfigRequestHead = stConfigRequest.mutable_head();
			pConfigRequestHead->set_cmd(::hoosho::msg::HOOSHO_NO_APPLY_REQ);
			pConfigRequestHead->set_seq(fsm._id);
			g_server_processor_config->send_datagram(stConfigRequest);

        	fsm.set_state(FsmPreAuthState::state_wait_apply_hoosho_no_reply);
        	return;		
		}

		//update wx info to userinfo
		::hoosho::commstruct::UserDetailInfo& stDBUserDetailInfo = element.m_user_detail_info;
		stDBUserDetailInfo.set_portrait_pic_url_wx(fsm._user_detail_info.portrait_pic_url_wx());
		stDBUserDetailInfo.set_nick_wx(fsm._user_detail_info.nick_wx());
		stDBUserDetailInfo.set_addr_country(fsm._user_detail_info.addr_country());
		stDBUserDetailInfo.set_addr_province(fsm._user_detail_info.addr_province());
		stDBUserDetailInfo.set_addr_city(fsm._user_detail_info.addr_city());
		stDBUserDetailInfo.set_sex(fsm._user_detail_info.sex());

		//keep newest userinfo data
		fsm._user_detail_info.Clear();
		fsm._user_detail_info.CopyFrom(stDBUserDetailInfo);
		
		//update userinfo to db
		ExecutorThreadRequestElement stUpdateUserInfoRequest(ExecutorThreadRequestType::T_USERINFO_UPDATE
														, FsmContainer<int>::FSM_TYPE_PREAUTH
														, fsm._id);
		stUpdateUserInfoRequest.need_reply();
		stUpdateUserInfoRequest.m_user_detail_info_from.CopyFrom(fsm._user_detail_info);
		g_executor_thread_processor->send_request(stUpdateUserInfoRequest);
		fsm.set_state(state_wait_user_info_update_reply);
		return;
    }

    void FsmPreAuthStateWaitUserInfoGetReply::timeout_event(FsmPreAuth& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmPreAuthStateWaitUserInfoGetReply time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_fail(::hoosho::msg::E_SERVER_TIMEOUT);
        fsm.set_state(FsmPreAuthState::state_end);
        return;
    }
    
    /*******************************************************************************************************/
    /*                                                                                  FsmPreAuthStateWaitApplyHooshoNOReply                                                          */
    /*******************************************************************************************************/
	void FsmPreAuthStateWaitApplyHooshoNOReply::enter(FsmPreAuth& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }
    void FsmPreAuthStateWaitApplyHooshoNOReply::exit(FsmPreAuth& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmPreAuthStateWaitApplyHooshoNOReply::apply_hoosho_no_reply_event(FsmPreAuth& fsm, const::hoosho::msg::Msg& stMsg)
    {
        //cancel timer first
        fsm.cancel_timer();

		//config return err
		if(stMsg.head().result() != ::hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "FsmPreAuthStateWaitApplyHooshoNOReply::apply_hoosho_no_reply_event failed, recv msg.result="<<stMsg.head().result());
			fsm.reply_fail(stMsg.head().result());
			fsm.set_state(state_end);
			return;
		}

		uint64_t new_hoosho_no = stMsg.hoosho_no_apply_res().hoosho_no();
		LOG4CPLUS_ERROR(logger, "FsmPreAuthStateWaitUserInfoUpdateReply::apply_hoosho_no_reply_event, get one new hooshono="<<new_hoosho_no);

		fsm._user_detail_info.set_hoosho_no(new_hoosho_no);

        //update userinfo to db
		ExecutorThreadRequestElement stUpdateUserInfoRequest(ExecutorThreadRequestType::T_USERINFO_UPDATE
														, FsmContainer<int>::FSM_TYPE_PREAUTH
														, fsm._id);
		stUpdateUserInfoRequest.need_reply();
		stUpdateUserInfoRequest.m_user_detail_info_from.CopyFrom(fsm._user_detail_info);
		g_executor_thread_processor->send_request(stUpdateUserInfoRequest);
		fsm.set_state(state_wait_user_info_update_reply);
		return;
    }

    void FsmPreAuthStateWaitApplyHooshoNOReply::timeout_event(FsmPreAuth& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmPreAuthStateWaitApplyHooshoNOReply time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_fail(::hoosho::msg::E_SERVER_TIMEOUT);
        fsm.set_state(FsmPreAuthState::state_end);
        return;
    }
    /*******************************************************************************************************/
    /*                                                                                  FsmPreAuthStateWaitUserInfoUpdateReply                                                       */
    /*******************************************************************************************************/
	void FsmPreAuthStateWaitUserInfoUpdateReply::enter(FsmPreAuth& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }
    void FsmPreAuthStateWaitUserInfoUpdateReply::exit(FsmPreAuth& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmPreAuthStateWaitUserInfoUpdateReply::user_info_update_reply_event(FsmPreAuth & fsm, const ExecutorThreadResponseElement & element)
    {
        //cancel timer first
        fsm.cancel_timer();

        //thread return not ok
		if(element.m_result_code != ExecutorThreadRequestType::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "FsmPreAuthStateWaitUserInfoUpdateReply::user_info_update_reply_event failed, recv thread.result="<<element.m_result_code);
			fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);
			fsm.set_state(state_end);
			return;	
		}

		//return newest userinto to client
		fsm.reply_ok();
		fsm.set_state(state_end);
		return;
    }

    void FsmPreAuthStateWaitUserInfoUpdateReply::timeout_event(FsmPreAuth& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmPreAuthStateWaitUserInfoUpdateReply time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_fail(::hoosho::msg::E_SERVER_TIMEOUT);
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

