#include "executor_thread.h"
#include "global_var.h"
#include "msg_content.h"
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
#define CASE_REQTYPE_FUNC(reqtype, func) \
	case reqtype: \
		func(request, reply); \
		break;

	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_CONTENT_QUERY, process_query);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_CONTENT_ADD, process_add);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_CONTENT_DELETE, process_delete);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_NEWMSG_STATUS_QUERY, process_query_newmsg_status);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_SESSION_LIST_QUERY, process_query_session_list);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_SESSION_DELETE, process_delete_session);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_RED_POINT_QUERY, process_red_point_query);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_RED_POINT_UPDATE, process_red_point_update);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_NOTICE_RECORD_ADD, process_notice_record_add);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_NOTICE_RECORD_QUERY, process_notice_record_query);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_NOTICE_RECORD_DELETE, process_notice_record_delete);
	CASE_REQTYPE_FUNC(ExecutorThreadRequestType::T_MSG_SYSTEM_MSG_QUERY, process_system_msg_query);

#undef CASE_REQTYPE_FUNC

#if 0  
            case ExecutorThreadRequestType::T_MSG_CONTENT_QUERY:
				process_query(request, reply);
            	break;

			case ExecutorThreadRequestType::T_MSG_CONTENT_ADD:
				process_add(request, reply);
				break;

			case ExecutorThreadRequestType::T_MSG_CONTENT_DELETE:
				process_delete(request, reply);
				break;

			case ExecutorThreadRequestType::T_MSG_NEWMSG_STATUS_QUERY:
				process_query_newmsg_status(request, reply);
				break;

			case ExecutorThreadRequestType::T_MSG_SESSION_LIST_QUERY:
				process_query_session_list(request, reply);
				break;

			case ExecutorThreadRequestType::T_MSG_SESSION_DELETE:
				process_delete_session(request, reply);
				break;

			case ExecutorThreadRequestType::T_MSG_RED_POINT_QUERY:
				process_red_point_query(request, reply);
				break;

			case ExecutorThreadRequestType::T_MSG_RED_POINT_UPDATE:
				process_red_point_update(request, reply);
				break;
#endif
			
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
	int iRet = request.m_msg_content.SelectFromDB(reply.vecMsgContent, m_mysql_helper, strErrMsg);
	if(iRet == 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, strErrMsg);
	}
	else if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
	}
	else
	{
		reply.m_result_code = ExecutorThreadRequestType::E_OK;
		//reply.m_msg_content = msgContent;
	}
	
	return;
}

void ExecutorThread::process_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	//int iRet = msgContent.UpdateToDB(m_mysql_helper, strErrMsg);
	int iRet = 0;
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
	}
	
	return;	
}

void ExecutorThread::process_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	int iRet = request.m_msg_content.DeleteFromDB(m_mysql_helper, strErrMsg);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
	}
	else if(iRet == 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, strErrMsg);
	}
	else
	{
		reply.m_result_code = ExecutorThreadRequestType::E_OK;
	}
	
	return;	
}

void ExecutorThread::process_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	int iRet = request.m_msg_content.AddToDB(m_mysql_helper, strErrMsg);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
	}
	else
	{
		reply.m_msg_content = request.m_msg_content;
	}
	
	return;	
}

void ExecutorThread::process_query_newmsg_status(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	uint64_t openid_md5 = request.m_msg_content.m_openid_md5_from;
    uint16_t table_index = (uint16_t)openid_md5 % 256;
    char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);

    std::ostringstream ossSql;
    ossSql.str("");
	ossSql<<"SELECT * FROM t_session_user_"<<strTableIndex
		<<" WHERE openid_md5_from="<<openid_md5
		<<" AND newmsg_status=1";

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if(!m_mysql_helper.Query(ossSql.str()))
	{
	    strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
	    reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
	    LOG4CPLUS_ERROR(logger, strErrMsg);
	}

	if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
	{		
		reply.newmsg_status = 1;
	}    
	else
	{
		reply.newmsg_status = 0;
	}

	return;    
}

