#include "executor_thread.h"
#include "global_var.h"
#include "dao.h"

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
				, stConfig.get_string_param("DB", "passwd")
				, stConfig.get_int_param("DB", "port")));
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
        ExecutorThreadResponseElement reply(request.m_request_type, request.m_fsm_id);

        switch(request.m_request_type)
        {					
            case ExecutorThreadRequestType::T_USER_INFO_QUERY:
				process_query(request, reply);
            	break;
			
			case ExecutorThreadRequestType::T_USER_INFO_UPDATE:
				process_update(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_INFO_DELETE:
				process_delete(request, reply);
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

void ExecutorThread::process_query(ExecutorThreadRequestElement & request, ExecutorThreadResponseElement & reply)
{
	std::string strErrMsg = "";
	DaoUserInfo daoUserInfo;
	daoUserInfo.m_id = request.m_user_info.m_id;
	int iRet = daoUserInfo.SelectFromDB(m_mysql_helper, strErrMsg);
	if(iRet == 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
	}
	else if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
	}
	else
	{
		reply.m_user_info = daoUserInfo;
	}
	
	return;
}

void ExecutorThread::process_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoUserInfo daoUserInfo = request.m_user_info;
	int iRet = daoUserInfo.UpdateToDB(m_mysql_helper, strErrMsg);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
	}
	
	return;	
}

void ExecutorThread::process_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoUserInfo daoUserInfo = request.m_user_info;
	int iRet = daoUserInfo.DeleteFromDB(m_mysql_helper, strErrMsg);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
	}
	
	return;	
}



