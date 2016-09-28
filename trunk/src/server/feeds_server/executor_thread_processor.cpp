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

		
		case ExecutorThreadRequestType::T_FEED_DETAIL_QUERY:
			process_query_feed_detail_resp(reply);
			break;

        case ExecutorThreadRequestType::T_FEED_QUERY:
            process_query_feed_resp(reply);
            break;

		case ExecutorThreadRequestType::T_FEED_ADD:
			process_add_feed_resp(reply);
			break;

		case ExecutorThreadRequestType::T_FEED_DELETE:
			process_delete_feed_resp(reply);
			break;
			
		
		case ExecutorThreadRequestType::T_REPORT_FEED_ADD:
			process_add_report_feed_resp(reply);
			break;

		case ExecutorThreadRequestType::T_USER_FORBID_QUERY:
			process_query_user_forbid_resp(reply);
			break;
			
		
		case ExecutorThreadRequestType::T_FOLLOW_DETAIL_QUERY:
			process_query_follow_detail_resp(reply);
			break;

		case ExecutorThreadRequestType::T_FOLLOW_LIST_QUERY:
			process_query_follow_list_resp(reply);
			break;

		case ExecutorThreadRequestType::T_FOLLOW_COMMENT_DETAIL_QUERY:
			process_query_follow_comment_detail_resp(reply);
			break;
			
		case ExecutorThreadRequestType::T_FOLLOW_COMMENT_QUERY:
			process_query_follow_comment_resp(reply);
			break;
			
		case ExecutorThreadRequestType::T_FOLLOW_ADD:
			process_add_follow_resp(reply);
			break;
		case ExecutorThreadRequestType::T_FOLLOW_DELETE:
			process_delete_follow_resp(reply);
			break;

			
		case ExecutorThreadRequestType::T_FEED_LIST_FAVORITE_QUERY:
			process_query_feed_list_favorite_resp(reply);
			break;
		case ExecutorThreadRequestType::T_FAVORITE_QUERY:
			process_query_favorite_resp(reply);
			break;

		case ExecutorThreadRequestType::T_FAVORITE_ADD:
			process_add_favorite_resp(reply);
			break;
			
		case ExecutorThreadRequestType::T_FAVORITE_DELETE:
			process_delete_favorite_resp(reply);
			break;


		case ExecutorThreadRequestType::T_FEED_LIST_COLLECT_QUERY:
			process_query_feed_list_collect_resp(reply);
			break;
			
		case ExecutorThreadRequestType::T_COLLECT_QUERY:
			process_query_collect_resp(reply);
			break;

		case ExecutorThreadRequestType::T_COLLECT_ADD:
			process_add_collect_resp(reply);
			break;
			
		case ExecutorThreadRequestType::T_COLLECT_DELETE:
			process_delete_collect_resp(reply);
			break;
			
		
        default:
            break;
    }

    return;
}


void ExecutorThreadProcessor::process_query_feed_detail_resp(ExecutorThreadResponseElement & reply)
{
	FsmFeed* fsm = g_fsm_feed_container->find_fsm(reply.m_fsm_id);
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

void ExecutorThreadProcessor::process_query_feed_resp(ExecutorThreadResponseElement & reply)
{
	FsmFeed* fsm = g_fsm_feed_container->find_fsm(reply.m_fsm_id);
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
	FsmFeed* fsm = g_fsm_feed_container->find_fsm(reply.m_fsm_id);
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

void ExecutorThreadProcessor::process_delete_feed_resp(ExecutorThreadResponseElement & reply)
{
	FsmFeed* fsm = g_fsm_feed_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_delete_feed_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_add_report_feed_resp(ExecutorThreadResponseElement & reply)
{
	FsmFeed* fsm = g_fsm_feed_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_delete_feed_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_query_user_forbid_resp(ExecutorThreadResponseElement & reply)
{
	FsmFeed* fsm = g_fsm_feed_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_delete_feed_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}




void ExecutorThreadProcessor::process_query_follow_detail_resp(ExecutorThreadResponseElement & reply)
{
	FsmFollow* fsm = g_fsm_follow_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_follow_detail_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_query_follow_list_resp(ExecutorThreadResponseElement & reply)
{
	FsmFollow* fsm = g_fsm_follow_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_follow_list_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_query_follow_comment_detail_resp(ExecutorThreadResponseElement & reply)
{
	FsmFollow* fsm = g_fsm_follow_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_follow_comment_detail_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_query_follow_comment_resp(ExecutorThreadResponseElement & reply)
{
	FsmFollow* fsm = g_fsm_follow_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_follow_comment_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_add_follow_resp(ExecutorThreadResponseElement & reply)
{
	FsmFollow* fsm = g_fsm_follow_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_add_follow_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_delete_follow_resp(ExecutorThreadResponseElement & reply)
{
	FsmFollow* fsm = g_fsm_follow_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_delete_follow_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}


void ExecutorThreadProcessor::process_query_feed_list_favorite_resp(ExecutorThreadResponseElement & reply)
{
	FsmFavorite* fsm = g_fsm_favorite_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_feed_list_favorite_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_query_favorite_resp(ExecutorThreadResponseElement & reply)
{
	FsmFavorite* fsm = g_fsm_favorite_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_favorite_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_add_favorite_resp(ExecutorThreadResponseElement & reply)
{
	FsmFavorite* fsm = g_fsm_favorite_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_add_favorite_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_delete_favorite_resp(ExecutorThreadResponseElement & reply)
{
	FsmFavorite* fsm = g_fsm_favorite_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_delete_favorite_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}



void ExecutorThreadProcessor::process_query_feed_list_collect_resp(ExecutorThreadResponseElement & reply)
{
	FsmCollect* fsm = g_fsm_collect_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_feed_list_collect_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_query_collect_resp(ExecutorThreadResponseElement & reply)
{
	FsmCollect* fsm = g_fsm_collect_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_collect_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_add_collect_resp(ExecutorThreadResponseElement & reply)
{
	FsmCollect* fsm = g_fsm_collect_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_add_collect_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_delete_collect_resp(ExecutorThreadResponseElement & reply)
{
	FsmCollect* fsm = g_fsm_collect_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_delete_collect_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}




