#include "fsm_business_state.h"
#include "global_var.h"


    /*******************************************************************************************************/
    /*					                       FsmBusinessState						       					   */
    /*******************************************************************************************************/
    IMPL_LOGGER(FsmBusinessState, logger);

#define IMPL_FSM_STATE(classname, name) classname FsmBusinessState::name(#classname)
    IMPL_FSM_STATE(FsmBusinessStateInit, state_init);
    IMPL_FSM_STATE(FsmBusinessStateWaitThread, state_wait_thread);
    IMPL_FSM_STATE(FsmBusinessStateEnd, state_end);
#undef IMPL_FSM_STATE

    void FsmBusinessState::enter(FsmBusiness& fsm)
    {

    }
    void FsmBusinessState::exit(FsmBusiness& fsm)
    {

    }

    void FsmBusinessState::client_req_event(FsmBusiness& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
    	LOG4CPLUS_DEBUG(logger, "default client_req_event, state: "<<this->name());
    }
	void FsmBusinessState::thread_reply_event(FsmBusiness& fsm, ExecutorThreadResponseElement& element)
	{
		LOG4CPLUS_DEBUG(logger, "default thread_reply_event, state: "<<this->name());
	}
    void FsmBusinessState::timeout_event(FsmBusiness& fsm, void* param)
    {
    	LOG4CPLUS_DEBUG(logger, "default timeout_event, state: "<<this->name());
    }
    /*******************************************************************************************************/
    /*                                                                                  FsmBusinessStateInit                                                                                         */
    /*******************************************************************************************************/
    void FsmBusinessStateInit::client_req_event(FsmBusiness& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();
        fsm._msg.CopyFrom(stMsg);

		if(::hoosho::msg::QUERY_USER_PA_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_PA_LIST
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			stThreadRequest.m_uin = stMsg.query_user_zombie_req().uin();
			g_executor_thread_processor->send_request(stThreadRequest);
		}
		else if(::hoosho::msg::QUERY_PA_TAB_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_PA_TAB
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			stThreadRequest.m_pa_appid_md5 = stMsg.pa_tab_query_req().appid_md5();
			g_executor_thread_processor->send_request(stThreadRequest);
		}		
		else if(::hoosho::msg::QUERY_PA_EXPERT_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_PA_EXPERT_QUERY
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			stThreadRequest.m_pa_appid_md5 = stMsg.pa_expert_query_req().appid_md5();
			stThreadRequest.m_begin_create_ts = stMsg.pa_expert_query_req().begin_create_ts();
			stThreadRequest.m_limit = stMsg.pa_expert_query_req().limit();
			g_executor_thread_processor->send_request(stThreadRequest);
		}		
		
        fsm.set_state(FsmBusinessState::state_wait_thread);
        return;		
    }

	/*******************************************************************************************************/
    /*                                                                                  FsmBusinessStateWaitThread                                                                  */
    /*******************************************************************************************************/
	void FsmBusinessStateWaitThread::enter(FsmBusiness& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }
    void FsmBusinessStateWaitThread::exit(FsmBusiness& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmBusinessStateWaitThread::thread_reply_event(FsmBusiness& fsm, ExecutorThreadResponseElement& element)
    {
        //cancel timer first
        fsm.cancel_timer();

		if(element.m_result_code != ExecutorThreadRequestType::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "FsmBusinessStateWaitThread::thread_reply_event failed"
                        <<", fsmid:"<<fsm._id
                        <<", thread result="<<element.m_result_code);
                        
			fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);
			return;
		}

		hoosho::msg::Msg stRespMsg;
	    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	    header->set_result(::hoosho::msg::E_OK);
	    header->set_seq(fsm._msg.head().seq());
		
		if(::hoosho::msg::QUERY_USER_PA_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::QUERY_USER_PA_RES);
			::hoosho::user::QueryUserPaRes* pQueryUserPaRes = stRespMsg.mutable_query_user_pa_res();
			for(size_t i=0; i!=element.m_appid_md5_list.size() ; ++i)
			{
				pQueryUserPaRes->add_appid_md5_list(element.m_appid_md5_list[i]);
			}
		}
		else if(::hoosho::msg::QUERY_PA_TAB_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::QUERY_PA_TAB_RES);
			::hoosho::pa::QueryPaTabRes* pQueryPaTabRes = stRespMsg.mutable_pa_tab_query_res();
			for(size_t i=0; i!=element.m_tab_info_list.size() ; ++i)
			{
				pQueryPaTabRes->add_tab_info()->CopyFrom(element.m_tab_info_list[i]);
			}
		}		
		else if(::hoosho::msg::QUERY_PA_EXPERT_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::QUERY_PA_EXPERT_RES);
			::hoosho::pa::QueryPaExpertRes* pQueryPaExpertRes = stRespMsg.mutable_pa_expert_query_res();
			for(size_t i=0; i!=element.m_openid_md5_list.size() ; ++i)
			{
				pQueryPaExpertRes->add_openid_md5_list(element.m_openid_md5_list[i]);
			}
		}		

		g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(state_end);
		return;
    }

    void FsmBusinessStateWaitThread::timeout_event(FsmBusiness& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmBusinessStateWaitThread time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_fail(::hoosho::msg::E_SERVER_TIMEOUT);
        fsm.set_state(FsmBusinessState::state_end);
        return;
    } 
    /*******************************************************************************************************/
    /*                                                                                     FsmBusinessStateEnd                                                                                     */
    /*******************************************************************************************************/
    void FsmBusinessStateEnd::enter(FsmBusiness& fsm)
    {
        g_fsm_business_container->del_fsm(&fsm);
    }

