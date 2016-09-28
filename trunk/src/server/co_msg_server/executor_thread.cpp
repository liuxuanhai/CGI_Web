#include "executor_thread.h"
#include "global_var.h"
#include "common_util.h"

IMPL_LOGGER(ExecutorThread, logger)

bool SortByNewmsgStatus(const ::hoosho::commstruct::Session &s1, const ::hoosho::commstruct::Session &s2)
{
	return s1.newmsg_status() < s2.newmsg_status(); 	//DESC
}

bool SortByCreateTS(const ::hoosho::commstruct::Session &s1, const ::hoosho::commstruct::Session &s2)
{
	return s1.create_ts() < s2.create_ts(); 	//DESC
}

void sessionSort(std::vector<hoosho::commstruct::Session>& vecSession)
{
	std::sort(vecSession.begin(), vecSession.end(), SortByCreateTS);
	std::sort(vecSession.begin(), vecSession.end(), SortByNewmsgStatus);
}


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
        if(m_queue->empty())
        {
            usleep(10);
            continue;
        }

        ExecutorThreadRequestElement request = m_queue->front();
        m_queue->pop();
		
        LOG4CPLUS_TRACE(logger, "ExecutorThread get request "<<request.ToString());

        int64_t coroutine_id = request.m_coroutine_id;
		if(coroutine_id == 0)
			return;

		//resp msg
		::hoosho::msg::Msg stRespMsg;
        switch(request.m_request_type)
        {					
#define CASE_REQ_TYPE_PROCESS(reqcmd, process) \
			case reqcmd: \
				process(request, stRespMsg); \
				break;

			CASE_REQ_TYPE_PROCESS(::hoosho::msg::QUERY_MSG_REQ, process_msg_query);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::ADD_MSG_REQ, process_msg_add);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::DELETE_MSG_REQ, process_msg_delete);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::QUERY_MSG_NEWMSG_STATUS_REQ, process_query_newmsg_status);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::QUERY_MSG_SESSION_LIST_REQ, process_query_session_list);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::DELETE_SESSION_REQ, process_delete_session);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::QUERY_SYSTEM_RED_POINT_REQ, process_red_point_query);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::UPDATE_SYSTEM_RED_POINT_REQ, process_red_point_update);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::ADD_NOTICE_RECORD_REQ, process_notice_record_add);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::QUERY_NOTICE_RECORD_REQ, process_notice_record_query);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::DELETE_NOTICE_RECORD_REQ, process_notice_record_delete);
			CASE_REQ_TYPE_PROCESS(::hoosho::msg::QUERY_SYSTEM_MSG_REQ, process_system_msg_query);		
			

#undef CASE_REQ_TYPE_PROCESS
            
			
            default:
                LOG4CPLUS_ERROR(logger, "ExecutorThread get unknown type:"<<request.m_request_type<<", "<<request.type_string());
                break;
        }


		CoroutineModuleBase<TimerModule>& co_module = Singleton<CoroutineModuleBase<TimerModule>>::Instance();

		co_module.ReadLock();
		co_module.Resume(coroutine_id, &stRespMsg);
		co_module.ReleaseLock();
        
    }
}

