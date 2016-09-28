#include "fsm_feeds_state.h"
#include "global_var.h"
#include "common_util.h"

/*******************************************************************************************************/
/*					                       FsmFeedsState					       					   */
/*******************************************************************************************************/
IMPL_LOGGER(FsmFeedsState, logger);

#define IMPL_FSM_STATE(classname, name) classname FsmFeedsState::name(#classname)
IMPL_FSM_STATE(FsmFeedsStateInit, state_init);
IMPL_FSM_STATE(FsmFeedsStateWaitQueryDB, state_wait_query_db);
IMPL_FSM_STATE(FsmFeedsStateWaitAddDB, state_wait_add_db);
IMPL_FSM_STATE(FsmFeedsStateWaitUpdateDB, state_wait_update_db);
IMPL_FSM_STATE(FsmFeedsStateWaitDeleteDB, state_wait_delete_db);
IMPL_FSM_STATE(FsmFeedsStateEnd, state_end);
#undef IMPL_FSM_STATE

void FsmFeedsState::enter(FsmFeeds& fsm)
{

}

void FsmFeedsState::exit(FsmFeeds& fsm)
{

}

void FsmFeedsState::client_req_event(FsmFeeds& fsm,  lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
    LOG4CPLUS_DEBUG(logger, "default  client_req_event, state: "<<this->name());
}

void FsmFeedsState::db_reply_event(FsmFeeds& fsm, const ExecutorThreadResponseElement& element)
{
    LOG4CPLUS_DEBUG(logger, "default  db_reply_event, state: "<<this->name());
}

void FsmFeedsState::timeout_event(FsmFeeds& fsm, void* param)
{
    LOG4CPLUS_DEBUG(logger, "default  timeout_event, state: "<<this->name());
}

/*******************************************************************************************************/
/*                                         FsmFeedsStateInit                                           */
/*******************************************************************************************************/
void FsmFeedsStateInit::client_req_event(FsmFeeds& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg)
{
    //save async req param to fsm
    fsm._conn_id = conn.get_id();
    fsm._req_type = stMsg.head().cmd();
    fsm._req_seq_id = stMsg.head().seq();

    //dispatch
    uint32_t dwReqType = stMsg.head().cmd();

    //get feed list
    if(hoosho::msg::J_GET_FEED_LIST_REQ == dwReqType)
    {
        const hoosho::j::feeds::GetFeedListReq& stGetFeedListReq = stMsg.j_feeds_get_feed_list_req();

        //query db
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_GET_FEED_LIST);
        req.m_fsm_id = fsm._id;
        req.m_openid = stGetFeedListReq.openid();
        req.m_begin_feed_id = stGetFeedListReq.begin_feed_id();
        req.m_limit = stGetFeedListReq.limit();
        req.m_scene_type = stGetFeedListReq.scene_type();
        req.m_need_sex = stGetFeedListReq.need_sex();
        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_query_db);
        return;
    }
    //get feed detail
    if(hoosho::msg::J_GET_FEED_DETAIL_REQ == dwReqType)
    {
        const hoosho::j::feeds::GetFeedDetailReq& stGetFeedDetailReq = stMsg.j_feeds_get_feed_detail_req();
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_GET_FEED_DETAIL);
        req.m_fsm_id = fsm._id;
        req.m_feed_id_list.clear();
        for(int i = 0; i < stGetFeedDetailReq.feed_id_list_size(); i++)
        {
            req.m_feed_id_list.push_back( stGetFeedDetailReq.feed_id_list(i) );
        }
        req.m_openid = stGetFeedDetailReq.openid();
        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_query_db);
        return;
    }
    //add feed
    if(hoosho::msg::J_ADD_FEED_REQ == dwReqType)
    {
        const hoosho::j::feeds::AddFeedReq& stAddFeedReq = stMsg.j_feeds_add_feed_req();

        //save async data
        //fsm._feed_info.CopyFrom(stAddFeedReq.feed_info());

        //add to db
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_ADD_FEED);
        req.m_fsm_id = fsm._id;
        req.m_feed_info.CopyFrom(stAddFeedReq.feed_info());

        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_add_db);
        return;
    }

    //get follow list
    if(hoosho::msg::J_GET_FOLLOW_LIST_REQ == dwReqType)
    {
        const hoosho::j::feeds::GetFollowListReq& stGetFollowListReq = stMsg.j_feeds_get_follow_list_req();
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_GET_FOLLOW_LIST);
        req.m_fsm_id = fsm._id;
        req.m_openid = stGetFollowListReq.openid();
        req.m_scene_type = stGetFollowListReq.scene_type();
        req.m_feed_id = stGetFollowListReq.feed_id();
        req.m_begin_follow_id = stGetFollowListReq.begin_follow_id();
        req.m_limit = stGetFollowListReq.limit();

        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_query_db);
        return;
    }
    //get follow detail
    if(hoosho::msg::J_GET_FOLLOW_DETAIL_REQ == dwReqType)
    {
        const hoosho::j::feeds::GetFollowDetailReq& stGetFollowDetailReq = stMsg.j_feeds_get_follow_detail_req();
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_GET_FOLLOW_DETAIL);
        req.m_fsm_id = fsm._id;
        req.m_follow_id_list.clear();
        for(int i = 0; i < stGetFollowDetailReq.follow_id_list_size(); i++)
        {
            req.m_follow_id_list.push_back( stGetFollowDetailReq.follow_id_list(i) );
        }
        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_query_db);
        return;
    }
    //add follow
