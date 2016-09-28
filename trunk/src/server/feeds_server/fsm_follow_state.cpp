#include "fsm_follow_state.h"
#include "global_var.h"
#include "db_feed.h"
#include "db_follow.h"
#include "common_util.h"

    /*******************************************************************************************************/
    /*					                       FsmFollowState						       				   */
    /*******************************************************************************************************/
    IMPL_LOGGER(FsmFollowState, logger);

#define IMPL_FSM_STATE(classname, name) classname FsmFollowState::name(#classname)
    IMPL_FSM_STATE(FsmFollowStateInit, state_init);
    IMPL_FSM_STATE(FsmFollowStateWaitQueryDB, state_wait_query_db);
	IMPL_FSM_STATE(FsmFollowStateWaitAddDB, state_wait_add_db);
    IMPL_FSM_STATE(FsmFollowStateWaitUpdateDB, state_wait_update_db);
	IMPL_FSM_STATE(FsmFollowStateWaitDeleteDB, state_wait_delete_db);
    IMPL_FSM_STATE(FsmFollowStateEnd, state_end);
#undef IMPL_FSM_STATE

    void FsmFollowState::enter(FsmFollow& fsm)
    {

    }

    void FsmFollowState::exit(FsmFollow& fsm)
    {

    }

    void FsmFollowState::client_req_event(FsmFollow & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
    }

	void FsmFollowState::db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element)
    {
		LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
    }

    void FsmFollowState::timeout_event(FsmFollow& fsm, void* param)
    {
		LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
    }

    /*******************************************************************************************************/
    /*                                         FsmFollowStateInit                                          */
    /*******************************************************************************************************/
    void FsmFollowStateInit::client_req_event(FsmFollow& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();
        fsm._req_type = stMsg.head().cmd();
        fsm._req_seq_id = stMsg.head().seq();

		//dispatch
		uint32_t dwReqType = stMsg.head().cmd();

		//query_req    follow detail
		if(hoosho::msg::QUERY_FOLLOW_DETAIL_REQ == dwReqType)
		{
			const hoosho::feeds::QueryFollowDetailReq& stQueryFollowDetailReq = stMsg.query_follow_detail_req();
			
			//query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FOLLOW_DETAIL_QUERY);
            req.m_fsm_id = fsm._id;

			req.m_follow_id_list.clear();
			for(int i = 0; i < stQueryFollowDetailReq.follow_id_list_size(); i++)
			{
				req.m_follow_id_list.push_back( stQueryFollowDetailReq.follow_id_list(i) );
			}
			
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(FsmFollowState::state_wait_query_db);
            return;
		}

		//query_req    follow list
		if(hoosho::msg::QUERY_FOLLOW_LIST_REQ == dwReqType)
		{
 		//////////////////////////don't need now ////////////////////////////////////////////////	
			LOG4CPLUS_ERROR(logger, "need code");
			return;
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FOLLOW_LIST_QUERY);
			req.need_reply();
			g_executor_thread_processor->send_request(req);
			fsm.set_state(FsmFollowState::state_wait_query_db);
			return;
		}

		//query_req    follow_comment detail
		if(hoosho::msg::QUERY_FOLLOW_COMMENT_DETAIL_REQ == dwReqType)
		{
			const hoosho::feeds::QueryFollowCommentDetailReq& stQueryFollowCommentDetailReq = stMsg.query_follow_comment_detail_req();
			
			//query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FOLLOW_COMMENT_DETAIL_QUERY);
            req.m_fsm_id = fsm._id;

			req.m_comment_id_list.clear();
			for(int i = 0; i < stQueryFollowCommentDetailReq.comment_id_list_size(); i++)
			{
				req.m_comment_id_list.push_back( stQueryFollowCommentDetailReq.comment_id_list(i) );
			}
			
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(FsmFollowState::state_wait_query_db);
            return;
		}
		
		//query_req    follow_comment list
		if(hoosho::msg::QUERY_FOLLOW_COMMENT_REQ == dwReqType)
		{
			const hoosho::feeds::QueryFollowCommentReq& stQueryFollowCommentReq = stMsg.query_follow_comment_req();
			
			//query db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FOLLOW_COMMENT_QUERY);
            req.m_fsm_id = fsm._id;
			
            req.m_comment_index_on_feedid.m_pa_appid_md5 = stQueryFollowCommentReq.pa_appid_md5();
            req.m_comment_index_on_feedid.m_openid_md5 = stQueryFollowCommentReq.openid_md5();			
			req.m_comment_index_on_feedid.m_feed_id = stQueryFollowCommentReq.feed_id();
			req.m_comment_index_on_feedid.m_begin_comment_id = stQueryFollowCommentReq.begin_comment_id();
			req.m_comment_index_on_feedid.m_limit = stQueryFollowCommentReq.limit();

            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(FsmFollowState::state_wait_query_db);
            return;
		}

		//add_req
        if(hoosho::msg::ADD_FOLLOW_REQ == dwReqType)
		{
			const hoosho::feeds::AddFollowReq& stAddFollowReq = stMsg.add_follow_req();
		
			//save async data
			fsm._follow_info.m_pa_appid_md5 = stAddFollowReq.pa_appid_md5();
			fsm._follow_info.m_openid_md5 = stAddFollowReq.openid_md5();
			fsm._follow_info.m_feed_id = stAddFollowReq.feed_id();
			fsm._follow_info.m_type= stAddFollowReq.type();

			fsm._follow_info.m_openid_md5_from= stAddFollowReq.openid_md5();
			fsm._follow_info.m_origin_comment_id= stAddFollowReq.origin_comment_id();
			fsm._follow_info.m_content= stAddFollowReq.content();	

			fsm._follow_info.m_feed_owner_openid_md5 = stAddFollowReq.feed_owner_openid_md5();
			fsm._follow_info.m_origin_comment_owner_openid_md5 = stAddFollowReq.origin_comment_owner_openid_md5();
			fsm._follow_info.m_openid_md5_to= stAddFollowReq.reply_openid_md5();
			
			//gen id
			fsm._follow_info.m_follow_id = common::util::generate_unique_id();
			if(stAddFollowReq.type() == hoosho::commenum::FEEDS_FOLLOW_COMMENT) ////!!!!!!!!
			{
				fsm._follow_info.m_origin_comment_id = fsm._follow_info.m_follow_id;
			}
			
			//set creat_ts
			timeval tv;
			gettimeofday(&tv, NULL);
			fsm._follow_info.m_create_ts = (uint64_t)tv.tv_sec;
			fsm._follow_info.m_del_ts = 0;
			
			//add db
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FOLLOW_ADD);
            req.m_fsm_id = fsm._id;
			
          	req.m_follow_info = fsm._follow_info;
            req.need_reply();
            g_executor_thread_processor->send_request(req);
            fsm.set_state(FsmFollowState::state_wait_add_db);
            return;
		}

		//delete req
		if(hoosho::msg::DELETE_FOLLOW_REQ == dwReqType)
		{
			const hoosho::feeds::DeleteFollowReq& stDeleteFollowReq = stMsg.delete_follow_req();

			//delete from db , not care reply
			LOG4CPLUS_DEBUG(logger, "then, delete db, follow_id="<<stDeleteFollowReq.follow_id());
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FOLLOW_DELETE);
            req.m_fsm_id = fsm._id;

			req.m_follow_info.m_follow_id = stDeleteFollowReq.follow_id();
            req.m_follow_info.m_feed_id = stDeleteFollowReq.feed_id();
			req.m_follow_info.m_pa_appid_md5 = stDeleteFollowReq.pa_appid_md5();
			req.m_follow_info.m_openid_md5 = stDeleteFollowReq.openid_md5();
			req.need_reply();
            g_executor_thread_processor->send_request(req);
			
			fsm.set_state(FsmFollowState::state_wait_delete_db);
			return;
		}

		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
		return;
    }


    /*******************************************************************************************************/
    /*                                         FsmFollowStateWaitQueryDB                               */
    /*******************************************************************************************************/
    void FsmFollowStateWaitQueryDB::enter(FsmFollow& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmFollowStateWaitQueryDB::exit(FsmFollow& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmFollowStateWaitQueryDB::db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& reply)
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

		
        if(hoosho::msg::QUERY_FOLLOW_DETAIL_REQ == dwReqType)
        {
			header->set_cmd(hoosho::msg::QUERY_FOLLOW_DETAIL_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
			{
				hoosho::feeds::QueryFollowDetailRes* pBody = stRespMsg.mutable_query_follow_detail_res();
				//!!!!!!!!!!!!!!!!!!
				const vector<FollowInfo>& stFollowList = reply.m_follow_list;
				LOG4CPLUS_TRACE(logger, "list_size = " << stFollowList.size());

				for(size_t i = 0; i < stFollowList.size(); i++)
				{

					hoosho::commstruct::FollowInfo* pBodyFollow = pBody->add_follow_list();

					const FollowInfo& stFollowInfo = stFollowList[i];		
					pBodyFollow->set_follow_id( stFollowInfo.m_follow_id );
					pBodyFollow->set_content( stFollowInfo.m_content );
					pBodyFollow->set_feed_id( stFollowInfo.m_feed_id );
					pBodyFollow->set_type( stFollowInfo.m_type );
					pBodyFollow->set_origin_comment_id( stFollowInfo.m_origin_comment_id );
					pBodyFollow->set_openid_md5_from( stFollowInfo.m_openid_md5_from );
					pBodyFollow->set_openid_md5_to( stFollowInfo.m_openid_md5_to );
					pBodyFollow->set_create_ts( stFollowInfo.m_create_ts );
					pBodyFollow->set_del_ts( stFollowInfo.m_del_ts );
				}
						
			}
					
			//when not data found,  still return 
			else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
			{
				
			}
			else
			{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
			}
		
			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmFollowState::state_end);
			return; 

        }
		
		if(hoosho::msg::QUERY_FOLLOW_LIST_REQ == dwReqType)
        {
//////////////////////////////////don't need now ////////////////////////////////////////////////	
			LOG4CPLUS_ERROR(logger, "need code");
			return;
			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmFollowState::state_end);
        	return;	
        }

		if(hoosho::msg::QUERY_FOLLOW_COMMENT_DETAIL_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::QUERY_FOLLOW_COMMENT_DETAIL_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
			{
				hoosho::feeds::QueryFollowCommentDetailRes* pBody = stRespMsg.mutable_query_follow_comment_detail_res();
				//!!!!!!!!!!!!!!!!!!
				const vector<FollowCommentInfo>& stFollowCommentList = reply.m_follow_comment_list;
				LOG4CPLUS_TRACE(logger, "list_size = " << stFollowCommentList.size());

				for(size_t i = 0; i < stFollowCommentList.size(); i++)
				{

					hoosho::commstruct::FollowCommentInfo* pBodyFollowComment = pBody->add_comment_list();
					hoosho::commstruct::FollowInfo* pBodyFollow = pBodyFollowComment->mutable_comment();

					const FollowCommentInfo& stFollowCommentInfo = stFollowCommentList[i];		
					pBodyFollow->set_follow_id( stFollowCommentInfo.m_comment.m_follow_id );
					pBodyFollow->set_content( stFollowCommentInfo.m_comment.m_content );
					pBodyFollow->set_feed_id( stFollowCommentInfo.m_comment.m_feed_id );
					pBodyFollow->set_type( stFollowCommentInfo.m_comment.m_type );
					pBodyFollow->set_origin_comment_id( stFollowCommentInfo.m_comment.m_origin_comment_id );
					pBodyFollow->set_openid_md5_from( stFollowCommentInfo.m_comment.m_openid_md5_from );
					pBodyFollow->set_openid_md5_to( stFollowCommentInfo.m_comment.m_openid_md5_to );
					pBodyFollow->set_create_ts( stFollowCommentInfo.m_comment.m_create_ts );
					pBodyFollow->set_del_ts( stFollowCommentInfo.m_comment.m_del_ts );

							
					for(size_t j = 0; j < stFollowCommentInfo.m_reply_list.size(); j++)
					{
						const FollowInfo& stFollowInfo = stFollowCommentInfo.m_reply_list[j];
						pBodyFollow = pBodyFollowComment->add_reply_list();

						pBodyFollow->set_follow_id( stFollowInfo.m_follow_id );
						pBodyFollow->set_content( stFollowInfo.m_content );
						pBodyFollow->set_feed_id( stFollowInfo.m_feed_id );
						pBodyFollow->set_type( stFollowInfo.m_type );
						pBodyFollow->set_origin_comment_id( stFollowInfo.m_origin_comment_id );
						pBodyFollow->set_openid_md5_from( stFollowInfo.m_openid_md5_from );
						pBodyFollow->set_openid_md5_to( stFollowInfo.m_openid_md5_to );
						pBodyFollow->set_create_ts( stFollowInfo.m_create_ts );
						pBodyFollow->set_del_ts( stFollowInfo.m_del_ts );
					}
							
				}
						
			}
					
			//when not data found,  still return 
			else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
			{
				
			}
			else
			{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
			}
		
			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmFollowState::state_end);
			return; 
		}
		
		if(hoosho::msg::QUERY_FOLLOW_COMMENT_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::QUERY_FOLLOW_COMMENT_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
			{
				hoosho::feeds::QueryFollowCommentRes* pBody = stRespMsg.mutable_query_follow_comment_res();
				//!!!!!!!!!!!!!!!!!!
				const vector<FollowCommentInfo>& stFollowCommentList = reply.m_follow_comment_list;
				LOG4CPLUS_TRACE(logger, "list_size = " << stFollowCommentList.size());

				pBody->set_total(reply.m_total);
				for(size_t i = 0; i < stFollowCommentList.size(); i++)
				{
//					LOG4CPLUS_TRACE(logger, "info = " << stFollowCommentList[i].ToString());
		
					hoosho::commstruct::FollowCommentInfo* pBodyFollowComment = pBody->add_comment_list();
					
					hoosho::commstruct::FollowInfo* pBodyFollow = pBodyFollowComment->mutable_comment();

					const FollowCommentInfo& stFollowCommentInfo = stFollowCommentList[i];
							
					pBodyFollow->set_follow_id( stFollowCommentInfo.m_comment.m_follow_id );
					pBodyFollow->set_content( stFollowCommentInfo.m_comment.m_content );
					pBodyFollow->set_feed_id( stFollowCommentInfo.m_comment.m_feed_id );
					pBodyFollow->set_type( stFollowCommentInfo.m_comment.m_type );
					pBodyFollow->set_origin_comment_id( stFollowCommentInfo.m_comment.m_origin_comment_id );
					pBodyFollow->set_openid_md5_from( stFollowCommentInfo.m_comment.m_openid_md5_from );
					pBodyFollow->set_openid_md5_to( stFollowCommentInfo.m_comment.m_openid_md5_to );
					pBodyFollow->set_create_ts( stFollowCommentInfo.m_comment.m_create_ts );
					pBodyFollow->set_del_ts( stFollowCommentInfo.m_comment.m_del_ts );

							
					for(size_t j = 0; j < stFollowCommentInfo.m_reply_list.size(); j++)
					{
						const FollowInfo& stFollowInfo = stFollowCommentInfo.m_reply_list[j];
						pBodyFollow = pBodyFollowComment->add_reply_list();

						pBodyFollow->set_follow_id( stFollowInfo.m_follow_id );
						pBodyFollow->set_content( stFollowInfo.m_content );
						pBodyFollow->set_feed_id( stFollowInfo.m_feed_id );
						pBodyFollow->set_type( stFollowInfo.m_type );
						pBodyFollow->set_origin_comment_id( stFollowInfo.m_origin_comment_id );
						pBodyFollow->set_openid_md5_from( stFollowInfo.m_openid_md5_from );
						pBodyFollow->set_openid_md5_to( stFollowInfo.m_openid_md5_to );
						pBodyFollow->set_create_ts( stFollowInfo.m_create_ts );
						pBodyFollow->set_del_ts( stFollowInfo.m_del_ts );
					}
							
				}
						
			}
					
			//when not data found,  still return ok
			else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
			{

			}
					
			else
			{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
			}
		
			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmFollowState::state_end);
			return; 
		}


		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
		return;
    }

    void FsmFollowStateWaitQueryDB::timeout_event(FsmFollow& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmFollowStateWaitQueryDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmFollowState::state_end);
        return;
    }


	/*******************************************************************************************************/
    /*                                         FsmFollowStateWaitAddDB                                     */
    /*******************************************************************************************************/
    void FsmFollowStateWaitAddDB::enter(FsmFollow & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmFollowStateWaitAddDB::exit(FsmFollow& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmFollowStateWaitAddDB::db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		uint32_t dwReqType = fsm._req_type;
		uint32_t dwStatus = reply.m_result_code;

        //resp msg
		hoosho::msg::Msg stRespMsg;
        hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
        header->set_seq(fsm._req_seq_id);
        
		if(hoosho::msg::ADD_FOLLOW_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::ADD_FOLLOW_RES);

			hoosho::feeds::AddFollowRes* pBody = stRespMsg.mutable_add_follow_res();
			pBody->set_follow_id(reply.m_follow_info.m_follow_id);
		
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
			fsm.set_state(FsmFollowState::state_end);

			
			//*** server notice
			if(dwStatus == ExecutorThreadRequestType::E_OK)
			{
			
				::hoosho::msg::Msg stMsgRequest;
				::hoosho::msg::MsgHead* pMsgRequestHead = stMsgRequest.mutable_head();
				pMsgRequestHead->set_cmd(::hoosho::msg::ADD_NOTICE_RECORD_REQ);
				pMsgRequestHead->set_seq(fsm._id);
				::hoosho::sixin::AddNoticeRecordReq* pAddNoticeRecordReq = stMsgRequest.mutable_notice_record_add_req();
				::hoosho::commstruct::NoticeRecord* pNoticeRecord = pAddNoticeRecordReq->mutable_notice_record();
				pNoticeRecord->set_pa_appid_md5(fsm._follow_info.m_pa_appid_md5);
				
				pNoticeRecord->set_type(::hoosho::commenum::NOTICE_RECORD_TYPE_COMMENT);
				pNoticeRecord->set_create_ts( fsm._follow_info.m_create_ts );
				pNoticeRecord->set_extra_data_0( int_2_str(fsm._follow_info.m_feed_id) );
				pNoticeRecord->set_extra_data_1( int_2_str(fsm._follow_info.m_origin_comment_id) );
				pNoticeRecord->set_extra_data_2( int_2_str(fsm._follow_info.m_follow_id) );


				//notify feed_owner
				if(fsm._follow_info.m_feed_owner_openid_md5 != fsm._follow_info.m_openid_md5)
				{
					pNoticeRecord->set_openid_md5(fsm._follow_info.m_feed_owner_openid_md5);
					g_server_processor_msg->send_datagram(stMsgRequest);
				}
				
				//notify origim_comment_owner
				if(fsm._follow_info.m_origin_comment_owner_openid_md5 != fsm._follow_info.m_openid_md5
					&& fsm._follow_info.m_origin_comment_owner_openid_md5 != 
					fsm._follow_info.m_feed_owner_openid_md5)
				{
					pNoticeRecord->set_openid_md5(fsm._follow_info.m_origin_comment_owner_openid_md5);				
					g_server_processor_msg->send_datagram(stMsgRequest);
				}

				//notify the reply
				if(fsm._follow_info.m_openid_md5_to != fsm._follow_info.m_openid_md5
					&& fsm._follow_info.m_openid_md5_to != 
					fsm._follow_info.m_origin_comment_owner_openid_md5 
					&& fsm._follow_info.m_openid_md5_to != 
					fsm._follow_info.m_feed_owner_openid_md5)
				{
					pNoticeRecord->set_openid_md5(fsm._follow_info.m_openid_md5_to);				
					g_server_processor_msg->send_datagram(stMsgRequest);
				}
			}
			return;
		}
		
        LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
        return;
    }

    void FsmFollowStateWaitAddDB::timeout_event(FsmFollow& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmFollowStateWaitAddDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmFollowState::state_end);
        return;
    }

	/*******************************************************************************************************/
    /*                                         FsmFollowStateWaitUpdateDB                                  */
    /*******************************************************************************************************/
    void FsmFollowStateWaitUpdateDB::enter(FsmFollow & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmFollowStateWaitUpdateDB::exit(FsmFollow& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmFollowStateWaitUpdateDB::db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& reply)
    {
    	LOG4CPLUS_ERROR(logger, "need code");
        return;
    }

    void FsmFollowStateWaitUpdateDB::timeout_event(FsmFollow& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmFollowStateWaitUpdateDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmFollowState::state_end);
        return;
    }


	
	/*******************************************************************************************************/
    /*                                         FsmFollowStateWaitDeleteDB                                  */
    /*******************************************************************************************************/
    void FsmFollowStateWaitDeleteDB::enter(FsmFollow & fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void FsmFollowStateWaitDeleteDB::exit(FsmFollow& fsm)
    {
        fsm.cancel_timer();
    }

    void FsmFollowStateWaitDeleteDB::db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& reply)
    {
  
		//cancel timer first
	    fsm.cancel_timer();

		uint32_t dwReqType = fsm._req_type;
		uint32_t dwStatus = reply.m_result_code;

	    //resp msg
		hoosho::msg::Msg stRespMsg;
	    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	    header->set_seq(fsm._req_seq_id);
	    
		if(hoosho::msg::DELETE_FOLLOW_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::DELETE_FOLLOW_RES);

			if(dwStatus == ExecutorThreadRequestType::E_OK)
			{
				header->set_result(hoosho::msg::E_OK);
			}
			else if(dwStatus == ExecutorThreadRequestType::E_INVALID_PARAM)
			{
				header->set_result(hoosho::msg::E_INVALID_REQ_PARAM);
			}
			else
			{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
			}
			
		    g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		    fsm.set_state(FsmFollowState::state_end);
			return;
		}
		
		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
        return;
    }

    void FsmFollowStateWaitDeleteDB::timeout_event(FsmFollow& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "FsmFollowStateWaitDeleteDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
        fsm.set_state(FsmFollowState::state_end);
        return;
    }

    /*******************************************************************************************************/
    /*                                         FsmFollowStateEnd                                           */
    /*******************************************************************************************************/
    void FsmFollowStateEnd::enter(FsmFollow& fsm)
    {
        g_fsm_follow_container->del_fsm(&fsm);
    }