void ExecutorThread::process_msg_query(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{			
	//resp msg
	//::hoosho::msg::Msg stRespMsg;
    ::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::QUERY_MSG_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);

	
	uint64_t session_id = ::common::util::generate_session_id(request.m_openid_md5_from, request.m_openid_md5_to);
	uint16_t table_index = (uint16_t)request.m_openid_md5_from % 256;
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);
	
	int del_status = request.m_openid_md5_from > request.m_openid_md5_to ? 0x02 : 0x01;

	std::string strErrMsg = "";
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"UPDATE t_session_user_"<<strTableIndex
		<<" SET newmsg_status=0"
		<<" WHERE openid_md5_from="<<request.m_openid_md5_from
		<<" AND openid_md5_to="<<request.m_openid_md5_to;
	
	if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return ;
	}
	
	
	table_index = (uint16_t)session_id % 256;
	sprintf(strTableIndex, "%02x", table_index);
	ossSql.str("");
	if(request.m_msg_id == 0)	//GET LATEST
	{
		ossSql<<"SELECT "
			<<" msg_id, create_ts FROM t_session_msg_"<<strTableIndex
			<<" WHERE session_id = "<<session_id
			<<" AND del_status != "<<del_status
			<<" ORDER BY create_ts DESC LIMIT "<<request.m_amount;
	}
	else	//GET OLD
	{
		ossSql<<"SELECT "
			<<" msg_id, create_ts FROM t_session_msg_"<<strTableIndex
			<<" WHERE session_id = "<<session_id
			<<" AND msg_id < "<<request.m_msg_id
			<<" AND del_status != "<<del_status
			<<" ORDER BY create_ts DESC LIMIT "<<request.m_amount;
	}
	
	
	if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return ;
	}
	
	uint64_t id_create_ts_array[request.m_amount][2];
	int i=0;	
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{			
			id_create_ts_array[i][0] = ::common::util::charToUint64_t(m_mysql_helper.GetRow(0));		//msg_id
			id_create_ts_array[i][1] = ::common::util::charToUint64_t(m_mysql_helper.GetRow(1));		//create_ts 
			i++;
		}
	}
	else
	{
		strErrMsg = "SELECT E_NOT_EXIST: " + ossSql.str();
		LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return ;		
	}

	::hoosho::sixin::QuerySixinRes* pBody = stRespMsg.mutable_msg_query_res();
	while(i>0)
	{
		i--;
		//::hoosho::commstruct::MsgContent stMsgContent;
		::hoosho::commstruct::MsgContent* stMsgContent = pBody->add_msg_content();
		
		
		stMsgContent->set_id(id_create_ts_array[i][0]);
		stMsgContent->set_create_ts(id_create_ts_array[i][1]);
	
		uint64_t msg_id = id_create_ts_array[i][0];
	
		table_index = (uint16_t)msg_id % 256;
		sprintf(strTableIndex, "%02x", table_index);
		ossSql.str("");
		ossSql<<"SELECT content, openid_md5_from, openid_md5_to FROM t_msg_content_"<<strTableIndex
			<<" WHERE msg_id = "<<msg_id;
	
		if(!m_mysql_helper.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			LOG4CPLUS_ERROR(logger, strErrMsg);
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			return ;
		}
	
		if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
		{
			stMsgContent->set_content(m_mysql_helper.GetRow(0));
			stMsgContent->set_openid_md5_from(strtoul(m_mysql_helper.GetRow(1), NULL, 10));
			stMsgContent->set_openid_md5_to(strtoul(m_mysql_helper.GetRow(2), NULL, 10));
		}	 
		else
		{
			strErrMsg = "SELECT E_NOT_EXIST: " + ossSql.str();
			LOG4CPLUS_ERROR(logger, strErrMsg);
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			return ;
		}	
	}

	return;
}

