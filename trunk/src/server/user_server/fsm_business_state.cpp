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

		if(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_USERINFO_BATCH_QUERY
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			for(int i=0; i<stMsg.query_user_detail_info_req().openid_md5_list_size(); ++i)
			{
				stThreadRequest.m_openid_md5_list.push_back(stMsg.query_user_detail_info_req().openid_md5_list(i));
			}
			g_executor_thread_processor->send_request(stThreadRequest);
		}
		else if(::hoosho::msg::UPDATE_USER_DETAIL_INFO_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_USERINFO_UPDATE
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			stThreadRequest.m_user_detail_info_from.CopyFrom(stMsg.update_user_detail_info_req().user_detail_info());
			g_executor_thread_processor->send_request(stThreadRequest);
		}
		else if(::hoosho::msg::QUERY_USER_FANS_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_USERFANS_LIST
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			stThreadRequest.m_user_detail_info_from.set_openid_md5(stMsg.query_user_fans_req().openid_md5());
			g_executor_thread_processor->send_request(stThreadRequest);
		}
		else if(::hoosho::msg::QUERY_USER_FANS_NUM_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_USERFANS_NUM
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			for(int i=0; i<stMsg.query_user_fans_num_req().openid_md5_list_size(); ++i)
			{
				stThreadRequest.m_openid_md5_list.push_back(stMsg.query_user_fans_num_req().openid_md5_list(i));
			}
			g_executor_thread_processor->send_request(stThreadRequest);
		}
		else if(::hoosho::msg::QUERY_USER_FOLLOWS_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_USERFOLLOWS_LIST
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			stThreadRequest.m_user_detail_info_from.set_openid_md5(stMsg.query_user_follows_req().openid_md5());
			g_executor_thread_processor->send_request(stThreadRequest);
		}
		else if(::hoosho::msg::USER_FOLLOW_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_FOLLOW
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			stThreadRequest.m_user_detail_info_from.set_openid_md5(stMsg.user_follow_req().openid_md5_from());
			stThreadRequest.m_user_detail_info_to.set_openid_md5(stMsg.user_follow_req().openid_md5_to());
			stThreadRequest.m_relation_from_to = stMsg.user_follow_req().relation();
			g_executor_thread_processor->send_request(stThreadRequest);
		}
		else if(::hoosho::msg::QUERY_USER_POWER_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_POWER_QUERY
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			stThreadRequest.m_openid_md5 = stMsg.query_user_power_req().openid_md5();
			stThreadRequest.m_pa_appid_md5 = stMsg.query_user_power_req().appid_md5();
			g_executor_thread_processor->send_request(stThreadRequest);
		}
		else if(::hoosho::msg::QUERY_USER_ZOMBIE_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_ZOMBIE_QUERY
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
			stThreadRequest.m_uin = stMsg.query_user_zombie_req().uin();
			g_executor_thread_processor->send_request(stThreadRequest);
		}
		else if(::hoosho::msg::QUERY_USER_ZOMANAGER_REQ == fsm._msg.head().cmd())
		{
			ExecutorThreadRequestElement stThreadRequest(ExecutorThreadRequestType::T_ZOMANAGER_QUERY
													, FsmContainer<int>::FSM_TYPE_BUSINESS
													, fsm._id);
			stThreadRequest.need_reply();
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
		if(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_RES);
			::hoosho::user::QueryUserDetailInfoRes* pQueryUserDetailInfoRes = stRespMsg.mutable_query_user_detail_info_res();
			for(size_t i=0; i!=element.m_user_detail_info_list.size(); ++i)
			{
				pQueryUserDetailInfoRes->add_user_detail_info_list()->CopyFrom(element.m_user_detail_info_list[i]);
			}
		}
		else if(::hoosho::msg::UPDATE_USER_DETAIL_INFO_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::UPDATE_USER_DETAIL_INFO_RES);
		}
		else if(::hoosho::msg::QUERY_USER_FANS_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::QUERY_USER_FANS_RES);
			::hoosho::user::QueryUserFansRes* pQueryUserFansRes = stRespMsg.mutable_query_user_fans_res();
			for(size_t i=0; i!=element.m_openid_md5_list.size(); ++i)
			{
				pQueryUserFansRes->add_openid_md5_list(element.m_openid_md5_list[i]);
			}

			//clear redpoint table 
			{
				::hoosho::msg::Msg stNoticeRequest;
				::hoosho::msg::MsgHead* pNoticeRequestHead = stNoticeRequest.mutable_head();
				pNoticeRequestHead->set_cmd(::hoosho::msg::QUERY_NOTICE_RECORD_REQ);
				pNoticeRequestHead->set_seq(fsm._id);
				::hoosho::sixin::QueryNoticeRecordReq* pQueryNoticeRecordReq = stNoticeRequest.mutable_notice_record_query_req();
				pQueryNoticeRecordReq->set_openid_md5(fsm._msg.query_user_fans_req().openid_md5());
				pQueryNoticeRecordReq->set_type(::hoosho::commenum::NOTICE_RECORD_TYPE_FANS);
				g_server_processor_msg->send_datagram(stNoticeRequest);
			}			
		}
		else if(::hoosho::msg::QUERY_USER_FANS_NUM_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::QUERY_USER_FANS_NUM_RES);
			for(std::map<uint64_t, uint32_t>::iterator iter = element.m_map_openid_md5_2_fans_num.begin()
				; iter != element.m_map_openid_md5_2_fans_num.end()
				; ++iter)
			{
				::hoosho::commstruct::KeyValueIntInt* pKeyValueIntInt = stRespMsg.mutable_query_user_fans_num_res()->add_fans_num_list();
				pKeyValueIntInt->set_key(iter->first);
				pKeyValueIntInt->set_value(iter->second);
			}
		}
		else if(::hoosho::msg::QUERY_USER_FOLLOWS_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::QUERY_USER_FOLLOWS_RES);
			::hoosho::user::QueryUserFollowsRes* pQueryUserFollowsRes = stRespMsg.mutable_query_user_follows_res();
			for(size_t i=0; i!=element.m_openid_md5_list.size(); ++i)
			{
				pQueryUserFollowsRes->add_openid_md5_list(element.m_openid_md5_list[i]);
			}			
		}
		else if(::hoosho::msg::QUERY_USER_ZOMBIE_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::QUERY_USER_ZOMBIE_RES);
			::hoosho::user::QueryUserZombieRes* pQueryUserZombieRes = stRespMsg.mutable_query_user_zombie_res();
			for(size_t i=0; i!=element.m_openid_md5_list.size(); ++i)
			{
				pQueryUserZombieRes->add_openid_md5_list(element.m_openid_md5_list[i]);
			}
		}
		else if(::hoosho::msg::QUERY_USER_ZOMANAGER_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::QUERY_USER_ZOMANAGER_RES);
			::hoosho::user::QueryUserZomanagerRes* pQueryUserZomanagerRes = stRespMsg.mutable_query_user_zomanager_res();
			for(size_t i=0; i!=element.m_uin_list.size(); ++i)
			{
				pQueryUserZomanagerRes->add_uin_list(element.m_uin_list[i]);
			}
		}
		else if(::hoosho::msg::USER_FOLLOW_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::USER_FOLLOW_RES);
			
			//new follow data created
			if(1 == fsm._msg.user_follow_req().relation())
			{
				//add notice to notice server
				::hoosho::msg::Msg stNoticeRequest;
				::hoosho::msg::MsgHead* pNoticeRequestHead = stNoticeRequest.mutable_head();
				pNoticeRequestHead->set_cmd(::hoosho::msg::ADD_NOTICE_RECORD_REQ);
				pNoticeRequestHead->set_seq(fsm._id);
				::hoosho::sixin::AddNoticeRecordReq* pAddNoticeRecordReq = stNoticeRequest.mutable_notice_record_add_req();
				::hoosho::commstruct::NoticeRecord* pNoticeRecord = pAddNoticeRecordReq->mutable_notice_record();
				pNoticeRecord->set_openid_md5(fsm._msg.user_follow_req().openid_md5_to());
				pNoticeRecord->set_type(::hoosho::commenum::NOTICE_RECORD_TYPE_FANS);
				pNoticeRecord->set_extra_data_0(lce::util::StringOP::TypeToStr(fsm._msg.user_follow_req().openid_md5_from()));
				g_server_processor_msg->send_datagram(stNoticeRequest);
			}
			else if(2 == fsm._msg.user_follow_req().relation())
			{
				//delete 
				::hoosho::msg::Msg stNoticeRequest;
				::hoosho::msg::MsgHead* pNoticeRequestHead = stNoticeRequest.mutable_head();
				pNoticeRequestHead->set_cmd(::hoosho::msg::DELETE_NOTICE_RECORD_REQ);
				pNoticeRequestHead->set_seq(fsm._id);
				::hoosho::sixin::DeleteNoticeRecordReq* pDeleteNoticeRecordReq = stNoticeRequest.mutable_notice_record_delete_req();
				pDeleteNoticeRecordReq->set_openid_md5(fsm._msg.user_follow_req().openid_md5_to());
				pDeleteNoticeRecordReq->set_type(::hoosho::commenum::NOTICE_RECORD_TYPE_FANS);
				g_server_processor_msg->send_datagram(stNoticeRequest);
			}
			
		}
		else if(::hoosho::msg::QUERY_USER_POWER_REQ == fsm._msg.head().cmd())
		{
			header->set_cmd(::hoosho::msg::QUERY_USER_POWER_RES);
			::hoosho::user::QueryUserPowerRes* pQueryUserPowerRes = stRespMsg.mutable_query_user_power_res();
			for(size_t i=0; i!=element.m_power_list.size(); ++i)
			{
				pQueryUserPowerRes->add_power_list(element.m_power_list[i]);
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

