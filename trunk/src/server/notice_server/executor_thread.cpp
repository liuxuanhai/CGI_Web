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
	assert(
			m_mysql_helper.Init(stConfig.get_string_param("DB", "ip"), stConfig.get_string_param("DB", "db_name"), stConfig.get_string_param("DB", "user"), stConfig.get_string_param("DB", "passwd"), stConfig.get_int_param("DB", "port")));
	return 0;
}

void ExecutorThread::run()
{
	LOG4CPLUS_TRACE(logger, "ExecutorThread "<<pthread_self()<<" running ...");

	while (true)
	{
		if (m_queue->request_empty())
		{
			usleep(10);
			continue;
		}

		ExecutorThreadRequestElement request = m_queue->get_request();
		m_queue->pop_request();

		LOG4CPLUS_TRACE(logger, "ExecutorThread get request, "<<request.ToString());
		ExecutorThreadResponseElement reply(request.m_request_type, request.m_fsm_id);

		switch (request.m_request_type)
		{
		case ExecutorThreadRequestType::T_SYS_NOTICE_ALL_QUERY:
		case ExecutorThreadRequestType::T_SYS_NOTICE_NEW_QUERY:
		case ExecutorThreadRequestType::T_NOTICE_NOTIFY_QUERY:
		case ExecutorThreadRequestType::T_NOTICE_NOTIFY_TYPE_LIST_QUERY:
		case ExecutorThreadRequestType::T_NOTICE_RECORD_QUERY:
		case ExecutorThreadRequestType::T_NOTICE_NOTIFY_SYS_NOTICE_QUERY:
			process_query(request, reply);
			break;

		case ExecutorThreadRequestType::T_SYS_NOTICE_SELECT_UPDATE:
		case ExecutorThreadRequestType::T_SYS_NOTICE_ADD:
		case ExecutorThreadRequestType::T_NOTICE_RECORD_SELECT_UPDATE:
		case ExecutorThreadRequestType::T_NOTICE_RECORD_ADD:
			process_update(request, reply);
			break;

		case ExecutorThreadRequestType::T_SYS_NOTICE_DELETE:
		case ExecutorThreadRequestType::T_NOTICE_RECORD_DELETE:
			process_delete(request, reply);
			break;

		default:
			LOG4CPLUS_ERROR(logger,
					"ExecutorThread get unknown type:" <<request.m_request_type<<"("<<ExecutorThreadRequestType::type_string(request.m_request_type)<<")");
			break;
		}

		if (!request.m_need_reply) //no need reply
		{
			continue;
		}

		while (m_queue->reply_full())
		{
			usleep(50);
		}

		m_queue->push_reply(reply);
	}
}

void ExecutorThread::process_query(ExecutorThreadRequestElement & request, ExecutorThreadResponseElement & reply)
{

	std::string strErrMsg = "";
	int iRet = 0;

	switch (request.m_request_type)
	{
//------------------------------------------------------------------------------------------------------------//
//------------------------------------------------sys_notice----------------------------------------------//
	//------------------------------------------------------------------------------------------------------------//
	case ExecutorThreadRequestType::T_SYS_NOTICE_ALL_QUERY:
		reply.m_sys_notice_list.m_pa_appid_md5 = request.m_sys_notice.m_pa_appid_md5;
		reply.m_sys_notice_list.m_limit_upper_ts = request.m_sys_notice.m_create_ts;
		reply.m_sys_notice_list.m_pagesize = request.m_pagesize;
		reply.m_sys_notice_list.m_queryflag = SYS_NOTICE_RECORD;
		iRet = reply.m_sys_notice_list.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
			reply.m_sys_notice_list.m_dao_sys_notice_list.clear();
		}
		else if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}
		break;

	case ExecutorThreadRequestType::T_SYS_NOTICE_NEW_QUERY:
		/*	reply.m_dao_notice_state.m_pa_appid_md5 = request.m_sys_notice.m_pa_appid_md5;
		 reply.m_dao_notice_state.m_openid_md5 = request.m_openid_md5;
		 reply.m_dao_notice_state.m_type = SYS_NOTICE_LT;
		 iRet = reply.m_dao_notice_state.SelectFromDB(m_mysql_helper, strErrMsg);
		 if (iRet == DB_NONEXIST)
		 {
		 reply.m_sys_notice_list.m_limit_lower_ts = 0;
		 }
		 else if (iRet == DB_SUCCESS)
		 {
		 reply.m_sys_notice_list.m_limit_lower_ts = reply.m_dao_notice_state.m_last_used_ts;
		 }
		 else
		 {
		 reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		 LOG4CPLUS_ERROR(logger, strErrMsg);
		 break;
		 }*/

		reply.m_sys_notice_list.m_pa_appid_md5 = request.m_sys_notice.m_pa_appid_md5;
		reply.m_sys_notice_list.m_pagesize = request.m_pagesize;
		reply.m_sys_notice_list.m_limit_upper_ts = request.m_sys_notice.m_create_ts;
		reply.m_sys_notice_list.m_queryflag = SYS_NOTICE_RECORD;
		iRet = reply.m_sys_notice_list.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;

		}
		else if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);

		}
		reply.m_dao_notice_state.m_pa_appid_md5 = request.m_sys_notice.m_pa_appid_md5;
		reply.m_dao_notice_state.m_openid_md5 = request.m_openid_md5;
		reply.m_dao_notice_state.m_type = SYS_NOTICE_LT;
		reply.m_dao_notice_state.m_last_used_ts = time(0);
		iRet = reply.m_dao_notice_state.UpdateToDB(m_mysql_helper, strErrMsg);
		if (iRet == DB_FAIL)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;
		}

		break;