void ExecutorThread::process_msg_add(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::ADD_MSG_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);
	
	/*
	 *INSERT INTO TABLE: t_msg_content_index
	 */
	 
	request.m_msg_id = ::common::util::generate_unique_id();
	uint16_t table_index = (uint16_t)request.m_msg_id % 256;

	 
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);

	std::string strErrMsg = "";
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"INSERT INTO t_msg_content_"<<strTableIndex
   		<<" VALUES ("<<request.m_msg_id
   		<<", '"<<sql_escape(request.m_content)
   		<<"', "<<request.m_openid_md5_from
   		<<", "<<request.m_openid_md5_to
   		<<")";

    if(!m_mysql_helper.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
        LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return ;
    }


	/*
	 *INSERT INTO TABLE: t_session_msg_index
	 */
	 
    uint64_t session_id = common::util::generate_session_id(request.m_openid_md5_from, request.m_openid_md5_to);
    table_index = (uint16_t)session_id % 256;
    sprintf(strTableIndex, "%02x", table_index);
    request.m_create_ts = (uint64_t)time(NULL);
    int del_status = 0x00;
    ossSql.str("");
    ossSql<<"INSERT INTO t_session_msg_"<<strTableIndex
   		<<" VALUES ("<<session_id
   		<<", "<<request.m_msg_id
   		<<", "<<request.m_create_ts
   		<<", "<<del_status
   		<<")";

    if(!m_mysql_helper.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
        LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return ;
    }

    /*
     *INSERT INTO TABLE: t_session_user_index
     */
    
    table_index = (uint16_t)request.m_openid_md5_from % 256;
    sprintf(strTableIndex, "%02x", table_index);
	
    //before insert, check data exists or not
	ossSql.str("");
	ossSql<<"SELECT * FROM t_session_user_"<<strTableIndex
		<<" WHERE session_id = "<<session_id;
		
	if(!m_mysql_helper.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
        LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return ;
    }

    if(m_mysql_helper.GetRowCount()== 0)
    {
		ossSql.str("");
	    ossSql<<"INSERT INTO t_session_user_"<<strTableIndex
	    	<<" VALUES ("<<request.m_openid_md5_from
	    	<<", "<<request.m_openid_md5_to
	    	<<", "<<session_id
	    	<<", "<<request.m_create_ts
	    	<<", 0)";
	    	
	    if(!m_mysql_helper.Query(ossSql.str()))
	    {
	        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
	        LOG4CPLUS_ERROR(logger, strErrMsg);
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			return ;
	    }

		table_index = (uint16_t)request.m_openid_md5_to % 256;
    	sprintf(strTableIndex, "%02x", table_index);
	    ossSql.str("");
	    ossSql<<"INSERT INTO t_session_user_"<<strTableIndex
	    	<<" VALUES ("<<request.m_openid_md5_to
	    	<<", "<<request.m_openid_md5_from
	    	<<", "<<session_id
	    	<<", "<<request.m_create_ts
	    	<<", 1)";
	    	
	    if(!m_mysql_helper.Query(ossSql.str()))
	    {
	        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
	        LOG4CPLUS_ERROR(logger, strErrMsg);
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			return ;
	    }
    }
	else
	{
		table_index = (uint16_t)request.m_openid_md5_to % 256;
    	sprintf(strTableIndex, "%02x", table_index);
	    ossSql.str("");
	    ossSql<<"UPDATE t_session_user_"<<strTableIndex
	    	<<" SET newmsg_status=1 WHERE openid_md5_from="<<request.m_openid_md5_to
	    	<<" AND openid_md5_to="<<request.m_openid_md5_from;
	    	
	    if(!m_mysql_helper.Query(ossSql.str()))
	    {
	        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
	        LOG4CPLUS_ERROR(logger, strErrMsg);
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			return ;
	    }
	}    

	::hoosho::sixin::AddSixinRes* stMsgAddRes = stRespMsg.mutable_msg_add_res();
	stMsgAddRes->set_msg_id(request.m_msg_id);
	stMsgAddRes->set_create_ts(request.m_create_ts);	

	return ;
}

