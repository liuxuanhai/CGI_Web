#include "fsm_follow_state.h"
#include "global_var.h"

/*******************************************************************************************************/
/*					                       FsmFollowState					       					   */
/*******************************************************************************************************/
IMPL_LOGGER(FsmFollowState, logger);

#define IMPL_FSM_STATE(classname, name) classname FsmFollowState::name(#classname)
IMPL_FSM_STATE(FsmFollowStateInit, state_init);
IMPL_FSM_STATE(FsmFollowStateWaitQueryDb, state_wait_query_db);
IMPL_FSM_STATE(FsmFollowStateWaitCheckOutTradeNoValid, state_wait_check_out_trade_no_valid);
IMPL_FSM_STATE(FsmFollowStateWaitCheckOutTradeNoUnique, state_wait_check_out_trade_no_unique);
IMPL_FSM_STATE(FsmFollowStateWaitTranscode, state_wait_transcode);
IMPL_FSM_STATE(FsmFollowStateWaitInsertIntoDb, state_wait_insert_into_db);
IMPL_FSM_STATE(FsmFollowStateEnd, state_end);
#undef IMPL_FSM_STATE

void FsmFollowState::enter(FsmFollow& fsm)
{

}

void FsmFollowState::exit(FsmFollow& fsm)
{

}

void FsmFollowState::client_req_event(FsmFollow& fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
    LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
}
void FsmFollowState::query_db_reply_event(FsmFollow &fsm, const ExecutorThreadResponseElement &element)
{
    LOG4CPLUS_DEBUG(logger, "default  query_db_reply_event, state: "<<this->name());
}
void FsmFollowState::check_out_trade_no_valid_reply_event(FsmFollow &fsm, const hoosho::msg::Msg &stMsg)
{
    LOG4CPLUS_DEBUG(logger, "default  check_out_trade_no_valid_reply_event, state: "<<this->name());
}
void FsmFollowState::check_out_trade_no_unique_reply_event(FsmFollow &fsm, const ExecutorThreadResponseElement &element)
{
    LOG4CPLUS_DEBUG(logger, "default  check_out_trade_no_unique_reply_event, state: "<<this->name());
}
void FsmFollowState::transcode_reply_event(FsmFollow &fsm, const hoosho::msg::Msg &stMsg)
{
    LOG4CPLUS_DEBUG(logger, "default  transcode_reply_event, state: "<<this->name());
}
void FsmFollowState::insert_into_db_reply_event(FsmFollow &fsm, const ExecutorThreadResponseElement &element)
{
    LOG4CPLUS_DEBUG(logger, "default  insert_into_db_reply_event, state: "<<this->name());
}

void FsmFollowState::timeout_event(FsmFollow& fsm, void* param)
{
    LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
}