//------------------------------------------------------------------------------------------------------------//
//------------------------------------------------notice_notify---------------------------------------------------//
//------------------------------------------------------------------------------------------------------------//
	case ExecutorThreadRequestType::T_NOTICE_NOTIFY_SYS_NOTICE_QUERY:
		reply.m_dao_notice_state.m_pa_appid_md5 = request.m_sys_notice.m_pa_appid_md5;
		reply.m_dao_notice_state.m_openid_md5 = request.m_openid_md5;
		reply.m_dao_notice_state.m_type = SYS_NOTICE_LT;
		iRet = reply.m_dao_notice_state.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == DB_NONEXIST)
		{
			reply.m_sys_notice_list.m_limit_lower_ts = 0;
		}
		else if (iRet == DB_SUCCESS)
		{
			reply.m_sys_notice_list.m_limit_lower_ts = reply.m_dao_notice_state.m_last_used_ts;
		}
		else
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;
		}

		reply.m_sys_notice_list.m_pa_appid_md5 = request.m_sys_notice.m_pa_appid_md5;
		reply.m_sys_notice_list.m_queryflag = NOTICE_NOTIFY_SYS_NOTICE;
		//	reply.m_notice_user_list.m_pagesize = request.m_pagesize;
		iRet = reply.m_sys_notice_list.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
			break;
		}
		else if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;
		}
		break;

	case ExecutorThreadRequestType::T_NOTICE_NOTIFY_QUERY:
	{
		int iFlag =0;
		reply.m_dao_notice_state.m_pa_appid_md5 = request.m_sys_notice.m_pa_appid_md5;
		reply.m_dao_notice_state.m_openid_md5 = request.m_openid_md5;
		reply.m_dao_notice_state.m_type = SYS_NOTICE_LT;
		iRet = reply.m_dao_notice_state.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == DB_NONEXIST)
		{
			reply.m_sys_notice_list.m_limit_lower_ts = 0;
		}
		else if (iRet == DB_SUCCESS)
		{
			reply.m_sys_notice_list.m_limit_lower_ts = reply.m_dao_notice_state.m_last_used_ts;
		}
		else
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;
		}

		reply.m_sys_notice_list.m_pa_appid_md5 = request.m_sys_notice.m_pa_appid_md5;
		reply.m_sys_notice_list.m_queryflag = NOTICE_NOTIFY_SYS_NOTICE;
		//	reply.m_notice_user_list.m_pagesize = request.m_pagesize;
		iRet = reply.m_sys_notice_list.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == 1)
		{
			iFlag=1;

		}
		else if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;
		}


		reply.m_dao_notice_state.m_pa_appid_md5 = request.m_notice_user.m_pa_appid_md5;
		reply.m_dao_notice_state.m_openid_md5 = request.m_notice_user.m_openid_md5;
		reply.m_dao_notice_state.m_type = NOTICE_USER_LT;
		iRet = reply.m_dao_notice_state.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == DB_NONEXIST)
		{
			reply.m_notice_user_list.m_limit_ts = 0;
		}
		else if (iRet == DB_SUCCESS)
		{
			reply.m_notice_user_list.m_limit_ts = reply.m_dao_notice_state.m_last_used_ts;
		}
		else
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;
		}
		reply.m_notice_user_list.m_openid_md5 = request.m_notice_user.m_openid_md5;
		reply.m_notice_user_list.m_pa_appid_md5 = request.m_notice_user.m_pa_appid_md5;
		reply.m_notice_user_list.m_queryflag = NOTICE_NOTIFY;
		//	reply.m_notice_user_list.m_pagesize = request.m_pagesize;
		iRet = reply.m_notice_user_list.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == 1)
		{
			iFlag=1;

		}
		else if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;
		}
		timeval tv;
		gettimeofday(&tv, NULL);
		uint64_t qwUS = (uint64_t)tv.tv_sec * 1000000;
		qwUS += ((uint64_t)tv.tv_usec);
		reply.m_dao_notice_state.m_last_used_ts = qwUS;
		iRet = reply.m_dao_notice_state.UpdateToDB(m_mysql_helper, strErrMsg);
		if (iRet == DB_FAIL)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;
		}

		if(iFlag==0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		}
		break;

	}
	case ExecutorThreadRequestType::T_NOTICE_NOTIFY_TYPE_LIST_QUERY:
	{
		DaoNoticeStateTypeList stDaoNoticeStateTypeList;
		stDaoNoticeStateTypeList.m_pa_appid_md5 = request.m_notice_user.m_pa_appid_md5;
		stDaoNoticeStateTypeList.m_openid_md5 = request.m_notice_user.m_openid_md5;

		iRet = stDaoNoticeStateTypeList.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == DB_FAIL)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;
		}

		LOG4CPLUS_DEBUG(logger, "stDaoNoticeStateTypeList: "<<stDaoNoticeStateTypeList.ToString());
		reply.m_notice_user_list.m_openid_md5 = request.m_notice_user.m_openid_md5;
		reply.m_notice_user_list.m_pa_appid_md5 = request.m_notice_user.m_pa_appid_md5;
		reply.m_notice_user_list.m_queryflag = NOTICE_NOTIFY_TYPE_LIST;
		reply.m_notice_user_list.m_limit_ts_like = stDaoNoticeStateTypeList.m_last_used_ts_like;
		reply.m_notice_user_list.m_limit_ts_comment = stDaoNoticeStateTypeList.m_last_used_ts_comment;
		reply.m_notice_user_list.m_limit_ts_follow = stDaoNoticeStateTypeList.m_last_used_ts_follow;
		//	reply.m_notice_user_list.m_limit_ts_message = stDaoNoticeStateTypeList.m_last_used_ts_message;
		iRet = reply.m_notice_user_list.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
			break;
		}
		else if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;

		}

		break;
	}