void ExecutorThread::process_msg_delete(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{	
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::DELETE_MSG_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);
	
    uint64_t session_id = ::common::util::generate_session_id(request.m_openid_md5_from, request.m_openid_md5_to);
    uint16_t table_index = (uint16_t)session_id % 256;
    char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);

    int del_status = request.m_openid_md5_from > request.m_openid_md5_to ? 0x02 : 0x01;

    std::string strErrMsg = "";
	std::ostringstream ossSql;
    ossSql.str("");
	ossSql<<"SELECT del_status FROM t_session_msg_"<<strTableIndex
		<<" WHERE session_id = "<<session_id
		<<" AND msg_id = "<<request.m_msg_id;

	if(!m_mysql_helper.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
        LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return ;
    }

    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
    	del_status = del_status | atoi(m_mysql_helper.GetRow(0));

		/*
		 *BOTH openid_a and openid_b del this message, then del this msg form TABLE: t_session_msg_ and t_msg_content_
		 */
		 
    	if(del_status == 0x03)	//DELETE
    	{
			ossSql.str("");
			ossSql<<"DELETE FROM t_session_msg_"<<strTableIndex
				<<" WHERE session_id = "<<session_id
				<<" AND msg_id = "<<request.m_msg_id;

			if(!m_mysql_helper.Query(ossSql.str()))
		    {
		        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		        LOG4CPLUS_ERROR(logger, strErrMsg);
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
				return ;
		    }

			table_index = (uint16_t)request.m_msg_id % 256;
			sprintf(strTableIndex, "%02x", table_index);
		    ossSql.str("");
			ossSql<<"DELETE FROM t_msg_content_"<<strTableIndex
				<<" WHERE msg_id = "<<request.m_msg_id;
				
			if(!m_mysql_helper.Query(ossSql.str()))
		    {
		        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		        LOG4CPLUS_ERROR(logger, strErrMsg);
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
				return ;
		    }
				
    	}
    	else	//UPDATE
    	{
			ossSql.str("");
			ossSql<<"UPDATE t_session_msg_"<<strTableIndex
				<<" SET del_status = "<<del_status
				<<" WHERE session_id = "<<session_id
				<<" AND msg_id = "<<request.m_msg_id;

			if(!m_mysql_helper.Query(ossSql.str()))
		    {
		        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		        LOG4CPLUS_ERROR(logger, strErrMsg);
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
				return ;
		    }
    	}
	}
	else
	{
		strErrMsg = "SELECT E_NOT_EXIST: " + ossSql.str();
		LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return ;
	}

	return ;
}

void ExecutorThread::process_query_newmsg_status(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::QUERY_MSG_NEWMSG_STATUS_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);
	
	std::string strErrMsg = "";
	uint64_t openid_md5 = request.m_openid_md5;
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
	    LOG4CPLUS_ERROR(logger, strErrMsg);
	    header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
	    return ;
	}

	::hoosho::sixin::QuerySixinNewMsgStatusRes* pBody = stRespMsg.mutable_msg_query_newmsg_status_res();
	if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
	{		
		pBody->set_newmsg_status(1);
	}    
	else
	{
		pBody->set_newmsg_status(0);
	}

	return;    
}

void ExecutorThread::process_query_session_list(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::QUERY_MSG_SESSION_LIST_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);

	std::string strErrMsg = "";
	uint64_t openid_md5 = request.m_openid_md5;
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
	    LOG4CPLUS_ERROR(logger, strErrMsg);
	    header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
	    return ;
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
		LOG4CPLUS_ERROR(logger, strErrMsg);
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
		    header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		    LOG4CPLUS_ERROR(logger, strErrMsg);
	    	return ;
		}
		
		if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
		{			
			session_status_array[i][3] = common::util::charToUint64_t(m_mysql_helper.GetRow(0));		//msg_id
			session_status_array[i][4] = common::util::charToUint64_t(m_mysql_helper.GetRow(1));		//create_ts
		}
		else
		{
			strErrMsg = "E_DB_NOFOUND: " + ossSql.str();
			LOG4CPLUS_ERROR(logger, strErrMsg);
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
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		    LOG4CPLUS_ERROR(logger, strErrMsg);
	    	return ;
		}

		std::vector<hoosho::commstruct::Session> vecSession;
		if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
		{
			hoosho::commstruct::Session m_session;
			m_session.set_openid_md5_from(session_status_array[i][2]);
			m_session.set_openid_md5_to(openid_md5);
			m_session.set_content(m_mysql_helper.GetRow(0));
			m_session.set_create_ts(session_status_array[i][4]);
			m_session.set_newmsg_status(session_status_array[i][1]);

			vecSession.push_back(m_session);
		}
		else
		{
			strErrMsg = "E_DB_NOFOUND: " + ossSql.str();
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return;
		}

		//sort first by create_ts then by newmsg_status
		sessionSort(vecSession);

		::hoosho::sixin::QuerySixinSessionListRes* pBody = stRespMsg.mutable_msg_query_session_list_res();
		for(size_t i=0; i<vecSession.size(); i++)
		{
			pBody->add_session()->CopyFrom(vecSession[i]);
		}
		
		vecSession.clear();
		
	}
	
    return;
}

