#include "pay_fsm_state.h"
#include "global_var.h"

    /*******************************************************************************************************/
    /*					                       PayState						       					   */
    /*******************************************************************************************************/
    IMPL_LOGGER(PayState, logger);

#define IMPL_FSM_STATE(classname, name) classname PayState::name(#classname)
    IMPL_FSM_STATE(PayFsmStateInit, state_init);
    IMPL_FSM_STATE(PayStateWaitQueryDB, state_wait_query_db);
    IMPL_FSM_STATE(PayStateWaitThreadPayOrder, state_wait_thread_payorder);
    IMPL_FSM_STATE(PayStateEnd, state_end);
#undef IMPL_FSM_STATE

    void PayState::enter(PayFsm& fsm)
    {

    }

    void PayState::exit(PayFsm& fsm)
    {

    }

    void PayState::client_req_event(PayFsm & fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
    }

	void PayState::db_reply_event(PayFsm& fsm, ExecutorThreadResponseElement& element)
    {
		LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
    }

    void PayState::timeout_event(PayFsm& fsm, void* param)
    {
		LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
    }

    /*******************************************************************************************************/
    /*                                         PayFsmStateInit                                         */
    /*******************************************************************************************************/
    void PayFsmStateInit::client_req_event(PayFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
    {
        //save async req param to fsm
        fsm._conn_id = conn.get_id();
        //fsm._msg.CopyFrom(stMsg);
        fsm._cmd = stMsg.head().cmd();
        fsm._seq = stMsg.head().seq();

        if(::hoosho::msg::J_PAY_UNIFIEDORDER_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_REQ, fsm._id);
            
            req.need_reply();
            req.m_business_type = stMsg.unifiedorder_req().type();
            req.m_business_id = stMsg.unifiedorder_req().business_id();
            req.m_client_ip = stMsg.unifiedorder_req().client_ip();            
            req.m_openid = stMsg.unifiedorder_req().openid();            
            
            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_thread_payorder);
        }
        else if(::hoosho::msg::J_PAY_TRANSFER_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_TRANSFER_REQ, fsm._id);
            
            req.need_reply();
            req.m_openid = stMsg.transfer_req().openid();
            req.m_client_ip = stMsg.transfer_req().client_ip();
            req.m_amount = stMsg.transfer_req().amount();
            
            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_thread_payorder);
        }        
        else if(::hoosho::msg::J_PAY_GET_USER_CASH_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_GET_USER_CASH_REQ, fsm._id);
            
            req.need_reply();
            req.m_openid = stMsg.get_user_cash_req().openid();
            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_query_db);
        }
        else if(::hoosho::msg::J_PAY_QUERY_CASH_FLOW_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_QUERY_CASH_FLOW_REQ, fsm._id);
            
            req.need_reply();
            req.m_openid = stMsg.query_cash_flow_req().openid();
            req.m_begin_ts = stMsg.query_cash_flow_req().begin_ts();
            req.m_limit = stMsg.query_cash_flow_req().limit();
            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_query_db);
        }
        else if(::hoosho::msg::J_PAY_UNIFIEDORDER_CALLBACK_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_CALLBACK_REQ, fsm._id);
            
            req.need_reply();
            req.m_client_ip = stMsg.unifiedorder_callback_req().client_ip();
            req.m_callback_xml = stMsg.unifiedorder_callback_req().callback_xml();
			g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_query_db);           
        }
		else if(::hoosho::msg::J_PAY_UNIFIEDORDER_QUERY_REQ == stMsg.head().cmd())
		{
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_QUERY_REQ, fsm._id);
            
            req.need_reply();
            req.m_openid = stMsg.query_unifiedorder_req().openid();   

			req.vecOutTradeNo.clear();
            for(int i=0; i<stMsg.query_unifiedorder_req().out_trade_no_list_size(); i++)
            {
				req.vecOutTradeNo.push_back(stMsg.query_unifiedorder_req().out_trade_no_list(i));
            }
            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_thread_payorder);	
		}
        else if(::hoosho::msg::J_PAY_LISTEN_INCOME_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_LISTEN_INCOME_REQ, fsm._id);
            
            req.need_reply();
            req.m_openid = stMsg.listen_income_req().openid();
            req.m_out_trade_no = stMsg.listen_income_req().out_trade_no();
            req.m_share_type = stMsg.listen_income_req().share_type();
            req.m_record_type = stMsg.listen_income_req().record_type();
            req.m_out_trade_openid = stMsg.listen_income_req().out_trade_openid();
            
            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_query_db);	
        }
        else if(::hoosho::msg::J_PAY_SENDREDPACK_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_SENDREDPACK_REQ, fsm._id);
            
            req.need_reply();
            req.m_openid = stMsg.send_redpack_req().openid();
            req.m_client_ip = stMsg.send_redpack_req().client_ip();
            req.m_table_id = stMsg.send_redpack_req().table_id();
            req.m_amount = stMsg.send_redpack_req().amount();            
            
            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_thread_payorder);
        }
        else if(::hoosho::msg::J_PAY_OUTCOME_QUERY_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_OUTCOME_QUERY_REQ, fsm._id);
            
            req.need_reply();
            req.m_openid = stMsg.query_outcome_req().openid();
			req.m_state = stMsg.query_outcome_req().state();
			req.m_limit = stMsg.query_outcome_req().limit();
			req.m_begin_ts = stMsg.query_outcome_req().begin_ts();            
            
            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_query_db);
        }
        else if(::hoosho::msg::J_PAY_OUTCOME_COMMIT_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_OUTCOME_COMMIT_REQ, fsm._id);
            
            req.need_reply();
            req.m_openid = stMsg.commit_outcome_req().openid();
            req.m_amount = stMsg.commit_outcome_req().amount();

            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_query_db);
        }
        else if(::hoosho::msg::J_PAY_OUTCOME_CHECK_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_OUTCOME_CHECK_REQ, fsm._id);
            
            req.need_reply();
            req.m_table_id = stMsg.check_outcome_req().table_id();
            req.m_openid = stMsg.check_outcome_req().openid();
            req.m_amount = stMsg.check_outcome_req().amount();

            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_query_db);
        }
        else if(::hoosho::msg::J_PAY_OUTCOME_UPDATE_REQ == stMsg.head().cmd())
        {
			ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_PAY_OUTCOME_UPDATE_REQ, fsm._id);
            
            req.need_reply();
            req.m_table_id = stMsg.update_outcome_req().table_id();
            req.m_state = stMsg.update_outcome_req().state();
            req.m_openid = stMsg.update_outcome_req().openid();
            req.m_amount = stMsg.update_outcome_req().amount();
            req.m_out_trade_no = stMsg.update_outcome_req().out_trade_no();

            g_executor_thread_processor->send_request(req);          
            fsm.set_state(PayState::state_wait_query_db);
        }
		
		
		return;		
    }


    /*******************************************************************************************************/
    /*                                         PayStateWaitQueryDB                               */
    /*******************************************************************************************************/
    void PayStateWaitQueryDB::enter(PayFsm& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void PayStateWaitQueryDB::exit(PayFsm& fsm)
    {
        fsm.cancel_timer();
    }

    void PayStateWaitQueryDB::db_reply_event(PayFsm& fsm, ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		if(reply.m_result_code != ExecutorThreadRequestType::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "PayStateWaitQueryDB::db_reply_event failed"
                        <<", fsmid:"<<fsm._id
                        <<", result code="<<ExecutorThreadRequestType::err_string(reply.m_result_code));
                        
            if(reply.m_result_code == ExecutorThreadRequestType::E_BALANCE_NOT_ENOUGH)   
            {
				fsm.reply_fail(::hoosho::msg::E_BALANCE_NOT_ENOUGH);
            }
            else if(reply.m_result_code == ExecutorThreadRequestType::E_PAY_FAIL)
            {
				fsm.reply_fail(::hoosho::msg::E_PAY_FAIL);
            }
            else
            {
				fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);
            }
			
			return;
		}
		
		//resp msg
		::hoosho::msg::Msg stRespMsg;
        ::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();        
	    header->set_result(::hoosho::msg::E_OK);
        header->set_seq(fsm._seq);

		if(::hoosho::msg::J_PAY_GET_USER_CASH_REQ == fsm._cmd)
		{
			::hoosho::j::pay::GetUserCashRes* pGetUserCashRes = stRespMsg.mutable_get_user_cash_res();
			pGetUserCashRes->set_total_income(reply.m_userCash.total_income);
			pGetUserCashRes->set_total_outcome(reply.m_userCash.total_outcome);
			pGetUserCashRes->set_total_outcome_pending(reply.m_userCash.total_outcome_pending);
			
			header->set_cmd(::hoosho::msg::J_PAY_GET_USER_CASH_RES);
		}       
		else if(::hoosho::msg::J_PAY_QUERY_CASH_FLOW_REQ == fsm._cmd)
		{
			::hoosho::j::pay::QueryCashFlowRes* pQueryCashFlowRes = stRespMsg.mutable_query_cash_flow_res();
			std::vector<hoosho::j::commstruct::CashFlowInfo>::iterator it = reply.vecCashFlowInfo.begin();
			for(; it != reply.vecCashFlowInfo.end(); it++)
			{
				pQueryCashFlowRes->add_cash_flow_info_list()->CopyFrom(*it);	
			}
			reply.vecCashFlowInfo.clear();

			header->set_cmd(::hoosho::msg::J_PAY_QUERY_CASH_FLOW_RES);
		}
		else if(::hoosho::msg::J_PAY_UNIFIEDORDER_CALLBACK_REQ == fsm._cmd)
		{
			//1. notify feeds server to update out_trade_no
			if(!strcmp(reply.m_return_code.c_str(), "SUCCESS"))
			{
				::hoosho::msg::Msg stFeedsMsg;
				::hoosho::msg::MsgHead* stFeedsMsgHeader = stFeedsMsg.mutable_head();
				stFeedsMsgHeader->set_cmd(::hoosho::msg::J_SET_OUT_TRADE_NO_REQ);
				stFeedsMsgHeader->set_seq(time(NULL));

				::hoosho::j::feeds::SetOutTradeNoReq* stSetOutTradeNoReq = stFeedsMsg.mutable_j_feeds_set_out_trade_no_req();
				stSetOutTradeNoReq->set_business_type(reply.m_business_type);
				stSetOutTradeNoReq->set_business_id(reply.m_business_id);
				stSetOutTradeNoReq->set_out_trade_no(reply.m_out_trade_no);

				g_feeds_server_processor->send_datagram(stFeedsMsg);
			}
			
			
			//2. return CGI		
			::hoosho::j::pay::UnifiedOrderCallbackRes* pUnifiedOrderCallbackRes = stRespMsg.mutable_unifiedorder_callback_res();

			pUnifiedOrderCallbackRes->set_return_code(reply.m_return_code);
			pUnifiedOrderCallbackRes->set_return_msg(reply.m_return_msg);

			header->set_cmd(::hoosho::msg::J_PAY_UNIFIEDORDER_CALLBACK_RES);
		}
		else if(::hoosho::msg::J_PAY_LISTEN_INCOME_REQ == fsm._cmd)
		{
			header->set_cmd(::hoosho::msg::J_PAY_LISTEN_INCOME_RES);
		}
		else if(::hoosho::msg::J_PAY_OUTCOME_QUERY_REQ == fsm._cmd)
		{
			::hoosho::j::pay::QueryOutcomeRes* pQueryOutcomeRes = stRespMsg.mutable_query_outcome_res();
			std::vector<hoosho::j::commstruct::OutcomeReqInfo>::iterator it = reply.vecOutcomeReqInfo.begin();
			for(; it != reply.vecOutcomeReqInfo.end(); it++)
			{
				pQueryOutcomeRes->add_outcome_req_info_list()->CopyFrom(*it);	
			}
			reply.vecOutcomeReqInfo.clear();

			header->set_cmd(::hoosho::msg::J_PAY_OUTCOME_QUERY_RES);
		}
		else if(::hoosho::msg::J_PAY_OUTCOME_COMMIT_REQ == fsm._cmd)
		{
			::hoosho::j::pay::CommitOutcomeRes* pCommitOutcomeRes = stRespMsg.mutable_commit_outcome_res();
			pCommitOutcomeRes->set_total_income(reply.m_userCash.total_income);
			pCommitOutcomeRes->set_total_outcome(reply.m_userCash.total_outcome);
			pCommitOutcomeRes->set_total_outcome_pending(reply.m_userCash.total_outcome_pending);

			header->set_cmd(::hoosho::msg::J_PAY_OUTCOME_COMMIT_RES);
		}
		else if(::hoosho::msg::J_PAY_OUTCOME_CHECK_REQ == fsm._cmd)
		{
			header->set_cmd(::hoosho::msg::J_PAY_OUTCOME_CHECK_RES);
		}
		else if(::hoosho::msg::J_PAY_OUTCOME_UPDATE_REQ == fsm._cmd)
		{
			header->set_cmd(::hoosho::msg::J_PAY_OUTCOME_UPDATE_RES);
		}

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(PayState::state_end);
		return;					
    }

    void PayStateWaitQueryDB::timeout_event(PayFsm& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "PayStateWaitQueryDB time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_fail(::hoosho::msg::E_SERVER_TIMEOUT);
        fsm.set_state(PayState::state_end);
        return;
    }

    /*******************************************************************************************************/
    /*                                         PayStateWaitThreadPayOrder                               */
    /*******************************************************************************************************/
    void PayStateWaitThreadPayOrder::enter(PayFsm& fsm)
    {
        fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
    }

    void PayStateWaitThreadPayOrder::exit(PayFsm& fsm)
    {
        fsm.cancel_timer();
    }

    void PayStateWaitThreadPayOrder::db_reply_event(PayFsm& fsm, ExecutorThreadResponseElement& reply)
    {
        //cancel timer first
        fsm.cancel_timer();

		if(reply.m_result_code != ExecutorThreadRequestType::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "PayStateWaitThreadPayOrder::db_reply_event failed"
                        <<", fsmid:"<<fsm._id
                        <<", result code="<<ExecutorThreadRequestType::err_string(reply.m_result_code));

            if(reply.m_result_code == ExecutorThreadRequestType::E_PAY_FAIL)
            {
            	if(::hoosho::msg::J_PAY_UNIFIEDORDER_QUERY_REQ == fsm._cmd)
            	{
            		::hoosho::msg::Msg stRespMsg;
        			::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();   
            		header->set_result(::hoosho::msg::E_PAY_FAIL);
					header->set_cmd(::hoosho::msg::J_PAY_UNIFIEDORDER_QUERY_RES);
					header->set_seq(fsm._seq);

					::hoosho::j::pay::UnifiedOrderQueryRes* pUnifiedOrderQueryRes = stRespMsg.mutable_query_unifiedorder_res();
					for(size_t i=0; i<reply.vecFaultOutTradeNo.size(); i++)
					{
						pUnifiedOrderQueryRes->add_out_trade_no_list(reply.vecFaultOutTradeNo[i]);
					}
					reply.vecFaultOutTradeNo.clear();

					g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
					fsm.set_state(PayState::state_end);
					return;					
            	}
            	else
            	{
					fsm.reply_fail(::hoosho::msg::E_PAY_FAIL);
            	}					
            }
            else
            {
				fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);
            }                        
			
			return;
		}
		
		//resp msg
		::hoosho::msg::Msg stRespMsg;
        ::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();        
	    header->set_result(::hoosho::msg::E_OK);
        header->set_seq(fsm._seq);

		if(::hoosho::msg::J_PAY_UNIFIEDORDER_REQ == fsm._cmd)
		{
			::hoosho::j::pay::UnifiedOrderRes* pUnifiedOrderRes = stRespMsg.mutable_unifiedorder_res();
			pUnifiedOrderRes->set_appid(reply.m_appid);
			pUnifiedOrderRes->set_timestamp(reply.m_time_stamp);
			pUnifiedOrderRes->set_nonce_str(reply.m_nonce_str);
			pUnifiedOrderRes->set_package(reply.m_package);
			pUnifiedOrderRes->set_sign_type(reply.m_sign_type);
			pUnifiedOrderRes->set_pay_sign(reply.m_pay_sign);
			pUnifiedOrderRes->set_out_trade_no(reply.m_out_trade_no);
			
			header->set_cmd(::hoosho::msg::J_PAY_UNIFIEDORDER_RES);
		}       
		else if(::hoosho::msg::J_PAY_TRANSFER_REQ == fsm._cmd)
		{
			::hoosho::j::pay::TransferRes* pTransferRes = stRespMsg.mutable_transfer_res();
			pTransferRes->set_total_income(reply.m_userCash.total_income);
			pTransferRes->set_total_outcome(reply.m_userCash.total_outcome);
			pTransferRes->set_total_outcome_pending(reply.m_userCash.total_outcome_pending);

			header->set_cmd(::hoosho::msg::J_PAY_TRANSFER_RES);
		}
		else if(::hoosho::msg::J_PAY_UNIFIEDORDER_QUERY_REQ == fsm._cmd)
		{
			//::hoosho::j::pay::UnifiedOrderQueryRes* pUnifiedOrderQueryRes = stRespMsg.mutable_query_unifiedorder_req();

			header->set_cmd(::hoosho::msg::J_PAY_UNIFIEDORDER_QUERY_RES);
		}
		else if(::hoosho::msg::J_PAY_SENDREDPACK_REQ == fsm._cmd)
		{
			::hoosho::j::pay::SendRedPackRes* pSendRedPackRes = stRespMsg.mutable_send_redpack_res();
			pSendRedPackRes->set_total_income(reply.m_userCash.total_income);
			pSendRedPackRes->set_total_outcome(reply.m_userCash.total_outcome);
			pSendRedPackRes->set_total_outcome_pending(reply.m_userCash.total_outcome_pending);

			header->set_cmd(::hoosho::msg::J_PAY_SENDREDPACK_RES);
		}

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
		fsm.set_state(PayState::state_end);
		return;					
    }

    void PayStateWaitThreadPayOrder::timeout_event(PayFsm& fsm, void* param)
    {
        LOG4CPLUS_ERROR(logger, "PayStateWaitThreadPayOrder time_out."
                        <<" fsmid:"<<fsm._id
                        <<" connid:"<<fsm._conn_id);
		fsm.reply_fail(::hoosho::msg::E_SERVER_TIMEOUT);
        fsm.set_state(PayState::state_end);
        return;
    }


    /*******************************************************************************************************/
    /*                                         PayStateEnd                                         */
    /*******************************************************************************************************/
    void PayStateEnd::enter(PayFsm& fsm)
    {
        g_pay_fsm_container->del_fsm(&fsm);
    }