//------------------------------------------------------------------------------------------------------------//
//-----------------------------------------notice_record-----------------------------------------------------//
//------------------------------------------------------------------------------------------------------------//
	case ExecutorThreadRequestType::ExecutorThreadRequestType::T_NOTICE_RECORD_QUERY:
	{
		reply.m_notice_user_list.m_openid_md5 = request.m_notice_user.m_openid_md5;
		reply.m_notice_user_list.m_pa_appid_md5 = request.m_notice_user.m_pa_appid_md5;
		reply.m_notice_user_list.m_type = request.m_notice_user.m_type;
		reply.m_notice_user_list.m_queryflag = NOTICE_RECORD;
		reply.m_notice_user_list.m_pagesize = request.m_pagesize;
		reply.m_notice_user_list.m_limit_ts = request.m_notice_user.m_create_ts;



		iRet = reply.m_notice_user_list.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet == 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
			reply.m_sys_notice_list.m_dao_sys_notice_list.clear();
		}
		else if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}

		DaoNoticeState stDaoNoticeState;
		stDaoNoticeState.m_openid_md5 = reply.m_notice_user_list.m_openid_md5;
		stDaoNoticeState.m_pa_appid_md5 = reply.m_notice_user_list.m_pa_appid_md5;
		timeval tv;
		gettimeofday(&tv, NULL);
		uint64_t qwUS = (uint64_t)tv.tv_sec * 1000000;
		qwUS += ((uint64_t)tv.tv_usec);


		stDaoNoticeState.m_last_used_ts = qwUS;
		switch (request.m_notice_user.m_type)
		{
		case NOTICE_USER_LIKED:
			stDaoNoticeState.m_type = NOTICE_USER_LIKED_LT;
			break;
		case NOTICE_USER_COMMENTED:
			stDaoNoticeState.m_type = NOTICE_USER_COMMENTED_LT;
			break;
		case NOTICE_USER_FOLLOWED:
			stDaoNoticeState.m_type = NOTICE_USER_FOLLOWED_LT;
			break;
			/*	case NOTICE_USER_MESSAGED:
			 stDaoNoticeState.m_type = NOTICE_USER_MESSAGED_LT;
			 break;*/
		}

		iRet = stDaoNoticeState.UpdateToDB(m_mysql_helper, strErrMsg);

		if (iRet == DB_FAIL)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			break;
		}
		break;
	}
	default:
		LOG4CPLUS_ERROR(logger,
				"ExecutorThread get unknown type:" <<request.m_request_type<<"("<<ExecutorThreadRequestType::type_string(request.m_request_type)<<")");
		break;

	}

	return;
}