void ExecutorThread::process_delete_session(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::DELETE_SESSION_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);
	
	std::string strErrMsg = "";
    uint16_t table_index = (uint16_t)request.m_openid_md5_from % 256;
    char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);

	/*
	 * UPDATE newmsg_status
	 */
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"UPDATE t_session_user_"<<strTableIndex
    	<<" SET newmsg_status=0 WHERE openid_md5_from="<<request.m_openid_md5_from
    	<<" AND openid_md5_to="<<request.m_openid_md5_to;

    LOG4CPLUS_DEBUG(logger, ossSql.str());

    if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		LOG4CPLUS_ERROR(logger, strErrMsg);		
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return;
	}

	/*
	 * SELECT session_id
	 */
	ossSql.str("");
	ossSql<<"SELECT session_id FROM t_session_user_"<<strTableIndex
		<<" WHERE openid_md5_from="<<request.m_openid_md5_from
		<<" AND openid_md5_to="<<request.m_openid_md5_to;

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if(!m_mysql_helper.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
		LOG4CPLUS_ERROR(logger, strErrMsg);		
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return;
	}

	uint64_t session_id = 0;
	if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
	{
		session_id = ::common::util::charToUint64_t(m_mysql_helper.GetRow(0));		//session_id
	}

	int del_status = request.m_openid_md5_from > request.m_openid_md5_to ? 0x02 : 0x01;

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
			LOG4CPLUS_ERROR(logger, strErrMsg);			
		    header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			return;
		}		



		ossSql.str("");
		ossSql<<"SELECT msg_id, del_status FROM t_session_msg_"<<strTableIndex
			<<" WHERE session_id="<<session_id;

		LOG4CPLUS_DEBUG(logger, ossSql.str());

		if(!m_mysql_helper.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + m_mysql_helper.GetErrMsg();
			LOG4CPLUS_ERROR(logger, strErrMsg);			
		    header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			return;
		}

		int n = m_mysql_helper.GetRowCount();
		uint64_t msg_id_del_array[n][2];
		
		int j=0;
		if(m_mysql_helper.GetRowCount())
		{
			while(m_mysql_helper.Next())
			{
				msg_id_del_array[j][0] = ::common::util::charToUint64_t(m_mysql_helper.GetRow(0));	//msg_id
				msg_id_del_array[j][1] = ::common::util::charToUint64_t(m_mysql_helper.GetRow(1));	//del_status
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
					LOG4CPLUS_ERROR(logger, strErrMsg);					
					header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
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
					LOG4CPLUS_ERROR(logger, strErrMsg);
					header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
					return;
			    }
			}
		}
	}
	return;
}

void ExecutorThread::process_red_point_query(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{	
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::QUERY_SYSTEM_RED_POINT_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);
	
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
		LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return;
	}

	std::vector<hoosho::commstruct::RedPointInfo> vecRedPointInfo;
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			::hoosho::commstruct::RedPointInfo redpointinfo;
			redpointinfo.set_type(common::util::charToUint64_t(m_mysql_helper.GetRow(0)));		//type
			redpointinfo.set_value(common::util::charToUint64_t(m_mysql_helper.GetRow(1)));		//value

			vecRedPointInfo.push_back(redpointinfo);
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
			LOG4CPLUS_ERROR(logger, strErrMsg);
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
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
		vecRedPointInfo.push_back(redpointinfo);
	}

	::hoosho::sixin::QuerySystemRedPointRes* pBody = stRespMsg.mutable_query_system_red_point_res();
	for(size_t i=0; i<vecRedPointInfo.size(); i++)
	{
		pBody->add_red_point_info()->CopyFrom(vecRedPointInfo[i]);
	}
	vecRedPointInfo.clear();	

	return;
}

