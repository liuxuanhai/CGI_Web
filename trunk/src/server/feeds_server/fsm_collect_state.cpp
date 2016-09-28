#include "fsm_collect_state.h"
#include "global_var.h"
#include "db_collect.h"
#include "common_util.h"

    /*******************************************************************************************************/
    /*					                       FsmCollectState						       				   */
    /*******************************************************************************************************/
    IMPL_LOGGER(FsmCollectState, logger);

#define IMPL_FSM_STATE(classname, name) classname FsmCollectState::name(#classname)
    IMPL_FSM_STATE(FsmCollectStateInit, state_init);
    IMPL_FSM_STATE(FsmCollectStateWaitQueryDB, state_wait_query_db);
	IMPL_FSM_STATE(FsmCollectStateWaitAddDB, state_wait_add_db);
    IMPL_FSM_STATE(FsmCollectStateWaitUpdateDB, state_wait_update_db);
	IMPL_FSM_STATE(FsmCollectStateWaitDeleteDB, state_wait_delete_db);
    IMPL_FSM_STATE(FsmCollectStateEnd, state_end);
#undef IMPL_FSM_STATE

    void FsmCollectState::enter(FsmCollect& fsm)
    {

    }

    void FsmCollectState::exit(FsmCollect& fsm)
    {

    }

    void FsmCollectState::client_req_event(FsmCollect & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
    }

	void FsmCollectState::db_reply_event(FsmCollect& fsm, const ExecutorThreadResponseElement& element)
    {
		LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
    }

    void FsmCollectState::timeout_event(FsmCollect& fsm, void* param)
    {
		LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
    }

    /*******************************************************************************************************/
    /*                                         FsmCollectStateInit                                        */
    /*******************************************************************************************************/
    void FsmCollectStateInit::client_req_event(FsmCollect& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();  
        fsm._req_type = stMsg.head().cmd();
        fsm._req_seq_id = stMsg.head().seq();

		//dispatch
		uint32_t dwReqType = stMsg.head().cmd();

		//query  feed_list Collect req
		if(hoosho::msg::QUERY_FEED_LIST_COLLECT_REQ == dwReqType)
		{
			const hoosho::feeds::QueryFeedListCollectReq& stQueryFeedListCollectReq = 
				stMsg.query_feed_list_collect_req();
			
			//query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FEED_LIST_COLLECT_QUERY);
            req.m_fsm_id = fsm._id;

			req.m_feed_list_collect.m_pa_appid_md5 = stQueryFeedListCollectReq.pa_appid_md5();
            req.m_feed_list_collect.m_openid_md5 = stQueryFeedListCollectReq.openid_md5();
			req.m_feed_list_collect.m_feed_id_list.clear();
			for(int i = 0; i < stQueryFeedListCollectReq.feed_id_list_size(); i++)
			{
				req.m_feed_list_collect.m_feed_id_list.push_back( stQueryFeedListCollectReq.feed_id_list(i) );
			}
			
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(FsmCollectState::state_wait_query_db);
            return;
		}
		
		//query_req
		if(hoosho::msg::QUERY_COLLECT_REQ == dwReqType)
		{
			const hoosho::feeds::QueryCollectReq& stQueryCollectReq = stMsg.query_collect_req();
			
			//query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_COLLECT_QUERY);
            req.m_fsm_id = fsm._id;

			req.m_collect_list.m_pa_appid_md5 = stQueryCollectReq.pa_appid_md5();
            req.m_collect_list.m_openid_md5 = stQueryCollectReq.openid_md5();
			
			req.m_collect_list.m_begin_create_ts = stQueryCollectReq.begin_create_ts();
			req.m_collect_list.m_limit = stQueryCollectReq.limit();
			
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(FsmCollectState::state_wait_query_db);
            return;
		}

		//add_req
        if(hoosho::msg::ADD_COLLECT_REQ == dwReqType)
		{

			LOG4CPLUS_TRACE(logger, "add Collect req");
			
			const hoosho::feeds::AddCollectReq& stAddCollectReq = stMsg.add_collect_req();
		
			//save async data
			fsm._collect_info.m_pa_appid_md5 = stAddCollectReq.pa_appid_md5();
			fsm._collect_info.m_openid_md5 = stAddCollectReq.openid_md5();
			fsm._collect_info.m_feed_id = stAddCollectReq.feed_id();
			 
			//set creat_ts
			timeval tv;
			gettimeofday(&tv, NULL);
			fsm._collect_info.m_create_ts = (uint64_t)tv.tv_sec;
						
			//query db
			
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FEED_LIST_COLLECT_QUERY);
            req.m_fsm_id = fsm._id;
			//**
			req.m_feed_list_collect.m_pa_appid_md5 = fsm._collect_info.m_pa_appid_md5;
			req.m_feed_list_collect.m_openid_md5 = fsm._collect_info.m_openid_md5;
			req.m_feed_list_collect.m_feed_id_list.clear();
			req.m_feed_list_collect.m_feed_id_list.push_back( fsm._collect_info.m_feed_id );
			
			LOG4CPLUS_TRACE(logger, req.m_collect_info.ToString());
			
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(FsmCollectState::state_wait_query_db);
            return;
		}

		//delete
		if(hoosho::msg::DELETE_COLLECT_REQ == dwReqType)
		{
			const hoosho::feeds::DeleteCollectReq& stDeleteCollectReq = stMsg.delete_collect_req();

			//delete from db , not care reply
			LOG4CPLUS_DEBUG(logger, "then, delete db, feed_id="<<stDeleteCollectReq.feed_id()
								<< ", appid = " << stDeleteCollectReq.pa_appid_md5()
								<< ", openid = " << stDeleteCollectReq.openid_md5());
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_COLLECT_DELETE);
            req.m_fsm_id = fsm._id;
			
            req.m_collect_info.m_feed_id = stDeleteCollectReq.feed_id();
			req.m_collect_info.m_pa_appid_md5 = stDeleteCollectReq.pa_appid_md5();
			req.m_collect_info.m_openid_md5 = stDeleteCollectReq.openid_md5();
			req.need_reply();
            g_executor_thread_processor->send_request(req);
			
			fsm.set_state(FsmCollectState::state_wait_delete_db);
			return;
		}

	
		
    }


    /*******************************************************************************************************/
    /*                                         FsmCollectStateWaitQueryDB                            	   */
    /*******************************************************************************************************/
    void FsmCollectStateWaitQueryDB::enter(FsmCollect& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmCollectStateWaitQueryDB::exit(FsmCollect& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmCollectStateWaitQueryDB::db_reply_event(FsmCollect& fsm, const ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		//resp msg
		hoosho::msg::Msg stRespMsg;
        hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_seq(fsm._req_seq_id);
        header->set_result(hoosho::msg::E_OK);

		uint32_t dwReqType = fsm._req_type;
        uint32_t dwStatus = reply.m_result_code;

		
        if(hoosho::msg::QUERY_COLLECT_REQ == dwReqType)
        {
        	const vector<uint64_t>& stFeedIdList = reply.m_collect_list.m_feed_id_list;
			const vector<uint64_t>& stCreateTsList = reply.m_collect_list.m_create_ts_list;
			
        	header->set_cmd(hoosho::msg::QUERY_COLLECT_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{

				hoosho::feeds::QueryCollectRes* pBody = stRespMsg.mutable_query_collect_res();
				//!!!!!!!!!!!!!!!!!!
				LOG4CPLUS_TRACE(logger, "list_size = " << stFeedIdList.size());

				for(size_t i = 0; i < stFeedIdList.size(); i++)
				{
					LOG4CPLUS_TRACE(logger, "id = " << stFeedIdList[i]);
					pBody->add_feed_id_list( stFeedIdList[i] );
					pBody->add_create_ts_list( stCreateTsList[i] );
				}
				
        	}
			
			else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
			{
//				hoosho::feeds::QueryCollectRes* pBody = stRespMsg.mutable_query_Collect_res();
			}
			
        	else
        	{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmCollectState::state_end);
        	return;	
        }

		if(hoosho::msg::ADD_COLLECT_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::ADD_COLLECT_RES);
			
			LOG4CPLUS_DEBUG(logger, "restult = " << reply.m_feed_list_collect.m_collect_list[0]);
			if(reply.m_feed_list_collect.m_collect_list[0] == 1)
			{
				//repeated
				header->set_result(hoosho::msg::E_ALREADY_EXIST);
				g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
				fsm.set_state(FsmCollectState::state_end);
	        	return;	
        	} 
        	else if(reply.m_feed_list_collect.m_collect_list[0] == 0)
        	{			
	            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_COLLECT_ADD);
	            req.m_fsm_id = fsm._id;
	            req.m_collect_info = fsm._collect_info;
	            req.need_reply();
				LOG4CPLUS_DEBUG(logger, "add a new Collect, update to db, Collect="<<req.m_collect_info.ToString());

				g_executor_thread_processor->send_request(req);
				fsm.set_state(FsmCollectState::state_wait_add_db);
				return;
        	}
        	else 
        	{
				//other query error
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
				g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
				fsm.set_state(FsmCollectState::state_end);
	        	return;	
        	}
		}

		if(hoosho::msg::QUERY_FEED_LIST_COLLECT_REQ == dwReqType)
        {
        	header->set_cmd(hoosho::msg::QUERY_FEED_LIST_COLLECT_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{
				hoosho::feeds::QueryFeedListCollectRes* pBody = stRespMsg.mutable_query_feed_list_collect_res();
				//!!!!!!!!!!!!!!!!!!

				const vector<uint64_t> &stFeeListCollect = reply.m_feed_list_collect.m_collect_list;
				LOG4CPLUS_TRACE(logger, "list_size = " << stFeeListCollect.size());

				for(size_t i = 0; i < stFeeListCollect.size(); i++)
				{
					LOG4CPLUS_TRACE(logger, "id = " << stFeeListCollect[i]);
					pBody->add_collect_list( stFeeListCollect[i] );
				}
				
        	}
        	else
        	{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmCollectState::state_end);
        	return;	
        }
		
    }

    void FsmCollectStateWaitQueryDB::timeout_event(FsmCollect& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmCollectStateWaitQueryDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmCollectState::state_end);
        return;
    }


	/*******************************************************************************************************/
    /*                                         FsmCollectStateWaitAddDB	                               */
    /*******************************************************************************************************/
    void FsmCollectStateWaitAddDB::enter(FsmCollect & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmCollectStateWaitAddDB::exit(FsmCollect& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmCollectStateWaitAddDB::db_reply_event(FsmCollect& fsm, const ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		uint32_t dwReqType = fsm._req_type;
		uint32_t dwStatus = reply.m_result_code;

        //resp msg
		hoosho::msg::Msg stRespMsg;
        hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_seq(fsm._req_seq_id);
        
		if(hoosho::msg::ADD_COLLECT_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::ADD_COLLECT_RES);
			if(dwStatus == ExecutorThreadRequestType::E_OK)
			{
				header->set_result(hoosho::msg::E_OK);
			}
			else if(dwStatus == ExecutorThreadRequestType::E_INVALID_PARAM)
			{
				header->set_result(hoosho::msg::E_INVALID_REQ_PARAM);
			}
			else if(dwStatus == ExecutorThreadRequestType::E_FEED_DELETED)
			{
				header->set_result(hoosho::msg::E_FEED_DELETED);
			}
			else if(dwStatus == ExecutorThreadRequestType::E_NOT_EXIST)
			{
				//not found feed_id
				header->set_result(hoosho::msg::E_INVALID_REQ_PARAM);
			}
			else 
			{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
			}
			
	        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
	        fsm.set_state(FsmCollectState::state_end);

			return;
		}
	
		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
        return;
    }

    void FsmCollectStateWaitAddDB::timeout_event(FsmCollect& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmCollectStateWaitAddDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmCollectState::state_end);
        return;
    }

	/*******************************************************************************************************/
    /*                                         FsmCollectStateWaitUpdateDB                                */
    /*******************************************************************************************************/
    void FsmCollectStateWaitUpdateDB::enter(FsmCollect & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmCollectStateWaitUpdateDB::exit(FsmCollect& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmCollectStateWaitUpdateDB::db_reply_event(FsmCollect& fsm, const ExecutorThreadResponseElement& reply)
    {
     	LOG4CPLUS_ERROR(logger, "need code");	
        return;
    }

    void FsmCollectStateWaitUpdateDB::timeout_event(FsmCollect& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmCollectStateWaitUpdateDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmCollectState::state_end);
        return;
    }

	/*******************************************************************************************************/
    /*                                         FsmCollectStateWaitDeleteDB                                */
    /*******************************************************************************************************/
    void FsmCollectStateWaitDeleteDB::enter(FsmCollect & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmCollectStateWaitDeleteDB::exit(FsmCollect& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmCollectStateWaitDeleteDB::db_reply_event(FsmCollect& fsm, const ExecutorThreadResponseElement& reply)
    {
     	//cancel timer first
	    fsm.cancel_timer();

		uint32_t dwReqType = fsm._req_type;
		uint32_t dwStatus = reply.m_result_code;

	    //resp msg
		hoosho::msg::Msg stRespMsg;
	    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	    header->set_seq(fsm._req_seq_id);
	    
		if(hoosho::msg::DELETE_COLLECT_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::DELETE_COLLECT_REQ);

			if(dwStatus == ExecutorThreadRequestType::E_OK)
			{
				header->set_result(hoosho::msg::E_OK);
			}
			else if(dwStatus == ExecutorThreadRequestType::E_INVALID_PARAM)
			{
				header->set_result(hoosho::msg::E_INVALID_REQ_PARAM);
			}
			else if(dwStatus == ExecutorThreadRequestType::E_NOT_EXIST)
			{
				header->set_result(hoosho::msg::E_INVALID_REQ_PARAM);
			}
			else
			{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
			}
			
		    g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		    fsm.set_state(FsmCollectState::state_end);
			return;
		}
		
		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
        return;
    }

    void FsmCollectStateWaitDeleteDB::timeout_event(FsmCollect& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmCollectStateWaitDeleteDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmCollectState::state_end);
        return;
    }
	

    /*******************************************************************************************************/
    /*                                         FsmCollectStateEnd                                         */
    /*******************************************************************************************************/
    void FsmCollectStateEnd::enter(FsmCollect& fsm)
    {
        g_fsm_collect_container->del_fsm(&fsm);
    }

