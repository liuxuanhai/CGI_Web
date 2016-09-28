#include "executor_thread.h"
#include "global_var.h"
#include "wx_api_oauth2.h"
#include "wx_api_ticket.h"
#include <cassert>

IMPL_LOGGER(ExecutorThread, logger)

ExecutorThread::ExecutorThread()
{
    m_queue = NULL;
}

ExecutorThread::~ExecutorThread()
{
}

int ExecutorThread::init(ExecutorThreadQueue * queue)
{
    m_queue = queue;
    const lce::app::Config& stConfig = g_server->config();
    assert(m_mysql_helper.Init(stConfig.get_string_param("DB", "ip")
                , stConfig.get_string_param("DB", "db_name")
                , stConfig.get_string_param("DB", "user")
                , stConfig.get_string_param("DB", "passwd")));
	return 0;
}

void ExecutorThread::run()
{
    LOG4CPLUS_TRACE(logger, "ExecutorThread "<<pthread_self()<<" running ...");

    while(true)
    {
        if(m_queue->request_empty())
        {
            usleep(10);
            continue;
        }

        ExecutorThreadRequestElement request = m_queue->get_request();
        m_queue->pop_request();
		
        LOG4CPLUS_TRACE(logger, "ExecutorThread get request, "<<request.ToString());
        ExecutorThreadResponseElement reply(request.m_request_type);

        switch(request.m_request_type)
        {					
            case ExecutorThreadRequestType::T_SAVE_HOOSHO_NO_ACCOUNT:
				process_save_hoosho_no_account(request, reply);
            	break;
			
            default:
                LOG4CPLUS_ERROR(logger, "ExecutorThread get unknown type:"
					           <<request.m_request_type<<"("<<ExecutorThreadRequestType::type_string(request.m_request_type)<<")");
                break;
        }

        if(!request.m_need_reply)//no need reply
        {
			continue;
        }

        while(m_queue->reply_full())
		{
			usleep(50);
		}
		
        m_queue->push_reply(reply);
    }
}

void ExecutorThread::process_save_hoosho_no_account(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	if(request.m_c_key != 1)
	{
		assert(false);
	}

	std::string strTableName = g_server->config().get_string_param("DB", "table_name");
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into "<<strTableName
        <<" set c_value="<<request.m_c_value
        <<" , c_key="<<request.m_c_key
        <<" on duplicate key update c_value="<<request.m_c_value;

    if(!m_mysql_helper.Query(ossSql.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query error, sql="<<ossSql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
        return;
    }

	LOG4CPLUS_DEBUG(logger, "process_save_hoosho_no_account succ, new c_value="<<request.m_c_value);

    return;
}