void ExecutorThread::process_red_point_update(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::UPDATE_SYSTEM_RED_POINT_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);
	
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
			LOG4CPLUS_ERROR(logger, strErrMsg);
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			return;
		}

		if(m_mysql_helper.GetRowCount()&&m_mysql_helper.Next())
		{
			int tmp_value = ::common::util::charToUint64_t(m_mysql_helper.GetRow(0));
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
		LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
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

void ExecutorThread::process_notice_record_add(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::ADD_NOTICE_RECORD_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);
	
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
			LOG4CPLUS_ERROR(logger, strErrMsg);
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
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
				LOG4CPLUS_ERROR(logger, strErrMsg);
				header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
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
		LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return;
	}

	//UPDATE REDPOINT TABLE:t_red_point_
	if(process_red_point_update(request.m_notice_record.openid_md5(), request.m_notice_record.pa_appid_md5(), request.m_notice_record.type(), RED_POINT_OPT_TYPE_INCREASE) < 0)
		LOG4CPLUS_ERROR(logger, "RED_POINT_OPT_TYPE_INCREASE failed!");
	
	return;
}

void ExecutorThread::process_notice_record_query(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{	
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::QUERY_NOTICE_RECORD_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);
	
	std::string strErrMsg = "";
	uint16_t table_index = (uint16_t)request.m_openid_md5 % 256;
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);	

	
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
		LOG4CPLUS_ERROR(logger, strErrMsg);
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		return;
	}

	::hoosho::sixin::QueryNoticeRecordRes* pBody = stRespMsg.mutable_notice_record_query_res();
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			::hoosho::commstruct::NoticeRecord* noticeRecord = pBody->add_notice_record_list();
			noticeRecord->set_openid_md5(::common::util::charToUint64_t(m_mysql_helper.GetRow(0)));		
			noticeRecord->set_pa_appid_md5(::common::util::charToUint64_t(m_mysql_helper.GetRow(1)));				
			noticeRecord->set_create_ts(::common::util::charToUint64_t(m_mysql_helper.GetRow(2)));
			noticeRecord->set_type(::common::util::charToUint64_t(m_mysql_helper.GetRow(3)));
			noticeRecord->set_status(::common::util::charToUint64_t(m_mysql_helper.GetRow(4)));
			noticeRecord->set_extra_data_0(m_mysql_helper.GetRow(5));
			noticeRecord->set_extra_data_1(m_mysql_helper.GetRow(6));
			noticeRecord->set_extra_data_2(m_mysql_helper.GetRow(7));
		}
	}

	//UPDATE REDPOINT TABLE:t_red_point_
	if(process_red_point_update(request.m_openid_md5, request.m_pa_appid_md5, request.m_notice_record_type, RED_POINT_OPT_TYPE_CLEAR) < 0)
		LOG4CPLUS_ERROR(logger, "RED_POINT_OPT_TYPE_CLEAR failed!");
	
	return;

}

void ExecutorThread::process_notice_record_delete(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::DELETE_NOTICE_RECORD_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);
	
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
			LOG4CPLUS_ERROR(logger, strErrMsg);
			header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
			return;
		}
	}	

	//UPDATE REDPOINT TABLE:t_red_point_
	if(process_red_point_update(request.m_openid_md5, request.m_pa_appid_md5, request.m_notice_record_type, RED_POINT_OPT_TYPE_REDUCE) < 0)
		LOG4CPLUS_ERROR(logger, "RED_POINT_OPT_TYPE_REDUCE failed!");

	return;	
}

void ExecutorThread::process_system_msg_query(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg)
{
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();      
    
    header->set_cmd(::hoosho::msg::QUERY_SYSTEM_MSG_RES); 
    header->set_seq(request.m_seq);    
	header->set_result(::hoosho::msg::E_OK);
	
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
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
        return;
    }

	::hoosho::sixin::QuerySystemMsgRes* pBody = stRespMsg.mutable_system_msg_query_res();
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			::hoosho::commstruct::SystemMsg* systemMsg = pBody->add_system_msg_list();
			systemMsg->set_pa_appid_md5(::common::util::charToUint64_t(m_mysql_helper.GetRow(0)));
			systemMsg->set_create_ts(::common::util::charToUint64_t(m_mysql_helper.GetRow(1)));
			systemMsg->set_title(m_mysql_helper.GetRow(2));
			systemMsg->set_content(m_mysql_helper.GetRow(3));			
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