void ExecutorThread::process_query_session_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	reply.vecSession.clear();
	std::string strErrMsg = "";
	uint64_t openid_md5 = request.m_msg_content.m_openid_md5_from;
    uint16_t table_index = (uint16_t)openid_md5 % 256;
    char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);

	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"SELECT session_id, newmsg_status, openid_md5_to FROM t_session_user_"<<strTableIndex
    	<<" WHERE openid_md5_from="<<openid_md5;

    LOG4CPLUS_DEBUG(logger, ossSql.str());

	if(!m_mysql_helper.Query(ossSql.str()))
	{
	    strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
	    reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
	    LOG4CPLUS_ERROR(logger, strErrMsg);
	}

	size_t row = m_mysql_helper.GetRowCount();
	uint64_t session_status_array[row][5];
	int i = 0;
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{						
			session_status_array[i][0] = common::util::charToUint64_t(m_mysql_helper.GetRow(0));		//session_id
			session_status_array[i][1] = common::util::charToUint64_t(m_mysql_helper.GetRow(1));		//newmsg_status
			session_status_array[i][2] = common::util::charToUint64_t(m_mysql_helper.GetRow(2));		//openid_md5_to
			i++;
		}
	}
	else
	{
		strErrMsg = "E_DB_NOFOUND: " + ossSql.str();
		LOG4CPLUS_DEBUG(logger, strErrMsg);
		return;
	}
	
	while(i>0)
	{
		i--;
		uint64_t session_id = session_status_array[i][0];
		uint64_t openid_md5_to = session_status_array[i][2];

		int del_status = openid_md5 > openid_md5_to ? 0x02 : 0x01;
		
		table_index = (uint16_t)session_id % 256;
		sprintf(strTableIndex, "%02x", table_index);
		ossSql.str("");
	    ossSql<<"SELECT msg_id, create_ts FROM t_session_msg_"<<strTableIndex
	    	<<" WHERE session_id="<<session_id
	    	<<" AND del_status !="<<del_status
	    	<<" ORDER BY create_ts DESC LIMIT 1";

		LOG4CPLUS_DEBUG(logger, ossSql.str());
		
		if(!m_mysql_helper.Query(ossSql.str()))
		{
		    strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		    reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		    LOG4CPLUS_ERROR(logger, strErrMsg);
		    return;
		}
		
		if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
		{			
			session_status_array[i][3] = common::util::charToUint64_t(m_mysql_helper.GetRow(0));		//msg_id
			session_status_array[i][4] = common::util::charToUint64_t(m_mysql_helper.GetRow(1));		//create_ts
		}
		else
		{
			strErrMsg = "E_DB_NOFOUND: " + ossSql.str();
			LOG4CPLUS_DEBUG(logger, strErrMsg);
		}
	}

	i = row;
	while(i>0)
	{
		i--;
		
		table_index = (uint16_t)session_status_array[i][3] % 256;
		sprintf(strTableIndex, "%02x", table_index);
		
		ossSql.str("");
		ossSql<<"SELECT content FROM t_msg_content_"<<strTableIndex
			<<" WHERE msg_id="<<session_status_array[i][3];

		LOG4CPLUS_DEBUG(logger, ossSql.str());
		
		if(!m_mysql_helper.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return;
		}

		if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
		{
			hoosho::commstruct::Session m_session;
			m_session.set_openid_md5_from(session_status_array[i][2]);
			m_session.set_openid_md5_to(openid_md5);
			m_session.set_content(m_mysql_helper.GetRow(0));
			m_session.set_create_ts(session_status_array[i][4]);
			m_session.set_newmsg_status(session_status_array[i][1]);

			reply.vecSession.push_back(m_session);
		}
		else
		{
			strErrMsg = "E_DB_NOFOUND: " + ossSql.str();
			LOG4CPLUS_DEBUG(logger, strErrMsg);
		}
		
		
	}
	
    return;
}

