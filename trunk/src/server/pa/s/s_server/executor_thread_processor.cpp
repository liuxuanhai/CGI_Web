#include "executor_thread_processor.h"
#include "global_var.h"
#include "util/lce_util.h"
#include <assert.h>

IMPL_LOGGER(ExecutorThreadProcessor, logger);

int ExecutorThreadProcessor::init(int size, int queue_capacity)
{
    m_size = size;
    m_queue_capacity = queue_capacity;
    
    m_queue_arr_user = new ExecutorThreadQueue[m_size];
    for(int i=0; i<m_size; i++)
    {
        assert(m_queue_arr_user[i].init(queue_capacity)==0);
    }

    m_queue_arr_feed = new ExecutorThreadQueue[m_size];
    for(int i=0; i<m_size; i++)
    {
        assert(m_queue_arr_feed[i].init(queue_capacity)==0);
    }

    return 0;
}

ExecutorThreadQueue* ExecutorThreadProcessor::get_user_queue(int n)
{
    if(n<0 || n>(m_size-1))
    {
        return NULL;
    }
	
    return &(m_queue_arr_user[n]);
}

ExecutorThreadQueue* ExecutorThreadProcessor::get_feed_queue(int n)
{
    if(n<0 || n>(m_size-1))
    {
        return NULL;
    }
	
    return &(m_queue_arr_feed[n]);
}


int ExecutorThreadProcessor::get_size()
{
    return m_size;
}

int ExecutorThreadProcessor::send_request(ExecutorThreadRequestElement & request)
{
	ExecutorThreadQueue* queue = NULL;
	uint64_t qwSelfUserid = request.m_self_user_id;
	
	uint64_t qwQueueIndex = qwSelfUserid % m_size;

	if(request.m_request_type < ExecutorThreadRequestType::_________T_I_AM_SPITOR_FOR_FEED_________)
	{
		//user thread
		if(ExecutorThreadRequestType::T_USER_LOGIN == request.m_request_type)
		{
			qwQueueIndex = 0;
		}
		
		queue = m_queue_arr_user + qwQueueIndex;
	}	
	else 
	{
		//feed thread
		queue = m_queue_arr_feed + qwQueueIndex;
	}

	if(!queue)
	{
		LOG4CPLUS_FATAL(logger, "ExecutorThreadProcessor::send_request faild, can not find a thread for request:"<<request.ToString());
		return -1;
	}

	if(queue->push_request(request) != 0)
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::send_request, push_request failed");
        return -1;
    }

    LOG4CPLUS_DEBUG(logger, "ExecutorThreadProcessor::send_request, request="<<request.ToString());
    return 0;
}

int ExecutorThreadProcessor::poll()
{
    for(int i=0; i<m_size; i++)
    {
        ExecutorThreadQueue* q = &(m_queue_arr_user[i]);
        while(!(q->reply_empty()))
        {
            ExecutorThreadResponseElement& reply = q->get_reply();
            process(reply);
            q->pop_reply();
        }

        q = &(m_queue_arr_feed[i]);
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

    if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_BUSINESS)
	{
		FsmBusiness* fsm = g_fsm_business_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->thread_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process failed, FSM_TYPE_BUSINESS not found,  fsm_id="<<reply.m_fsm_id);
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}