/*******************************************************************************************************/
/*                                         FsmFollowStateInit                                           */
/*******************************************************************************************************/
void FsmFollowStateInit::client_req_event(FsmFollow& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
    //save async req param to fsm
    fsm._conn_id = conn.get_id();
    fsm._req_type = stMsg.head().cmd();
    fsm._req_seq_id = stMsg.head().seq();

    //dispatch
    uint32_t dwReqType = stMsg.head().cmd();


    //add follow
    if(hoosho::msg::J_ADD_FOLLOW_REQ == dwReqType)
    {
        const hoosho::j::feeds::AddFollowReq& stAddFollowReq = stMsg.j_feeds_add_follow_req();

        //save async data
        fsm._follow_info.CopyFrom(stAddFollowReq.follow_info());

        // conneect  transcode_server to download
        ::hoosho::msg::Msg stMsgRequest;
        ::hoosho::msg::MsgHead* pMsgRequestHead = stMsgRequest.mutable_head();
        pMsgRequestHead->set_cmd(::hoosho::msg::J_TRANSCODE_REQ);
        pMsgRequestHead->set_seq(fsm._id);
        ::hoosho::j::transcode::TranscodeReq* pTranscodeReq = stMsgRequest.mutable_transcode_req();
        pTranscodeReq->set_type(fsm._follow_info.content_type());
        pTranscodeReq->set_wx_media_id(fsm._follow_info.content_id());
        g_server_processor_transcode->send_datagram(stMsgRequest);
        fsm.set_state(state_wait_transcode);
        return;
    }


    //add listen
    if(hoosho::msg::J_ADD_LISTEN_REQ == dwReqType)
    {
        const hoosho::j::feeds::AddListenReq& stAddListenReq = stMsg.j_feeds_add_listen_req();

        //save async data
        fsm._listen_info.CopyFrom(stAddListenReq.listen_info());

        // save db
        ExecutorThreadRequestElement request(ExecutorThreadRequestType::T_ADD_LISTEN);
        request.m_listen_info.CopyFrom(fsm._listen_info);
        request.need_reply();
        request.m_fsm_id = fsm._id;
        g_executor_thread_processor->send_request(request);
        fsm.set_state(state_wait_insert_into_db);
        return;
    }

    //get listen detail
    if(hoosho::msg::J_GET_LISTEN_DETAIL_REQ == dwReqType)
    {
        const hoosho::j::feeds::GetListenDetailReq& stGetListenDetailReq = stMsg.j_feeds_get_listen_detail_req();
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_GET_LISTEN_DETAIL);

        //save
        fsm._listen_info.set_openid(stGetListenDetailReq.openid());
        fsm._listen_info.set_follow_id(stGetListenDetailReq.follow_id());
//        fsm._listen_info.set_out_trade_no(stGetListenDetailReq.out_trade_no());

        req.m_fsm_id = fsm._id;
        req.m_openid = stGetListenDetailReq.openid();
        req.m_follow_id = stGetListenDetailReq.follow_id();
//        req.m_out_trade_no = stGetListenDetailReq.out_trade_no();

        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(state_wait_query_db);
        return;
    }

    LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
    return;

}

/*******************************************************************************************************/
/*                                        FsmFollowStateQueryDb                                     */
/*******************************************************************************************************/
void FsmFollowStateWaitQueryDb::enter(FsmFollow& fsm)
{
    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void FsmFollowStateWaitQueryDb::exit(FsmFollow& fsm)
{
    fsm.cancel_timer();
}

void FsmFollowStateWaitQueryDb::query_db_reply_event(FsmFollow &fsm, const ExecutorThreadResponseElement &reply)
{
    LOG4CPLUS_DEBUG(logger, "FsmFollowStateQueryDb::query_db_reply_event");

    //cancel timer first
    fsm.cancel_timer();
    uint32_t dwReqType = fsm._req_type;


    //notify pay_server
    if(dwReqType == ::hoosho::msg::J_GET_LISTEN_DETAIL_REQ)
    {
        for(size_t i = 0; i < reply.m_auto_comment_follow_list.size(); i++)
        {
            const AutoCommentFollowInfo& stInfo = reply.m_auto_comment_follow_list[i];
            ::hoosho::msg::Msg stMsgRequest;
            ::hoosho::msg::MsgHead* pMsgRequestHead = stMsgRequest.mutable_head();
            pMsgRequestHead->set_cmd(::hoosho::msg::J_PAY_LISTEN_INCOME_REQ);
            pMsgRequestHead->set_seq(time(NULL));
            ::hoosho::j::pay::ListenIncomeReq* pListenIncomeReq = stMsgRequest.mutable_listen_income_req();
            pListenIncomeReq->set_openid(stInfo.m_openid);
            pListenIncomeReq->set_out_trade_no(stInfo.m_out_trade_no);
            pListenIncomeReq->set_out_trade_openid(stInfo.m_out_trade_openid);
            pListenIncomeReq->set_share_type(stInfo.m_comment_type);
            pListenIncomeReq->set_record_type(stInfo.m_record_type);
            g_server_processor_pay->send_datagram(stMsgRequest);
        }
    }
    //////

    //listen self, not need to check out_trade_no
    if(reply.m_result_code == ExecutorThreadRequestType::E_LISTEN_SELF)
    {
        fsm._follow_info.CopyFrom(reply.m_follow_info);
        fsm.reply_ok();
        fsm.set_state(state_end);
        return;
    }
    //query db fail
    if(reply.m_result_code != ExecutorThreadRequestType::E_OK)
    {
        LOG4CPLUS_ERROR(logger, "FsmFollowStateQueryDb::query_db_reply_event failed, recv result="<<reply.m_result_code);
        if(reply.m_result_code == ExecutorThreadRequestType::E_NO_PAY
           || reply.m_result_code == ExecutorThreadRequestType::E_NOT_EXIST)
        {
            fsm.reply_fail(::hoosho::msg::E_NO_PAY);
        }
        else
        {
            fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);
        }
        fsm.set_state(state_end);
        return;
    }

    // get listen detail
    if(hoosho::msg::J_GET_LISTEN_DETAIL_REQ == dwReqType)
    {
        LOG4CPLUS_DEBUG(logger, "get listen detail, check out_trade_no");
        //save async data
        fsm._listen_info.CopyFrom(reply.m_listen_info);
        fsm._follow_info.CopyFrom(reply.m_follow_info);

        fsm.reply_ok();
        fsm.set_state(FsmFollowState::state_end);
        return;
    }

    LOG4CPLUS_ERROR(logger, "unknow req_type = " << fsm._req_type);
    return;
}