void ExecutorThread::process_delete_session(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
    uint16_t table_index = (uint16_t)request.m_msg_content.m_openid_md5_from % 256;
    char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);

	/*
	 * UPDATE newmsg_status
	 */
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"UPDATE t_session_user_"<<strTableIndex
    	<<" SET newmsg_status=0 WHERE openid_md5_from="<<request.m_msg_content.m_openid_md5_from
    	<<" AND openid_md5_to="<<request.m_msg_content.m_openid_md5_to;

    LOG4CPLUS_DEBUG(logger, ossSql.str());

    if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return;
	}

	/*
	 * SELECT session_id
	 */
	ossSql.str("");
	ossSql<<"SELECT session_id FROM t_session_user_"<<strTableIndex
		<<" WHERE openid_md5_from="<<request.m_msg_content.m_openid_md5_from
		<<" AND openid_md5_to="<<request.m_msg_content.m_openid_md5_to;

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return;
	}

	uint64_t session_id = 0;
	if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
	{
		session_id = common::util::charToUint64_t(m_mysql_helper.GetRow(0));		//session_id
	}

	int del_status = request.m_msg_content.m_openid_md5_from > request.m_msg_content.m_openid_md5_to ? 0x02 : 0x01;

	{
		table_index = (uint16_t)session_id % 256;
		sprintf(strTableIndex, "%02x", table_index);

		/*
		 * UPDATE del_status IN TABLE: t_session_msg_index
		 */
		ossSql.str("");
		ossSql<<"UPDATE t_session_msg_"<<strTableIndex
			<<" SET del_status=del_status|"<<del_status
			<<" WHERE session_id="<<session_id;

		LOG4CPLUS_DEBUG(logger, ossSql.str());

		if(!m_mysql_helper.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return;
		}		



		ossSql.str("");
		ossSql<<"SELECT msg_id, del_status FROM t_session_msg_"<<strTableIndex
			<<" WHERE session_id="<<session_id;

		LOG4CPLUS_DEBUG(logger, ossSql.str());

		if(!m_mysql_helper.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return;
		}

		int n = m_mysql_helper.GetRowCount();
		uint64_t msg_id_del_array[n][2];
		
		int j=0;
		if(m_mysql_helper.GetRowCount())
		{
			while(m_mysql_helper.Next())
			{
				msg_id_del_array[j][0] = common::util::charToUint64_t(m_mysql_helper.GetRow(0));	//msg_id
				msg_id_del_array[j][1] = common::util::charToUint64_t(m_mysql_helper.GetRow(1));	//del_status
				j++;
			}
		}

		while(j>0)
		{
			j--;
			if(msg_id_del_array[j][1] == 0x03)	//DELETE
			{
				table_index = (uint16_t)session_id % 256;
				sprintf(strTableIndex, "%02x", table_index);
				ossSql.str("");
				ossSql<<"DELETE FROM t_session_msg_"<<strTableIndex
					<<" WHERE session_id = "<<session_id
					<<" AND msg_id = "<<msg_id_del_array[j][0];

				LOG4CPLUS_DEBUG(logger, ossSql.str());

				if(!m_mysql_helper.Query(ossSql.str()))
			    {
			        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
					LOG4CPLUS_ERROR(logger, strErrMsg);
					return;
			    }

				table_index = (uint16_t)msg_id_del_array[j][0] % 256;
				sprintf(strTableIndex, "%02x", table_index);
			    ossSql.str("");
				ossSql<<"DELETE FROM t_msg_content_"<<strTableIndex
					<<" WHERE msg_id = "<<msg_id_del_array[j][0];

				LOG4CPLUS_DEBUG(logger, ossSql.str());
					
				if(!m_mysql_helper.Query(ossSql.str()))
			    {
			        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
					LOG4CPLUS_ERROR(logger, strErrMsg);
					return;
			    }
			}
		}
	}
	return;
}