//    if(hoosho::msg::J_ADD_FOLLOW_REQ == dwReqType)
//    {
//        const hoosho::j::feeds::AddFollowReq& stAddFollowReq = stMsg.j_feeds_add_follow_req();
//
//        //save async data
////        fsm._follow_info.CopyFrom(stAddFollowReq.follow_info());
//
//        //add to db
//        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_ADD_FOLLOW);
//        req.m_fsm_id = fsm._id;
//        req.m_follow_info.CopyFrom(stAddFollowReq.follow_info());
//
//        req.need_reply();
//        g_executor_thread_processor->send_request(req);
//        fsm.set_state(FsmFeedsState::state_wait_add_db);
//        return;
//    }

    //get listen list
    if(hoosho::msg::J_GET_LISTEN_LIST_REQ == dwReqType)
    {
        const hoosho::j::feeds::GetListenListReq& stGetListenListReq = stMsg.j_feeds_get_listen_list_req();
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_GET_LISTEN_LIST);
        req.m_fsm_id = fsm._id;
        req.m_openid = stGetListenListReq.openid();
        req.m_scene_type = stGetListenListReq.scene_type();
        req.m_feed_id = stGetListenListReq.feed_id();
        req.m_begin_ts = stGetListenListReq.begin_ts();
        req.m_limit = stGetListenListReq.limit();

        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_query_db);
        return;
    }
    //get listen detail
//    if(hoosho::msg::J_GET_LISTEN_DETAIL_REQ == dwReqType)
//    {
//        const hoosho::j::feeds::GetListenDetailReq& stGetListenDetailReq = stMsg.j_feeds_get_listen_detail_req();
//        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_GET_LISTEN_DETAIL);
//        req.m_fsm_id = fsm._id;
//        req.m_openid = stGetListenDetailReq.openid();
//        req.m_follow_id = stGetListenDetailReq.follow_id();
//        req.m_out_trade_no = stGetListenDetailReq.out_trade_no();
//
//        req.need_reply();
//        g_executor_thread_processor->send_request(req);
//        fsm.set_state(FsmFeedsState::state_wait_query_db);
//        return;
//    }
    //add listen
