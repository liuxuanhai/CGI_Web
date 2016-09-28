#include "executor_thread.h"
#include "global_var.h"
#include "common_util.h"

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
            case ExecutorThreadRequestType::T_USER_MARK_QUERY:
				process_query(request, reply);
            	break;

			case ExecutorThreadRequestType::T_USER_MARK_ADD:
				process_add(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_MARK_UPDATE:
				process_update(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_MARK_DELETE:
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
	
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"SELECT * FROM t_user_mark_appid"
		<<" WHERE openid_md5 = "<<request.m_openid_md5;
	
	if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return;
	}
	
	reply.vecUserMark.clear();
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			::hoosho::commstruct::UserMark stUserMark;

			stUserMark.set_openid_md5(request.m_openid_md5);
			stUserMark.set_pa_appid_md5(::common::util::charToUint64_t(m_mysql_helper.GetRow(1)));
			stUserMark.set_pa_openid(::common::util::charToUint64_t(m_mysql_helper.GetRow(2)));
			stUserMark.set_is_follow(::common::util::charToUint64_t(m_mysql_helper.GetRow(3)));
			stUserMark.set_create_ts(::common::util::charToUint64_t(m_mysql_helper.GetRow(4)));

			reply.vecUserMark.push_back(stUserMark);
		}
	}
	else
	{
		strErrMsg = "SELECT E_NOT_EXIST: " + ossSql.str();
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return;		
	}

	return;
}

void ExecutorThread::process_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	return;	
}

void ExecutorThread::process_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";

	std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"DELETE FROM t_user_mark_appid WHERE openid_md5="<<request.m_openid_md5;
	
	if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return;
	}
	
	return;	
}

void ExecutorThread::process_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	::hoosho::commstruct::UserMark& stUserMark = request.m_usermark;

	std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"INSERT INTO t_user_mark_appid"
		<<" VALUES ("<<stUserMark.openid_md5()
		<<", "<<stUserMark.pa_appid_md5()
		<<", "<<stUserMark.pa_openid()
		<<", "<<stUserMark.is_follow()
		<<", "<<time(NULL)
		<<")"
		<<" ON DUPLICATE KEY UPDATE"
		<<" is_follow = "<<stUserMark.is_follow();
		
	if(!m_mysql_helper.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
        return;
    }

	return;
}




