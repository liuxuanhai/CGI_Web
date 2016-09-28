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
	uint32_t index = request.m_sys_notice.m_pa_appid_md5 % m_size;

	if(m_queue_arr[index].push_request(request) != 0)
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadProcessor::send_request, push_request failed. index"<<index);
        return -1;
    }

    LOG4CPLUS_DEBUG(logger, "ExecutorThreadProcessor::send_request,"
                    <<", index="<<index
                    <<", requst=\n"<<request.ToString()<<"\n");

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
        case ExecutorThreadRequestType::T_SYS_NOTICE_ALL_QUERY:
        case ExecutorThreadRequestType::T_SYS_NOTICE_NEW_QUERY:
        case ExecutorThreadRequestType::T_NOTICE_RECORD_QUERY:
        case ExecutorThreadRequestType::T_NOTICE_NOTIFY_QUERY:
        case ExecutorThreadRequestType::T_NOTICE_NOTIFY_TYPE_LIST_QUERY:
        case ExecutorThreadRequestType::T_NOTICE_NOTIFY_SYS_NOTICE_QUERY:
            process_query_resp(reply);
            break;


		case ExecutorThreadRequestType::T_SYS_NOTICE_ADD:
		case ExecutorThreadRequestType::T_SYS_NOTICE_SELECT_UPDATE:

		case ExecutorThreadRequestType::T_NOTICE_RECORD_SELECT_UPDATE:
		case ExecutorThreadRequestType::T_NOTICE_RECORD_ADD:

			process_update_resp(reply);
			break;

		case ExecutorThreadRequestType::T_SYS_NOTICE_DELETE:
		case ExecutorThreadRequestType::T_NOTICE_RECORD_DELETE:
			process_delete_resp(reply);
			break;
			
        default:
        	LOG4CPLUS_ERROR(logger,
        						"ExecutorThreadProcessor get unknown type:\n" <<reply.m_request_type<<"("<<ExecutorThreadRequestType::type_string(reply.m_request_type)<<")");
            break;
    }

    return;
}

void ExecutorThreadProcessor::process_query_resp(ExecutorThreadResponseElement & reply)
{
	SysNoticeFsm* fsm = g_sys_notice_fsm_container->find_fsm(reply.m_fsm_id);
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
	SysNoticeFsm* fsm = g_sys_notice_fsm_container->find_fsm(reply.m_fsm_id);
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
	SysNoticeFsm* fsm = g_sys_notice_fsm_container->find_fsm(reply.m_fsm_id);
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