//    if(hoosho::msg::J_ADD_LISTEN_REQ == dwReqType)
//    {
//        const hoosho::j::feeds::AddListenReq& stAddListenReq = stMsg.j_feeds_add_listen_req();
//
//        //save async data
////        fsm._listen_info.CopyFrom(stAddListenReq.listen_info());
//
//        //add to db
//        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_ADD_LISTEN);
//        req.m_fsm_id = fsm._id;
//        req.m_listen_info.CopyFrom(stAddListenReq.listen_info());
//
//        req.need_reply();
//        g_executor_thread_processor->send_request(req);
//        fsm.set_state(FsmFeedsState::state_wait_add_db);
//        return;
//    }
    //comment follow
    if(hoosho::msg::J_COMMENT_FOLLOW_REQ == dwReqType)
    {
        const hoosho::j::feeds::CommentFollowReq& stCommentFollowReq = stMsg.j_feeds_comment_follow_req();

        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_COMMENT_FOLLOW);
        req.m_fsm_id = fsm._id;

        req.m_comment_follow_info.CopyFrom(stCommentFollowReq.comment_follow_info());

        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_add_db);
        return;
    }

    //get history list
    if(hoosho::msg::J_GET_HISTORY_LIST_REQ == dwReqType)
    {
        const hoosho::j::feeds::GetHistoryListReq& stGetHistoryListReq = stMsg.j_feeds_get_history_list_req();
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_GET_HISTORY_LIST);
        req.m_fsm_id = fsm._id;
        req.m_openid = stGetHistoryListReq.openid();
        req.m_begin_ts = stGetHistoryListReq.begin_ts();
        req.m_limit = stGetHistoryListReq.limit();

        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_query_db);
        return;
    }

    //check listen
    if(hoosho::msg::J_CHECK_LISTEN_REQ == dwReqType)
    {
        const hoosho::j::feeds::CheckListenReq& stCheckListenReq = stMsg.j_feeds_check_listen_req();
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_CHECK_LISTEN);
        req.m_fsm_id = fsm._id;
        req.m_check_listen_list.clear();
        for(int i = 0; i < stCheckListenReq.check_listen_list_size(); i++)
        {
            req.m_check_listen_list.push_back(stCheckListenReq.check_listen_list(i));
        }

        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_query_db);
        return;
    }

    //get comment follow
    if(hoosho::msg::J_GET_COMMENT_FOLLOW_REQ == dwReqType)
    {
        const hoosho::j::feeds::GetCommentFollowReq& stGetCommentFollowReq = stMsg.j_feeds_get_comment_follow_req();
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_GET_COMMNET_FOLLOW);
        req.m_fsm_id = fsm._id;
        req.m_comment_follow_list.clear();
        for(int i = 0; i < stGetCommentFollowReq.comment_follow_list_size(); i++)
        {
            req.m_comment_follow_list.push_back(stGetCommentFollowReq.comment_follow_list(i));
        }

        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_query_db);
        return;
    }

    if(hoosho::msg::J_SET_OUT_TRADE_NO_REQ == dwReqType)
    {
        const hoosho::j::feeds::SetOutTradeNoReq& stSetOutTradeNoReq = stMsg.j_feeds_set_out_trade_no_req();
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_SET_OUT_TRADE_NO);
        req.m_fsm_id = fsm._id;
        req.m_business_id = stSetOutTradeNoReq.business_id();
        req.m_business_type = stSetOutTradeNoReq.business_type();
        req.m_out_trade_no = stSetOutTradeNoReq.out_trade_no();

        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_add_db);
        return;
    }

    if(hoosho::msg::J_CHECK_BUSINESS_ID_VALID_REQ == dwReqType)
    {
        const hoosho::j::feeds::CheckBusinessIdValidReq& stCheckBusinessIdValidReq = stMsg.j_feeds_check_business_id_valid_req();
        ExecutorThreadRequestElement req(ExecutorThreadRequestType::T_CHECK_BUSINESS_ID_VALID);
        req.m_fsm_id = fsm._id;
        req.m_business_id = stCheckBusinessIdValidReq.business_id();
        req.m_business_type = stCheckBusinessIdValidReq.business_type();

        req.need_reply();
        g_executor_thread_processor->send_request(req);
        fsm.set_state(FsmFeedsState::state_wait_query_db);
        return;
    }

    LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
    return;

}


