#include "fsm_feed_state.h"
#include "global_var.h"
#include "db_feed.h"
#include "common_util.h"

	/*******************************************************************************************************/
	/*					                       FsmFeedState						       					   */
	/*******************************************************************************************************/
	IMPL_LOGGER(FsmFeedState, logger);

#define IMPL_FSM_STATE(classname, name) classname FsmFeedState::name(#classname)
    IMPL_FSM_STATE(FsmFeedStateInit, state_init);
    IMPL_FSM_STATE(FsmFeedStateWaitQueryDB, state_wait_query_db);
	IMPL_FSM_STATE(FsmFeedStateWaitAddDB, state_wait_add_db);
    IMPL_FSM_STATE(FsmFeedStateWaitUpdateDB, state_wait_update_db);
	IMPL_FSM_STATE(FsmFeedStateWaitDeleteDB, state_wait_delete_db);
    IMPL_FSM_STATE(FsmFeedStateEnd, state_end);
#undef IMPL_FSM_STATE

	void FsmFeedState::enter(FsmFeed& fsm)
	{

	}

	void FsmFeedState::exit(FsmFeed& fsm)
	{

	}

	void FsmFeedState::client_req_event(FsmFeed & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
	{
	    LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
	}

	void FsmFeedState::db_reply_event(FsmFeed& fsm, const ExecutorThreadResponseElement& element)
	{
		LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
	}

	void FsmFeedState::timeout_event(FsmFeed& fsm, void* param)
	{
		LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
	}

	/*******************************************************************************************************/
	/*                                         FsmFeedStateInit                                            */
	/*******************************************************************************************************/
	void FsmFeedStateInit::client_req_event(FsmFeed& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
	{
	    //save async req param to fsm
	    fsm._conn_id = conn.get_id();
	    fsm._req_type = stMsg.head().cmd();
	    fsm._req_seq_id = stMsg.head().seq();

		//dispatch
		uint32_t dwReqType = stMsg.head().cmd();

		//query_req 	feed_detail
		if(hoosho::msg::QUERY_FEED_DETAIL_REQ == dwReqType)
		{
			const hoosho::feeds::QueryFeedDetailReq& stQueryFeedDetailReq = stMsg.query_feed_detail_req();
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FEED_DETAIL_QUERY);
			req.m_fsm_id = fsm._id;
			req.m_feed_id_list.clear();
			for(int i = 0; i < stQueryFeedDetailReq.feed_id_list_size(); i++)
			{
				req.m_feed_id_list.push_back( stQueryFeedDetailReq.feed_id_list(i) );
			}
			req.need_reply();
	        g_executor_thread_processor->send_request(req);
	        fsm.set_state(FsmFeedState::state_wait_query_db);
	        return;
		}

		//query_req		feed list
		if(hoosho::msg::QUERY_FEED_REQ == dwReqType)
		{
			const hoosho::feeds::QueryFeedReq& stQueryFeedReq = stMsg.query_feed_req();

			//query db
	        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FEED_QUERY);
	        req.m_fsm_id = fsm._id;
	        req.m_feed_index_on_appid.m_pa_appid_md5 = stQueryFeedReq.pa_appid_md5();
	        req.m_feed_index_on_appid.m_openid_md5 = stQueryFeedReq.openid_md5();
			req.m_feed_index_on_appid.m_begin_feed_id = stQueryFeedReq.begin_feed_id();
			req.m_feed_index_on_appid.m_limit = stQueryFeedReq.limit();
			req.m_feed_index_on_appid.m_type = stQueryFeedReq.type();
			req.m_feed_index_on_appid.m_feed_type = stQueryFeedReq.feed_type();
			req.m_feed_index_on_appid.m_origin_feed_id = stQueryFeedReq.origin_feed_id();
			req.m_feed_index_on_appid.m_order_type = stQueryFeedReq.order_type();
			req.m_feed_index_on_appid.m_offset = stQueryFeedReq.offset();
	        req.need_reply();
	        g_executor_thread_processor->send_request(req);
	        fsm.set_state(FsmFeedState::state_wait_query_db);
	        return;
		}

		//add_req
	    if(hoosho::msg::ADD_FEED_REQ == dwReqType)
		{
			const hoosho::feeds::AddFeedReq& stAddFeedReq = stMsg.add_feed_req();

			//save async data
			fsm._feed_info.m_pa_appid_md5 = stAddFeedReq.pa_appid_md5();
			fsm._feed_info.m_openid_md5 = stAddFeedReq.openid_md5();
			fsm._feed_info.m_cover_pic_id = stAddFeedReq.cover_pic_id();
			fsm._feed_info.m_pic_id_list = stAddFeedReq.pic_id_list();
			fsm._feed_info.m_content = stAddFeedReq.content();
			fsm._feed_info.m_feed_type = stAddFeedReq.feed_type();
			fsm._feed_info.m_extra_data_0 =
				fsm._feed_info.m_extra_data_1 =
				fsm._feed_info.m_extra_data_2 = "";
			fsm._feed_info.m_origin_feed_id = 0;
			if(fsm._feed_info.m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_FEED_OF_TOPIC)
			{
				fsm._feed_info.m_origin_feed_id = stAddFeedReq.origin_feed_id();
				fsm._feed_info.m_extra_data_0 = int_2_str( stAddFeedReq.origin_feed_id() );
			}
			else if(fsm._feed_info.m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_BANNER_FEED)
			{
				fsm._feed_info.m_extra_data_0 = int_2_str( stAddFeedReq.banner_feed_subtype() );
				if(stAddFeedReq.banner_feed_subtype() == ::hoosho::commenum::FEEDS_BANNER_FEED_SUBTYPE_AD)
				{
					fsm._feed_info.m_extra_data_1 = stAddFeedReq.url();
				}
				else if(stAddFeedReq.banner_feed_subtype() == ::hoosho::commenum::FEEDS_BANNER_FEED_SUBTYPE_PUSH)
				{
					fsm._feed_info.m_extra_data_1 = int_2_str( stAddFeedReq.origin_feed_id() );
				}
			}
//			LOG4CPLUS_TRACE(logger, "ex_data_1 = " << fsm._feed_info.m_extra_data_1);

			//gen id
			fsm._feed_info.m_feed_id = common::util::generate_unique_id();
			//set creat_ts
			timeval tv;
			gettimeofday(&tv, NULL);
			fsm._feed_info.m_create_ts = (uint64_t)tv.tv_sec;
			fsm._feed_info.m_del_ts = 0;

			//add to db
	        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FEED_ADD);
	        req.m_fsm_id = fsm._id;
			req.m_feed_info = fsm._feed_info;

	        req.need_reply();
	        g_executor_thread_processor->send_request(req);
	        fsm.set_state(FsmFeedState::state_wait_add_db);
	        return;
		}

		//delete_req
		if(hoosho::msg::DELETE_FEED_REQ == dwReqType)
		{
			const hoosho::feeds::DeleteFeedReq& stDeleteFeedReq = stMsg.delete_feed_req();

			//delete from db , not care reply
			LOG4CPLUS_DEBUG(logger, "then, delete db, feed_id="<<stDeleteFeedReq.feed_id());
	        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_FEED_DELETE);
	        req.m_fsm_id = fsm._id;
	        req.m_feed_info.m_feed_id = stDeleteFeedReq.feed_id();
			req.m_feed_info.m_pa_appid_md5 = stDeleteFeedReq.pa_appid_md5();
			req.m_feed_info.m_openid_md5 = stDeleteFeedReq.openid_md5();
			req.need_reply();
	        g_executor_thread_processor->send_request(req);

			fsm.set_state(FsmFeedState::state_wait_delete_db);
			return;
		}

		//add report_feed
		if(hoosho::msg::ADD_REPORT_FEED_REQ == dwReqType)
		{
			const hoosho::feeds::AddReportFeedReq& stAddReportFeedReq = stMsg.add_report_feed_req();

	        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_REPORT_FEED_ADD);
	        req.m_fsm_id = fsm._id;
	        
			req.m_feed_report.m_pa_appid_md5 = stAddReportFeedReq.feed_report_info().pa_appid_md5();
			req.m_feed_report.m_openid_md5_from = stAddReportFeedReq.feed_report_info().openid_md5_from();
			req.m_feed_report.m_openid_md5_to = stAddReportFeedReq.feed_report_info().openid_md5_to();
			req.m_feed_report.m_feed_id = stAddReportFeedReq.feed_report_info().feed_id();
			req.m_feed_report.m_report_ts = stAddReportFeedReq.feed_report_info().report_ts();
			req.m_feed_report.m_report_reason = stAddReportFeedReq.feed_report_info().report_reason();
			
			req.need_reply();
	        g_executor_thread_processor->send_request(req);

			fsm.set_state(FsmFeedState::state_wait_add_db);
			return;
		}

		//query forbid_user
		if(hoosho::msg::QUERY_USER_FORBID_REQ == dwReqType)
		{
			const hoosho::feeds::QueryUserForbidReq& stQueryUserForbidReq = stMsg.query_user_forbid_req();

			//query db
	        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_USER_FORBID_QUERY);
	        req.m_fsm_id = fsm._id;
	        req.m_user_forbid.m_pa_appid_md5 = stQueryUserForbidReq.pa_appid_md5();
	        req.m_user_forbid.m_openid_md5 = stQueryUserForbidReq.openid_md5();
			
	        req.need_reply();
	        g_executor_thread_processor->send_request(req);
	        fsm.set_state(FsmFeedState::state_wait_query_db);
	        return;
		}

		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
		return;

	}


	/*******************************************************************************************************/
	/*                                         FsmFeedStateWaitQueryDB                                     */
	/*******************************************************************************************************/
	void FsmFeedStateWaitQueryDB::enter(FsmFeed& fsm)
	{
	    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
	}

	void FsmFeedStateWaitQueryDB::exit(FsmFeed& fsm)
	{
	    fsm.cancel_timer();
	}

	void FsmFeedStateWaitQueryDB::db_reply_event(FsmFeed& fsm, const ExecutorThreadResponseElement& reply)
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

		const vector<FeedInfo>& stFeedList = reply.m_feed_list;

		if(hoosho::msg::QUERY_FEED_DETAIL_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::QUERY_FEED_DETAIL_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
	    	{

				hoosho::feeds::QueryFeedDetailRes* pBody = stRespMsg.mutable_query_feed_detail_res();
				//!!!!!!!!!!!!!!!!!!
				LOG4CPLUS_TRACE(logger, "list_size = " << stFeedList.size());

				for(size_t i = 0; i < stFeedList.size(); i++)
				{
					LOG4CPLUS_TRACE(logger, "info = " << stFeedList[i].ToString());

					hoosho::commstruct::FeedInfo* pBodyFeed = pBody->add_feed_list();
					pBodyFeed->set_feed_id( stFeedList[i].m_feed_id );
					pBodyFeed->set_pa_appid_md5( stFeedList[i].m_pa_appid_md5 );
					pBodyFeed->set_openid_md5( stFeedList[i].m_openid_md5 );
					pBodyFeed->set_cover_pic_id( stFeedList[i].m_cover_pic_id );
					pBodyFeed->set_pic_id_list( stFeedList[i].m_pic_id_list );
					pBodyFeed->set_content( stFeedList[i].m_content );
					pBodyFeed->set_num_favorite( stFeedList[i].m_num_favorite );
					pBodyFeed->set_num_share( stFeedList[i].m_num_share );
					pBodyFeed->set_num_comment( stFeedList[i].m_num_comment );
					pBodyFeed->set_num_read( stFeedList[i].m_num_read );
					pBodyFeed->set_create_ts( stFeedList[i].m_create_ts );
					pBodyFeed->set_del_ts( stFeedList[i].m_del_ts );
					pBodyFeed->set_feed_type( stFeedList[i].m_feed_type );
					pBodyFeed->set_extra_data_0 ( stFeedList[i].m_extra_data_0 );
					pBodyFeed->set_extra_data_1 ( stFeedList[i].m_extra_data_1 );
					pBodyFeed->set_extra_data_2 ( stFeedList[i].m_extra_data_2 );

                    if(stFeedList[i].m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED)
                    {
                        pBodyFeed->set_num_feed( stFeedList[i].m_num_feed );
                    }
                    else
                    {
                        pBodyFeed->set_num_feed( 0 );
                    }
				}
			}
			else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
			{

			}
			else if(ExecutorThreadRequestType::E_FEED_DELETED == dwStatus)
			{
				header->set_result(hoosho::msg::E_FEED_DELETED);
			}
	    	else
	    	{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
	    	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmFeedState::state_end);

			return;
		}

		if(hoosho::msg::QUERY_FEED_REQ == dwReqType)
	    {
	    	header->set_cmd(hoosho::msg::QUERY_FEED_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
	    	{

				hoosho::feeds::QueryFeedRes* pBody = stRespMsg.mutable_query_feed_res();
				//!!!!!!!!!!!!!!!!!!
				LOG4CPLUS_TRACE(logger, "list_size = " << stFeedList.size());

				for(size_t i = 0; i < stFeedList.size(); i++)
				{
					LOG4CPLUS_TRACE(logger, "info = " << stFeedList[i].ToString());

					hoosho::commstruct::FeedInfo* pBodyFeed = pBody->add_feed_list();
					pBodyFeed->set_feed_id( stFeedList[i].m_feed_id );
					pBodyFeed->set_pa_appid_md5( stFeedList[i].m_pa_appid_md5 );
					pBodyFeed->set_openid_md5( stFeedList[i].m_openid_md5 );
					pBodyFeed->set_cover_pic_id( stFeedList[i].m_cover_pic_id );
					pBodyFeed->set_pic_id_list( stFeedList[i].m_pic_id_list );
					pBodyFeed->set_content( stFeedList[i].m_content );
					pBodyFeed->set_num_favorite( stFeedList[i].m_num_favorite );
					pBodyFeed->set_num_share( stFeedList[i].m_num_share );
					pBodyFeed->set_num_comment( stFeedList[i].m_num_comment );
					pBodyFeed->set_num_read( stFeedList[i].m_num_read );
					pBodyFeed->set_create_ts( stFeedList[i].m_create_ts );
					pBodyFeed->set_del_ts( stFeedList[i].m_del_ts );
					pBodyFeed->set_feed_type( stFeedList[i].m_feed_type );
					pBodyFeed->set_extra_data_0 ( stFeedList[i].m_extra_data_0 );
					pBodyFeed->set_extra_data_1 ( stFeedList[i].m_extra_data_1 );
					pBodyFeed->set_extra_data_2 ( stFeedList[i].m_extra_data_2 );

					if(stFeedList[i].m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED)
                    {
                        pBodyFeed->set_num_feed( stFeedList[i].m_num_feed );
                    }
                    else
                    {
                        pBodyFeed->set_num_feed( 0 );
                    }
				}

				pBody->set_total(reply.m_total);

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
			fsm.set_state(FsmFeedState::state_end);
	    	return;
	    }

	    if(hoosho::msg::QUERY_USER_FORBID_REQ == dwReqType)
	    {
	    	header->set_cmd(hoosho::msg::QUERY_USER_FORBID_RES);
			if(ExecutorThreadRequestType::E_OK == dwStatus)
	    	{

				hoosho::feeds::QueryUserForbidRes* pBody = stRespMsg.mutable_query_user_forbid_res();
				
				hoosho::commstruct::UserForbidInfo* stUserForbidInfo = pBody->mutable_user_forbid_info();
				stUserForbidInfo->set_pa_appid_md5( reply.m_user_forbid.m_pa_appid_md5 );
				stUserForbidInfo->set_openid_md5( reply.m_user_forbid.m_openid_md5);
				stUserForbidInfo->set_until_ts( reply.m_user_forbid.m_until_ts);
	    	}
	    	else
	    	{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
	    	}

			g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
			fsm.set_state(FsmFeedState::state_end);
	    	return;
	    }


		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
		return;

	}

	void FsmFeedStateWaitQueryDB::timeout_event(FsmFeed& fsm, void* param)
	{
	    LOG4CPLUS_ERROR(logger, "FsmFeedStateWaitQueryDB time_out."
	                    <<" fsmid:"<<fsm._id
	                    <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
	    fsm.set_state(FsmFeedState::state_end);
	    return;
	}

	/*******************************************************************************************************/
	/*                                         FsmFeedStateWaitAddDB          		                       */
	/*******************************************************************************************************/
	void FsmFeedStateWaitAddDB::enter(FsmFeed& fsm)
	{
	    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
	}

	void FsmFeedStateWaitAddDB::exit(FsmFeed& fsm)
	{
	    fsm.cancel_timer();
	}

	void FsmFeedStateWaitAddDB::db_reply_event(FsmFeed& fsm, const ExecutorThreadResponseElement& reply)
	{
		//cancel timer first
	    fsm.cancel_timer();

		uint32_t dwReqType = fsm._req_type;
		uint32_t dwStatus = reply.m_result_code;

	    //resp msg
		hoosho::msg::Msg stRespMsg;
	    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	    header->set_seq(fsm._req_seq_id);


		if(hoosho::msg::ADD_FEED_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::ADD_FEED_RES);

			hoosho::feeds::AddFeedRes* pBody = stRespMsg.mutable_add_feed_res();
			pBody->set_feed_id(reply.m_feed_info.m_feed_id);

			if(dwStatus == ExecutorThreadRequestType::E_OK)
			{
				header->set_result(hoosho::msg::E_OK);

			}
			else
			{
				header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
			}

		    g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		    fsm.set_state(FsmFeedState::state_end);
			return;
		}

		if(hoosho::msg::ADD_REPORT_FEED_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::ADD_FEED_RES);

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
		    fsm.set_state(FsmFeedState::state_end);
			return;
		}

		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
	    return;

	}

	void FsmFeedStateWaitAddDB::timeout_event(FsmFeed& fsm, void* param)
	{
	    LOG4CPLUS_ERROR(logger, "FsmFeedStateWaitAddDB time_out."
	                    <<" fsmid:"<<fsm._id
	                    <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
	    fsm.set_state(FsmFeedState::state_end);
	    return;
	}


	/*******************************************************************************************************/
	/*                                         FsmFeedStateWaitUpdateDB                                    */
	/*******************************************************************************************************/
	void FsmFeedStateWaitUpdateDB::enter(FsmFeed & fsm)
	{
	    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
	}

	void FsmFeedStateWaitUpdateDB::exit(FsmFeed& fsm)
	{
	    fsm.cancel_timer();
	}

	void FsmFeedStateWaitUpdateDB::db_reply_event(FsmFeed& fsm, const ExecutorThreadResponseElement& reply)
	{
	 	LOG4CPLUS_ERROR(logger, "not code");
		return;
	}

	void FsmFeedStateWaitUpdateDB::timeout_event(FsmFeed& fsm, void* param)
	{
	    LOG4CPLUS_ERROR(logger, "FsmFeedStateWaitUpdateDB time_out."
	                    <<" fsmid:"<<fsm._id
	                    <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
	    fsm.set_state(FsmFeedState::state_end);
	    return;
	}

	/*******************************************************************************************************/
	/*                                         FsmFeedStateWaitDeleteDB                                    */
	/*******************************************************************************************************/
	void FsmFeedStateWaitDeleteDB::enter(FsmFeed & fsm)
	{
	    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
	}

	void FsmFeedStateWaitDeleteDB::exit(FsmFeed& fsm)
	{
	    fsm.cancel_timer();
	}

	void FsmFeedStateWaitDeleteDB::db_reply_event(FsmFeed& fsm, const ExecutorThreadResponseElement& reply)
	{
	    //cancel timer first
	    fsm.cancel_timer();

		uint32_t dwReqType = fsm._req_type;
		uint32_t dwStatus = reply.m_result_code;

	    //resp msg
		hoosho::msg::Msg stRespMsg;
	    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	    header->set_seq(fsm._req_seq_id);

		if(hoosho::msg::DELETE_FEED_REQ == dwReqType)
		{
			header->set_cmd(hoosho::msg::DELETE_FEED_RES);

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
		    fsm.set_state(FsmFeedState::state_end);
			return;
		}

		LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
	    return;
	}

	void FsmFeedStateWaitDeleteDB::timeout_event(FsmFeed& fsm, void* param)
	{
	    LOG4CPLUS_ERROR(logger, "FsmFeedStateWaitDeleteDB time_out."
	                    <<" fsmid:"<<fsm._id
	                    <<" connid:"<<fsm._conn_id);
		fsm.reply_timeout();
	    fsm.set_state(FsmFeedState::state_end);
	    return;
	}


	/*******************************************************************************************************/
	/*                                         FsmFeedStateEnd                                             */
	/*******************************************************************************************************/
	void FsmFeedStateEnd::enter(FsmFeed& fsm)
	{
	    g_fsm_feed_container->del_fsm(&fsm);
	}