void ExecutorThread::process_red_point_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{	
	reply.vecRedPointInfo.clear();
	std::string strErrMsg = "";
	uint64_t openid_md5 = request.m_openid_md5;
	uint16_t table_index = (uint16_t)request.m_openid_md5 % 256;
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);	
	
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"SELECT type, value FROM t_red_point_"<<strTableIndex
		<<" WHERE openid_md5="<<openid_md5
		<<" AND pa_appid_md5="<<request.m_pa_appid_md5
		<<" OR pa_appid_md5=0";		//pa_appid_md5=0 是为了获取粉丝红点
	
	LOG4CPLUS_DEBUG(logger, ossSql.str());
	
	if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return;
	}

	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			::hoosho::commstruct::RedPointInfo redpointinfo;
			redpointinfo.set_type(common::util::charToUint64_t(m_mysql_helper.GetRow(0)));		//type
			redpointinfo.set_value(common::util::charToUint64_t(m_mysql_helper.GetRow(1)));		//value

			reply.vecRedPointInfo.push_back(redpointinfo);
		}
	}

	//Get sixin RED POINT
	{
		ossSql.str("");
		ossSql<<"SELECT * FROM t_session_user_"<<strTableIndex
			<<" WHERE openid_md5_from="<<openid_md5
			<<" AND newmsg_status=1";
		
		LOG4CPLUS_DEBUG(logger, ossSql.str());
		
		if(!m_mysql_helper.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return;
		}

		::hoosho::commstruct::RedPointInfo redpointinfo;
		redpointinfo.set_type(::hoosho::commenum::NOTICE_RECORD_TYPE_MESSAGE);	//私信
		if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
		{		
			redpointinfo.set_value(1);
		}	 
		else
		{
			redpointinfo.set_value(0);
		}
		reply.vecRedPointInfo.push_back(redpointinfo);
	}

	return;
}

void ExecutorThread::process_red_point_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	uint16_t table_index = (uint16_t)request.m_openid_md5 % 256;
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);	
	
	std::ostringstream ossSql;
	ossSql.str("");
	if(request.m_redpoint_opt == 0)	//-1
	{
		ossSql<<"SELECT value from t_red_point_"<<strTableIndex
			<<" WHERE openid_md5="<<request.m_openid_md5
			<<" AND pa_appid_md5="<<request.m_pa_appid_md5
			<<" AND type="<<request.m_redpoint_type;

		LOG4CPLUS_DEBUG(logger, ossSql.str());
		if(!m_mysql_helper.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return;
		}

		if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
		{
			int tmp_value = common::util::charToUint64_t(m_mysql_helper.GetRow(0));
			if(tmp_value<=0)
			{
				ossSql.str("");
				ossSql<<"UPDATE t_red_point_"<<strTableIndex
					<<" SET value=0"
					<<" WHERE openid_md5="<<request.m_openid_md5
					<<" AND pa_appid_md5="<<request.m_pa_appid_md5
					<<" AND type="<<request.m_redpoint_type;
			}
			else
			{
				ossSql.str("");
				ossSql<<"UPDATE t_red_point_"<<strTableIndex
					<<" SET value=value-1"
					<<" WHERE openid_md5="<<request.m_openid_md5
					<<" AND pa_appid_md5="<<request.m_pa_appid_md5
					<<" AND type="<<request.m_redpoint_type;
			}
		}
		else
		{
			ossSql.str("");
			ossSql<<"INSERT INTO t_red_point_"<<strTableIndex
				<<" SET openid_md5="<<request.m_openid_md5
				<<", pa_appid_md5="<<request.m_pa_appid_md5
				<<", type = "<<request.m_redpoint_type
				<<", value = 0";				
		}
	}
	else if(request.m_redpoint_opt == 1)	//+1
	{
		ossSql<<"INSERT INTO t_red_point_"<<strTableIndex
			<<" SET openid_md5="<<request.m_openid_md5
			<<", pa_appid_md5="<<request.m_pa_appid_md5
			<<", type = "<<request.m_redpoint_type
			<<", value = 1"
			<<" ON DUPLICATE KEY UPDATE"
			<<" value=value+1";
	}
	else if(request.m_redpoint_opt == 2)	//reset 0
	{
		ossSql<<"INSERT INTO t_red_point_"<<strTableIndex
			<<" SET openid_md5="<<request.m_openid_md5
			<<", pa_appid_md5="<<request.m_pa_appid_md5
			<<", type = "<<request.m_redpoint_type
			<<", value = 0"
			<<" ON DUPLICATE KEY UPDATE"
			<<" value=0";
	}
	
	LOG4CPLUS_DEBUG(logger, ossSql.str());
	
	if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return;
	}

	return;
}