/*******************************************************************************************************/
/*                                        FsmFeedsStateWaitQueryDB                                     */
/*******************************************************************************************************/
void FsmFeedsStateWaitQueryDB::enter(FsmFeeds& fsm)
{
    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void FsmFeedsStateWaitQueryDB::exit(FsmFeeds& fsm)
{
    fsm.cancel_timer();
}

void FsmFeedsStateWaitQueryDB::db_reply_event(FsmFeeds& fsm, const ExecutorThreadResponseElement& reply)
{
    //cancel timer first
    fsm.cancel_timer();

    //resp msg
    hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(fsm._req_seq_id);

    uint32_t dwReqType = fsm._req_type;
    uint32_t dwStatus = reply.m_result_code;

    const vector<hoosho::j::commstruct::FeedInfo>& stFeedList = reply.m_feed_list;
    const vector<hoosho::j::commstruct::FollowInfo>& stFollowList = reply.m_follow_list;

    if(hoosho::msg::J_GET_FEED_LIST_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_GET_FEED_LIST_RES);
        if(ExecutorThreadRequestType::E_OK == dwStatus)
        {

            hoosho::j::feeds::GetFeedListRes* pBody = stRespMsg.mutable_j_feeds_get_feed_list_res();
            LOG4CPLUS_TRACE(logger, "list_size = " << stFeedList.size());

            for(size_t i = 0; i < stFeedList.size(); i++)
            {
                LOG4CPLUS_TRACE(logger, "info = " << stFeedList[i].Utf8DebugString());
                hoosho::j::commstruct::FeedInfo* pBodyFeed = pBody->add_feed_list();
                pBodyFeed->CopyFrom(stFeedList[i]);
            }
            header->set_result(hoosho::msg::E_OK);
        }

        //when not data found,  still return ok
        else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
        {
            header->set_result(hoosho::msg::E_OK);
        }

        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }
    if(hoosho::msg::J_GET_FEED_DETAIL_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_GET_FEED_DETAIL_RES);
        if(ExecutorThreadRequestType::E_OK == dwStatus)
        {

            hoosho::j::feeds::GetFeedDetailRes* pBody = stRespMsg.mutable_j_feeds_get_feed_detail_res();
            LOG4CPLUS_TRACE(logger, "list_size = " << stFeedList.size());

            for(size_t i = 0; i < stFeedList.size(); i++)
            {
                LOG4CPLUS_TRACE(logger, "info = " << stFeedList[i].Utf8DebugString());

                hoosho::j::commstruct::FeedInfo* pBodyFeed = pBody->add_feed_list();
                pBodyFeed->CopyFrom(stFeedList[i]);
            }
            header->set_result(hoosho::msg::E_OK);
        }
        else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
        {
            header->set_result(hoosho::msg::E_OK);
        }
        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);

        return;
    }


    if(hoosho::msg::J_GET_FOLLOW_LIST_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_GET_FOLLOW_LIST_RES);
        if(ExecutorThreadRequestType::E_OK == dwStatus)
        {

            hoosho::j::feeds::GetFollowListRes* pBody = stRespMsg.mutable_j_feeds_get_follow_list_res();
            LOG4CPLUS_TRACE(logger, "list_size = " << stFollowList.size());

            for(size_t i = 0; i < stFollowList.size(); i++)
            {
                LOG4CPLUS_TRACE(logger, "info = " << stFollowList[i].Utf8DebugString());
                hoosho::j::commstruct::FollowInfo* pBodyFollow = pBody->add_follow_list();
                pBodyFollow->CopyFrom(stFollowList[i]);
            }
            header->set_result(hoosho::msg::E_OK);
        }

        //when not data found,  still return ok
        else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
        {
            header->set_result(hoosho::msg::E_OK);
        }

        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }
    if(hoosho::msg::J_GET_FOLLOW_DETAIL_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_GET_FOLLOW_DETAIL_RES);
                if(ExecutorThreadRequestType::E_OK == dwStatus)
        {

            hoosho::j::feeds::GetFollowDetailRes* pBody = stRespMsg.mutable_j_feeds_get_follow_detail_res();
            LOG4CPLUS_TRACE(logger, "list_size = " << stFollowList.size());

            for(size_t i = 0; i < stFollowList.size(); i++)
            {
                LOG4CPLUS_TRACE(logger, "info = " << stFollowList[i].Utf8DebugString());
                hoosho::j::commstruct::FollowInfo* pBodyFollow = pBody->add_follow_list();
                pBodyFollow->CopyFrom(stFollowList[i]);
            }
            header->set_result(hoosho::msg::E_OK);
        }

        //when not data found,  still return ok
        else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
        {
            header->set_result(hoosho::msg::E_OK);
        }

        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }

    if(hoosho::msg::J_GET_LISTEN_LIST_REQ == dwReqType)
    {
        //notify pay_server
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
        //////


        header->set_cmd(hoosho::msg::J_GET_LISTEN_LIST_RES);
        if(ExecutorThreadRequestType::E_OK == dwStatus)
        {

            hoosho::j::feeds::GetListenListRes* pBody = stRespMsg.mutable_j_feeds_get_listen_list_res();
            LOG4CPLUS_TRACE(logger, "list_size = " << stFollowList.size());

            for(size_t i = 0; i < stFollowList.size(); i++)
            {
                LOG4CPLUS_TRACE(logger, "info = " << stFollowList[i].Utf8DebugString());
                hoosho::j::commstruct::FollowInfo* pBodyFollow = pBody->add_follow_list();
                pBodyFollow->CopyFrom(stFollowList[i]);
            }
            header->set_result(hoosho::msg::E_OK);
        }

        //when not data found,  still return ok
        else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
        {
            header->set_result(hoosho::msg::E_OK);
        }

        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }
