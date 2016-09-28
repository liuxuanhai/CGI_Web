#include "executor_thread_processor.h"
#include "global_var.h"
#include <assert.h>

IMPL_LOGGER(ExecutorThreadProcessor, logger);

int ExecutorThreadProcessor::init(int size, int queue_capacity)
{
    m_size = size;
    m_queue_capacity = queue_capacity;
    m_queue_arr = new ExecutorThreadQueue[m_size];
    for(int i=0; i<m_size; i++)
    {
        assert(m_queue_arr[i].init(queue_capacity)==0);
    }

    return 0;
}

ExecutorThreadQueue* ExecutorThreadProcessor::get_queue(int n)
{
    if(n<0 || n>(m_size-1))
    {
        return NULL;
    }

    return &(m_queue_arr[n]);
}

int ExecutorThreadProcessor::get_size()
{
    return m_size;
}

int ExecutorThreadProcessor::send_request(ExecutorThreadRequestElement & request)
{
    //!!!!!!!!!!!!!!
    uint32_t index = request.m_fsm_id % m_size;

    if(m_queue_arr[index].push_request(request) != 0)
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::send_request, push_request failed. index"<<index);
        return -1;
    }

    LOG4CPLUS_DEBUG(logger, "ExecutorThreadProcessor::send_request"
                    <<", index="<<index
                    <<", requst="<<request.ToString());

    return 0;
}

int ExecutorThreadProcessor::poll()
{
    for(int i=0; i<m_size; i++)
    {
        ExecutorThreadQueue* q = &(m_queue_arr[i]);
        while(!(q->reply_empty()))
        {
            ExecutorThreadResponseElement& reply = q->get_reply();
            process(reply);
            q->pop_reply();
        }
    }

    return 0;
}

void ExecutorThreadProcessor::process(ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_DEBUG(logger, "ExecutorThreadProcessor::process, "<<reply.ToString());

    uint32_t type = reply.m_request_type;
    switch(type)
    {
        case ExecutorThreadRequestType::T_CHECK_OUT_TRADE_NO_UNIQUE:
            process_check_out_trade_no_unique(reply);
            break;

        case ExecutorThreadRequestType::T_GET_FEED_LIST:
            process_get_feed_list_resp(reply);
            break;

        case ExecutorThreadRequestType::T_GET_FEED_DETAIL:
            process_get_feed_detail_resp(reply);
            break;

        case ExecutorThreadRequestType::T_ADD_FEED:
            process_add_feed_resp(reply);
            break;

        case ExecutorThreadRequestType::T_GET_FOLLOW_LIST:
            process_get_follow_list_resp(reply);
            break;

        case ExecutorThreadRequestType::T_GET_FOLLOW_DETAIL:
            process_get_follow_detail_resp(reply);
            break;

        case ExecutorThreadRequestType::T_ADD_FOLLOW:
            process_add_follow_resp(reply);
            break;

        case ExecutorThreadRequestType::T_GET_LISTEN_LIST:
            process_get_listen_list_resp(reply);
            break;

        case ExecutorThreadRequestType::T_GET_LISTEN_DETAIL:
            process_get_listen_detail_resp(reply);
            break;

        case ExecutorThreadRequestType::T_ADD_LISTEN:
            process_add_listen_resp(reply);
            break;

        case ExecutorThreadRequestType::T_COMMENT_FOLLOW:
            process_comment_follow_resp(reply);
            break;

        case ExecutorThreadRequestType::T_GET_HISTORY_LIST:
            process_get_history_list_resp(reply);
            break;

        case ExecutorThreadRequestType::T_CHECK_LISTEN:
            process_check_listen_resp(reply);
            break;

        case ExecutorThreadRequestType::T_GET_COMMNET_FOLLOW:
            process_get_comment_follow_resp(reply);
            break;

        case ExecutorThreadRequestType::T_SET_OUT_TRADE_NO:
            process_set_out_trade_no_resp(reply);
            break;

        case ExecutorThreadRequestType::T_CHECK_BUSINESS_ID_VALID:
            process_check_business_id_resp(reply);
            break;

        default:
            break;
    }

    return;
}

void ExecutorThreadProcessor::process_check_out_trade_no_unique(ExecutorThreadResponseElement &reply)
{
    FsmFollow* fsm = g_fsm_follow_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->check_out_trade_no_unique_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_check_out_trade_no_unique failed, not found fsm_id="<<reply.m_fsm_id);
    }
}

void ExecutorThreadProcessor::process_get_feed_list_resp(ExecutorThreadResponseElement & reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_feed_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }

    return;
}

void ExecutorThreadProcessor::process_get_feed_detail_resp(ExecutorThreadResponseElement & reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_feed_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }

    return;
}

void ExecutorThreadProcessor::process_add_feed_resp(ExecutorThreadResponseElement & reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_add_feed_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }

    return;
}

void ExecutorThreadProcessor::process_get_follow_list_resp(ExecutorThreadResponseElement &reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_get_follow_list_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_get_follow_detail_resp(ExecutorThreadResponseElement &reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_get_follow_detail_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_add_follow_resp(ExecutorThreadResponseElement &reply)
{
    FsmFollow* fsm = g_fsm_follow_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->insert_into_db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_add_follow_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_get_listen_list_resp(ExecutorThreadResponseElement &reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_get_listen_list_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_get_listen_detail_resp(ExecutorThreadResponseElement &reply)
{
    FsmFollow* fsm = g_fsm_follow_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->query_db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_get_listen_detail_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_add_listen_resp(ExecutorThreadResponseElement &reply)
{
    FsmFollow* fsm = g_fsm_follow_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->insert_into_db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_add_listen_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_comment_follow_resp(ExecutorThreadResponseElement &reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_comment_follow failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_get_history_list_resp(ExecutorThreadResponseElement &reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_get_history_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_check_listen_resp(ExecutorThreadResponseElement &reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_check_listen_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_get_comment_follow_resp(ExecutorThreadResponseElement &reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_get_comment_follow_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_set_out_trade_no_resp(ExecutorThreadResponseElement &reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_set_out_trade_no_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}

void ExecutorThreadProcessor::process_check_business_id_resp(ExecutorThreadResponseElement &reply)
{
    FsmFeeds* fsm = g_fsm_feeds_container->find_fsm(reply.m_fsm_id);
    if(fsm)
    {
        fsm->db_reply_event(reply);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_check_business_id_resp failed, not found fsm_id="<<reply.m_fsm_id);
    }
    return;
}