void ExecutorThread::process_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	int iRet = 0;
	switch (request.m_request_type)
	{
//-------------------------------------------------------------------------------------------------------------//
//-----------------------------------------------sys_notice-------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------//
	case ExecutorThreadRequestType::T_SYS_NOTICE_ADD:
		//daoSysNotice = &request.m_sys_notice;
		iRet = request.m_sys_notice.InsertToDB(m_mysql_helper, strErrMsg);
		if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}
		else if (iRet == 2)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ALREADY_EXIST;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}
		break;
	case ExecutorThreadRequestType::T_SYS_NOTICE_SELECT_UPDATE:
		//daoSysNotice = &request.m_sys_notice;
	{
		DaoSysNotice stDaoSysNotice;
		stDaoSysNotice.m_pa_appid_md5 = request.m_sys_notice.m_pa_appid_md5;
		stDaoSysNotice.m_create_ts = request.m_sys_notice.m_create_ts;
		iRet = stDaoSysNotice.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}
		if (iRet == 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
			break;
		}

		iRet = request.m_sys_notice.UpdateToDB(m_mysql_helper, strErrMsg);
		if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}
		break;
	}
//-------------------------------------------------------------------------------------------------------------//
//-----------------------------------------------notice_record-------------------------------------------------//
		//-------------------------------------------------------------------------------------------------------------//
	case ExecutorThreadRequestType::T_NOTICE_RECORD_ADD:

		//	daoNoticeUser = &request.m_notice_user;
		iRet = request.m_notice_user.InsertToDB(m_mysql_helper, strErrMsg);
		if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}
		else if (iRet == 2)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ALREADY_EXIST;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}
		break;
	case ExecutorThreadRequestType::T_NOTICE_RECORD_SELECT_UPDATE:
		//	daoNoticeUser = &request.m_notice_user;
	{

		DaoNoticeUser stDaoNoticeUser;
		stDaoNoticeUser.m_pa_appid_md5 = request.m_notice_user.m_pa_appid_md5;
		stDaoNoticeUser.m_openid_md5 = request.m_notice_user.m_openid_md5;
		stDaoNoticeUser.m_type = request.m_notice_user.m_type;
		stDaoNoticeUser.m_create_ts = request.m_notice_user.m_create_ts;
		iRet = stDaoNoticeUser.SelectFromDB(m_mysql_helper, strErrMsg);
		if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}
		if (iRet == 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
			break;
		}
		stDaoNoticeUser.m_status = request.m_notice_user.m_status;
		iRet = stDaoNoticeUser.UpdateToDB(m_mysql_helper, strErrMsg);
		if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}

		break;
	}

	default:
		LOG4CPLUS_ERROR(logger,
				"ExecutorThread get unknown type:" <<request.m_request_type<<"("<<ExecutorThreadRequestType::type_string(request.m_request_type)<<")");
		break;
	}

	return;
}

void ExecutorThread::process_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	int iRet = 0;
//	DaoSysNotice *daoSysNotice;
//	DaoNoticeUser *daoNoticeUser;
	switch (request.m_request_type)
	{
	case ExecutorThreadRequestType::T_SYS_NOTICE_DELETE:

		iRet = request.m_sys_notice.DeleteFromDB(m_mysql_helper, strErrMsg);
		if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}
		break;
	case ExecutorThreadRequestType::T_NOTICE_RECORD_DELETE:
		//	daoNoticeUser = &request.m_notice_user;
		iRet = request.m_notice_user.DeleteFromDB(m_mysql_helper, strErrMsg);
		if (iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, strErrMsg);
		}
		break;
	default:
		LOG4CPLUS_ERROR(logger,
				"ExecutorThread get unknown type:" <<request.m_request_type<<"("<<ExecutorThreadRequestType::type_string(request.m_request_type)<<")");
		break;
	}
	return;
}