//    if(hoosho::msg::J_GET_LISTEN_DETAIL_REQ == dwReqType)
//    {
//        header->set_cmd(hoosho::msg::J_GET_LISTEN_DETAIL_RES);
//        if(ExecutorThreadRequestType::E_OK == dwStatus)
//        {
//
//            hoosho::j::feeds::GetListenDetailRes* pBody = stRespMsg.mutable_j_feeds_get_listen_detail_res();
//            hoosho::j::commstruct::ListenInfo* pListenInfo = pBody->mutable_listen_info();
//            hoosho::j::commstruct::FollowInfo* pFollowInfo = pBody->mutable_follow_info();
//            pListenInfo->CopyFrom(reply.m_listen_info);
//            pFollowInfo->CopyFrom(reply.m_follow_info);
//
//            header->set_result(hoosho::msg::E_OK);
//        }
//
//        //when not data found,  still return ok
//        else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
//        {
//            header->set_result(hoosho::msg::E_OK);
//        }
//
//        else
//        {
//            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
//        }
//
//        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
//        fsm.set_state(FsmFeedsState::state_end);
//        return;
//    }

    if(hoosho::msg::J_GET_HISTORY_LIST_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_GET_HISTORY_LIST_RES);
        if(ExecutorThreadRequestType::E_OK == dwStatus)
        {

            hoosho::j::feeds::GetHistoryListRes* pBody = stRespMsg.mutable_j_feeds_get_history_list_res();
            LOG4CPLUS_TRACE(logger, "list_size = " << stFeedList.size());

            for(size_t i = 0; i < stFeedList.size(); i++)
            {
                LOG4CPLUS_TRACE(logger, "info = " << stFeedList[i].Utf8DebugString());
                hoosho::j::commstruct::FeedInfo* pBodyFeed = pBody->add_feed_list();
                pBodyFeed->CopyFrom(stFeedList[i]);
            }
            header->set_result(hoosho::msg::E_OK);
        }

        //when not data found,  still return ok
        else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
        {
            header->set_result(hoosho::msg::E_OK);
        }

        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }

    const std::vector<hoosho::j::commstruct::CheckListenInfo>& stCheckListenList = reply.m_check_listen_list;
    if(hoosho::msg::J_CHECK_LISTEN_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_CHECK_LISTEN_RES);
        if(ExecutorThreadRequestType::E_OK == dwStatus)
        {

            hoosho::j::feeds::CheckListenRes* pBody = stRespMsg.mutable_j_feeds_check_listen_res();
            LOG4CPLUS_TRACE(logger, "list_size = " << stCheckListenList.size());

            for(size_t i = 0; i < stCheckListenList.size(); i++)
            {
                LOG4CPLUS_TRACE(logger, "info = " << stCheckListenList[i].Utf8DebugString());
                hoosho::j::commstruct::CheckListenInfo* pBodyCheckListen = pBody->add_check_listen_list();
                pBodyCheckListen->CopyFrom(stCheckListenList[i]);
            }
            header->set_result(hoosho::msg::E_OK);
        }

        //when not data found,  still return ok
        else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
        {
            header->set_result(hoosho::msg::E_OK);
        }

        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }

    const std::vector<hoosho::j::commstruct::CommentFollowInfo>& stCommentFollowList = reply.m_comment_follow_list;
    if(hoosho::msg::J_GET_COMMENT_FOLLOW_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_GET_COMMENT_FOLLOW_RES);
        if(ExecutorThreadRequestType::E_OK == dwStatus)
        {

            hoosho::j::feeds::GetCommentFollowRes* pBody = stRespMsg.mutable_j_feeds_get_comment_follow_res();
            LOG4CPLUS_TRACE(logger, "list_size = " << stCommentFollowList.size());

            for(size_t i = 0; i < stCommentFollowList.size(); i++)
            {
                LOG4CPLUS_TRACE(logger, "info = " << stCommentFollowList[i].Utf8DebugString());
                hoosho::j::commstruct::CommentFollowInfo* pBodyCommentFollow = pBody->add_comment_follow_list();
                pBodyCommentFollow->CopyFrom(stCommentFollowList[i]);
            }
            header->set_result(hoosho::msg::E_OK);
        }

        //when not data found,  still return ok
        else if(ExecutorThreadRequestType::E_NOT_EXIST== dwStatus)
        {
            header->set_result(hoosho::msg::E_OK);
        }

        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }

        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }

    if(hoosho::msg::J_CHECK_BUSINESS_ID_VALID_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_CHECK_BUSINESS_ID_VALID_RES);
        if(ExecutorThreadRequestType::E_OK == dwStatus)
        {
            header->set_result(hoosho::msg::E_OK);
        }
		else if(ExecutorThreadRequestType::E_INVALID_PARAM == dwStatus)
        {
            header->set_result(hoosho::msg::E_INVALID_REQ_PARAM);
        }
        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }
        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }


    LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
    return;

}