void FsmFollowStateWaitQueryDb::timeout_event(FsmFollow& fsm, void* param)
{
    LOG4CPLUS_ERROR(logger, "FsmFollowStateQueryDb time_out."
                    <<" fsmid:"<<fsm._id
                    <<" connid:"<<fsm._conn_id);
    fsm.reply_timeout();
    fsm.set_state(FsmFollowState::state_end);
    return;
}


/*******************************************************************************************************/
/*                                        FsmFollowStateCheckOutTradeNoValid                                     */
/*******************************************************************************************************/
void FsmFollowStateWaitCheckOutTradeNoValid::enter(FsmFollow& fsm)
{
    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void FsmFollowStateWaitCheckOutTradeNoValid::exit(FsmFollow& fsm)
{
    fsm.cancel_timer();
}

void FsmFollowStateWaitCheckOutTradeNoValid::check_out_trade_no_valid_reply_event(FsmFollow &fsm, const hoosho::msg::Msg &stMsg)
{
    LOG4CPLUS_DEBUG(logger, "FsmFollowStateCheckOutTradeNoValid::check_out_trade_no_valid_reply_event");

    //cancel timer first
    fsm.cancel_timer();

    //pay server return err
    if(stMsg.head().result() != ::hoosho::msg::E_OK)
    {
        LOG4CPLUS_ERROR(logger, "FsmFollowStateCheckOutTradeNoValid::check_out_trade_no_valid_reply_event failed, recv msg.result="<<stMsg.head().result());
        fsm.reply_fail(::hoosho::msg::E_NO_PAY);
        fsm.set_state(state_end);
        return;
    }


    if(fsm._req_type == ::hoosho::msg::J_ADD_FOLLOW_REQ)
    {
        ExecutorThreadRequestElement request(ExecutorThreadRequestType::T_CHECK_OUT_TRADE_NO_UNIQUE);
        request.m_out_trade_no = fsm._follow_info.out_trade_no();
        request.m_fsm_id = fsm._id;
        request.need_reply();
        g_executor_thread_processor->send_request(request);
        fsm.set_state(state_wait_check_out_trade_no_unique);
        return;
    }

    if(fsm._req_type == ::hoosho::msg::J_ADD_LISTEN_REQ)
    {
        ExecutorThreadRequestElement request(ExecutorThreadRequestType::T_CHECK_OUT_TRADE_NO_UNIQUE);
        request.m_out_trade_no = fsm._listen_info.out_trade_no();
        request.m_fsm_id = fsm._id;
        request.need_reply();
        g_executor_thread_processor->send_request(request);
        fsm.set_state(state_wait_check_out_trade_no_unique);
        return;
    }

    if(fsm._req_type == ::hoosho::msg::J_GET_LISTEN_DETAIL_REQ)
    {
        fsm.reply_ok();
        fsm.set_state(state_end);
        return;
    }

    LOG4CPLUS_ERROR(logger, "unknow req_type = " << fsm._req_type);
    return;
}

void FsmFollowStateWaitCheckOutTradeNoValid::timeout_event(FsmFollow& fsm, void* param)
{
    LOG4CPLUS_ERROR(logger, "FsmFollowStateCheckOutTradeNoValid time_out."
                    <<" fsmid:"<<fsm._id
                    <<" connid:"<<fsm._conn_id);
    fsm.reply_timeout();
    fsm.set_state(FsmFollowState::state_end);
    return;
}

/*******************************************************************************************************/
/*                                         FsmFollowStateCheckOutTradeNoUnique          		                       */
/*******************************************************************************************************/
void FsmFollowStateWaitCheckOutTradeNoUnique::enter(FsmFollow& fsm)
{
    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void FsmFollowStateWaitCheckOutTradeNoUnique::exit(FsmFollow& fsm)
{
    fsm.cancel_timer();
}

void FsmFollowStateWaitCheckOutTradeNoUnique::check_out_trade_no_unique_reply_event(FsmFollow &fsm, const ExecutorThreadResponseElement &reply)
{
    fsm.cancel_timer();

    if(reply.m_result_code != ExecutorThreadRequestType::E_OK)
    {
        if(reply.m_result_code == ExecutorThreadRequestType::E_NO_PAY)
        {
            fsm.reply_fail(::hoosho::msg::E_NO_PAY);
        }
        else if(reply.m_result_code == ExecutorThreadRequestType::E_INVALID_PARAM)
        {
            fsm.reply_fail(::hoosho::msg::E_INVALID_REQ_PARAM);
        }
        else
        {
            fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);
        }
        fsm.set_state(state_end);
        return;
    }

    if(fsm._req_type == ::hoosho::msg::J_ADD_FOLLOW_REQ)
    {
        // conneect  transcode_server to download
        ::hoosho::msg::Msg stMsgRequest;
        ::hoosho::msg::MsgHead* pMsgRequestHead = stMsgRequest.mutable_head();
        pMsgRequestHead->set_cmd(::hoosho::msg::J_TRANSCODE_REQ);
        pMsgRequestHead->set_seq(fsm._id);
        ::hoosho::j::transcode::TranscodeReq* pTranscodeReq = stMsgRequest.mutable_transcode_req();
        pTranscodeReq->set_type(fsm._follow_info.content_type());
        pTranscodeReq->set_wx_media_id(fsm._follow_info.content_id());
        g_server_processor_transcode->send_datagram(stMsgRequest);
        fsm.set_state(state_wait_transcode);
        return;
    }

    if(fsm._req_type == ::hoosho::msg::J_ADD_LISTEN_REQ)
    {
        // save db
        ExecutorThreadRequestElement request(ExecutorThreadRequestType::T_ADD_LISTEN);
        request.m_listen_info.CopyFrom(fsm._listen_info);
        request.need_reply();
        request.m_fsm_id = fsm._id;
        g_executor_thread_processor->send_request(request);
        fsm.set_state(state_wait_insert_into_db);
        return;
    }

    LOG4CPLUS_ERROR(logger, "unknow req_type = " << fsm._req_type);
    return;
}


void FsmFollowStateWaitCheckOutTradeNoUnique::timeout_event(FsmFollow& fsm, void* param)
{
    LOG4CPLUS_ERROR(logger, "FsmFollowStateCheckOutTradeNoUnique time_out."
                    <<" fsmid:"<<fsm._id
                    <<" connid:"<<fsm._conn_id);
    fsm.reply_timeout();
    fsm.set_state(FsmFollowState::state_end);
    return;
}


/*******************************************************************************************************/
/*                                        FsmFollowStateTranscode                                    */
/*******************************************************************************************************/
void FsmFollowStateWaitTranscode::enter(FsmFollow& fsm)
{
    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void FsmFollowStateWaitTranscode::exit(FsmFollow& fsm)
{
    fsm.cancel_timer();
}

void FsmFollowStateWaitTranscode::transcode_reply_event(FsmFollow &fsm, const hoosho::msg::Msg &stMsg)
{
    //cancel timer first
    fsm.cancel_timer();

#ifdef LOCAL_TEST
	LOG4CPLUS_DEBUG(logger, "not care reply of transcode.");

#else
    //transcode server return err
    if(stMsg.head().result() != ::hoosho::msg::E_OK)
    {
        LOG4CPLUS_ERROR(logger, "FsmFollowStateTranscode::transcode_reply_event failed, recv msg.result="<<stMsg.head().result());
        fsm.reply_fail(stMsg.head().result());
        fsm.set_state(state_end);
        return;
    }
#endif


    if(fsm._req_type == ::hoosho::msg::J_ADD_FOLLOW_REQ)
    {
        //save id
        const ::hoosho::j::transcode::TranscodeRes& stTranscodeRes = stMsg.transcode_res();
        fsm._follow_info.set_content_id(stTranscodeRes.local_media_id());

        ExecutorThreadRequestElement request(ExecutorThreadRequestType::T_ADD_FOLLOW);
        request.m_follow_info.CopyFrom(fsm._follow_info);
        request.need_reply();
        request.m_fsm_id = fsm._id;
        g_executor_thread_processor->send_request(request);
        fsm.set_state(state_wait_insert_into_db);
        return;
    }

    LOG4CPLUS_ERROR(logger, "unknow req_type = " << fsm._req_type);
    return;
}

void FsmFollowStateWaitTranscode::timeout_event(FsmFollow& fsm, void* param)
{
    LOG4CPLUS_ERROR(logger, "FsmFollowStateTranscode time_out."
                    <<" fsmid:"<<fsm._id
                    <<" connid:"<<fsm._conn_id);
    fsm.reply_timeout();
    fsm.set_state(FsmFollowState::state_end);
    return;
}

/*******************************************************************************************************/
/*                                        FsmFollowStateInsertIntoDb                                    */
/*******************************************************************************************************/
void FsmFollowStateWaitInsertIntoDb::enter(FsmFollow& fsm)
{
    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void FsmFollowStateWaitInsertIntoDb::exit(FsmFollow& fsm)
{
    fsm.cancel_timer();
}

void FsmFollowStateWaitInsertIntoDb::insert_into_db_reply_event(FsmFollow &fsm, const ExecutorThreadResponseElement &element)
{
    //cancel timer first
    fsm.cancel_timer();

    //thread return not ok
    if(element.m_result_code != ExecutorThreadRequestType::E_OK)
    {
        LOG4CPLUS_ERROR(logger, "FsmFollowStateInsertIntoDb::insert_into_db_reply_event failed, recv thread.result="<<element.m_result_code);
        fsm.reply_fail(::hoosho::msg::E_SERVER_INNER_ERROR);
        fsm.set_state(state_end);
        return;
    }

    //return ok to client
    fsm.reply_ok();
    fsm.set_state(state_end);
    return;
}


void FsmFollowStateWaitInsertIntoDb::timeout_event(FsmFollow& fsm, void* param)
{
    LOG4CPLUS_ERROR(logger, "FsmFollowStateInsertIntoDb time_out."
                    <<" fsmid:"<<fsm._id
                    <<" connid:"<<fsm._conn_id);
    fsm.reply_timeout();
    fsm.set_state(FsmFollowState::state_end);
    return;
}


/*******************************************************************************************************/
/*                                        FsmFollowStateEnd                                             */
/*******************************************************************************************************/
void FsmFollowStateEnd::enter(FsmFollow& fsm)
{
    g_fsm_follow_container->del_fsm(&fsm);
}

