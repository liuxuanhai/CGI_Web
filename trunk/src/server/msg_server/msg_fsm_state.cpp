#include "msg_fsm_state.h"
#include "global_var.h"
#include "msg_content.h"
#include "util/str_op.h"


    /*******************************************************************************************************/
    /*					                       MsgFsmState						       					   */
    /*******************************************************************************************************/
    IMPL_LOGGER(MsgFsmState, logger);

#define IMPL_FSM_STATE(classname, name) classname MsgFsmState::name(#classname)
    IMPL_FSM_STATE(MsgFsmStateInit, state_init);
    IMPL_FSM_STATE(MsgFsmStateWaitQueryDB, state_wait_query_db);
    IMPL_FSM_STATE(MsgFsmStateWaitAddDB, state_wait_add_db);
    IMPL_FSM_STATE(MsgFsmStateWaitDelDB, state_wait_del_db);
    IMPL_FSM_STATE(MsgFsmStateEnd, state_end);
#undef IMPL_FSM_STATE

	bool SortByNewmsgStatus(const ::hoosho::commstruct::Session &s1, const ::hoosho::commstruct::Session &s2)
	{
		return s1.newmsg_status() < s2.newmsg_status(); 	//DESC
	}

	bool SortByCreateTS(const ::hoosho::commstruct::Session &s1, const ::hoosho::commstruct::Session &s2)
	{
		return s1.create_ts() < s2.create_ts(); 	//DESC
	}

	void sessionSort(std::vector<hoosho::commstruct::Session>& vecSession)
	{
		std::sort(vecSession.begin(), vecSession.end(), SortByCreateTS);
		std::sort(vecSession.begin(), vecSession.end(), SortByNewmsgStatus);
	}
	
    void MsgFsmState::enter(MsgFsm& fsm)
    {

    }

    void MsgFsmState::exit(MsgFsm& fsm)
    {

    }

    void MsgFsmState::client_req_event(MsgFsm & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
    }

	void MsgFsmState::db_reply_event(MsgFsm& fsm, ExecutorThreadResponseElement& element)
    {
		LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
    }

    void MsgFsmState::timeout_event(MsgFsm& fsm, void* param)
    {
		LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
    }

    /*******************************************************************************************************/
    /*                                         MsgFsmStateInit                                         */
    /*******************************************************************************************************/
    void MsgFsmStateInit::client_req_event(MsgFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        fsm._conn_id = conn.get_id();
        fsm._msg.CopyFrom(stMsg);       

		if(::hoosho::msg::QUERY_MSG_NEWMSG_STATUS_REQ == stMsg.head().cmd())
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_NEWMSG_STATUS_QUERY, fsm._id);			
            req.need_reply();
            
            req.m_msg_content.m_openid_md5_from = stMsg.msg_query_newmsg_status_req().openid_md5();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(MsgFsmState::state_wait_query_db);
            return;
		}
		else if(::hoosho::msg::QUERY_MSG_SESSION_LIST_REQ == stMsg.head().cmd())
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_SESSION_LIST_QUERY, fsm._id);
            req.need_reply();
            
            req.m_msg_content.m_openid_md5_from = stMsg.msg_query_session_list_req().openid_md5();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(MsgFsmState::state_wait_query_db);
            return;
		}		
		else if(::hoosho::msg::QUERY_MSG_REQ == stMsg.head().cmd())
		{
			const ::hoosho::sixin::QuerySixinReq& stMsgQueryReq = stMsg.msg_query_req();
			/*
			MsgContent* daoUserInfo = g_msg_content_cache_pool->get(stQueryReq.id());
			if(daoUserInfo)
			{
				LOG4CPLUS_DEBUG(logger, "in cache, user.id="<<stQueryReq.id());

				//msg
				hoosho::msg::Msg stRespMsg;
				//header
				hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
				header->set_cmd(hoosho::msg::QUERY_USER_INFO_RES);
				header->set_seq(stMsg.head().seq());
				header->set_result(0);
				//body
				hoosho::test::QueryRes* pBody = stRespMsg.mutable_query_res();
				hoosho::commstruct::UserInfo* pBodyUserInfo = pBody->mutable_user_info();
				pBodyUserInfo->set_id(daoUserInfo->m_id);
				pBodyUserInfo->set_name(daoUserInfo->m_name);
				pBodyUserInfo->set_age(daoUserInfo->m_age);

				//send
				g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
				fsm.set_state(MsgFsmState::state_end);
				return;
			}
			*/

			//if not exsts in cache, query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_CONTENT_QUERY, fsm._id);
            req.need_reply();
            
            req.m_msg_content.m_id = stMsgQueryReq.msg_id();
            req.m_msg_content.m_amount = stMsgQueryReq.amount();
            req.m_msg_content.m_openid_md5_from = stMsgQueryReq.openid_md5_from();
            req.m_msg_content.m_openid_md5_to = stMsgQueryReq.openid_md5_to();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(MsgFsmState::state_wait_query_db);
            return;
		}		
        else if(hoosho::msg::ADD_MSG_REQ == stMsg.head().cmd())
		{
			const hoosho::sixin::AddSixinReq& stMsgAddReq = stMsg.msg_add_req();
			/*
			MsgContent* daoUserInfo = g_msg_content_cache_pool->get(stMsgAddReq.user_info().id());
			if(daoUserInfo)
			{
				LOG4CPLUS_DEBUG(logger, "in cache, repeated, failed, user.id="<<stMsgAddReq.user_info().id());
				
				//msg
				hoosho::msg::Msg stRespMsg;
				//header
				hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
				header->set_cmd(hoosho::msg::ADD_USER_INFO_RES);
				header->set_seq(stMsg.head().seq());
				header->set_result(-1);

				//send
				g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
				fsm.set_state(MsgFsmState::state_end);
				return;
			}
			*/

			//save async data
			const hoosho::commstruct::MsgContent& msg_content = stMsgAddReq.msg_content();
			
			//if not exsts in cache, query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_CONTENT_ADD, fsm._id);
            req.need_reply();
            
            req.m_msg_content.m_content = msg_content.content();
            req.m_msg_content.m_openid_md5_from = msg_content.openid_md5_from();
            req.m_msg_content.m_openid_md5_to = msg_content.openid_md5_to();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(MsgFsmState::state_wait_add_db);
            return;
		}		
		else if(hoosho::msg::DELETE_MSG_REQ == stMsg.head().cmd())
		{
			const hoosho::sixin::DeleteSixinReq& stMsgDeleteReq = stMsg.msg_del_req();
			/*
			MsgContent* daoUserInfo = g_msg_content_cache_pool->get(stDeleteReq.id());
			if(daoUserInfo)
			{
				//remove from cache
				LOG4CPLUS_DEBUG(logger, "in cache, delete cache, user.id="<<stDeleteReq.id());
				g_msg_content_cache_pool->remove(stDeleteReq.id());
			}
			*/

			//delete from db , not care reply
			LOG4CPLUS_DEBUG(logger, "then, delete db, msg_id="<<stMsgDeleteReq.msg_id());
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_CONTENT_DELETE, fsm._id);            
            req.need_reply();

            req.m_msg_content.m_id = stMsgDeleteReq.msg_id();
            req.m_msg_content.m_openid_md5_from = stMsgDeleteReq.openid_md5_from();
            req.m_msg_content.m_openid_md5_to = stMsgDeleteReq.openid_md5_to();
            g_executor_thread_processor->send_request(req);
			fsm.set_state(MsgFsmState::state_wait_del_db);
			return;
		}
		else if(::hoosho::msg::DELETE_SESSION_REQ == stMsg.head().cmd())
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_SESSION_DELETE, fsm._id);
            req.need_reply();
            
            req.m_msg_content.m_openid_md5_from = stMsg.session_del_req().openid_md5_from();
            req.m_msg_content.m_openid_md5_to = stMsg.session_del_req().openid_md5_to();
            g_executor_thread_processor->send_request(req);
			fsm.set_state(MsgFsmState::state_wait_del_db);
			return;
		}
		else if(::hoosho::msg::QUERY_SYSTEM_RED_POINT_REQ == stMsg.head().cmd())
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_RED_POINT_QUERY, fsm._id);
			req.need_reply();
			
			req.m_openid_md5 = stMsg.query_system_red_point_req().openid_md5();
			req.m_pa_appid_md5 = stMsg.query_system_red_point_req().pa_appid_md5();
            g_executor_thread_processor->send_request(req);
			fsm.set_state(MsgFsmState::state_wait_query_db);
			return;
		}
		else if(::hoosho::msg::UPDATE_SYSTEM_RED_POINT_REQ == stMsg.head().cmd())
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_RED_POINT_UPDATE, fsm._id);
			req.need_reply();
			
			req.m_openid_md5 = stMsg.update_system_red_point_req().openid_md5();
			req.m_pa_appid_md5 = stMsg.update_system_red_point_req().pa_appid_md5();
			req.m_redpoint_type = stMsg.update_system_red_point_req().type();
			req.m_redpoint_opt = stMsg.update_system_red_point_req().opt();
            g_executor_thread_processor->send_request(req);
			fsm.set_state(MsgFsmState::state_wait_del_db);
			return;
		}
		else if(::hoosho::msg::ADD_NOTICE_RECORD_REQ == stMsg.head().cmd())
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_NOTICE_RECORD_ADD, fsm._id);
			req.need_reply();

			req.m_notice_record.CopyFrom(stMsg.notice_record_add_req().notice_record());
			
			g_executor_thread_processor->send_request(req);
			fsm.set_state(MsgFsmState::state_wait_add_db);
			return;
		}
		else if(::hoosho::msg::QUERY_NOTICE_RECORD_REQ == stMsg.head().cmd())
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_NOTICE_RECORD_QUERY, fsm._id);
			req.need_reply();

			req.m_pa_appid_md5 = stMsg.notice_record_query_req().pa_appid_md5();
			req.m_openid_md5 = stMsg.notice_record_query_req().openid_md5();
			req.m_notice_record_type = stMsg.notice_record_query_req().type();
			req.m_limit_ts = stMsg.notice_record_query_req().limit_ts();
			req.m_pagesize = stMsg.notice_record_query_req().pagesize();
			
			g_executor_thread_processor->send_request(req);
			fsm.set_state(MsgFsmState::state_wait_query_db);
			return;
		}
		else if(::hoosho::msg::DELETE_NOTICE_RECORD_REQ == stMsg.head().cmd())
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_NOTICE_RECORD_DELETE, fsm._id);
			//req.need_reply();

			req.m_pa_appid_md5 = stMsg.notice_record_delete_req().pa_appid_md5();
			req.m_openid_md5 = stMsg.notice_record_delete_req().openid_md5();
			req.m_create_ts = stMsg.notice_record_delete_req().create_ts();
			req.m_notice_record_type = stMsg.notice_record_delete_req().type();
			
			g_executor_thread_processor->send_request(req);
			fsm.set_state(MsgFsmState::state_end);
			return;
		}
		else if(::hoosho::msg::QUERY_SYSTEM_MSG_REQ == stMsg.head().cmd())
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_MSG_SYSTEM_MSG_QUERY, fsm._id);
			req.need_reply();

			req.m_openid_md5 = stMsg.system_msg_query_req().openid_md5();
			req.m_pa_appid_md5 = stMsg.system_msg_query_req().pa_appid_md5();
			req.m_limit_ts = stMsg.system_msg_query_req().begin_ts();
			req.m_pagesize = stMsg.system_msg_query_req().limit();
			
			g_executor_thread_processor->send_request(req);
			fsm.set_state(MsgFsmState::state_wait_query_db);
			return;
		}
		
    }


    /*******************************************************************************************************/
    /*                                         MsgFsmStateWaitQueryDB                               */
    /*******************************************************************************************************/
    void MsgFsmStateWaitQueryDB::enter(MsgFsm& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void MsgFsmStateWaitQueryDB::exit(MsgFsm& fsm)
    {
        fsm.cancel_timer();
    }

    void MsgFsmStateWaitQueryDB::db_reply_event(MsgFsm& fsm, ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		//resp msg
		hoosho::msg::Msg stRespMsg;
        hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_seq(fsm._msg.head().seq());
        header->set_result(::hoosho::msg::E_OK);

		uint32_t dwReqType = fsm._msg.head().cmd();
        uint32_t dwStatus = reply.m_result_code;
        
        if(::hoosho::msg::QUERY_MSG_REQ == dwReqType)
        {
        	header->set_cmd(hoosho::msg::QUERY_MSG_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{
        		//update cache
				//g_msg_content_cache_pool->add(msgContent);

				::hoosho::sixin::QuerySixinRes* pBody = stRespMsg.mutable_msg_query_res();
				for(size_t i=0; i<reply.vecMsgContent.size(); i++)
				{
					pBody->add_msg_content()->CopyFrom(reply.vecMsgContent[i]);
				}
				reply.vecMsgContent.clear();
        	}
        	else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
        	{
				header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
        	}
        	else //E_DB_FAIL
        	{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(MsgFsmState::state_end);
        	return;	
        }

        if(::hoosho::msg::QUERY_MSG_NEWMSG_STATUS_REQ == dwReqType)
        {
        	header->set_cmd(hoosho::msg::QUERY_MSG_NEWMSG_STATUS_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{
        		//update cache
				//g_msg_content_cache_pool->add(msgContent);

				::hoosho::sixin::QuerySixinNewMsgStatusRes* pBody = stRespMsg.mutable_msg_query_newmsg_status_res();
				pBody->set_newmsg_status(reply.newmsg_status);
				
        	}
        	else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
        	{
				header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
        	}
        	else //E_DB_FAIL
        	{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(MsgFsmState::state_end);
        	return;	
        }

        if(::hoosho::msg::QUERY_MSG_SESSION_LIST_REQ == dwReqType)
        {
        	header->set_cmd(hoosho::msg::QUERY_MSG_SESSION_LIST_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{
        		//update cache
				//g_msg_content_cache_pool->add(msgContent);

				//sort first by create_ts then by newmsg_status
				sessionSort(reply.vecSession);

				::hoosho::sixin::QuerySixinSessionListRes* pBody = stRespMsg.mutable_msg_query_session_list_res();
				for(size_t i=0; i<reply.vecSession.size(); i++)
				{
					pBody->add_session()->CopyFrom(reply.vecSession[i]);
				}
				reply.vecSession.clear();
				
        	}
        	else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
        	{
				header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
        	}
        	else //E_DB_FAIL
        	{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(MsgFsmState::state_end);
        	return;	
        }

        if(::hoosho::msg::QUERY_SYSTEM_RED_POINT_REQ == dwReqType)
        {
			header->set_cmd(hoosho::msg::QUERY_SYSTEM_RED_POINT_RES);

			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{        		
				::hoosho::sixin::QuerySystemRedPointRes* pBody = stRespMsg.mutable_query_system_red_point_res();
				for(size_t i=0; i<reply.vecRedPointInfo.size(); i++)
				{
					pBody->add_red_point_info()->CopyFrom(reply.vecRedPointInfo[i]);
				}
				reply.vecRedPointInfo.clear();
				
        	}
        	else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
        	{
				header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
        	}
        	else //E_DB_FAIL
        	{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(MsgFsmState::state_end);
        	return;	
        }

        if(::hoosho::msg::QUERY_NOTICE_RECORD_REQ == dwReqType)
        {
			header->set_cmd(hoosho::msg::QUERY_NOTICE_RECORD_RES);

			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{        		
				::hoosho::sixin::QueryNoticeRecordRes* pBody = stRespMsg.mutable_notice_record_query_res();
				for(size_t i=0; i<reply.vecNoticeRecord.size(); i++)
				{
					pBody->add_notice_record_list()->CopyFrom(reply.vecNoticeRecord[i]);
				}
				reply.vecNoticeRecord.clear();
				
        	}
        	else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
        	{
				header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
        	}
        	else //E_DB_FAIL
        	{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(MsgFsmState::state_end);
        	return;	
        }

        if(::hoosho::msg::QUERY_SYSTEM_MSG_REQ == dwReqType)
        {
			header->set_cmd(hoosho::msg::QUERY_SYSTEM_MSG_RES);

			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{        		
				::hoosho::sixin::QuerySystemMsgRes* pBody = stRespMsg.mutable_system_msg_query_res();
				for(size_t i=0; i<reply.vecSystemMsg.size(); i++)
				{
					pBody->add_system_msg_list()->CopyFrom(reply.vecSystemMsg[i]);
				}
				reply.vecSystemMsg.clear();
				
        	}
        	else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
        	{
				header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
        	}
        	else //E_DB_FAIL
        	{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(MsgFsmState::state_end);
        	return;	
        }        

    }

    void MsgFsmStateWaitQueryDB::timeout_event(MsgFsm& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "MsgFsmStateWaitQueryDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(MsgFsmState::state_end);
        return;
    }


	/*******************************************************************************************************/
    /*                                         MsgFsmStateWaitAddDB                               		   */
    /*******************************************************************************************************/
    void MsgFsmStateWaitAddDB::enter(MsgFsm & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void MsgFsmStateWaitAddDB::exit(MsgFsm& fsm)
    {
        fsm.cancel_timer();
    }

    void MsgFsmStateWaitAddDB::db_reply_event(MsgFsm& fsm, ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();		

        //resp msg
		hoosho::msg::Msg stRespMsg;
        hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_seq(fsm._msg.head().seq());
        header->set_result(::hoosho::msg::E_OK);

		uint32_t dwReqType = fsm._msg.head().cmd();
        uint32_t dwStatus = reply.m_result_code;
        const MsgContent& msgContent = reply.m_msg_content;

        if(::hoosho::msg::ADD_MSG_REQ == dwReqType)
		{
			header->set_cmd(::hoosho::msg::ADD_MSG_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{
        		//update cache
				//g_msg_content_cache_pool->add(msgContent);

				::hoosho::sixin::AddSixinRes* stMsgAddRes = stRespMsg.mutable_msg_add_res();
				stMsgAddRes->set_msg_id(msgContent.m_id);
				stMsgAddRes->set_create_ts(msgContent.m_create_ts);				
        	} 
        	else	//E_DB_FAIL
        	{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
        	}
        	
			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        	fsm.set_state(MsgFsmState::state_end);
			return;	
		}

		if(::hoosho::msg::ADD_NOTICE_RECORD_REQ == dwReqType)
		{
			header->set_cmd(::hoosho::msg::ADD_NOTICE_RECORD_RES);
			if(ExecutorThreadRequestType::E_OK != dwStatus) //E_DB_FAIL        		
        	{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
        	}
        	
			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        	fsm.set_state(MsgFsmState::state_end);
			return;	
		}
    }

    void MsgFsmStateWaitAddDB::timeout_event(MsgFsm& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "MsgFsmStateWaitAddDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(MsgFsmState::state_end);
        return;
    }

    /*******************************************************************************************************/
    /*                                         MsgFsmStateWaitDelDB                               		   */
    /*******************************************************************************************************/
    void MsgFsmStateWaitDelDB::enter(MsgFsm & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void MsgFsmStateWaitDelDB::exit(MsgFsm& fsm)
    {
        fsm.cancel_timer();
    }

    void MsgFsmStateWaitDelDB::db_reply_event(MsgFsm& fsm, ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();		

        //resp msg
		hoosho::msg::Msg stRespMsg;
        hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_seq(fsm._msg.head().seq());
        header->set_result(::hoosho::msg::E_OK);

		uint32_t dwReqType = fsm._msg.head().cmd();
        uint32_t dwStatus = reply.m_result_code;

        if(::hoosho::msg::DELETE_MSG_REQ == dwReqType)
		{
			header->set_cmd(::hoosho::msg::DELETE_MSG_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
			{
				header->set_result(::hoosho::msg::E_OK);
			}
			else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
			{
				header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
			}
			else	//E_DB_FAIL
			{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        	fsm.set_state(MsgFsmState::state_end);
			return;
		}

		if(::hoosho::msg::DELETE_SESSION_REQ == dwReqType)
		{
			header->set_cmd(::hoosho::msg::DELETE_SESSION_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
			{
				header->set_result(::hoosho::msg::E_OK);
			}
			else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
			{
				header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
			}
			else	//E_DB_FAIL
			{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        	fsm.set_state(MsgFsmState::state_end);
			return;
		}

		if(::hoosho::msg::UPDATE_SYSTEM_RED_POINT_REQ == dwReqType)
		{
			header->set_cmd(::hoosho::msg::UPDATE_SYSTEM_RED_POINT_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
			{
				header->set_result(::hoosho::msg::E_OK);
			}
			else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
			{
				header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
			}
			else	//E_DB_FAIL
			{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        	fsm.set_state(MsgFsmState::state_end);
			return;
		}

		if(::hoosho::msg::DELETE_NOTICE_RECORD_REQ == dwReqType)
		{
			header->set_cmd(::hoosho::msg::DELETE_NOTICE_RECORD_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
			{
				header->set_result(::hoosho::msg::E_OK);
			}
			else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
			{
				header->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
			}
			else	//E_DB_FAIL
			{
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        	fsm.set_state(MsgFsmState::state_end);
			return;
		}
    }

    void MsgFsmStateWaitDelDB::timeout_event(MsgFsm& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "MsgFsmStateWaitDelDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(MsgFsmState::state_end);
        return;
    }


    /*******************************************************************************************************/
    /*                                         MsgFsmStateEnd                                         */
    /*******************************************************************************************************/
    void MsgFsmStateEnd::enter(MsgFsm& fsm)
    {
        g_msg_content_fsm_container->del_fsm(&fsm);
    }

