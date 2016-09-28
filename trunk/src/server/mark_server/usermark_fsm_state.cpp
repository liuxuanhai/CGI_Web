#include "usermark_fsm_state.h"
#include "global_var.h"

    /*******************************************************************************************************/
    /*					                       UserMarkState						       					   */
    /*******************************************************************************************************/
    IMPL_LOGGER(UserMarkState, logger);

#define IMPL_FSM_STATE(classname, name) classname UserMarkState::name(#classname)
    IMPL_FSM_STATE(UserMarkFsmStateInit, state_init);
    IMPL_FSM_STATE(UserMarkStateWaitQueryDB, state_wait_query_db);
    IMPL_FSM_STATE(UserMarkStateEnd, state_end);
#undef IMPL_FSM_STATE

    void UserMarkState::enter(UserMarkFsm& fsm)
    {

    }

    void UserMarkState::exit(UserMarkFsm& fsm)
    {

    }

    void UserMarkState::client_req_event(UserMarkFsm & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
    }

	void UserMarkState::db_reply_event(UserMarkFsm& fsm, ExecutorThreadResponseElement& element)
    {
		LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
    }

    void UserMarkState::timeout_event(UserMarkFsm& fsm, void* param)
    {
		LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
    }

    /*******************************************************************************************************/
    /*                                         UserMarkFsmStateInit                                         */
    /*******************************************************************************************************/
    void UserMarkFsmStateInit::client_req_event(UserMarkFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();
        fsm._msg.CopyFrom(stMsg);
		
		if(::hoosho::msg::QUERY_USER_MARK_REQ == stMsg.head().cmd())
		{		
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_USER_MARK_QUERY, fsm._id);
            
            req.need_reply();
            req.m_openid_md5 = stMsg.usermark_query_req().openid_md5();
            g_executor_thread_processor->send_request(req);            
		}
        else if(hoosho::msg::ADD_USER_MARK_REQ == stMsg.head().cmd())
		{
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_USER_MARK_ADD, fsm._id);
            
            req.need_reply();
            req.m_usermark.CopyFrom(stMsg.usermark_add_req().usermark());   
            g_executor_thread_processor->send_request(req);            
		}
		else if(hoosho::msg::DELETE_USER_MARK_REQ == stMsg.head().cmd())
		{
            ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_USER_MARK_DELETE, fsm._id);
            
            req.need_reply();
			req.m_openid_md5 = stMsg.usermark_del_req().openid_md5();
            g_executor_thread_processor->send_request(req);			
		}

		fsm.set_state(UserMarkState::state_wait_query_db);
		return;		
    }


    /*******************************************************************************************************/
    /*                                         UserMarkStateWaitQueryDB                               */
    /*******************************************************************************************************/
    void UserMarkStateWaitQueryDB::enter(UserMarkFsm& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void UserMarkStateWaitQueryDB::exit(UserMarkFsm& fsm)
    {
        fsm.cancel_timer();
    }

    void UserMarkStateWaitQueryDB::db_reply_event(UserMarkFsm& fsm, ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		if(reply.m_result_code != ExecutorThreadRequestType::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "UserMarkStateWaitQueryDB::db_reply_event failed"
                        <<", fsmid:"<<fsm._id
                        <<", result code="<<reply.m_result_code);
                        
			fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);
			return;
		}
		
		//resp msg
		::hoosho::msg::Msg stRespMsg;
        ::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();        
	    header->set_result(::hoosho::msg::E_OK);
        header->set_seq(fsm._msg.head().seq());
        
        if(::hoosho::msg::QUERY_USER_MARK_REQ == fsm._msg.head().cmd())
        {
        	header->set_cmd(::hoosho::msg::QUERY_USER_MARK_RES);        	
			::hoosho::usermark::QueryUserMarkRes* pQueryUserMarkRes = stRespMsg.mutable_usermark_query_res();
			for(size_t i=0; i<reply.vecUserMark.size(); i++)
			{
				//pQueryUserMarkRes->add_usermark(reply.vecUserMark[i]);	
				::hoosho::commstruct::UserMark* pUserMark = pQueryUserMarkRes->add_usermark();
				pUserMark->CopyFrom(reply.vecUserMark[i]);								
			}   	
        }
        else if(::hoosho::msg::ADD_USER_MARK_REQ == fsm._msg.head().cmd())
        {
        	header->set_cmd(::hoosho::msg::ADD_USER_MARK_RES);        	
        }
        else if(::hoosho::msg::DELETE_USER_MARK_REQ == fsm._msg.head().cmd())
        {
			header->set_cmd(::hoosho::msg::DELETE_USER_MARK_RES);        	
        }

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(UserMarkState::state_end);
		return;					
    }

    void UserMarkStateWaitQueryDB::timeout_event(UserMarkFsm& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "UserMarkStateWaitQueryDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_fail(::hoosho::msg::E_SERVER_TIMEOUT);
        fsm.set_state(UserMarkState::state_end);
        return;
    }


    /*******************************************************************************************************/
    /*                                         UserMarkStateEnd                                         */
    /*******************************************************************************************************/
    void UserMarkStateEnd::enter(UserMarkFsm& fsm)
    {
        g_user_mark_fsm_container->del_fsm(&fsm);
    }