void FsmFeedsStateWaitQueryDB::timeout_event(FsmFeeds& fsm, void* param)
{
    LOG4CPLUS_ERROR(logger, "FsmFeedsStateWaitQueryDB time_out."
                    <<" fsmid:"<<fsm._id
                    <<" connid:"<<fsm._conn_id);
    fsm.reply_timeout();
    fsm.set_state(FsmFeedsState::state_end);
    return;
}

/*******************************************************************************************************/
/*                                         FsmFeedsStateWaitAddDB          		                       */
/*******************************************************************************************************/
void FsmFeedsStateWaitAddDB::enter(FsmFeeds& fsm)
{
    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void FsmFeedsStateWaitAddDB::exit(FsmFeeds& fsm)
{
    fsm.cancel_timer();
}

void FsmFeedsStateWaitAddDB::db_reply_event(FsmFeeds& fsm, const ExecutorThreadResponseElement& reply)
{
    //cancel timer first
    fsm.cancel_timer();

    uint32_t dwReqType = fsm._req_type;
    uint32_t dwStatus = reply.m_result_code;

    //resp msg
    hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_seq(fsm._req_seq_id);


    if(hoosho::msg::J_ADD_FEED_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_ADD_FEED_RES);

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
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }

//    if(hoosho::msg::J_ADD_FOLLOW_REQ == dwReqType)
//    {
//        header->set_cmd(hoosho::msg::J_ADD_FOLLOW_RES);
//
//        if(dwStatus == ExecutorThreadRequestType::E_OK)
//        {
//            header->set_result(hoosho::msg::E_OK);
//
//        }
//        else if(dwStatus == ExecutorThreadRequestType::E_INVALID_PARAM)
//        {
//            header->set_result(hoosho::msg::E_INVALID_REQ_PARAM);
//        }
//        else if(dwStatus == ExecutorThreadRequestType::E_NO_PAY)
//        {
//            header->set_result(hoosho::msg::E_NO_PAY);
//        }
//        else
//        {
//            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
//        }
//
//        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
//        fsm.set_state(FsmFeedsState::state_end);
//        return;
//    }

//    if(hoosho::msg::J_ADD_LISTEN_REQ == dwReqType)
//    {
//        header->set_cmd(hoosho::msg::J_ADD_LISTEN_RES);
//
//        if(dwStatus == ExecutorThreadRequestType::E_OK)
//        {
//            header->set_result(hoosho::msg::E_OK);
//
//        }
//        else if(dwStatus == ExecutorThreadRequestType::E_INVALID_PARAM)
//        {
//            header->set_result(hoosho::msg::E_INVALID_REQ_PARAM);
//        }
//        else if(dwStatus == ExecutorThreadRequestType::E_NO_PAY)
//        {
//            header->set_result(hoosho::msg::E_NO_PAY);
//        }
//        else
//        {
//            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
//        }
//
//        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
//        fsm.set_state(FsmFeedsState::state_end);
//        return;
//    }

    if(hoosho::msg::J_COMMENT_FOLLOW_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_COMMENT_FOLLOW_RES);

        if(dwStatus == ExecutorThreadRequestType::E_OK)
        {
            header->set_result(hoosho::msg::E_OK);

            //notify pay_server
            ::hoosho::msg::Msg stMsgRequest;
            ::hoosho::msg::MsgHead* pMsgRequestHead = stMsgRequest.mutable_head();
            pMsgRequestHead->set_cmd(::hoosho::msg::J_PAY_LISTEN_INCOME_REQ);
            pMsgRequestHead->set_seq(fsm._id);
            ::hoosho::j::pay::ListenIncomeReq* pListenIncomeReq = stMsgRequest.mutable_listen_income_req();
            pListenIncomeReq->set_openid(reply.m_openid);
            pListenIncomeReq->set_out_trade_no(reply.m_out_trade_no);
            pListenIncomeReq->set_out_trade_openid(reply.m_out_trade_openid);
            pListenIncomeReq->set_share_type(reply.m_comment_follow_info.comment_type());
            pListenIncomeReq->set_record_type(1);
            g_server_processor_pay->send_datagram(stMsgRequest);
        }
        else if(dwStatus == ExecutorThreadRequestType::E_INVALID_PARAM)
        {
            header->set_result(hoosho::msg::E_INVALID_REQ_PARAM);
        }
        else if(dwStatus == ExecutorThreadRequestType::E_NO_PAY)
        {
            header->set_result(hoosho::msg::E_NO_PAY);
        }
        else if(dwStatus == ExecutorThreadRequestType::E_FOLLOW_HAS_COMMENT)
        {
            header->set_result(hoosho::msg::E_FOLLOW_HAS_COMMENT);
        }
        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }
        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }


    if(hoosho::msg::J_SET_OUT_TRADE_NO_REQ == dwReqType)
    {
        header->set_cmd(hoosho::msg::J_SET_OUT_TRADE_NO_RES);
        if(ExecutorThreadRequestType::E_OK == dwStatus)
        {
            header->set_result(hoosho::msg::E_OK);
        }
        else if(ExecutorThreadRequestType::E_INVALID_PARAM == dwStatus)
        {
            header->set_result(hoosho::msg::E_INVALID_REQ_PARAM);
        }
        else
        {
            header->set_result(hoosho::msg::E_SERVER_INNER_ERROR);
        }
        g_client_processor->send_datagram(fsm._conn_id, stRespMsg);
        fsm.set_state(FsmFeedsState::state_end);
        return;
    }


    LOG4CPLUS_ERROR(logger, "unknwo req_type = " << dwReqType);
    return;

}

