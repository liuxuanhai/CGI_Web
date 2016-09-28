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

		const ::hoosho::msg::z::MsgReq& stZMsgReq = msg.z_msg_req();
		uint32_t dwCmd = stZMsgReq.sub_cmd();
		ExecutorThreadRequestElement stThreadRequest;
		switch(dwCmd)
		{
			case ::hoosho::msg::z::USER_LOGIN_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_LOGIN
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, "");
				stThreadRequest.need_reply();
				stThreadRequest.m_snsapi_base_pre_auth_code = stZMsgReq.code();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_PHONE_VC_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_GET_PHONE_VC
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_phone = stZMsgReq.get_phone_vc_req().phone();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::SET_PHONE_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_SET_PHONE
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_snsapi_base_pre_auth_code = stZMsgReq.code();
				stThreadRequest.m_phone = stZMsgReq.set_phone_req().phone();			
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_USER_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USERINFO_QUERY
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				//���������openid_listΪ�գ���˵���ǲ�ѯ���˵��û���Ϣ��������Բ������˵�openid
				if(stZMsgReq.get_user_info_req().openid_list_size() == 0)
				{
					stThreadRequest.m_openid_set.insert(fsm._self_user_info.openid());
				}
				
				for(int i=0; i!=stZMsgReq.get_user_info_req().openid_list_size(); ++i)
				{
					stThreadRequest.m_openid_set.insert(stZMsgReq.get_user_info_req().openid_list(i));
				}
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::UPDATE_USER_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USERINFO_UPDATE
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_snsapi_base_pre_auth_code = stZMsgReq.code();
				stThreadRequest.m_user_info.set_openid(fsm._self_user_info.openid());
				stThreadRequest.m_user_info.set_self_desc(stZMsgReq.update_user_info_req().self_desc());
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_USER_CASH_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_GET_CASH
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_USER_RECOMMONED_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_RECOMMEND_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_offset_openid = stZMsgReq.get_user_recommend_list_req().offset_openid();
				stThreadRequest.m_len = stZMsgReq.get_user_recommend_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::TRUST_USER_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_TRUST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_third_openid = stZMsgReq.trust_user_req().openid();
				stThreadRequest.m_trust_opt = stZMsgReq.trust_user_req().opt();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::USER_TRUST_SB_CHECK_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_TRUST_SB_CHECK
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				for(int i=0; i!=stZMsgReq.user_trust_sb_check_req().openid_list_size(); ++i)
				{
					stThreadRequest.m_openid_set.insert(stZMsgReq.user_trust_sb_check_req().openid_list(i));
				}				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_MY_TRUST_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_MY_TRUST_LIST_QUERY
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_third_openid = stZMsgReq.get_my_trust_list_req().openid();
				stThreadRequest.m_offset_openid = stZMsgReq.get_my_trust_list_req().offset_openid();
				stThreadRequest.m_len = stZMsgReq.get_my_trust_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_TRUST_ME_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_TRUST_ME_LIST_QUERY
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_third_openid = stZMsgReq.get_trust_me_list_req().openid();
				stThreadRequest.m_offset_openid = stZMsgReq.get_trust_me_list_req().offset_openid();
				stThreadRequest.m_len = stZMsgReq.get_trust_me_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::UPDATE_USER_ACTIVITY_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_ACTIVITY_INFO_UPDATE
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_user_activity_info.CopyFrom(stZMsgReq.update_user_activity_info_req().user_activity_info());
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_USER_ACTIVITY_INFO_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_ACTIVITY_INFO_QUERY
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_user_activity_info.set_info_id(stZMsgReq.get_user_activity_info_req().info_id());
				stThreadRequest.m_user_activity_info.set_activity_type(stZMsgReq.get_user_activity_info_req().activity_type());
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_RANKING_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_GET_RANKING_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_ranking_type = stZMsgReq.get_ranking_list_req().type();
				stThreadRequest.m_len = stZMsgReq.get_ranking_list_req().limit();
				stThreadRequest.m_ranking_page = stZMsgReq.get_ranking_list_req().page();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_AVAILABLE_LOTTERY_TIME_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_GET_AVAILABLE_LOTTERY_TIME
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::DO_LOTTERY_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_DO_LOTTERY
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_LOTTERY_RECORD_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_USER_GET_LOTTERY_RECORD_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();	
				stThreadRequest.m_begin_ts = stZMsgReq.get_lottery_record_list_req().begin_ts();
				stThreadRequest.m_len = stZMsgReq.get_lottery_record_list_req().len();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			
				
			/******************************************** MSG & NOTICE *************************************************/
			//msg
			case ::hoosho::msg::z::ADD_NEW_MSG_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_MSG_ADD
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_msg_content.CopyFrom(stZMsgReq.add_new_msg_req().msg_content());
				g_executor_thread_processor->send_request(stThreadRequest);
				break;	

			case ::hoosho::msg::z::GET_MSG_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_MSG_GET_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_openid_set.insert(stZMsgReq.get_msg_list_req().openid_to());
				stThreadRequest.m_begin_msg_id = stZMsgReq.get_msg_list_req().begin_msg_id();
				stThreadRequest.m_len = stZMsgReq.get_msg_list_req().len();
				stThreadRequest.m_msg_openid_to = stZMsgReq.get_msg_list_req().openid_to();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;	

			case ::hoosho::msg::z::DEL_MSG_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_MSG_DEL
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;	

			case ::hoosho::msg::z::GET_SESSION_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_MSG_GET_SESSION_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_begin_ts = stZMsgReq.get_session_list_req().begin_ts();
				stThreadRequest.m_len = stZMsgReq.get_session_list_req().len();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;	

			case ::hoosho::msg::z::DEL_SESSION_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_MSG_DEL_SESSION
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_msg_openid_to = stZMsgReq.del_session_req().openid_to();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;	
			
			case ::hoosho::msg::z::GET_NEWMSG_STATUS_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_MSG_GET_NEWMSG_STATUS
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;	

			//redpoint
			case ::hoosho::msg::z::GET_RED_POINT_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_REDPOINT_GET
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();				
				g_executor_thread_processor->send_request(stThreadRequest);
				break;
			
			case ::hoosho::msg::z::UPDATE_RED_POINT_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_REDPOINT_UPDATE
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_redpoint_type = stZMsgReq.update_red_point_req().type();
				stThreadRequest.m_redpoint_opt = stZMsgReq.update_red_point_req().opt();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;	

			//notice
			case ::hoosho::msg::z::ADD_NOTICE_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_NOTICE_ADD
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_notice_info.CopyFrom(stZMsgReq.add_notice_req().notice_info());
				g_executor_thread_processor->send_request(stThreadRequest);
				break;	
			
			case ::hoosho::msg::z::GET_NOTICE_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_NOTICE_GET
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_notice_type = stZMsgReq.get_notice_req().type();
				stThreadRequest.m_begin_ts = stZMsgReq.get_notice_req().begin_ts();
				stThreadRequest.m_len = stZMsgReq.get_notice_req().len();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;			

			/*******************************************ORDER**********************************************************/	

			case ::hoosho::msg::z::CREATE_ORDER_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_CREATE
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
														
				stThreadRequest.need_reply();
				stThreadRequest.m_order_info.CopyFrom(stZMsgReq.create_order_req().order_info());
				stThreadRequest.m_user_ip = stZMsgReq.create_order_req().user_ip();
				stThreadRequest.m_media_server_id = stZMsgReq.create_order_req().media_server_id();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;
				
			case ::hoosho::msg::z::FETCH_ORDER_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_FETCH
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
														
				stThreadRequest.need_reply();
				stThreadRequest.m_order_id = stZMsgReq.fetch_order_req().orderid();
				stThreadRequest.m_extra_data = stZMsgReq.fetch_order_req().extra_data();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::FINISH_ORDER_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_FINISH
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
														
				stThreadRequest.need_reply();
				stThreadRequest.m_order_id = stZMsgReq.finish_order_req().orderid();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::CANCEL_ORDER_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_CANCEL
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
														
				stThreadRequest.need_reply();
				stThreadRequest.m_order_id = stZMsgReq.cancel_order_req().orderid();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::COMMENT_ORDER_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_COMMENT
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
														
				stThreadRequest.need_reply();
				stThreadRequest.m_order_id = stZMsgReq.comment_order_req().orderid();
				stThreadRequest.m_order_star = stZMsgReq.comment_order_req().star();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::SEARCH_ORDER_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_SEARCH
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_longitude = stZMsgReq.seach_order_req().addr_longitude();
				stThreadRequest.m_latitude = stZMsgReq.seach_order_req().addr_latitude();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_CASH_FLOW_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_CASH_FLOW_QUERY
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_offset = stZMsgReq.get_cash_flow_req().begin_ts();
				stThreadRequest.m_len = stZMsgReq.get_cash_flow_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_OUTCOME_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_OUTCOME_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_offset = stZMsgReq.get_outcome_list_req().begin_ts();
				stThreadRequest.m_len = stZMsgReq.get_outcome_list_req().limit();
				stThreadRequest.m_outcome_state = stZMsgReq.get_outcome_list_req().state();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::COMMIT_OUTCOME_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_OUTCOME_COMMIT
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_amount = stZMsgReq.commit_outcome_req().amount();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_ORDER_DETAIL_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_INFO_QUERY
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				for(int i=0; i<stZMsgReq.get_order_detail_req().orderid_list_size(); ++i)
				{
					stThreadRequest.m_order_id_list.push_back(stZMsgReq.get_order_detail_req().orderid_list(i));
				}
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_USER_CREATE_ORDER_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_LIST_USER_CREATED
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_third_openid = stZMsgReq.get_user_create_order_list_req().openid();
				stThreadRequest.m_offset = stZMsgReq.get_user_create_order_list_req().begin_orderid();
				stThreadRequest.m_len = stZMsgReq.get_user_create_order_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_USER_FETCH_ORDER_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_LIST_USER_FETCHED
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_third_openid = stZMsgReq.get_user_fetch_order_list_req().openid();
				stThreadRequest.m_offset = stZMsgReq.get_user_fetch_order_list_req().begin_orderid();
				stThreadRequest.m_len = stZMsgReq.get_user_fetch_order_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::REWARD_ORDER_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_REWARD
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_order_id = stZMsgReq.reward_order_req().orderid();
				stThreadRequest.m_amount = stZMsgReq.reward_order_req().amount();
				stThreadRequest.m_user_ip = stZMsgReq.reward_order_req().user_ip();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			/*******************************************FEEDS**********************************************************/	
			case ::hoosho::msg::z::CHECK_ORDER_FAVOR_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_FAVOR_CHECK
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				for(int i = 0; i < stZMsgReq.check_order_favor_req().orderid_list_size(); i++)
				{
					stThreadRequest.m_order_id_list.push_back(stZMsgReq.check_order_favor_req().orderid_list(i));
				}
				g_executor_thread_processor->send_request(stThreadRequest);
				break;
				
			case ::hoosho::msg::z::UPDATE_ORDER_FAVOR_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_FAVOR_UPDATE
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_order_id = stZMsgReq.update_order_favor_req().orderid();
				stThreadRequest.m_oper = stZMsgReq.update_order_favor_req().oper();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_ORDER_FAVOR_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_FAVOR_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_order_id = stZMsgReq.get_order_favor_list_req().orderid();
				stThreadRequest.m_begin_ts = stZMsgReq.get_order_favor_list_req().begin_ts();
				stThreadRequest.m_len = stZMsgReq.get_order_favor_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::ADD_ORDER_FOLLOW_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_FOLLOW_ADD
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_order_follow_info.CopyFrom(stZMsgReq.add_order_follow_req().follow_info());
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_ORDER_COMMENT_LIST_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_COMMENT_LIST
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_order_id = stZMsgReq.get_order_comment_list_req().orderid();
				stThreadRequest.m_begin_comment_id = stZMsgReq.get_order_comment_list_req().begin_comment_id();
				stThreadRequest.m_len = stZMsgReq.get_order_comment_list_req().limit();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::GET_ORDER_COMMENT_DETAIL_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_COMMENT_DETAIL
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				for(int i = 0; i < stZMsgReq.get_order_comment_detail_req().origin_comment_id_list_size(); i++)
				{
					stThreadRequest.m_origin_comment_id_list.push_back(stZMsgReq.get_order_comment_detail_req().origin_comment_id_list(i));
				}
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::DEL_ORDER_FOLLOW_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_FOLLOW_DEL
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_follow_id = stZMsgReq.del_order_follow_req().follow_id();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::ORDER_FETCH_CANCEL_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_FETCH_CANCEL
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_order_id = stZMsgReq.fetch_cancel_req().orderid();
				g_executor_thread_processor->send_request(stThreadRequest);
				break;

			case ::hoosho::msg::z::ORDER_FETCH_ACCEPT_REQ:
				stThreadRequest.init(ExecutorThreadRequestType::T_ORDER_FETCH_ACCEPT
														, FsmContainer<int>::FSM_TYPE_BUSINESS
														, fsm._id
														, fsm._self_user_info.openid());
				stThreadRequest.need_reply();
				stThreadRequest.m_order_id = stZMsgReq.fetch_accept_req().orderid();
				stThreadRequest.m_slave_openid = stZMsgReq.fetch_accept_req().slave_openid();
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

			if(element.m_result_code == ExecutorThreadRequestType::E_ORDER_CANNOT_FETCH)
			{
				fsm.reply_fail(::hoosho::msg::E_ORDER_CANNOT_FETCH);
			}
			else if(element.m_result_code == ExecutorThreadRequestType::E_PARAM_ERR)
			{
				fsm.reply_fail(::hoosho::msg::E_INVALID_REQ_PARAM);
			}
			else if(element.m_result_code == ExecutorThreadRequestType::E_ORDER_CANNOT_FETCH_CANCEL_AFTER_CHOSEN)
			{
				fsm.reply_fail(::hoosho::msg::E_ORDER_CANNOT_FETCH_CANCEL_AFTER_CHOSEN);
			}
			else if(element.m_result_code == ExecutorThreadRequestType::E_ORDER_CANNOT_FETCH_REPEATED)
			{
				fsm.reply_fail(::hoosho::msg::E_ORDER_CANNOT_FETCH_REPEATED);
			}
			else if(element.m_result_code == ExecutorThreadRequestType::E_ORDER_CANNOT_FETCH_TOO_MANY_WAITING)
			{
				fsm.reply_fail(::hoosho::msg::E_ORDER_CANNOT_FETCH_TOOMANY);
			}
			else if(element.m_result_code == ExecutorThreadRequestType::E_ORDER_CANNOT_CANCEL_WHILE_WAITING_EXISTS)
			{	
				fsm.reply_fail(::hoosho::msg::E_ORDER_CANNOT_CANCEL_WHILE_WAITING_EXISTS);
			}
			else if(element.m_result_code == ExecutorThreadRequestType::E_AVAILABLE_LOTTERY_TIME_NOT_ENOUGH)
			{	
				fsm.reply_fail(::hoosho::msg::E_AVAILABLE_LOTTERY_TIME_NOT_ENOUGH);
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
	    header->set_cmd(::hoosho::msg::Z_PROJECT_RES);

	    ::hoosho::msg::z::MsgRes* pZMsgRes = stRespMsg.mutable_z_msg_res();
	    uint32_t dwCmd = fsm._msg.z_msg_req().sub_cmd();


		switch(dwCmd)
		{
			case ::hoosho::msg::z::USER_LOGIN_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::USER_LOGIN_RES);
				pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info);

				//add login session			
				g_client_processor->add_login_session(element.m_auth_code, element.m_user_info);
				break;

			case ::hoosho::msg::z::GET_PHONE_VC_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_PHONE_VC_RES);

				//add phonevc session
				g_client_processor->add_phonevc_session(element.m_phone, element.m_phone_vc);
				break;

			case ::hoosho::msg::z::SET_PHONE_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::SET_PHONE_RES);

				//delete phonevc session
				g_client_processor->delete_phonevc_session(element.m_phone);

				//update login session 
				g_client_processor->add_login_session(element.m_auth_code, element.m_user_info);
				break;

			case ::hoosho::msg::z::GET_USER_INFO_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_USER_INFO_RES);
				for (size_t i = 0; i != element.m_user_info_list.size(); ++i)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				element.m_user_info_list.clear();

				::hoosho::msg::z::GetUserInfoRes *pGetUserInfoRes = pZMsgRes->mutable_get_user_info_res();
				for (size_t i = 0; i != element.m_user_extra_info_list.size(); ++i)
				{
					pGetUserInfoRes->add_user_extra_info_list()->CopyFrom(element.m_user_extra_info_list[i]);
				}
				element.m_user_extra_info_list.clear();
				break;
			}

			case ::hoosho::msg::z::UPDATE_USER_INFO_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::UPDATE_USER_INFO_RES);

				//update login session 
				g_client_processor->add_login_session(element.m_auth_code, element.m_user_info);
				break;

			case ::hoosho::msg::z::GET_USER_CASH_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_USER_CASH_RES);
				::hoosho::msg::z::GetUserCashRes *pGetUserCashRes = pZMsgRes->mutable_get_user_cash_res();
				pGetUserCashRes->set_balance(element.m_balance);
				pGetUserCashRes->set_pending(element.m_pending);
				break;
			}

			case ::hoosho::msg::z::GET_USER_RECOMMONED_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_USER_RECOMMONED_LIST_RES);
				for (size_t i = 0; i != element.m_user_info_list.size(); ++i)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				element.m_user_info_list.clear();

				::hoosho::msg::z::GetUserRecommendListRes *pGetUserRecommendListRes = pZMsgRes->mutable_get_user_recommend_list_res();
				for (size_t i = 0; i != element.m_user_extra_info_list.size(); ++i)
				{
					pGetUserRecommendListRes->add_user_extra_info_list()->CopyFrom(element.m_user_extra_info_list[i]);
				}
				element.m_user_extra_info_list.clear();

				for (size_t i = 0; i != element.m_user_trust_sb_status_list.size(); i++)
				{
					pGetUserRecommendListRes->add_user_trust_sb_status_list()->CopyFrom(element.m_user_trust_sb_status_list[i]);
				}
				element.m_user_trust_sb_status_list.clear();
				break;
			}

			case ::hoosho::msg::z::TRUST_USER_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::TRUST_USER_RES);
				break;

			case ::hoosho::msg::z::USER_TRUST_SB_CHECK_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::USER_TRUST_SB_CHECK_RES);

				::hoosho::msg::z::UserTrustSBCheckRes *pUserTrustSBCheckRes = pZMsgRes->mutable_user_trust_sb_check_res();
				for (size_t i = 0; i < element.m_user_trust_sb_status_list.size(); i++)
				{
					pUserTrustSBCheckRes->add_user_trust_sb_status_list()->CopyFrom(element.m_user_trust_sb_status_list[i]);
				}
				element.m_user_trust_sb_status_list.clear();
				break;
			}

			case ::hoosho::msg::z::GET_MY_TRUST_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_MY_TRUST_LIST_RES);
				for (size_t i = 0; i != element.m_user_info_list.size(); ++i)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				element.m_user_info_list.clear();

				::hoosho::msg::z::GetMyTrustListRes *pGetMyTrustListRes = pZMsgRes->mutable_get_my_trust_list_res();
				for (size_t i = 0; i != element.m_user_extra_info_list.size(); ++i)
				{
					pGetMyTrustListRes->add_user_extra_info_list()->CopyFrom(element.m_user_extra_info_list[i]);
				}
				element.m_user_extra_info_list.clear();
				break;
			}

			case ::hoosho::msg::z::GET_TRUST_ME_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_TRUST_ME_LIST_RES);
				for (size_t i = 0; i != element.m_user_info_list.size(); ++i)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				element.m_user_info_list.clear();

				::hoosho::msg::z::GetTrustMeListRes *pGetTrustMeListRes = pZMsgRes->mutable_get_trust_me_list_res();
				for (size_t i = 0; i != element.m_user_extra_info_list.size(); ++i)
				{
					pGetTrustMeListRes->add_user_extra_info_list()->CopyFrom(element.m_user_extra_info_list[i]);
				}
				element.m_user_extra_info_list.clear();

				for (size_t i = 0; i < element.m_user_trust_sb_status_list.size(); i++)
				{
					pGetTrustMeListRes->add_user_trust_sb_status_list()->CopyFrom(element.m_user_trust_sb_status_list[i]);
				}
				element.m_user_trust_sb_status_list.clear();
				break;
			}

			case ::hoosho::msg::z::UPDATE_USER_ACTIVITY_INFO_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::UPDATE_USER_ACTIVITY_INFO_RES);
				break;

			case ::hoosho::msg::z::GET_USER_ACTIVITY_INFO_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_USER_ACTIVITY_INFO_RES);
				pZMsgRes->mutable_get_user_activity_info_res()->mutable_user_activity_info()->CopyFrom(element.m_user_activity_info);	
				break;

			case ::hoosho::msg::z::GET_RANKING_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_RANKING_LIST_RES);
				for (size_t i = 0; i != element.m_user_info_list.size(); ++i)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				element.m_user_info_list.clear();
				break;
			}

			case ::hoosho::msg::z::GET_AVAILABLE_LOTTERY_TIME_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_AVAILABLE_LOTTERY_TIME_RES);

				::hoosho::msg::z::GetAvailableLotteryTimeRes *pGetAvailableLotteryTimeRes = pZMsgRes->mutable_get_available_lottery_time_res();
				pGetAvailableLotteryTimeRes->set_times(element.m_available_lottery_times);				
				break;
			}

			case ::hoosho::msg::z::DO_LOTTERY_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::DO_LOTTERY_RES);
				pZMsgRes->mutable_do_lottery_res()->set_prize(element.m_prize);
				pZMsgRes->mutable_do_lottery_res()->set_prize_desc(element.m_prize_desc);
				break;

			case ::hoosho::msg::z::GET_LOTTERY_RECORD_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_LOTTERY_RECORD_LIST_RES);

				::hoosho::msg::z::GetLotteryRecordListRes *pGetLotteryRecordListRes = pZMsgRes->mutable_get_lottery_record_list_res();
				for(size_t i = 0; i != element.m_lottery_info_list.size(); i++)
				{
					pGetLotteryRecordListRes->add_lottery_info_list()->CopyFrom(element.m_lottery_info_list[i]);
				}
				element.m_lottery_info_list.clear();
				break;
			}

			/******************************************** MSG & NOTICE *************************************************/
			//msg
			case ::hoosho::msg::z::ADD_NEW_MSG_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::ADD_NEW_MSG_RES);
				::hoosho::msg::z::AddNewMsgRes *pAddNewMsgRes = pZMsgRes->mutable_add_new_msg_res();
				pAddNewMsgRes->set_msg_id(element.m_msg_id);
				pAddNewMsgRes->set_create_ts(element.m_msg_create_ts);				
				break;
			}

			case ::hoosho::msg::z::GET_MSG_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_MSG_LIST_RES);
				for(size_t i = 0; i != element.m_user_info_list.size(); i++)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				element.m_user_info_list.clear();

				::hoosho::msg::z::GetMsgListRes *pGetMsgListRes = pZMsgRes->mutable_get_msg_list_res();
				for(size_t i = 0; i != element.m_msg_content_list.size(); i++)
				{
					pGetMsgListRes->add_msg_content_list()->CopyFrom(element.m_msg_content_list[i]);
				}
				element.m_msg_content_list.clear();
				break;
			}
			
			case ::hoosho::msg::z::DEL_MSG_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::DEL_MSG_RES);
				break;
				
			case ::hoosho::msg::z::GET_SESSION_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_SESSION_LIST_RES);
				for(size_t i = 0; i != element.m_user_info_list.size(); i++)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				element.m_user_info_list.clear();

				::hoosho::msg::z::GetSessionListRes *pGetSessionListRes = pZMsgRes->mutable_get_session_list_res();
				for(size_t i = 0; i != element.m_msg_session_list.size(); i++)
				{
					pGetSessionListRes->add_session_list()->CopyFrom(element.m_msg_session_list[i]);
				}
				element.m_msg_session_list.clear();

				for(size_t i = 0; i != element.m_user_extra_info_list.size(); i++)
				{
					pGetSessionListRes->add_user_extra_info_list()->CopyFrom(element.m_user_extra_info_list[i]);
				}
				element.m_user_extra_info_list.clear();
				break;
			}	
			case ::hoosho::msg::z::DEL_SESSION_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::DEL_SESSION_RES);
				break;
				
			case ::hoosho::msg::z::GET_NEWMSG_STATUS_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_NEWMSG_STATUS_RES);

				::hoosho::msg::z::GetNewMsgStatusRes *pGetNewMsgStatusRes = pZMsgRes->mutable_get_newmsg_status_res();
				pGetNewMsgStatusRes->set_newmsg_status(element.m_newmsg_status);
				break;
			}
			
			//redpoint
			case ::hoosho::msg::z::GET_RED_POINT_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_RED_POINT_RES);

				::hoosho::msg::z::GetRedPointRes *pGetRedPointRes = pZMsgRes->mutable_get_red_point_res();
				for(size_t i = 0; i != element.m_red_point_info_list.size(); i++)
				{
					pGetRedPointRes->add_red_point_info_list()->CopyFrom(element.m_red_point_info_list[i]);
				}
				element.m_red_point_info_list.clear();
				break;
			}
			
			case ::hoosho::msg::z::UPDATE_RED_POINT_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::UPDATE_RED_POINT_RES);
				break;

			//notice
			case ::hoosho::msg::z::ADD_NOTICE_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::ADD_NOTICE_REQ);
				break;		
				
			case ::hoosho::msg::z::GET_NOTICE_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_NOTICE_RES);

				::hoosho::msg::z::GetNoticeRes *pGetNoticeRes = pZMsgRes->mutable_get_notice_res();
				for(size_t i = 0; i != element.m_notice_info_list.size(); i++)
				{
					pGetNoticeRes->add_notice_info_list()->CopyFrom(element.m_notice_info_list[i]);
				}
				element.m_notice_info_list.clear();

				for(size_t i = 0; i != element.m_order_follow_info_list.size(); i++)
				{
					pGetNoticeRes->add_order_follow_info_list()->CopyFrom(element.m_order_follow_info_list[i]);
				}
				element.m_order_follow_info_list.clear();

				for(size_t i = 0; i != element.m_order_info_list.size(); i++)
				{
					pGetNoticeRes->add_order_info_list()->CopyFrom(element.m_order_info_list[i]);
				}
				element.m_order_info_list.clear();

				for(size_t i = 0; i != element.m_user_info_list.size(); i++)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				element.m_user_info_list.clear();

				for(size_t i = 0; i != element.m_user_extra_info_list.size(); i++)
				{
					pGetNoticeRes->add_user_extra_info_list()->CopyFrom(element.m_user_extra_info_list[i]);
				}
				element.m_user_extra_info_list.clear();
				break;
			}				
				
			/******************************************** ORDER ********************************************************/

			case ::hoosho::msg::z::CREATE_ORDER_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::CREATE_ORDER_RES);
				::hoosho::msg::z::CreateOrderRes *pCreateOrderRes = pZMsgRes->mutable_create_order_res();
				pCreateOrderRes->set_orderid(element.m_order_info.order_id());
				pCreateOrderRes->set_cost_type(element.m_cost_type);
				pCreateOrderRes->mutable_wx_unified_order_param()->CopyFrom(element.m_wx_unified_order_param);
				pCreateOrderRes->set_reward_fee(element.m_reward_fee);
				break;
			}

			case ::hoosho::msg::z::FETCH_ORDER_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::FETCH_ORDER_RES);
				break;

			case ::hoosho::msg::z::CANCEL_ORDER_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::CANCEL_ORDER_RES);
				break;

			case ::hoosho::msg::z::FINISH_ORDER_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::FINISH_ORDER_RES);
				break;

			case ::hoosho::msg::z::COMMENT_ORDER_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::COMMENT_ORDER_RES);
				break;

			case ::hoosho::msg::z::SEARCH_ORDER_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::SEARCH_ORDER_RES);
				for (size_t i = 0; i < element.m_order_info_list.size(); i++)
				{
					pZMsgRes->add_order_info_list()->CopyFrom(element.m_order_info_list[i]);
				}
				for (size_t i = 0; i < element.m_user_info_list.size(); i++)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				::hoosho::msg::z::SearchOrderRes *pSearchOrderRes = pZMsgRes->mutable_search_order_res();
				for (size_t i = 0; i < element.m_user_extra_info_list.size(); i++)
				{
					pSearchOrderRes->add_user_extra_info_list()->CopyFrom(element.m_user_extra_info_list[i]);
				}

				break;
			}

			case ::hoosho::msg::z::GET_CASH_FLOW_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_CASH_FLOW_RES);
				for (size_t i = 0; i < element.m_cash_flow_list.size(); i++)
				{
					pZMsgRes->mutable_get_cash_flow_res()->add_cash_flow_info_list()->CopyFrom(element.m_cash_flow_list[i]);
				}
				break;

			case ::hoosho::msg::z::GET_OUTCOME_LIST_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_OUTCOME_LIST_RES);
				for (size_t i = 0; i < element.m_outcome_req_list.size(); i++)
				{
					pZMsgRes->mutable_get_outcomet_res()->add_outcome_req_info_list()->CopyFrom(element.m_outcome_req_list[i]);
				}
				break;

			case ::hoosho::msg::z::COMMIT_OUTCOME_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::COMMIT_OUTCOME_RES);
				pZMsgRes->mutable_commit_outcome_res()->set_balance(element.m_balance);
				pZMsgRes->mutable_commit_outcome_res()->set_pending(element.m_pending);
				break;

			case ::hoosho::msg::z::GET_ORDER_DETAIL_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_ORDER_DETAIL_RES);

				for (size_t i = 0; i < element.m_order_info_list.size(); i++)
				{
					pZMsgRes->add_order_info_list()->CopyFrom(element.m_order_info_list[i]);
				}

				for (size_t i = 0; i < element.m_user_info_list.size(); i++)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}

				::hoosho::msg::z::GetOrderDetailRes *pGetOrderDetailRes = pZMsgRes->mutable_get_order_detail_res();
				for (size_t i = 0; i < element.m_user_extra_info_list.size(); i++)
				{
					pGetOrderDetailRes->add_user_extra_info_list()->CopyFrom(element.m_user_extra_info_list[i]);
				}
				break;
			}

			case ::hoosho::msg::z::GET_USER_CREATE_ORDER_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_USER_CREATE_ORDER_LIST_RES);

				for (size_t i = 0; i < element.m_order_info_list.size(); i++)
				{
					pZMsgRes->add_order_info_list()->CopyFrom(element.m_order_info_list[i]);
				}
				for (size_t i = 0; i < element.m_user_info_list.size(); i++)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}

				::hoosho::msg::z::GetUserCreateOrderListRes *pGetUserCreateOrderListRes = pZMsgRes->mutable_get_user_create_order_list_res();
				for (size_t i = 0; i < element.m_user_extra_info_list.size(); i++)
				{
					pGetUserCreateOrderListRes->add_user_extra_info_list()->CopyFrom(element.m_user_extra_info_list[i]);
				}
				break;
			}

			case ::hoosho::msg::z::GET_USER_FETCH_ORDER_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_USER_FETCH_ORDER_LIST_RES);

				for(size_t i=0; i<element.m_order_info_list.size(); i++)
				{
					pZMsgRes->add_order_info_list()->CopyFrom(element.m_order_info_list[i]);
				}
				for(size_t i=0; i<element.m_user_info_list.size(); i++)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}

				::hoosho::msg::z::GetUserFetchOrderListRes *pGetUserFetchOrderListRes = pZMsgRes->mutable_get_user_fetch_order_list_res();
				for (size_t i = 0; i < element.m_user_extra_info_list.size(); i++)
				{
					pGetUserFetchOrderListRes->add_user_extra_info_list()->CopyFrom(element.m_user_extra_info_list[i]);
				}
				for (size_t i = 0; i < element.m_order_openid_fetch_state_list.size(); i++)
				{
					pGetUserFetchOrderListRes->add_order_openid_fetch_state_list()->CopyFrom(element.m_order_openid_fetch_state_list[i]);
				}
				break;
			}

			case ::hoosho::msg::z::REWARD_ORDER_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::REWARD_ORDER_RES);
				::hoosho::msg::z::RewardOrderRes *pRewardOrderRes = pZMsgRes->mutable_reward_order_res();
				pRewardOrderRes->set_cost_type(element.m_cost_type);
				pRewardOrderRes->mutable_wx_unified_order_param()->CopyFrom(element.m_wx_unified_order_param);
				break;
			}
			
			/*******************************************FEEDS**********************************************************/
			case ::hoosho::msg::z::CHECK_ORDER_FAVOR_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::CHECK_ORDER_FAVOR_RES);
				::hoosho::msg::z::CheckOrderFavorRes *pCheckOrderFavorRes = pZMsgRes->mutable_check_order_favor_res();
				for(size_t i = 0; i < element.m_order_favor_info_list.size(); i++)
				{
					pCheckOrderFavorRes->add_favor_list()->CopyFrom(element.m_order_favor_info_list[i]);
				}
				break;
			}

			case ::hoosho::msg::z::UPDATE_ORDER_FAVOR_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::UPDATE_ORDER_FAVOR_RES);
				break;

			case ::hoosho::msg::z::GET_ORDER_FAVOR_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_ORDER_FAVOR_LIST_RES);
				for(size_t i = 0; i < element.m_user_info_list.size(); i++)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				::hoosho::msg::z::GetOrderFavorListRes *pGetOrderFavorListRes = pZMsgRes->mutable_get_order_favor_list_res();
				for(size_t i = 0; i < element.m_order_favor_info_list.size(); i++)
				{
					pGetOrderFavorListRes->add_favor_list()->CopyFrom(element.m_order_favor_info_list[i]);
				}
				break;
			}

			case ::hoosho::msg::z::ADD_ORDER_FOLLOW_REQ: 
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::ADD_ORDER_FOLLOW_RES);
				::hoosho::msg::z::AddOrderFollowRes *pAddOrderFollowRes = pZMsgRes->mutable_add_order_follow_res();
				pAddOrderFollowRes->set_follow_id(element.m_follow_id);
				break;
			}

			case ::hoosho::msg::z::GET_ORDER_COMMENT_LIST_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_ORDER_COMMENT_LIST_RES);
				for(size_t i = 0; i < element.m_user_info_list.size(); i++)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				::hoosho::msg::z::GetOrderCommentListRes *pGetOrderCommentListRes = pZMsgRes->mutable_get_order_comment_list_res();
				for(size_t i = 0; i < element.m_order_comment_info_list.size(); i++)
				{
					pGetOrderCommentListRes->add_comment_list()->CopyFrom(element.m_order_comment_info_list[i]);
				}
				break;
			}

			case ::hoosho::msg::z::GET_ORDER_COMMENT_DETAIL_REQ:
			{
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::GET_ORDER_COMMENT_DETAIL_RES);
				for(size_t i = 0; i < element.m_user_info_list.size(); i++)
				{
					pZMsgRes->add_user_info_list()->CopyFrom(element.m_user_info_list[i]);
				}
				::hoosho::msg::z::GetOrderCommentDetailRes *pGetOrderCommentDetailRes = pZMsgRes->mutable_get_order_comment_detail_res();
				for(size_t i = 0; i < element.m_order_comment_info_list.size(); i++)
				{
					pGetOrderCommentDetailRes->add_comment_list()->CopyFrom(element.m_order_comment_info_list[i]);
				}
				break;
			}

			case ::hoosho::msg::z::DEL_ORDER_FOLLOW_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::DEL_ORDER_FOLLOW_RES);
				break;

			case ::hoosho::msg::z::ORDER_FETCH_CANCEL_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::ORDER_FETCH_CANCEL_RES);
				break;
			
			case ::hoosho::msg::z::ORDER_FETCH_ACCEPT_REQ:
				pZMsgRes->set_sub_cmd(::hoosho::msg::z::ORDER_FETCH_ACCEPT_RES);
				break;
				
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