int ExecutorThread::process_red_point_update(uint64_t st_openid_md5, uint64_t st_pa_appid_md5, uint64_t st_redpoint_type, uint64_t st_redpoint_opt)
{
	std::string strErrMsg = "";
	uint16_t table_index = (uint16_t)st_openid_md5 % 256;
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);	
	
	std::ostringstream ossSql;
	ossSql.str("");
	if(st_redpoint_opt == RED_POINT_OPT_TYPE_REDUCE)	//-1
	{
		ossSql<<"SELECT value from t_red_point_"<<strTableIndex
			<<" WHERE openid_md5="<<st_openid_md5
			<<" AND pa_appid_md5="<<st_pa_appid_md5
			<<" AND type="<<st_redpoint_type;

		LOG4CPLUS_DEBUG(logger, ossSql.str());
		if(!m_mysql_helper.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}

		if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
		{
			int tmp_value = common::util::charToUint64_t(m_mysql_helper.GetRow(0));
			if(tmp_value<=0)
			{
				ossSql.str("");
				ossSql<<"UPDATE t_red_point_"<<strTableIndex
					<<" SET value=0"
					<<" WHERE openid_md5="<<st_openid_md5
					<<" AND pa_appid_md5="<<st_pa_appid_md5
					<<" AND type="<<st_redpoint_type;
			}
			else
			{
				ossSql.str("");
				ossSql<<"UPDATE t_red_point_"<<strTableIndex
					<<" SET value=value-1"
					<<" WHERE openid_md5="<<st_openid_md5
					<<" AND pa_appid_md5="<<st_pa_appid_md5
					<<" AND type="<<st_redpoint_type;
			}
		}
		else
		{
			ossSql.str("");
			ossSql<<"INSERT INTO t_red_point_"<<strTableIndex
				<<" SET openid_md5="<<st_openid_md5
				<<", pa_appid_md5="<<st_pa_appid_md5
				<<", type = "<<st_redpoint_type
				<<", value = 0";				
		}
	}
	else if(st_redpoint_opt == RED_POINT_OPT_TYPE_INCREASE)	//+1
	{
		ossSql<<"INSERT INTO t_red_point_"<<strTableIndex
			<<" SET openid_md5="<<st_openid_md5
			<<", pa_appid_md5="<<st_pa_appid_md5
			<<", type = "<<st_redpoint_type
			<<", value = 1"
			<<" ON DUPLICATE KEY UPDATE"
			<<" value=value+1";
	}
	else if(st_redpoint_opt == RED_POINT_OPT_TYPE_CLEAR)	//reset 0
	{
		ossSql<<"INSERT INTO t_red_point_"<<strTableIndex
			<<" SET openid_md5="<<st_openid_md5
			<<", pa_appid_md5="<<st_pa_appid_md5
			<<", type = "<<st_redpoint_type
			<<", value = 0"
			<<" ON DUPLICATE KEY UPDATE"
			<<" value=0";
	}
	
	LOG4CPLUS_DEBUG(logger, ossSql.str());
	
	if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return -1;
	}

	return 0;
}

