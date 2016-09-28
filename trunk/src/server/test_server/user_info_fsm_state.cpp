#include "user_info_fsm_state.h"
#include "global_var.h"
#include "dao.h"

    /*******************************************************************************************************/
    /*					                       UserInfoFsmState						       					   */
    /*******************************************************************************************************/
    IMPL_LOGGER(UserInfoFsmState, logger);

#define IMPL_FSM_STATE(classname, name) classname UserInfoFsmState::name(#classname)
    IMPL_FSM_STATE(UserInfoFsmStateInit, state_init);
    IMPL_FSM_STATE(UserInfoFsmStateWaitQueryDB, state_wait_query_db);
    IMPL_FSM_STATE(UserInfoFsmStateWaitUpdateDB, state_wait_update_db);
    IMPL_FSM_STATE(UserInfoFsmStateEnd, state_end);
#undef IMPL_FSM_STATE

    void UserInfoFsmState::enter(UserInfoFsm& fsm)
    {

    }

    void UserInfoFsmState::exit(UserInfoFsm& fsm)
    {

    }

    void UserInfoFsmState::client_req_event(UserInfoFsm & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
    }

	void UserInfoFsmState::db_reply_event(UserInfoFsm& fsm, const ExecutorThreadResponseElement& element)
    {
		LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
    }

    void UserInfoFsmState::timeout_event(UserInfoFsm& fsm, void* param)
    {
		LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
    }

    /*******************************************************************************************************/
    /*                                         UserInfoFsm                                         */
    /*******************************************************************************************************/
    void UserInfoFsmStateInit::client_req_event(UserInfoFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();
        fsm._req_type = stMsg.head().cmd();
        fsm._req_seq_id = stMsg.head().seq();

		//dispatch
		uint32_t dwReqType = stMsg.head().cmd();

		//query_req
		if(hoosho::msg::QUERY_USER_INFO_REQ == dwReqType)
		{
			const hoosho::test::QueryReq& stQueryReq = stMsg.query_req();
			DaoUserInfo* daoUserInfo = g_user_info_cache_pool->get(stQueryReq.id());
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
				fsm.set_state(UserInfoFsmState::state_end);
				return;
			}

			//if not exsts in cache, query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_USER_INFO_QUERY);
            req.m_fsm_id = fsm._id;
            req.m_user_info.m_id = stQueryReq.id();
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(UserInfoFsmState::state_wait_query_db);
            return;
		}

		//add_req
        if(hoosho::msg::ADD_USER_INFO_REQ == dwReqType)
		{
			const hoosho::test::AddReq& stAddReq = stMsg.add_req();
			DaoUserInfo* daoUserInfo = g_user_info_cache_pool->get(stAddReq.user_info().id());
			if(daoUserInfo)
			{
				LOG4CPLUS_DEBUG(logger, "in cache, repeated, failed, user.id="<<stAddReq.user_info().id());
				
				//msg
				hoosho::msg::Msg stRespMsg;
				//header
				hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
				header->set_cmd(hoosho::msg::ADD_USER_INFO_RES);
				header->set_seq(stMsg.head().seq());
				header->set_result(-1);

				//send
				g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
				fsm.set_state(UserInfoFsmState::state_end);
				return;
			}

			//save async data
			fsm._user_info.m_id = stAddReq.user_info().id();
			fsm._user_info.m_name = stAddReq.user_info().name();
			fsm._user_info.m_age = stAddReq.user_info().age();
			
			//if not exsts in cache, query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_USER_INFO_QUERY);
            req.m_fsm_id = fsm._id;
            req.m_user_info.m_id = stAddReq.user_info().id();
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(UserInfoFsmState::state_wait_query_db);
            return;
		}

		if(hoosho::msg::UPDATE_USER_INFO_REQ == dwReqType)
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_USER_INFO_UPDATE);
            req.m_fsm_id = fsm._id;
            req.need_reply();
            
			const hoosho::test::UpdateReq& stUpdateReq = stMsg.update_req();
			DaoUserInfo* daoUserInfo = g_user_info_cache_pool->get(stUpdateReq.user_info().id());
			if(daoUserInfo)
			{
				//update cache
				LOG4CPLUS_DEBUG(logger, "in cache, modified cache, user.id="<<stUpdateReq.user_info().id());
				daoUserInfo->m_name = stUpdateReq.user_info().name();
				daoUserInfo->m_age = stUpdateReq.user_info().age();
				req.m_user_info = *daoUserInfo;
			}
			else
			{
				//add new to cache
				LOG4CPLUS_DEBUG(logger, "not in cache, add to cache, user.id="<<stUpdateReq.user_info().id());
				fsm._user_info.m_id = stUpdateReq.user_info().id();
				fsm._user_info.m_name = stUpdateReq.user_info().name();
				fsm._user_info.m_age = stUpdateReq.user_info().age();
				g_user_info_cache_pool->add(fsm._user_info);
				req.m_user_info = fsm._user_info;
			}
			
			//update to db
			LOG4CPLUS_DEBUG(logger, "then, update to db, user.id="<<stUpdateReq.user_info().id());
            g_executor_thread_processor->send_request(req);
			fsm.set_state(UserInfoFsmState::state_wait_update_db);
			return;
		}

		if(hoosho::msg::DELETE_USER_INFO_REQ == dwReqType)
		{
			const hoosho::test::DeleteReq& stDeleteReq = stMsg.delete_req();
			DaoUserInfo* daoUserInfo = g_user_info_cache_pool->get(stDeleteReq.id());
			if(daoUserInfo)
			{
				//remove from cache
				LOG4CPLUS_DEBUG(logger, "in cache, delete cache, user.id="<<stDeleteReq.id());
				g_user_info_cache_pool->remove(stDeleteReq.id());
			}

			//delete from db , not care reply
			LOG4CPLUS_DEBUG(logger, "then, delete db, user.id="<<stDeleteReq.id());
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_USER_INFO_DELETE);
            req.m_fsm_id = fsm._id;
            req.m_user_info.m_id = daoUserInfo->m_id;
            g_executor_thread_processor->send_request(req);
			fsm.set_state(UserInfoFsmState::state_end);
			return;
		}
    }


    /*******************************************************************************************************/
    /*                                         UserInfoFsmStateWaitQueryDB                               */
    /*******************************************************************************************************/
    void UserInfoFsmStateWaitQueryDB::enter(UserInfoFsm& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void UserInfoFsmStateWaitQueryDB::exit(UserInfoFsm& fsm)
    {
        fsm.cancel_timer();
    }

    void UserInfoFsmStateWaitQueryDB::db_reply_event(UserInfoFsm& fsm, const ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		//resp msg
		hoosho::msg::Msg stRespMsg;
        hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_seq(fsm._req_seq_id);
        header->set_result(0);

		uint32_t dwReqType = fsm._req_type;
        uint32_t dwStatus = reply.m_result_code;
        const DaoUserInfo& daoUserInfo = reply.m_user_info;
        if(hoosho::msg::QUERY_USER_INFO_REQ == dwReqType)
        {
        	header->set_cmd(hoosho::msg::QUERY_USER_INFO_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{
        		//update cache
				g_user_info_cache_pool->add(daoUserInfo);

				hoosho::test::QueryRes* pBody = stRespMsg.mutable_query_res();
				hoosho::commstruct::UserInfo* pBodyUserInfo = pBody->mutable_user_info();
				pBodyUserInfo->set_id(daoUserInfo.m_id);
				pBodyUserInfo->set_name(daoUserInfo.m_name);
				pBodyUserInfo->set_age(daoUserInfo.m_age);
        	}
        	else
        	{
				header->set_result(-1);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(UserInfoFsmState::state_end);
        	return;	
        }

		if(hoosho::msg::ADD_USER_INFO_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::ADD_USER_INFO_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{
        		//update cache
				g_user_info_cache_pool->add(daoUserInfo);

				//repeated
				header->set_result(-1);
				g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
				fsm.set_state(UserInfoFsmState::state_end);
	        	return;	
        	} 
        	else if(ExecutorThreadRequestType::E_NOT_EXIST == dwStatus)
        	{
        		//a new user
				g_user_info_cache_pool->add(fsm._user_info);
				
				LOG4CPLUS_DEBUG(logger, "add a new user, update to db, user="<<daoUserInfo.ToString());
	            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_USER_INFO_UPDATE);
	            req.m_fsm_id = fsm._id;
	            req.m_user_info = daoUserInfo;
	            req.need_reply();
	            g_executor_thread_processor->send_request(req);
				fsm.set_state(UserInfoFsmState::state_wait_update_db);
				return;
        	}
        	else 
        	{
				//other query error
				header->set_result(-1);
				g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
				fsm.set_state(UserInfoFsmState::state_end);
	        	return;	
        	}
		}
    }

    void UserInfoFsmStateWaitQueryDB::timeout_event(UserInfoFsm& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "UserInfoFsmStateWaitQueryDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(UserInfoFsmState::state_end);
        return;
    }


	/*******************************************************************************************************/
    /*                                         UserInfoFsmStateWaitUpdateDB                               */
    /*******************************************************************************************************/
    void UserInfoFsmStateWaitUpdateDB::enter(UserInfoFsm & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void UserInfoFsmStateWaitUpdateDB::exit(UserInfoFsm& fsm)
    {
        fsm.cancel_timer();
    }

    void UserInfoFsmStateWaitUpdateDB::db_reply_event(UserInfoFsm& fsm, const ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		uint32_t dwReqType = fsm._req_type;
		uint32_t dwStatus = reply.m_result_code;

        //resp msg
		hoosho::msg::Msg stRespMsg;
        hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_seq(fsm._req_seq_id);
        header->set_result(0);
		if(hoosho::msg::ADD_USER_INFO_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::ADD_USER_INFO_RES);
		}
		else if(hoosho::msg::UPDATE_USER_INFO_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::UPDATE_USER_INFO_RES);
		}

		if(dwStatus != ExecutorThreadRequestType::E_OK)
		{
			header->set_result(-1);
		}

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(UserInfoFsmState::state_end);
        return;
    }

    void UserInfoFsmStateWaitUpdateDB::timeout_event(UserInfoFsm& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "UserInfoFsmStateWaitUpdateDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(UserInfoFsmState::state_end);
        return;
    }


    /*******************************************************************************************************/
    /*                                         UserInfoFsmStateEnd                                         */
    /*******************************************************************************************************/
    void UserInfoFsmStateEnd::enter(UserInfoFsm& fsm)
    {
        g_user_info_fsm_container->del_fsm(&fsm);
    }