void FsmFeedsStateWaitAddDB::timeout_event(FsmFeeds& fsm, void* param)
{
    LOG4CPLUS_ERROR(logger, "FsmFeedsStateWaitAddDB time_out."
                    <<" fsmid:"<<fsm._id
                    <<" connid:"<<fsm._conn_id);
    fsm.reply_timeout();
    fsm.set_state(FsmFeedsState::state_end);
    return;
}


/*******************************************************************************************************/
/*                                        FsmFeedsStateWaitUpdateDB                                    */
/*******************************************************************************************************/
void FsmFeedsStateWaitUpdateDB::enter(FsmFeeds& fsm)
{
    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void FsmFeedsStateWaitUpdateDB::exit(FsmFeeds& fsm)
{
    fsm.cancel_timer();
}

void FsmFeedsStateWaitUpdateDB::db_reply_event(FsmFeeds& fsm, const ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_ERROR(logger, "not code");
    return;
}

void FsmFeedsStateWaitUpdateDB::timeout_event(FsmFeeds& fsm, void* param)
{
    LOG4CPLUS_ERROR(logger, "FsmFeedsStateWaitUpdateDB time_out."
                    <<" fsmid:"<<fsm._id
                    <<" connid:"<<fsm._conn_id);
    fsm.reply_timeout();
    fsm.set_state(FsmFeedsState::state_end);
    return;
}

/*******************************************************************************************************/
/*                                        FsmFeedsStateWaitDeleteDB                                    */
/*******************************************************************************************************/
void FsmFeedsStateWaitDeleteDB::enter(FsmFeeds& fsm)
{
    fsm.reset_timer(g_server->config().get_int_param("FSM_CONTAINER", "timeout"));
}

void FsmFeedsStateWaitDeleteDB::exit(FsmFeeds& fsm)
{
    fsm.cancel_timer();
}

void FsmFeedsStateWaitDeleteDB::db_reply_event(FsmFeeds& fsm, const ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_ERROR(logger, "not code");
    return;
}

void FsmFeedsStateWaitDeleteDB::timeout_event(FsmFeeds& fsm, void* param)
{
    LOG4CPLUS_ERROR(logger, "FsmFeedsStateWaitDeleteDB time_out."
                    <<" fsmid:"<<fsm._id
                    <<" connid:"<<fsm._conn_id);
    fsm.reply_timeout();
    fsm.set_state(FsmFeedsState::state_end);
    return;
}


/*******************************************************************************************************/
/*                                        FsmFeedsStateEnd                                             */
/*******************************************************************************************************/
void FsmFeedsStateEnd::enter(FsmFeeds& fsm)
{
    g_fsm_feeds_container->del_fsm(&fsm);
}