void ExecutorThread::process_notice_record_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	uint16_t table_index = (uint16_t)request.m_notice_record.openid_md5() % 256;
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);	
	
	std::ostringstream ossSql;
	ossSql.str("");
	
	if(request.m_notice_record.type() == ::hoosho::commenum::NOTICE_RECORD_TYPE_FANS)
	{
		//UPDATE REDPOINT TABLE:t_red_point_
		if(process_red_point_update(request.m_notice_record.openid_md5(), request.m_notice_record.pa_appid_md5(), request.m_notice_record.type(), RED_POINT_OPT_TYPE_INCREASE) < 0)
			LOG4CPLUS_ERROR(logger, "RED_POINT_OPT_TYPE_INCREASE failed!");

		return;
	}

	if(request.m_notice_record.type() == ::hoosho::commenum::NOTICE_RECORD_TYPE_FAVORITE) //点赞不能重复插入
	{
		ossSql<<"select * from t_notice_user_"<<strTableIndex
			<<" where openid_md5="<<request.m_notice_record.openid_md5()
			<<" and pa_appid_md5="<<request.m_notice_record.pa_appid_md5()
			<<" and type="<<request.m_notice_record.type()
			<<" and extra_data_0='"<<lce::cgi::CMysql::MysqlEscape(request.m_notice_record.extra_data_0())<<"'"
			<<" and extra_data_1='"<<lce::cgi::CMysql::MysqlEscape(request.m_notice_record.extra_data_1())<<"'";
		
		LOG4CPLUS_DEBUG(logger, ossSql.str());
		
		if (!m_mysql_helper.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return;
		}
	
		if (m_mysql_helper.GetRowCount() && m_mysql_helper.Next())	//update create_ts
		{
			//1.delete
			ossSql.str("");
			ossSql<<"delete from t_notice_user_"<<strTableIndex
				<<" where openid_md5="<<request.m_notice_record.openid_md5()
				<<" and pa_appid_md5="<<request.m_notice_record.pa_appid_md5()
				<<" and type="<<request.m_notice_record.type()
				<<" and extra_data_0='"<<lce::cgi::CMysql::MysqlEscape(request.m_notice_record.extra_data_0())<<"'"
				<<" and extra_data_1='"<<lce::cgi::CMysql::MysqlEscape(request.m_notice_record.extra_data_1())<<"'";
	
			LOG4CPLUS_DEBUG(logger, ossSql.str());
			if (!m_mysql_helper.Query(ossSql.str()))
			{
				strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
				reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
				LOG4CPLUS_ERROR(logger, strErrMsg);
				return;
			}
			
		}
		
		//insert		
		ossSql.str("");
		ossSql<<"insert into t_notice_user_"<<strTableIndex
			<<" set openid_md5="<<request.m_notice_record.openid_md5()
			<<", pa_appid_md5="<<request.m_notice_record.pa_appid_md5()
			<<", create_ts="<<request.m_notice_record.create_ts()
			<<", type="<<request.m_notice_record.type()
			<<", status="<<request.m_notice_record.status()
			<<", extra_data_0='"<<lce::cgi::CMysql::MysqlEscape(request.m_notice_record.extra_data_0())<<"'"
			<<", extra_data_1='"<<lce::cgi::CMysql::MysqlEscape(request.m_notice_record.extra_data_1())<<"'"
			<<", extra_data_2='"<<lce::cgi::CMysql::MysqlEscape(request.m_notice_record.extra_data_2())<<"'";
		
	}	
	else if(request.m_notice_record.type() == ::hoosho::commenum::NOTICE_RECORD_TYPE_COMMENT)
	{
		ossSql<<"insert into t_notice_user_"<<strTableIndex
			<<" set openid_md5="<<request.m_notice_record.openid_md5()
			<<", pa_appid_md5="<<request.m_notice_record.pa_appid_md5()
			<<", create_ts="<<request.m_notice_record.create_ts()
			<<", type="<<request.m_notice_record.type()
			<<", status="<<request.m_notice_record.status()
			<<", extra_data_0='"<<lce::cgi::CMysql::MysqlEscape(request.m_notice_record.extra_data_0())<<"'"
			<<", extra_data_1='"<<lce::cgi::CMysql::MysqlEscape(request.m_notice_record.extra_data_1())<<"'"
			<<", extra_data_2='"<<lce::cgi::CMysql::MysqlEscape(request.m_notice_record.extra_data_2())<<"'";
	}
	
	
	LOG4CPLUS_DEBUG(logger, ossSql.str());
	
	if (!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return;
	}

	//UPDATE REDPOINT TABLE:t_red_point_
	if(process_red_point_update(request.m_notice_record.openid_md5(), request.m_notice_record.pa_appid_md5(), request.m_notice_record.type(), RED_POINT_OPT_TYPE_INCREASE) < 0)
		LOG4CPLUS_ERROR(logger, "RED_POINT_OPT_TYPE_INCREASE failed!");
	
	return;
}

