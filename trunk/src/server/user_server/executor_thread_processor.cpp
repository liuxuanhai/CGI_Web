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
        case ExecutorThreadRequestType::T_USERINFO_QUERY:
            process_user_info_get_resp(reply);
            break;

		case ExecutorThreadRequestType::T_USERINFO_UPDATE:
			process_user_info_update_resp(reply);
			break;

		case ExecutorThreadRequestType::T_USERINFO_BATCH_QUERY:
			process_user_info_get_batch_resp(reply);
			break;

		case ExecutorThreadRequestType::T_USERFANS_LIST:
			process_user_fans_list_resp(reply);
			break;

		case ExecutorThreadRequestType::T_USERFANS_NUM:
			process_user_fans_num_resp(reply);
			break;

		case ExecutorThreadRequestType::T_USERFOLLOWS_LIST:
			process_user_follows_list_resp(reply);
			break;

		case ExecutorThreadRequestType::T_FOLLOW:
			process_follow_resp(reply);
			break;

		case ExecutorThreadRequestType::T_POWER_QUERY:
			process_user_power_get_resp(reply);
			break;

		case ExecutorThreadRequestType::T_ZOMBIE_QUERY:
			process_user_zombie_get_resp(reply);
			break;

		
		case ExecutorThreadRequestType::T_ZOMANAGER_QUERY:
			process_user_zomanager_get_resp(reply);
			break;

        default:
        	LOG4CPLUS_ERROR(logger, "unknown thread element type="<<type<<", ignore !!!");
            break;
    }

    return;
}

void ExecutorThreadProcessor::process_user_info_get_resp(ExecutorThreadResponseElement & reply)
{
	if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_PREAUTH)
	{
		FsmPreAuth* fsm = g_fsm_preauth_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->user_info_get_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_info_get_resp failed, FSM_TYPE_PREAUTH not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_info_get_resp failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}




void ExecutorThreadProcessor::process_user_info_update_resp(ExecutorThreadResponseElement& reply)
{
	if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_PREAUTH)
	{
		FsmPreAuth* fsm = g_fsm_preauth_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->user_info_update_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_info_update_resp failed, FSM_TYPE_PREAUTH not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	else if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_BUSINESS)
	{
		FsmBusiness* fsm = g_fsm_business_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->thread_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_info_update_resp failed, FSM_TYPE_BUSINESS not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_info_update_resp failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}

void ExecutorThreadProcessor::process_user_info_get_batch_resp(ExecutorThreadResponseElement& reply)
{
	if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_BUSINESS)
	{
		FsmBusiness* fsm = g_fsm_business_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->thread_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_info_get_batch_resp failed, FSM_TYPE_BUSINESS not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_info_get_batch_resp failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}

void ExecutorThreadProcessor::process_user_fans_list_resp(ExecutorThreadResponseElement& reply)
{
	if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_BUSINESS)
	{
		FsmBusiness* fsm = g_fsm_business_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->thread_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_fans_list_resp failed, FSM_TYPE_BUSINESS not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_fans_list_resp failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}

void ExecutorThreadProcessor::process_user_fans_num_resp(ExecutorThreadResponseElement& reply)
{
	if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_BUSINESS)
	{
		FsmBusiness* fsm = g_fsm_business_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->thread_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_fans_num_resp failed, FSM_TYPE_BUSINESS not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_fans_num_resp failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}

void ExecutorThreadProcessor::process_user_follows_list_resp(ExecutorThreadResponseElement& reply)
{
	if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_BUSINESS)
	{
		FsmBusiness* fsm = g_fsm_business_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->thread_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_follows_list_resp failed, FSM_TYPE_BUSINESS not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_follows_list_resp failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}

void ExecutorThreadProcessor::process_follow_resp(ExecutorThreadResponseElement& reply)
{
	if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_BUSINESS)
	{
		FsmBusiness* fsm = g_fsm_business_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->thread_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_follow_resp failed, FSM_TYPE_BUSINESS not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_follow_resp failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}

void ExecutorThreadProcessor::process_user_power_get_resp(ExecutorThreadResponseElement& reply)
{
	if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_BUSINESS)
	{
		FsmBusiness* fsm = g_fsm_business_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->thread_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_power_get_resp failed, FSM_TYPE_BUSINESS not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_power_get_resp failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}

void ExecutorThreadProcessor::process_user_zombie_get_resp(ExecutorThreadResponseElement& reply)
{
	if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_BUSINESS)
	{
		FsmBusiness* fsm = g_fsm_business_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->thread_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_zombie_get_resp failed, FSM_TYPE_BUSINESS not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_zombie_get_resp failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}

void ExecutorThreadProcessor::process_user_zomanager_get_resp(ExecutorThreadResponseElement& reply)
{
	if(reply.m_fsm_type == FsmContainer<int>::FSM_TYPE_BUSINESS)
	{
		FsmBusiness* fsm = g_fsm_business_container->find_fsm(reply.m_fsm_id);
		if(fsm)
		{
			fsm->thread_reply_event(reply);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_zomanager_get_resp failed, FSM_TYPE_BUSINESS not found,  fsm_id="<<reply.m_fsm_id);	
		}

		return;
	}
	
	LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::process_user_zomanager_get_resp failed, unknwon fsmtype="<<FsmContainer<int>::fsm_type_string(reply.m_fsm_type));
	return;
}



