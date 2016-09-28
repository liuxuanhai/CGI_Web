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
        case ExecutorThreadRequestType::T_USER_MARK_QUERY:
            process_query_resp(reply);
            break;

        case ExecutorThreadRequestType::T_USER_MARK_UPDATE:
        	process_update_resp(reply);
        	break;

		case ExecutorThreadRequestType::T_USER_MARK_ADD:
			process_add_resp(reply);
			break;
			
		case ExecutorThreadRequestType::T_USER_MARK_DELETE:
			process_delete_resp(reply);
			break;
			
        default:
            break;
    }

    return;
}

void ExecutorThreadProcessor::process_query_resp(ExecutorThreadResponseElement & reply)
{
	UserMarkFsm* fsm = g_user_mark_fsm_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_query_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_update_resp(ExecutorThreadResponseElement & reply)
{
	UserMarkFsm* fsm = g_user_mark_fsm_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_update_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_delete_resp(ExecutorThreadResponseElement & reply)
{
	UserMarkFsm* fsm = g_user_mark_fsm_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_delete_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}

void ExecutorThreadProcessor::process_add_resp(ExecutorThreadResponseElement & reply)
{
	UserMarkFsm* fsm = g_user_mark_fsm_container->find_fsm(reply.m_fsm_id);
	if(fsm)
	{
		fsm->db_reply_event(reply);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_add_resp failed, not found fsm_id="<<reply.m_fsm_id);	
	}
	
	return;
}