void ExecutorThread::process_notice_record_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{	
	std::string strErrMsg = "";
	uint16_t table_index = (uint16_t)request.m_openid_md5 % 256;
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);	

	reply.vecNoticeRecord.clear();
	
	if(request.m_notice_record_type == ::hoosho::commenum::NOTICE_RECORD_TYPE_FANS)
	{
		request.m_pa_appid_md5 = 0;
		
		//UPDATE REDPOINT TABLE:t_red_point_
		if(process_red_point_update(request.m_openid_md5, request.m_pa_appid_md5, request.m_notice_record_type, RED_POINT_OPT_TYPE_CLEAR) < 0)
			LOG4CPLUS_ERROR(logger, "RED_POINT_OPT_TYPE_CLEAR failed!");
		
		return;
	}
	
	std::ostringstream ossSql;
	ossSql.str("");
	if(request.m_limit_ts == 0) //GET LATEST
	{
		ossSql<<"SELECT * FROM t_notice_user_"<<strTableIndex
			<<" WHERE pa_appid_md5 = "<<request.m_pa_appid_md5
			<<" AND openid_md5 = "<<request.m_openid_md5
			<<" AND type = "<<request.m_notice_record_type
			<<" ORDER BY create_ts DESC LIMIT "<<request.m_pagesize;
	}
	else	//GET OLD
	{
		ossSql<<"SELECT * FROM t_notice_user_"<<strTableIndex
			<<" WHERE pa_appid_md5 = "<<request.m_pa_appid_md5
			<<" AND openid_md5 = "<<request.m_openid_md5
			<<" AND type = "<<request.m_notice_record_type
			<<" AND create_ts < "<<request.m_limit_ts
			<<" ORDER BY create_ts DESC LIMIT "<<request.m_pagesize;
	}

	LOG4CPLUS_DEBUG(logger, ossSql.str());
	if (!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return;
	}
	
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			::hoosho::commstruct::NoticeRecord noticeRecord;
			noticeRecord.set_openid_md5(::common::util::charToUint64_t(m_mysql_helper.GetRow(0)));		
			noticeRecord.set_pa_appid_md5(::common::util::charToUint64_t(m_mysql_helper.GetRow(1)));				
			noticeRecord.set_create_ts(::common::util::charToUint64_t(m_mysql_helper.GetRow(2)));
			noticeRecord.set_type(::common::util::charToUint64_t(m_mysql_helper.GetRow(3)));
			noticeRecord.set_status(::common::util::charToUint64_t(m_mysql_helper.GetRow(4)));
			noticeRecord.set_extra_data_0(m_mysql_helper.GetRow(5));
			noticeRecord.set_extra_data_1(m_mysql_helper.GetRow(6));
			noticeRecord.set_extra_data_2(m_mysql_helper.GetRow(7));

			reply.vecNoticeRecord.push_back(noticeRecord);
		}
	}

	//UPDATE REDPOINT TABLE:t_red_point_
	if(process_red_point_update(request.m_openid_md5, request.m_pa_appid_md5, request.m_notice_record_type, RED_POINT_OPT_TYPE_CLEAR) < 0)
		LOG4CPLUS_ERROR(logger, "RED_POINT_OPT_TYPE_CLEAR failed!");
	
	return;

}

