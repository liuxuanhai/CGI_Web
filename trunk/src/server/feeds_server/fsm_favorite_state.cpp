#include "fsm_favorite_state.h"
#include "global_var.h"
#include "db_favorite.h"
#include "common_util.h"

    /*******************************************************************************************************/
    /*					                       FsmFavoriteState						       				   */
    /*******************************************************************************************************/
    IMPL_LOGGER(FsmFavoriteState, logger);

#define IMPL_FSM_STATE(classname, name) classname FsmFavoriteState::name(#classname)
    IMPL_FSM_STATE(FsmFavoriteStateInit, state_init);
    IMPL_FSM_STATE(FsmFavoriteStateWaitQueryDB, state_wait_query_db);
	IMPL_FSM_STATE(FsmFavoriteStateWaitAddDB, state_wait_add_db);
    IMPL_FSM_STATE(FsmFavoriteStateWaitUpdateDB, state_wait_update_db);
	IMPL_FSM_STATE(FsmFavoriteStateWaitDeleteDB, state_wait_delete_db);
    IMPL_FSM_STATE(FsmFavoriteStateEnd, state_end);
#undef IMPL_FSM_STATE

    void FsmFavoriteState::enter(FsmFavorite& fsm)
    {

    }

    void FsmFavoriteState::exit(FsmFavorite& fsm)
    {

    }

    void FsmFavoriteState::client_req_event(FsmFavorite & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
    }

	void FsmFavoriteState::db_reply_event(FsmFavorite& fsm, const ExecutorThreadResponseElement& element)
    {
		LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
    }

    void FsmFavoriteState::timeout_event(FsmFavorite& fsm, void* param)
    {
		LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
    }

    /*******************************************************************************************************/
    /*                                         FsmFavoriteStateInit                                        */
    /*******************************************************************************************************/
    void FsmFavoriteStateInit::client_req_event(FsmFavorite& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();  
        fsm._req_type = stMsg.head().cmd();
        fsm._req_seq_id = stMsg.head().seq();

		//dispatch
		uint32_t dwReqType = stMsg.head().cmd();

		//query  feed_list favorite req
		if(hoosho::msg::QUERY_FEED_LIST_FAVORITE_REQ == dwReqType)
		{
			const hoosho::feeds::QueryFeedListFavoriteReq& stQueryFeedListFavoriteReq = 
				stMsg.query_feed_list_favorite_req();
			
			//query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FEED_LIST_FAVORITE_QUERY);
            req.m_fsm_id = fsm._id;

			req.m_feed_list_favorite.m_pa_appid_md5 = stQueryFeedListFavoriteReq.pa_appid_md5();
            req.m_feed_list_favorite.m_openid_md5 = stQueryFeedListFavoriteReq.openid_md5();
			req.m_feed_list_favorite.m_feed_id_list.clear();
			for(int i = 0; i < stQueryFeedListFavoriteReq.feed_id_list_size(); i++)
			{
				req.m_feed_list_favorite.m_feed_id_list.push_back( stQueryFeedListFavoriteReq.feed_id_list(i) );
			}
			
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(FsmFavoriteState::state_wait_query_db);
            return;
		}
		
		//query_req
		if(hoosho::msg::QUERY_FAVORITE_REQ == dwReqType)
		{
			const hoosho::feeds::QueryFavoriteReq& stQueryFavoriteReq = stMsg.query_favorite_req();
			
			//query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FAVORITE_QUERY);
            req.m_fsm_id = fsm._id;

			req.m_favorite_list.m_pa_appid_md5 = stQueryFavoriteReq.pa_appid_md5();
            req.m_favorite_list.m_openid_md5 = stQueryFavoriteReq.openid_md5();
			req.m_favorite_list.m_feed_id = stQueryFavoriteReq.feed_id();
			req.m_favorite_list.m_begin_create_ts = stQueryFavoriteReq.begin_create_ts();
			req.m_favorite_list.m_limit = stQueryFavoriteReq.limit();
			
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(FsmFavoriteState::state_wait_query_db);
            return;
		}

		//add_req
        if(hoosho::msg::ADD_FAVORITE_REQ == dwReqType)
		{

			LOG4CPLUS_TRACE(logger, "add favorite req");
			
			const hoosho::feeds::AddFavoriteReq& stAddFavoriteReq = stMsg.add_favorite_req();
		
			//save async data
			fsm._favorite_info.m_pa_appid_md5 = stAddFavoriteReq.pa_appid_md5();
			fsm._favorite_info.m_openid_md5 = stAddFavoriteReq.openid_md5();
			fsm._favorite_info.m_feed_id = stAddFavoriteReq.feed_id();
			fsm._favorite_info.m_feed_owner_openid_md5 = stAddFavoriteReq.feed_owner_openid_md5();
			 
			//set creat_ts
			timeval tv;
			gettimeofday(&tv, NULL);
			fsm._favorite_info.m_create_ts = (uint64_t)tv.tv_sec;
						
			//query db
			
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FEED_LIST_FAVORITE_QUERY);
            req.m_fsm_id = fsm._id;
			//**
			req.m_feed_list_favorite.m_pa_appid_md5 = fsm._favorite_info.m_pa_appid_md5;
			req.m_feed_list_favorite.m_openid_md5 = fsm._favorite_info.m_openid_md5;
			req.m_feed_list_favorite.m_feed_id_list.clear();
			req.m_feed_list_favorite.m_feed_id_list.push_back( fsm._favorite_info.m_feed_id );
			
			LOG4CPLUS_TRACE(logger, req.m_favorite_info.ToString());
			
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(FsmFavoriteState::state_wait_query_db);
            return;
		}

		//delete
		if(hoosho::msg::DELETE_FAVORITE_REQ == dwReqType)
		{
			const hoosho::feeds::DeleteFavoriteReq& stDeleteFavoriteReq = stMsg.delete_favorite_req();

			//delete from db , not care reply
			LOG4CPLUS_DEBUG(logger, "then, delete db, feed_id="<<stDeleteFavoriteReq.feed_id()
								<< ", openid = " << stDeleteFavoriteReq.openid_md5());
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FAVORITE_DELETE);
            req.m_fsm_id = fsm._id;
			
            req.m_favorite_info.m_feed_id = stDeleteFavoriteReq.feed_id();
			req.m_favorite_info.m_pa_appid_md5 = stDeleteFavoriteReq.pa_appid_md5();
			req.m_favorite_info.m_openid_md5 = stDeleteFavoriteReq.openid_md5();
			req.need_reply();
            g_executor_thread_processor->send_request(req);
			
			fsm.set_state(FsmFavoriteState::state_wait_delete_db);
			return;
		}

	
		
    }


    /*******************************************************************************************************/
    /*                                         FsmFavoriteStateWaitQueryDB                            	   */
    /*******************************************************************************************************/
    void FsmFavoriteStateWaitQueryDB::enter(FsmFavorite& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmFavoriteStateWaitQueryDB::exit(FsmFavorite& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmFavoriteStateWaitQueryDB::db_reply_event(FsmFavorite& fsm, const ExecutorThreadResponseElement& reply)
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

		
        if(hoosho::msg::QUERY_FAVORITE_REQ == dwReqType)
        {
        	const vector<uint64_t>& stOpenidMd5List = reply.m_favorite_list.m_openid_md5_list;
			const vector<uint64_t>& stCreateTsList = reply.m_favorite_list.m_create_ts_list;
			
        	header->set_cmd(hoosho::msg::QUERY_FAVORITE_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{

				hoosho::feeds::QueryFavoriteRes* pBody = stRespMsg.mutable_query_favorite_res();
				//!!!!!!!!!!!!!!!!!!
				LOG4CPLUS_TRACE(logger, "list_size = " << stOpenidMd5List.size());

				for(size_t i = 0; i < stOpenidMd5List.size(); i++)
				{
					LOG4CPLUS_TRACE(logger, "id = " << stOpenidMd5List[i]);
					pBody->add_openid_md5_list( stOpenidMd5List[i] );
					pBody->add_create_ts_list( stCreateTsList[i] );
				}
				
        	}
			
			else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
			{
//				hoosho::feeds::QueryFavoriteRes* pBody = stRespMsg.mutable_query_Favorite_res();
			}
			
        	else
        	{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmFavoriteState::state_end);
        	return;	
        }

		if(hoosho::msg::ADD_FAVORITE_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::ADD_FAVORITE_RES);
//			if(ExecutorThreadRequestType::E_OK == dwStatus)
			
			LOG4CPLUS_DEBUG(logger, "restult = " << reply.m_feed_list_favorite.m_favorite_list[0]);
			if(reply.m_feed_list_favorite.m_favorite_list[0] == 1)
			{
				//repeated
				header->set_result(hoosho::msg::E_ALREADY_EXIST);
				g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
				fsm.set_state(FsmFavoriteState::state_end);
	        	return;	
        	} 
        	else if(reply.m_feed_list_favorite.m_favorite_list[0] == 0)
        	{			
	            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FAVORITE_ADD);
	            req.m_fsm_id = fsm._id;
	            req.m_favorite_info = fsm._favorite_info;
	            req.need_reply();
				LOG4CPLUS_DEBUG(logger, "add a new favorite, update to db, favorite="<<req.m_favorite_info.ToString());

				g_executor_thread_processor->send_request(req);
				fsm.set_state(FsmFavoriteState::state_wait_add_db);
				return;
        	}
        	else 
        	{
				//other query error
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
				g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
				fsm.set_state(FsmFavoriteState::state_end);
	        	return;	
        	}
		}

		if(hoosho::msg::QUERY_FEED_LIST_FAVORITE_REQ == dwReqType)
        {
        	header->set_cmd(hoosho::msg::QUERY_FEED_LIST_FAVORITE_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
        	{
				hoosho::feeds::QueryFeedListFavoriteRes* pBody = stRespMsg.mutable_query_feed_list_favorite_res();
				//!!!!!!!!!!!!!!!!!!

				const vector<uint64_t> &stFeeListFavorite = reply.m_feed_list_favorite.m_favorite_list;
				LOG4CPLUS_TRACE(logger, "list_size = " << stFeeListFavorite.size());

				for(size_t i = 0; i < stFeeListFavorite.size(); i++)
				{
					LOG4CPLUS_TRACE(logger, "id = " << stFeeListFavorite[i]);
					pBody->add_favorite_list( stFeeListFavorite[i] );
				}
				
        	}
        	else
        	{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmFavoriteState::state_end);
        	return;	
        }
		
    }

    void FsmFavoriteStateWaitQueryDB::timeout_event(FsmFavorite& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmFavoriteStateWaitQueryDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmFavoriteState::state_end);
        return;
    }


	/*******************************************************************************************************/
    /*                                         FsmFavoriteStateWaitAddDB	                               */
    /*******************************************************************************************************/
    void FsmFavoriteStateWaitAddDB::enter(FsmFavorite & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmFavoriteStateWaitAddDB::exit(FsmFavorite& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmFavoriteStateWaitAddDB::db_reply_event(FsmFavorite& fsm, const ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		uint32_t dwReqType = fsm._req_type;
		uint32_t dwStatus = reply.m_result_code;

        //resp msg
		hoosho::msg::Msg stRespMsg;
        hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_seq(fsm._req_seq_id);
        
		if(hoosho::msg::ADD_FAVORITE_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::ADD_FAVORITE_RES);
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
	        fsm.set_state(FsmFavoriteState::state_end);

			////notify feed_owner
			if(dwStatus == ExecutorThreadRequestType::E_OK && fsm._favorite_info.m_feed_owner_openid_md5 != fsm._favorite_info.m_openid_md5)
			{
				
				::hoosho::msg::Msg stMsgRequest;
				::hoosho::msg::MsgHead* pMsgRequestHead = stMsgRequest.mutable_head();
				pMsgRequestHead->set_cmd(::hoosho::msg::ADD_NOTICE_RECORD_REQ);
				pMsgRequestHead->set_seq(fsm._id);
				::hoosho::sixin::AddNoticeRecordReq* pAddNoticeRecordReq = stMsgRequest.mutable_notice_record_add_req();
				::hoosho::commstruct::NoticeRecord* pNoticeRecord = pAddNoticeRecordReq->mutable_notice_record();
				pNoticeRecord->set_pa_appid_md5(fsm._favorite_info.m_pa_appid_md5);
				pNoticeRecord->set_openid_md5(fsm._favorite_info.m_feed_owner_openid_md5);
				pNoticeRecord->set_type(::hoosho::commenum::NOTICE_RECORD_TYPE_FAVORITE);
				LOG4CPLUS_TRACE(logger, "feedid=" <<  fsm._favorite_info.m_feed_id << ", ts=" <<  fsm._favorite_info.m_create_ts );
				pNoticeRecord->set_create_ts(  fsm._favorite_info.m_create_ts  );
				pNoticeRecord->set_extra_data_0( int_2_str(fsm._favorite_info.m_feed_id) );
				pNoticeRecord->set_extra_data_1( int_2_str(fsm._favorite_info.m_openid_md5) );
				
				g_server_processor_msg->send_datagram(stMsgRequest);
			}
			return;
		}
	
		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
        return;
    }

    void FsmFavoriteStateWaitAddDB::timeout_event(FsmFavorite& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmFavoriteStateWaitAddDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmFavoriteState::state_end);
        return;
    }

	/*******************************************************************************************************/
    /*                                         FsmFavoriteStateWaitUpdateDB                                */
    /*******************************************************************************************************/
    void FsmFavoriteStateWaitUpdateDB::enter(FsmFavorite & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmFavoriteStateWaitUpdateDB::exit(FsmFavorite& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmFavoriteStateWaitUpdateDB::db_reply_event(FsmFavorite& fsm, const ExecutorThreadResponseElement& reply)
    {
     	LOG4CPLUS_ERROR(logger, "need code");	
        return;
    }

    void FsmFavoriteStateWaitUpdateDB::timeout_event(FsmFavorite& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmFavoriteStateWaitUpdateDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmFavoriteState::state_end);
        return;
    }

	/*******************************************************************************************************/
    /*                                         FsmFavoriteStateWaitDeleteDB                                */
    /*******************************************************************************************************/
    void FsmFavoriteStateWaitDeleteDB::enter(FsmFavorite & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmFavoriteStateWaitDeleteDB::exit(FsmFavorite& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmFavoriteStateWaitDeleteDB::db_reply_event(FsmFavorite& fsm, const ExecutorThreadResponseElement& reply)
    {
     	//cancel timer first
	    fsm.cancel_timer();

		uint32_t dwReqType = fsm._req_type;
		uint32_t dwStatus = reply.m_result_code;

	    //resp msg
		hoosho::msg::Msg stRespMsg;
	    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	    header->set_seq(fsm._req_seq_id);
	    
		if(hoosho::msg::DELETE_FAVORITE_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::DELETE_FAVORITE_REQ);

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
		    fsm.set_state(FsmFavoriteState::state_end);
		    

		    ////delete recored of notify feed_owner
			if(dwStatus == ExecutorThreadRequestType::E_OK)
			{
				::hoosho::msg::Msg stMsgRequest;
				::hoosho::msg::MsgHead* pMsgRequestHead = stMsgRequest.mutable_head();
				pMsgRequestHead->set_cmd(::hoosho::msg::DELETE_NOTICE_RECORD_REQ);
				pMsgRequestHead->set_seq(fsm._id);
				::hoosho::sixin::DeleteNoticeRecordReq* pDeleteNoticeRecordReq = stMsgRequest.mutable_notice_record_delete_req();
				
				pDeleteNoticeRecordReq->set_pa_appid_md5(fsm._favorite_info.m_pa_appid_md5);
				pDeleteNoticeRecordReq->set_openid_md5(fsm._favorite_info.m_feed_owner_openid_md5);
				pDeleteNoticeRecordReq->set_type(::hoosho::commenum::NOTICE_RECORD_TYPE_FAVORITE);
				LOG4CPLUS_TRACE(logger, "feedid=" <<  fsm._favorite_info.m_feed_id << ", ts=" <<  fsm._favorite_info.m_create_ts );
				pDeleteNoticeRecordReq->set_create_ts( fsm._favorite_info.m_create_ts );
				
				g_server_processor_msg->send_datagram(stMsgRequest);
			}
			return;
		}
		
		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
        return;
    }

    void FsmFavoriteStateWaitDeleteDB::timeout_event(FsmFavorite& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmFavoriteStateWaitDeleteDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmFavoriteState::state_end);
        return;
    }
	

    /*******************************************************************************************************/
    /*                                         FsmFavoriteStateEnd                                         */
    /*******************************************************************************************************/
    void FsmFavoriteStateEnd::enter(FsmFavorite& fsm)
    {
        g_fsm_favorite_container->del_fsm(&fsm);
    }

