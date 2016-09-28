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

    void FsmBusinessState::client_req_event(FsmBusiness& fsm, lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
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
    void FsmBusinessStateInit::client_req_event(FsmBusiness& fsm, lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();
        fsm._msg.CopyFrom(msg);

		const ::hoosho::msg::s::MsgReq& stSMsgReq = msg.s_msg_req();
		uint32_t dwCmd = stSMsgReq.sub_cmd();
		ExecutorThreadRequestElement stThreadRequest;
		switch(dwCmd)
		{
			case ::hoosho::msg::s::USER_LOGIN_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_LOGIN
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														);
				stThreadRequest.need_reply();
				stThreadRequest.m_snsapi_base_pre_auth_code = stSMsgReq.code();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_PHONE_VC_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_PHONE_VC
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_phone = stSMsgReq.get_phone_vc_req().phone();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::SET_PHONE_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_SET_PHONE
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_snsapi_base_pre_auth_code = stSMsgReq.code();
				stThreadRequest.m_phone = stSMsgReq.set_phone_req().phone();				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::UPDATE_USER_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_UPDATE_USER_INFO
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_snsapi_base_pre_auth_code = stSMsgReq.code();
				stThreadRequest.m_self_desc = stSMsgReq.update_user_info_req().self_desc();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_USER_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_USER_INFO
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				if(stSMsgReq.get_user_info_req().user_id_list_size() == 0)
				{
					stThreadRequest.m_user_id_set.insert(fsm._self_user_info.user_id());
				}
				
				for(int i=0; i!=stSMsgReq.get_user_info_req().user_id_list_size(); ++i)
				{
					stThreadRequest.m_user_id_set.insert(stSMsgReq.get_user_info_req().user_id_list(i));
				}
				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::UPDATE_USER_IDENTITY_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_UPDATE_USER_IDENTITY_INFO
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_user_identity_info.CopyFrom(stSMsgReq.update_user_identity_info_req().user_identity_info());
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_USER_IDENTITY_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_USER_IDENTITY_INFO
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::ADD_USER_RECV_ADDR_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ADD_USER_RECV_ADDR_INFO
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_user_recv_addr_info.CopyFrom(stSMsgReq.add_user_recv_addr_info_req().user_recv_addr_info());
				stThreadRequest.m_set_default_addr = stSMsgReq.add_user_recv_addr_info_req().set_default_addr();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::DEL_USER_RECV_ADDR_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_DEL_USER_RECV_ADDR_INFO
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_addr_num = stSMsgReq.del_user_recv_addr_info_req().addr_num();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::UPDATE_USER_RECV_ADDR_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_UPDATE_USER_RECV_ADDR_INFO
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_user_recv_addr_info.CopyFrom(stSMsgReq.update_user_recv_addr_info_req().user_recv_addr_info());
				stThreadRequest.m_set_default_addr = stSMsgReq.update_user_recv_addr_info_req().set_default_addr();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_USER_RECV_ADDR_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_USER_RECV_ADDR_INFO
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			/****************************************** FEED ************************************************/

			case ::hoosho::msg::s::GET_TYPE_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_GOOD_TYPE
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_GOOD_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_GOOD_TYPE
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::ADD_TICKET_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ADD_TICKET
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_ticket_info.CopyFrom(stSMsgReq.add_ticket_req().ticket_info());
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_USER_FEED_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_USER_FEED_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_user_id = stSMsgReq.get_user_feed_list_req().user_id();
				stThreadRequest.m_offset = stSMsgReq.get_user_feed_list_req().offset();
				stThreadRequest.m_limit = stSMsgReq.get_user_feed_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_FEED_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_FEED_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_good_id = stSMsgReq.get_feed_list_req().good_id();
				stThreadRequest.m_begin_feed_id = stSMsgReq.get_feed_list_req().begin_feed_id();
				stThreadRequest.m_limit = stSMsgReq.get_feed_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_USER_SHOW_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_USER_SHOW_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_user_id = stSMsgReq.get_user_show_list_req().user_id();
				stThreadRequest.m_begin_show_id = stSMsgReq.get_user_show_list_req().begin_show_id();
				stThreadRequest.m_limit = stSMsgReq.get_user_show_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_SHOW_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_SHOW_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_begin_show_id = stSMsgReq.get_show_list_req().begin_show_id();
				stThreadRequest.m_limit = stSMsgReq.get_show_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_SHOW_DETAIL_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_SHOW_DETAIL
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_show_id = stSMsgReq.get_show_detail_req().show_id();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::ADD_SHOW_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ADD_SHOW
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_show_info.CopyFrom(stSMsgReq.add_show_req().show_info());
				stThreadRequest.m_show_info.set_show_id(++max_show_id);				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_USER_COLLECT_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_USER_COLLECT_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_begin_ts = stSMsgReq.get_user_collect_list_req().begin_ts();
				stThreadRequest.m_limit = stSMsgReq.get_user_collect_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::ADD_COLLECT_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ADD_COLLECT
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_collect_info.CopyFrom(stSMsgReq.add_collect_req().collect_info());
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_FEED_JOIN_USER_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_FEED_JOIN_USER_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());
				stThreadRequest.need_reply();
				stThreadRequest.m_feed_id = stSMsgReq.get_feed_join_user_list_req().feed_id();
				stThreadRequest.m_begin_ticket_id = stSMsgReq.get_feed_join_user_list_req().begin_ticket_id();
				stThreadRequest.m_limit = stSMsgReq.get_feed_join_user_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_USER_FEED_CONTEND_ID_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_USER_FEED_CONTEND_ID_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());

				stThreadRequest.need_reply();
				stThreadRequest.m_user_id = stSMsgReq.get_user_feed_contend_id_list_req().user_id();
				for(int i = 0; i < stSMsgReq.get_user_feed_contend_id_list_req().feed_id_list_size(); i++)
				{
					stThreadRequest.m_feed_id_list.insert(stSMsgReq.get_user_feed_contend_id_list_req().feed_id_list(i));
				}
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_BANNER_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_BANNER_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());

				stThreadRequest.need_reply();
				stThreadRequest.m_begin_banner_id = stSMsgReq.get_banner_list_req().begin_banner_id();
				stThreadRequest.m_limit = stSMsgReq.get_banner_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::DEL_COLLECT_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_DEL_COLLECT
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());

				stThreadRequest.need_reply();
				stThreadRequest.m_feed_id = stSMsgReq.del_collect_req().feed_id();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;				

			case ::hoosho::msg::s::CHECK_USER_COLLECT_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_CHECK_USER_COLLECT
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());

				stThreadRequest.need_reply();
				for(int i = 0; i < stSMsgReq.check_user_collect_req().feed_id_list_size(); i++)
				{
					stThreadRequest.m_feed_id_list.insert(stSMsgReq.check_user_collect_req().feed_id_list(i));
				}
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_USER_LUCK_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_USER_LUCK_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());

				stThreadRequest.need_reply();
				stThreadRequest.m_user_id = stSMsgReq.get_user_luck_list_req().user_id();
				stThreadRequest.m_begin_feed_id = stSMsgReq.get_user_luck_list_req().begin_feed_id();
				stThreadRequest.m_limit = stSMsgReq.get_user_luck_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_FEED_DETAIL_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_FEED_DETATIL
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());

				stThreadRequest.need_reply();
				for(int i = 0; i < stSMsgReq.get_feed_detail_req().feed_id_list_size(); i++)
				{
					stThreadRequest.m_feed_id_list.insert(stSMsgReq.get_feed_detail_req().feed_id_list(i));
				}
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::s::GET_USER_TICKET_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_GET_USER_TICKET_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.user_id());

				stThreadRequest.need_reply();
				stThreadRequest.m_begin_ticket_id = stSMsgReq.get_user_ticket_list_req().begin_ticket_id();
				stThreadRequest.m_limit = stSMsgReq.get_user_ticket_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;
			
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
                        <<", thread result="<<ExecutorThreadRequestType::err_string(element.m_result_code));

            if(ExecutorThreadRequestType::E_PARAM_ERR == element.m_result_code)
            {
				fsm.reply_fail(::hoosho::msg::E_INVALID_REQ_PARAM);	
            }
            else if(ExecutorThreadRequestType::E_SHOW_REPEATED == element.m_result_code)
            {
				fsm.reply_fail(::hoosho::msg::E_SHOW_REPEATED);	
            }
            else if(ExecutorThreadRequestType::E_UPLOAD_TICKET_LIMIT == element.m_result_code)
            {
				fsm.reply_fail(::hoosho::msg::E_UPLOAD_TICKET_LIMIT);
            }
            else
            {
				fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);			
			}
			return;
		}

		::hoosho::msg::Msg stRespMsg;
	    ::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	    header->set_result(::hoosho::msg::E_OK);
	    header->set_seq(fsm._msg.head().seq());
	    header->set_cmd(::hoosho::msg::S_PROJECT_RES);

	    ::hoosho::msg::s::MsgRes* pSMsgRes = stRespMsg.mutable_s_msg_res();
	    uint32_t dwCmd = fsm._msg.s_msg_req().sub_cmd();


		switch(dwCmd)
		{
			case ::hoosho::msg::s::USER_LOGIN_REQ:
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::USER_LOGIN_RES);
				pSMsgRes->add_user_info_list()->CopyFrom(element.m_user_info);

				//add login session			
				g_client_processor->add_login_session(element.m_auth_code, element.m_user_info);
				break;

			case ::hoosho::msg::s::GET_PHONE_VC_REQ:
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_PHONE_VC_RES);

				//add phonevc session
				g_client_processor->add_phonevc_session(element.m_phone, element.m_phone_vc);
				break;

			case ::hoosho::msg::s::SET_PHONE_REQ:
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::SET_PHONE_RES);

				//delete phonevc session
				g_client_processor->delete_phonevc_session(element.m_phone);

				//update login session 
				g_client_processor->add_login_session(element.m_auth_code, element.m_user_info);
				break;

			case ::hoosho::msg::s::UPDATE_USER_INFO_REQ:
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::UPDATE_USER_INFO_RES);

				//update login session 
				g_client_processor->add_login_session(element.m_auth_code, element.m_user_info);
				break;

			case ::hoosho::msg::s::GET_USER_INFO_REQ:
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_USER_INFO_RES);

				for (size_t i = 0; i != element.m_user_info_list.size(); ++i)
				{
					pSMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				element.m_user_info_list.clear();
				break;

			case ::hoosho::msg::s::UPDATE_USER_IDENTITY_INFO_REQ:
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::UPDATE_USER_IDENTITY_INFO_RES);
				break;

			case ::hoosho::msg::s::GET_USER_IDENTITY_INFO_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_USER_IDENTITY_INFO_RES);

				::hoosho::msg::s::GetUserIdentityInfoRes *pGetUserIdentityInfoRes = pSMsgRes->mutable_get_user_identity_info_res();
				pGetUserIdentityInfoRes->mutable_user_identity_info()->CopyFrom(element.m_user_identity_info);
				break;
			}

			case ::hoosho::msg::s::ADD_USER_RECV_ADDR_INFO_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::ADD_USER_RECV_ADDR_INFO_RES);

				::hoosho::msg::s::AddUserRecvAddrInfoRes *pAddUserRecvAddrInfoRes = pSMsgRes->mutable_add_user_recv_addr_info_res();
				pAddUserRecvAddrInfoRes->set_addr_num(element.m_addr_num);
				break;
			}
			
			case ::hoosho::msg::s::DEL_USER_RECV_ADDR_INFO_REQ:
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::DEL_USER_RECV_ADDR_INFO_RES);
				break;

			case ::hoosho::msg::s::UPDATE_USER_RECV_ADDR_INFO_REQ:
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::UPDATE_USER_RECV_ADDR_INFO_RES);
				break;

			case ::hoosho::msg::s::GET_USER_RECV_ADDR_INFO_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_USER_RECV_ADDR_INFO_RES);

				::hoosho::msg::s::GetUserRecvAddrInfoRes *pGetUserRecvAddrInfoRes = pSMsgRes->mutable_get_user_recv_addr_info_res();
				for(size_t i = 0; i < element.m_user_recv_addr_info_list.size(); i++)
				{
					pGetUserRecvAddrInfoRes->add_user_recv_addr_info_list()->CopyFrom(element.m_user_recv_addr_info_list[i]);
				}
				element.m_user_recv_addr_info_list.clear();
				
				break;
			}

			/****************************************** FEED ************************************************/

			case ::hoosho::msg::s::GET_TYPE_INFO_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_TYPE_INFO_RES);

				::hoosho::msg::s::GetTypeInfoRes *pGetTypeInfoRes = pSMsgRes->mutable_get_type_info_res();
				for(size_t i = 0; i < element.m_good_type_list.size(); i++)
				{
					pGetTypeInfoRes->add_good_type_list()->CopyFrom(element.m_good_type_list[i]);
				}
				element.m_good_type_list.clear();
				break;
			}

			case ::hoosho::msg::s::GET_GOOD_INFO_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_GOOD_INFO_RES);

				::hoosho::msg::s::GetGoodInfoRes *pGetGoodInfoRes = pSMsgRes->mutable_get_good_info_res();
				for(size_t i = 0; i < element.m_good_info_list.size(); i++)
				{
					pGetGoodInfoRes->add_good_info_list()->CopyFrom(element.m_good_info_list[i]);
				}
				element.m_good_info_list.clear();
				break;
			}

			case ::hoosho::msg::s::ADD_TICKET_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::ADD_TICKET_RES);

				::hoosho::msg::s::AddTicketRes *pAddTicketRes = pSMsgRes->mutable_add_ticket_res();
				pAddTicketRes->set_ticket_id(element.m_ticket_id);
				break;
			}

			case ::hoosho::msg::s::GET_USER_FEED_LIST_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_USER_FEED_LIST_RES);

				::hoosho::msg::s::GetUserFeedListRes *pGetUserFeedListRes = pSMsgRes->mutable_get_user_feed_list_res();
				for(size_t i = 0; i < element.m_feed_info_list.size(); i++)
				{
					pGetUserFeedListRes->add_feed_info_list()->CopyFrom(element.m_feed_info_list[i]);
				}
				element.m_feed_info_list.clear();

				for(size_t i = 0; i < element.m_good_info_list.size(); i++)
				{
					pGetUserFeedListRes->add_good_info_list()->CopyFrom(element.m_good_info_list[i]);
				}
				element.m_good_info_list.clear();
				
				break;
			}

			case ::hoosho::msg::s::GET_FEED_LIST_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_FEED_LIST_RES);

				::hoosho::msg::s::GetFeedListRes *pGetFeedListRes = pSMsgRes->mutable_get_feed_list_res();
				for(size_t i = 0; i < element.m_feed_info_list.size(); i++)
				{
					pGetFeedListRes->add_feed_info_list()->CopyFrom(element.m_feed_info_list[i]);
				}
				element.m_feed_info_list.clear();

				for(size_t i = 0; i < element.m_good_info_list.size(); i++)
				{
					pGetFeedListRes->add_good_info_list()->CopyFrom(element.m_good_info_list[i]);
				}
				element.m_good_info_list.clear();
				break;
			}

			case ::hoosho::msg::s::GET_USER_SHOW_LIST_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_USER_SHOW_LIST_RES);

				::hoosho::msg::s::GetUserShowListRes *pGetUserShowListRes = pSMsgRes->mutable_get_user_show_list_res();
				for(size_t i = 0; i < element.m_show_info_list.size(); i++)
				{
					pGetUserShowListRes->add_show_info_list()->CopyFrom(element.m_show_info_list[i]);
				}
				element.m_show_info_list.clear();
				break;
			}

			case ::hoosho::msg::s::GET_SHOW_LIST_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_SHOW_LIST_RES);

				::hoosho::msg::s::GetShowListRes *pGetShowListRes = pSMsgRes->mutable_get_show_list_res();
				for(size_t i = 0; i < element.m_show_info_list.size(); i++)
				{
					pGetShowListRes->add_show_info_list()->CopyFrom(element.m_show_info_list[i]);
				}
				element.m_show_info_list.clear();
				break;
			}

			case ::hoosho::msg::s::GET_SHOW_DETAIL_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_SHOW_DETAIL_RES);
				pSMsgRes->add_user_info_list()->CopyFrom(element.m_user_info);				

				::hoosho::msg::s::GetShowDetailRes *pGetShowDetailRes = pSMsgRes->mutable_get_show_detail_res();
				pGetShowDetailRes->mutable_show_info()->CopyFrom(element.m_show_info);
				pGetShowDetailRes->mutable_feed_info()->CopyFrom(element.m_feed_info);
				break;
			}

			case ::hoosho::msg::s::ADD_SHOW_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::ADD_SHOW_RES);

				::hoosho::msg::s::AddShowRes *pAddShowRes = pSMsgRes->mutable_add_show_res();
				pAddShowRes->set_show_id(element.m_show_id);
				break;
			}

			case ::hoosho::msg::s::GET_USER_COLLECT_LIST_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_USER_COLLECT_LIST_RES);

				::hoosho::msg::s::GetUserCollectListRes *pGetUserCollectListRes = pSMsgRes->mutable_get_user_collect_list_res();
				for(size_t i = 0; i < element.m_collect_info_list.size(); i++)
				{
					pGetUserCollectListRes->add_collect_info_list()->CopyFrom(element.m_collect_info_list[i]);
				}
				element.m_collect_info_list.clear();

				for(size_t i = 0; i < element.m_feed_info_list.size(); i++)
				{
					pGetUserCollectListRes->add_feed_info_list()->CopyFrom(element.m_feed_info_list[i]);
				}
				element.m_feed_info_list.clear();

				for(size_t i = 0; i < element.m_good_info_list.size(); i++)
				{
					pGetUserCollectListRes->add_good_info_list()->CopyFrom(element.m_good_info_list[i]);
				}
				element.m_good_info_list.clear();
				break;
			}

			case ::hoosho::msg::s::ADD_COLLECT_REQ:
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::ADD_COLLECT_RES);
				break;

			case ::hoosho::msg::s::GET_FEED_JOIN_USER_LIST_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_FEED_JOIN_USER_LIST_RES);

				::hoosho::msg::s::GetFeedJoinUserListRes *pGetFeedJoinUserListRes = pSMsgRes->mutable_get_feed_join_user_list_res();
				for(size_t i = 0; i < element.m_contend_info_list.size(); i++)
				{
					pGetFeedJoinUserListRes->add_contend_info_list()->CopyFrom(element.m_contend_info_list[i]);
				}
				element.m_contend_info_list.clear();

				for(size_t i = 0; i < element.m_user_info_list.size(); i++)
				{
					pSMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				element.m_user_info_list.clear();
				
				break;
			}

			case ::hoosho::msg::s::GET_USER_FEED_CONTEND_ID_LIST_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_USER_FEED_CONTEND_ID_LIST_RES);

				::hoosho::msg::s::GetUserFeedContendIdListRes *pGetUserFeedContendIdListRes = pSMsgRes->mutable_get_user_feed_contend_id_list_res();
				for(size_t i = 0; i < element.m_feed_contend_info_list.size(); i++)
				{
					pGetUserFeedContendIdListRes->add_feed_contend_info_list()->CopyFrom(element.m_feed_contend_info_list[i]);
				}
				element.m_feed_contend_info_list.clear();

				break;
			}

			case ::hoosho::msg::s::GET_BANNER_LIST_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_BANNER_LIST_RES);

				::hoosho::msg::s::GetBannerListRes *pGetBannerListRes = pSMsgRes->mutable_get_banner_list_res();
				for(size_t i = 0; i < element.m_banner_info_list.size(); i++)
				{
					pGetBannerListRes->add_banner_info_list()->CopyFrom(element.m_banner_info_list[i]);
				}
				element.m_banner_info_list.clear();

				break;
			}

			case ::hoosho::msg::s::DEL_COLLECT_REQ:			
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::DEL_COLLECT_RES);
				break;				

			case ::hoosho::msg::s::CHECK_USER_COLLECT_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::CHECK_USER_COLLECT_RES);

				::hoosho::msg::s::CheckUserCollectRes *pCheckUserCollectRes = pSMsgRes->mutable_check_user_collect_res();
				for(size_t i = 0; i < element.m_user_collect_flag_list.size(); i++)
				{
					pCheckUserCollectRes->add_user_collect_flag_list()->CopyFrom(element.m_user_collect_flag_list[i]);
				}
				element.m_user_collect_flag_list.clear();								
				break;	
			}				

			case ::hoosho::msg::s::GET_USER_LUCK_LIST_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_USER_LUCK_LIST_RES);
				
				::hoosho::msg::s::GetUserLuckListRes *pGetUserLuckListRes = pSMsgRes->mutable_get_user_luck_list_res();
				for(size_t i = 0; i < element.m_feed_info_list.size(); i++)
				{
					pGetUserLuckListRes->add_feed_info_list()->CopyFrom(element.m_feed_info_list[i]);
				}
				element.m_feed_info_list.clear();

				for(size_t i = 0; i < element.m_good_info_list.size(); i++)
				{
					pGetUserLuckListRes->add_good_info_list()->CopyFrom(element.m_good_info_list[i]);
				}
				element.m_good_info_list.clear();
				break;	
			}

			case ::hoosho::msg::s::GET_FEED_DETAIL_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_FEED_DETAIL_RES);

				::hoosho::msg::s::GetFeedDetailRes *pGetFeedDetailRes = pSMsgRes->mutable_get_feed_detail_res();
				for(size_t i = 0; i < element.m_feed_info_list.size(); i++)
				{
					pGetFeedDetailRes->add_feed_info_list()->CopyFrom(element.m_feed_info_list[i]);
				}
				element.m_feed_info_list.clear();

				for(size_t i = 0; i < element.m_good_info_list.size(); i++)
				{
					pGetFeedDetailRes->add_good_info_list()->CopyFrom(element.m_good_info_list[i]);
				}
				element.m_good_info_list.clear();
				break;	
			}

			case ::hoosho::msg::s::GET_USER_TICKET_LIST_REQ:
			{
				pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_USER_TICKET_LIST_RES);

				::hoosho::msg::s::GetUserTicketListRes *pGetUserTicketListRes = pSMsgRes->mutable_get_user_ticket_list_res();
				for(size_t i = 0; i < element.m_ticket_info_list.size(); i++)
				{
					pGetUserTicketListRes->add_ticket_info_list()->CopyFrom(element.m_ticket_info_list[i]);
				}
				element.m_ticket_info_list.clear();
				
				for(size_t i = 0; i < element.m_feed_info_list.size(); i++)
				{
					pGetUserTicketListRes->add_feed_info_list()->CopyFrom(element.m_feed_info_list[i]);
				}
				element.m_feed_info_list.clear();

				for(size_t i = 0; i < element.m_good_info_list.size(); i++)
				{
					pGetUserTicketListRes->add_good_info_list()->CopyFrom(element.m_good_info_list[i]);
				}
				element.m_good_info_list.clear();
				break;
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