void ExecutorThread::process_notice_record_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	uint16_t table_index = (uint16_t)request.m_openid_md5 % 256;
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);	

	if(request.m_notice_record_type != ::hoosho::commenum::NOTICE_RECORD_TYPE_FANS)
	{
		std::ostringstream ossSql;
		ossSql.str("");
		ossSql<<"DELETE FROM t_notice_user_"<<strTableIndex
			<<" WHERE pa_appid_md5="<<request.m_pa_appid_md5
			<<" AND openid_md5="<<request.m_openid_md5
			<<" AND type="<<request.m_notice_record_type
			<<" AND create_ts="<<request.m_create_ts;

		LOG4CPLUS_DEBUG(logger, ossSql.str());
		if (!m_mysql_helper.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return;
		}
	}	

	//UPDATE REDPOINT TABLE:t_red_point_
	if(process_red_point_update(request.m_openid_md5, request.m_pa_appid_md5, request.m_notice_record_type, RED_POINT_OPT_TYPE_REDUCE) < 0)
		LOG4CPLUS_ERROR(logger, "RED_POINT_OPT_TYPE_REDUCE failed!");

	return;	
}

void ExecutorThread::process_system_msg_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";

	std::ostringstream ossSql;
	ossSql.str("");
	if(request.m_limit_ts == 0)	//GET LATEST
    {
		ossSql<<"SELECT * FROM t_notice_common"			
			<<" WHERE pa_appid_md5 = "<<request.m_pa_appid_md5
			<<" ORDER BY create_ts DESC LIMIT "<<request.m_pagesize;
    }
    else	//GET OLD
    {
		ossSql<<"SELECT * FROM t_notice_common"			
			<<" WHERE pa_appid_md5 = "<<request.m_pa_appid_md5
			<<" AND create_ts < "<<request.m_limit_ts
			<<" ORDER BY create_ts DESC LIMIT "<<request.m_pagesize;
    }
	
	LOG4CPLUS_DEBUG(logger, ossSql.str());
	if (!m_mysql_helper.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
        LOG4CPLUS_ERROR(logger, strErrMsg);
        return;
    }

	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			::hoosho::commstruct::SystemMsg systemMsg;
			systemMsg.set_pa_appid_md5(::common::util::charToUint64_t(m_mysql_helper.GetRow(0)));
			systemMsg.set_create_ts(::common::util::charToUint64_t(m_mysql_helper.GetRow(1)));
			systemMsg.set_title(m_mysql_helper.GetRow(2));
			systemMsg.set_content(m_mysql_helper.GetRow(3));
			
			reply.vecSystemMsg.push_back(systemMsg);
		}
	}

	if(request.m_pagesize > 1)
	{		
		//UPDATE REDPOINT TABLE:t_red_point_
		if(process_red_point_update(request.m_openid_md5, request.m_pa_appid_md5, ::hoosho::commenum::NOTICE_RECORD_TYPE_SYSTEM_MSG, RED_POINT_OPT_TYPE_CLEAR) < 0)
			LOG4CPLUS_ERROR(logger, "RED_POINT_OPT_TYPE_REDUCE failed!");
	}
	
    return;	
}






