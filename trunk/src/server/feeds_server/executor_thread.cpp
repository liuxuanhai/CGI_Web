#include "executor_thread.h"
#include "global_var.h"
#include "db_feed.h"
#include "db_follow.h"
#include "db_favorite.h"

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

	m_table_name_prefix_feed = stConfig.get_string_param("DB", "table_name_prefix_feed");
	m_table_name_prefix_feed_index_on_appid = stConfig.get_string_param("DB", "table_name_prefix_feed_index_on_appid");
	m_table_name_prefix_feed_report = stConfig.get_string_param("DB", "table_name_prefix_feed_report");
	m_table_name_prefix_user_forbid= stConfig.get_string_param("DB", "table_name_prefix_user_forbid");
	m_table_name_prefix_feed_favorite = stConfig.get_string_param("DB", "table_name_prefix_feed_favorite");
	m_table_name_prefix_feed_collect = stConfig.get_string_param("DB", "table_name_prefix_feed_collect");
	m_table_name_prefix_feed_follow = stConfig.get_string_param("DB", "table_name_prefix_feed_follow");
	m_table_name_prefix_comment_index_on_feedid = stConfig.get_string_param("DB", "table_name_prefix_comment_index_on_feedid");
	m_table_name_prefix_reply_index_on_origin_comment = stConfig.get_string_param("DB", "table_name_prefix_reply_index_on_origin_comment");
	
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

			case ExecutorThreadRequestType::T_FEED_DETAIL_QUERY:
				process_feed_detail_query(request, reply);
				break;
				
            case ExecutorThreadRequestType::T_FEED_QUERY:
				process_feed_query(request, reply);
            	break;

			case ExecutorThreadRequestType::T_FEED_ADD:
				process_feed_add(request, reply);
				break;

			case ExecutorThreadRequestType::T_FEED_DELETE:
				process_feed_delete(request, reply);
				break;

			case ExecutorThreadRequestType::T_REPORT_FEED_ADD:
				process_report_feed_add(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_FORBID_QUERY:
				process_user_forbid_query(request, reply);
				break;


			case ExecutorThreadRequestType::T_FOLLOW_COMMENT_DETAIL_QUERY:
				process_follow_comment_detail_query(request, reply);
            	break;
				
            case ExecutorThreadRequestType::T_FOLLOW_COMMENT_QUERY:
				process_follow_comment_query(request, reply);
            	break;

			case ExecutorThreadRequestType::T_FOLLOW_DETAIL_QUERY:
				process_follow_detail_query(request, reply);
				break;

			case ExecutorThreadRequestType::T_FOLLOW_LIST_QUERY:
				process_follow_list_query(request, reply);
				break;

            case ExecutorThreadRequestType::T_FOLLOW_ADD:
				process_follow_add(request, reply);
            	break;
				
			case ExecutorThreadRequestType::T_FOLLOW_DELETE:
				process_follow_delete(request, reply);
				break;



			case ExecutorThreadRequestType::T_FEED_LIST_FAVORITE_QUERY:
				process_feed_list_favorite_query(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_FAVORITE_QUERY:
				process_favorite_query(request, reply);
				break;

			case ExecutorThreadRequestType::T_FAVORITE_ADD:
				process_favorite_add(request, reply);
				break;

			case ExecutorThreadRequestType::T_FAVORITE_DELETE:
				process_favorite_delete(request, reply);
				break;
				
		
			case ExecutorThreadRequestType::T_FEED_LIST_COLLECT_QUERY:
				process_feed_list_collect_query(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_COLLECT_QUERY:
				process_collect_query(request, reply);
				break;

			case ExecutorThreadRequestType::T_COLLECT_ADD:
				process_collect_add(request, reply);
				break;

			case ExecutorThreadRequestType::T_COLLECT_DELETE:
				process_collect_delete(request, reply);
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


void ExecutorThread::process_feed_detail_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_feed_detail_query");

	FeedInfo stFeedInfo;
	int iRet = 0;
	string strErrMsg;
	reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
	reply.m_feed_list.clear();
	for(size_t i = 0; i < request.m_feed_id_list.size(); i++)
	{
		stFeedInfo.m_feed_id = request.m_feed_id_list[i];
		iRet = stFeedInfo.SelectFromDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
		if(iRet == TableBase::DB_RET_NOT_EXIST)
		{
			LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " not found");
			continue;
		}
		if(iRet == TableBase::DB_RET_FAIL)
		{
			LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
			continue;
		}
/*
		if(stFeedInfo.m_del_ts)
		{
			LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " has deleted");
			reply.m_result_code = ExecutorThreadRequestType::E_FEED_DELETED;
			return;
		}
*/		
		//add num_read
		stFeedInfo.m_num_read++;
		
		reply.m_result_code = ExecutorThreadRequestType::E_OK;
		reply.m_feed_list.push_back(stFeedInfo);

		//save 
		iRet = stFeedInfo.UpdateToDB(m_table_name_prefix_feed , m_mysql_helper, strErrMsg);
		if(iRet == TableBase::DB_RET_FAIL)
		{
			LOG4CPLUS_ERROR(logger, "stFeedInfo.UpdateToDB failed, errmsg = " << strErrMsg);
			continue;
		}
	}
}

void ExecutorThread::process_feed_query(ExecutorThreadRequestElement & request, ExecutorThreadResponseElement & reply)
{
	LOG4CPLUS_TRACE(logger, "process_feed_query");

	std::string strErrMsg = "";
	FeedIndexOnAppid stFeedIndexOnAppid;
	stFeedIndexOnAppid = request.m_feed_index_on_appid;

	//select by appid
	int iRet = stFeedIndexOnAppid.SelectFromDB(m_table_name_prefix_feed_index_on_appid, m_mysql_helper, strErrMsg);
	reply.m_total = stFeedIndexOnAppid.m_total;
	
	LOG4CPLUS_TRACE(logger, "sql = " << strErrMsg);
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		return;
	}
	else if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedIndexOnAppid.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}

	//get feed_info
	FeedInfo stFeedInfo;
	reply.m_feed_list.clear();
	
	for(size_t i = 0; i < stFeedIndexOnAppid.m_feed_id_list.size(); i++)
	{
		stFeedInfo.m_feed_id = stFeedIndexOnAppid.m_feed_id_list[i];
		iRet = stFeedInfo.SelectFromDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
		LOG4CPLUS_TRACE(logger, "feed id = " << stFeedInfo.m_feed_id 
							<< ", openid_md5 = " << stFeedInfo.m_openid_md5);
		if(iRet == TableBase::DB_RET_FAIL)
		{
			LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
			continue;
		}
		else if(iRet == TableBase::DB_RET_NOT_EXIST)
		{
			LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " not found");
			continue;
		}

		reply.m_feed_list.push_back(stFeedInfo);
	}
	LOG4CPLUS_TRACE(logger, "list size = " << reply.m_feed_list.size());
	
	if(reply.m_feed_list.empty())
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_TRACE(logger, "not data found ");
	}
	return;
}

void ExecutorThread::process_feed_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_feed_add");
	std::string strErrMsg = "";
	//
	FeedInfo stFeedInfo = request.m_feed_info;
	int iRet = stFeedInfo.InsertToDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.InsertDB failed, errmsg = " << strErrMsg);
		return;
	}

	FeedIndexOnAppid stFeedIndexOnAppid;
	stFeedIndexOnAppid.m_pa_appid_md5 = request.m_feed_info.m_pa_appid_md5;
	stFeedIndexOnAppid.m_openid_md5 = request.m_feed_info.m_openid_md5;
	stFeedIndexOnAppid.m_feed_id = request.m_feed_info.m_feed_id;
	stFeedIndexOnAppid.m_feed_type = request.m_feed_info.m_feed_type;
	stFeedIndexOnAppid.m_origin_feed_id = request.m_feed_info.m_origin_feed_id;
	iRet = stFeedIndexOnAppid.UpdateToDB(m_table_name_prefix_feed_index_on_appid, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedIndexOnAppid.UpdateDB failed, errmsg = " << strErrMsg);
		return;
	}
	reply.m_feed_info.m_feed_id = stFeedInfo.m_feed_id;
	return;
}

void ExecutorThread::process_feed_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_feed_delete");
	std::string strErrMsg = "";
	FeedInfo stFeedInfo = request.m_feed_info;

	
	int iRet = stFeedInfo.SelectFromDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, "not found, feed_id = " << stFeedInfo.m_feed_id);
		return;
	}
	if(stFeedInfo.m_openid_md5 != request.m_feed_info.m_openid_md5)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		LOG4CPLUS_ERROR(logger, "invalid op, feed_id = " << stFeedInfo.m_feed_id 
					<< " belong to openid = " << stFeedInfo.m_openid_md5 
					<< ", but not openid = " << request.m_feed_info.m_openid_md5);
		return;
	}
	if(stFeedInfo.m_del_ts)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FEED_DELETED;
		LOG4CPLUS_ERROR(logger, "invalid op, feed_id = " << stFeedInfo.m_feed_id << " has deleted at ts = " << stFeedInfo.m_del_ts);
		return;
	}
	
	//set del_ts
	timeval tv;
	gettimeofday(&tv, NULL);
	stFeedInfo.m_del_ts = (uint64_t)tv.tv_sec;

	iRet = stFeedInfo.UpdateToDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.UpdateToDB failed, errmsg = " << strErrMsg);
		return;
	}

	//delete record in index_table
	FeedIndexOnAppid stFeedIndexOnAppid;
	stFeedIndexOnAppid.m_pa_appid_md5 = request.m_feed_info.m_pa_appid_md5;
	stFeedIndexOnAppid.m_feed_id = request.m_feed_info.m_feed_id;
	iRet = stFeedIndexOnAppid.DeleteFromDB(m_table_name_prefix_feed_index_on_appid, m_mysql_helper, strErrMsg);

//	LOG4CPLUS_TRACE(logger, "sql = " << strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedIndexOnAppid.DeleteFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "not found, appid = " << stFeedIndexOnAppid.m_pa_appid_md5 << ", feed_id = " << stFeedIndexOnAppid.m_feed_id);
		return;
	}

	return;
}


void ExecutorThread::process_report_feed_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_report_feed_add");
	std::string strErrMsg = "";
	FeedReport stFeedReport = request.m_feed_report;
	int iRet;

	//check feed_id & owner
	FeedInfo stFeedInfo;
	stFeedInfo.m_feed_id = stFeedReport.m_feed_id;
	iRet = stFeedInfo.SelectFromDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		LOG4CPLUS_ERROR(logger, "feedid = " << stFeedInfo.m_feed_id << " not found");
		return;
	}
	if(stFeedInfo.m_openid_md5 != stFeedReport.m_openid_md5_to)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		LOG4CPLUS_ERROR(logger, "feedid = " << stFeedInfo.m_feed_id << " own by openid = " << stFeedInfo.m_openid_md5
			<< "(in db), not openid = " << stFeedReport.m_openid_md5_to << "(in req)");
		return;
	}
	
	iRet = stFeedReport.UpdateToDB(m_table_name_prefix_feed_report, m_mysql_helper, strErrMsg);
	LOG4CPLUS_TRACE(logger, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedReport.UpdateToDB failed, errmsg = " << strErrMsg);
		return;
	}

	UserForbid stUserForbid;
	stUserForbid.m_pa_appid_md5 = stFeedReport.m_pa_appid_md5;
	stUserForbid.m_openid_md5 = stFeedReport.m_openid_md5_to;
	iRet = stUserForbid.SelectFromDB(m_table_name_prefix_user_forbid, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stUserForbid.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		stUserForbid.m_until_ts = 0;
	}

	iRet = stUserForbid.UpdateToDB(m_table_name_prefix_user_forbid, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedReport.UpdateToDB failed, errmsg = " << strErrMsg);
		return;
	}
	reply.m_result_code = ExecutorThreadRequestType::E_OK;
	return;
}

void ExecutorThread::process_user_forbid_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_user_forbid_query");
	std::string strErrMsg = "";
	UserForbid stUserForbid = request.m_user_forbid;

	
	int iRet = stUserForbid.SelectFromDB(m_table_name_prefix_user_forbid, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		stUserForbid.m_until_ts = 0;
	}
	reply.m_user_forbid = stUserForbid;
	reply.m_result_code = ExecutorThreadRequestType::E_OK;

	return;
}





void ExecutorThread::process_follow_detail_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process follow detail query");

	std::string strErrMsg = "";

	FollowInfo stFollowInfo;
	reply.m_follow_list.clear();
	int iRet;
	
	for(size_t i = 0; i < request.m_follow_id_list.size(); i++)
	{
		stFollowInfo.m_follow_id = request.m_follow_id_list[i];
		iRet = stFollowInfo.SelectFromDB(m_table_name_prefix_feed_follow, m_mysql_helper, strErrMsg);
		if(iRet == TableBase::DB_RET_FAIL)
		{
			LOG4CPLUS_ERROR(logger, "stFollowInfo.SelectFromDB failed, errmsg = " << strErrMsg);
			continue;
		}
		else if(iRet == TableBase::DB_RET_NOT_EXIST)
		{
			LOG4CPLUS_TRACE(logger, "follow_id = " << stFollowInfo.m_follow_id << " not found");
			continue;
		}
		reply.m_follow_list.push_back(stFollowInfo);
	}
	
	if(reply.m_follow_list.empty())
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_TRACE(logger, "not data found ");
	}
	return;

}

void ExecutorThread::process_follow_list_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
///////////////////////////////don't need now/////////////////////////////////////////////////////
	LOG4CPLUS_ERROR(logger, "need code");
}

void ExecutorThread::process_follow_comment_detail_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_follow_comment_detail_query");

	//get comment_info & reply_list

	int iRet;
	string strErrMsg;
	FollowInfo stFollowInfo;
	FollowCommentInfo stFollowCommentInfo;
	reply.m_follow_comment_list.clear();

	LOG4CPLUS_TRACE(logger, "list size = " << request.m_comment_id_list.size());
	
	for(size_t i = 0; i < request.m_comment_id_list.size(); i++)
	{
		stFollowInfo.m_follow_id = request.m_comment_id_list[i];
		iRet = stFollowInfo.SelectFromDB(m_table_name_prefix_feed_follow, m_mysql_helper, strErrMsg);
		
		if(iRet == TableBase::DB_RET_FAIL)
		{
			LOG4CPLUS_ERROR(logger, "stFollowInfo.SelectFromDB failed, errmsg = " << strErrMsg);
			continue;
		}
		else if(iRet == TableBase::DB_RET_NOT_EXIST)
		{
			LOG4CPLUS_ERROR(logger, "follow_id = " << stFollowInfo.m_follow_id << " not found");
			continue;
		}

		stFollowCommentInfo.m_comment = stFollowInfo;
		stFollowCommentInfo.m_reply_list.clear();
		
		//get reply_list
		ReplyIndexOnOriginComment stReplyIndexOnOriginComment;
		stReplyIndexOnOriginComment.m_origin_comment_id = stFollowInfo.m_follow_id;
		iRet = stReplyIndexOnOriginComment.SelectFromDB(m_table_name_prefix_reply_index_on_origin_comment, m_mysql_helper, strErrMsg);
		if(iRet == TableBase::DB_RET_FAIL)
		{
			LOG4CPLUS_ERROR(logger, "stReplyIndexOnOriginComment.SelectFromDB failed, errmsg = " << strErrMsg);
			continue;
		}
		else if(iRet == TableBase::DB_RET_NOT_EXIST)
		{
			LOG4CPLUS_TRACE(logger, "comment_id = " << stReplyIndexOnOriginComment.m_origin_comment_id << " not reply");
		/////	continue;
		}
		
		for(size_t j = 0; j < stReplyIndexOnOriginComment.m_reply_id_list.size(); j++)
		{
			stFollowInfo.m_follow_id = stReplyIndexOnOriginComment.m_reply_id_list[j];
			stFollowInfo.SelectFromDB(m_table_name_prefix_feed_follow, m_mysql_helper, strErrMsg);
			if(iRet == TableBase::DB_RET_FAIL)
			{
				LOG4CPLUS_ERROR(logger, "stFollowInfo.SelectFromDB failed, errmsg = " << strErrMsg);
				continue;
			}
			else if(iRet == TableBase::DB_RET_NOT_EXIST)
			{
				LOG4CPLUS_TRACE(logger, "follow_id = " << stFollowInfo.m_follow_id << " not found");
				continue;
			}
			stFollowCommentInfo.m_reply_list.push_back(stFollowInfo);
		}
		
		reply.m_follow_comment_list.push_back(stFollowCommentInfo);
		
	}
	LOG4CPLUS_TRACE(logger, "list size = " << reply.m_follow_comment_list.size());
	
	if(reply.m_follow_comment_list.empty())
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_TRACE(logger, "not data found ");
	}
	return;
	
}


void ExecutorThread::process_follow_comment_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process follow comment query");

	std::string strErrMsg = "";
	CommentIndexOnFeedid stCommentIndexOnFeedid;
	stCommentIndexOnFeedid = request.m_comment_index_on_feedid;

	//select by feed id
	int iRet = stCommentIndexOnFeedid.SelectFromDB(m_table_name_prefix_comment_index_on_feedid, m_mysql_helper, strErrMsg);
	
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		return;
	}
	else if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stCommentIndexOnFeedid.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}

	//set total num
	reply.m_total = stCommentIndexOnFeedid.m_total;
	LOG4CPLUS_TRACE(logger, "total comment num = " << reply.m_total);
	
	//get comment_info & reply_list
	
	FollowInfo stFollowInfo;
	FollowCommentInfo stFollowCommentInfo;
	reply.m_follow_comment_list.clear();

	LOG4CPLUS_TRACE(logger, "list size = " << stCommentIndexOnFeedid.m_comment_id_list.size());
	
	for(size_t i = 0; i < stCommentIndexOnFeedid.m_comment_id_list.size(); i++)
	{
		stFollowInfo.m_follow_id = stCommentIndexOnFeedid.m_comment_id_list[i];
		iRet = stFollowInfo.SelectFromDB(m_table_name_prefix_feed_follow , m_mysql_helper, strErrMsg);
		
		if(iRet == TableBase::DB_RET_FAIL)
		{
			LOG4CPLUS_ERROR(logger, "stFollowInfo.SelectFromDB failed, errmsg = " << strErrMsg);
			continue;
		}
		else if(iRet == TableBase::DB_RET_NOT_EXIST)
		{
			LOG4CPLUS_ERROR(logger, "follow_id = " << stFollowInfo.m_follow_id << " not found");
			continue;
		}

		stFollowCommentInfo.m_comment = stFollowInfo;
		stFollowCommentInfo.m_reply_list.clear();
		
		//get reply_list
		ReplyIndexOnOriginComment stReplyIndexOnOriginComment;
		stReplyIndexOnOriginComment.m_origin_comment_id = stFollowInfo.m_follow_id;
		iRet = stReplyIndexOnOriginComment.SelectFromDB(m_table_name_prefix_reply_index_on_origin_comment, m_mysql_helper, strErrMsg);
		if(iRet == TableBase::DB_RET_FAIL)
		{
			LOG4CPLUS_ERROR(logger, "stReplyIndexOnOriginComment.SelectFromDB failed, errmsg = " << strErrMsg);
			continue;
		}
		else if(iRet == TableBase::DB_RET_NOT_EXIST)
		{
			LOG4CPLUS_TRACE(logger, "comment_id = " << stReplyIndexOnOriginComment.m_origin_comment_id << " not reply");
		/////	continue;
		}
		
		for(size_t j = 0; j < stReplyIndexOnOriginComment.m_reply_id_list.size(); j++)
		{
			stFollowInfo.m_follow_id = stReplyIndexOnOriginComment.m_reply_id_list[j];
			stFollowInfo.SelectFromDB(m_table_name_prefix_feed_follow , m_mysql_helper, strErrMsg);
			if(iRet == TableBase::DB_RET_FAIL)
			{
				LOG4CPLUS_ERROR(logger, "stFollowInfo.SelectFromDB failed, errmsg = " << strErrMsg);
				continue;
			}
			else if(iRet == TableBase::DB_RET_NOT_EXIST)
			{
				LOG4CPLUS_TRACE(logger, "follow_id = " << stFollowInfo.m_follow_id << " not found");
				continue;
			}
			stFollowCommentInfo.m_reply_list.push_back(stFollowInfo);
		}
		
		reply.m_follow_comment_list.push_back(stFollowCommentInfo);
		
	}
	LOG4CPLUS_TRACE(logger, "list size = " << reply.m_follow_comment_list.size());
	
	if(reply.m_follow_comment_list.empty())
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_TRACE(logger, "not data found ");
	}
	return;
	
}


void ExecutorThread::process_follow_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_follow_add");
	std::string strErrMsg = "";
	int iRet;
/*	reply to self
	if(request.m_follow_info.m_openid_md5_from == request.m_follow_info.m_openid_md5_to)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_REPLY_TO_SELF_INVALID;
		LOG4CPLUS_ERROR(logger, "'from' equal to 'to'(" << request.m_follow_info.m_openid_md5_from << "), invalid");
		return;
	}
*/	
	//get feed_info
	FeedInfo stFeedInfo;
	stFeedInfo.m_feed_id = request.m_follow_info.m_feed_id;
	iRet = stFeedInfo.SelectFromDB(m_table_name_prefix_feed , m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, "not found feed_id = " << stFeedInfo.m_feed_id);
		return;
	}
	//check del
	if(stFeedInfo.m_del_ts)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FEED_DELETED;
		LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " has deleted, del_ts = " << stFeedInfo.m_del_ts);
		return;
	}
	//topic_feed can't comment
	if(stFeedInfo.m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " is TOPIC_FEED, can't comment");
		return;
	}
	//check owner
	if(stFeedInfo.m_openid_md5 != request.m_follow_info.m_feed_owner_openid_md5)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " own by openid_md5 = " << stFeedInfo.m_openid_md5
			<< "(in db), not openid_md5 = " << request.m_follow_info.m_feed_owner_openid_md5 << "(in request)");
		return;
	}

	// origin
	FollowInfo stFollowInfo = request.m_follow_info;
	iRet = stFollowInfo.InsertToDB(m_table_name_prefix_feed_follow , m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFollowInfo.InsertDB failed, errmsg = " << strErrMsg);
		return;
	}

	////!!!!!!
	if(request.m_follow_info.m_type == hoosho::commenum::FEEDS_FOLLOW_COMMENT)
	{
		CommentIndexOnFeedid stCommentIndexOnFeedid;
		stCommentIndexOnFeedid.m_comment_id= request.m_follow_info.m_follow_id;
		stCommentIndexOnFeedid.m_feed_id = request.m_follow_info.m_feed_id;
		iRet = stCommentIndexOnFeedid.UpdateToDB(m_table_name_prefix_comment_index_on_feedid, m_mysql_helper, strErrMsg);
		if(iRet == TableBase::DB_RET_FAIL)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "stCommentIndexOnFeedid.UpdateDB failed, errmsg = " << strErrMsg);
			return;
		}
	}
	else if(request.m_follow_info.m_type == hoosho::commenum::FEEDS_FOLLOW_REPLY)
	{
		//get origin_comment
		stFollowInfo.m_follow_id = request.m_follow_info.m_origin_comment_id;
		iRet = stFollowInfo.SelectFromDB(m_table_name_prefix_feed_follow , m_mysql_helper, strErrMsg);
		if(iRet == TableBase::DB_RET_FAIL)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "stFollowInfo.SelectFromDB failed, errmsg = " << strErrMsg);
			return;
		}
		if(iRet == TableBase::DB_RET_NOT_EXIST)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
			LOG4CPLUS_ERROR(logger, "origin_comment_id = " << stFollowInfo.m_follow_id << " not found in db, req param invalid");
			return;
		}
		//check del
		if(stFollowInfo.m_del_ts)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_FEED_DELETED;
			LOG4CPLUS_ERROR(logger, "origin_comment_id = " << stFollowInfo.m_follow_id << " has deleted, del_ts = " << stFollowInfo.m_del_ts);
			return;
		}
		//check owner
		if(stFollowInfo.m_openid_md5_from != request.m_follow_info.m_origin_comment_owner_openid_md5)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
			LOG4CPLUS_ERROR(logger, "follow_id = " << stFollowInfo.m_follow_id << " own by openid_md5 = " << stFollowInfo.m_openid_md5_from
				<< "(in db), not openid_md5 = " << request.m_follow_info.m_origin_comment_owner_openid_md5 << "(in request)");
			return;
		}
		
		ReplyIndexOnOriginComment stReplyIndexOnOriginComment;
		stReplyIndexOnOriginComment.m_origin_comment_id= request.m_follow_info.m_origin_comment_id;
		stReplyIndexOnOriginComment.m_reply_id = request.m_follow_info.m_follow_id;
		iRet = stReplyIndexOnOriginComment.UpdateToDB(m_table_name_prefix_reply_index_on_origin_comment, m_mysql_helper, strErrMsg);
		if(iRet == TableBase::DB_RET_FAIL)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "stReplyIndexOnOriginComment.UpdateDB failed, errmsg = " << strErrMsg);
			return;
		}
	}
	else
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		LOG4CPLUS_ERROR(logger, "unknow follow type = " << request.m_follow_info.m_type);
		return;
	}

	//add num_comment & save
	stFeedInfo.m_num_comment++;
	iRet = stFeedInfo.UpdateToDB(m_table_name_prefix_feed , m_mysql_helper, strErrMsg);
	if(iRet != TableBase::DB_RET_OK)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.UpdateToDB failed, errmsg = " << strErrMsg);
		return;
	}
	
	reply.m_follow_info.m_follow_id = stFollowInfo.m_follow_id;
	
	return;
}


void ExecutorThread::process_follow_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_follow_delete");
	std::string strErrMsg = "";
	int iRet;
	FollowInfo stFollowInfo;
	stFollowInfo = request.m_follow_info;

	
	iRet = stFollowInfo.SelectFromDB(m_table_name_prefix_feed_follow , m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFollowInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, "not found, follow_id = " << stFollowInfo.m_follow_id);
		return;
	}
	if(stFollowInfo.m_openid_md5_from != request.m_follow_info.m_openid_md5)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		LOG4CPLUS_ERROR(logger, "invalid op, follow_id = " << stFollowInfo.m_follow_id
							<< " belong to openid = " << request.m_follow_info.m_openid_md5
							<< ", not openid = " << stFollowInfo.m_openid_md5);
		return;
	}
	//set del_ts
	timeval tv;
	gettimeofday(&tv, NULL);
	stFollowInfo.m_del_ts = (uint64_t)tv.tv_sec;

	iRet = stFollowInfo.UpdateToDB(m_table_name_prefix_feed_follow , m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFollowInfo.UpdateToDB failed, errmsg = " << strErrMsg);
		return;
	}

	//delete record in index_table
	if(stFollowInfo.m_type == hoosho::commenum::FEEDS_FOLLOW_COMMENT)
	{
		CommentIndexOnFeedid stCommentIndexOnFeedid;
		stCommentIndexOnFeedid.m_comment_id = stFollowInfo.m_follow_id;
		stCommentIndexOnFeedid.m_feed_id = stFollowInfo.m_feed_id;
		iRet = stCommentIndexOnFeedid.DeleteFromDB(m_table_name_prefix_comment_index_on_feedid, m_mysql_helper, strErrMsg);

		if(iRet == TableBase::DB_RET_FAIL)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "stCommentIndexOnFeedid.DeleteFromDB failed, errmsg = " << strErrMsg);
			return;
		}
		if(iRet == TableBase::DB_RET_NOT_EXIST)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "not found, feed id = " << stCommentIndexOnFeedid.m_feed_id 
							<< ", comment id = " << stCommentIndexOnFeedid.m_comment_id);
			return;
		}

	}
	else
	{
		ReplyIndexOnOriginComment stReplyIndexOnOriginComment;
		stReplyIndexOnOriginComment.m_origin_comment_id = stFollowInfo.m_origin_comment_id;
		stReplyIndexOnOriginComment.m_reply_id = stFollowInfo.m_follow_id;
		iRet = stReplyIndexOnOriginComment.DeleteFromDB(m_table_name_prefix_reply_index_on_origin_comment, m_mysql_helper, strErrMsg);

		if(iRet == TableBase::DB_RET_FAIL)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "stReplyIndexOnOriginComment.DeleteFromDB failed, errmsg = " << strErrMsg);
			return;
		}
		if(iRet == TableBase::DB_RET_NOT_EXIST)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "not found, comment_id = " << stReplyIndexOnOriginComment.m_origin_comment_id
						<< ", reply_id = " << stReplyIndexOnOriginComment.m_reply_id);
			return;
		}
	}


	//reduce num_comment
	FeedInfo stFeedInfo;
	stFeedInfo.m_feed_id = request.m_follow_info.m_feed_id;
	iRet = stFeedInfo.SelectFromDB(m_table_name_prefix_feed , m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, "not found feed_id = " << stFeedInfo.m_feed_id);
		return;
	}
	
	if(stFeedInfo.m_num_comment == 0)
	{
		LOG4CPLUS_ERROR(logger, "Fuck, feed_id = " << stFeedInfo.m_feed_id << ", num_comment = " << stFeedInfo.m_num_comment);
	}
	else
	{
		stFeedInfo.m_num_comment--;
		iRet = stFeedInfo.UpdateToDB(m_table_name_prefix_feed , m_mysql_helper, strErrMsg);
		if(iRet != TableBase::DB_RET_OK)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "stFeedInfo.UpdateToDB failed, errmsg = " << strErrMsg);
			return;
		}
	}
	return;	
}


void ExecutorThread::process_feed_list_favorite_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_feed_list_favorite_query");
	std::string strErrMsg = "";
	int iRet;
	reply.m_feed_list_favorite.m_openid_md5 = request.m_feed_list_favorite.m_openid_md5;
	reply.m_feed_list_favorite.m_feed_id_list.assign( request.m_feed_list_favorite.m_feed_id_list.begin(),
									request.m_feed_list_favorite.m_feed_id_list.end() );
	iRet = reply.m_feed_list_favorite.SelectFromDB(m_table_name_prefix_feed_favorite, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		LOG4CPLUS_ERROR(logger, "request.m_feed_list_favorite.SelectFromDB failed, errmsg = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		return;
	}
	reply.m_result_code = ExecutorThreadRequestType::E_OK;
	return;
}

void ExecutorThread::process_favorite_query(ExecutorThreadRequestElement & request, ExecutorThreadResponseElement & reply)
{
	LOG4CPLUS_TRACE(logger, "process favorite query");

	std::string strErrMsg = "";
	FavoriteList stFavoriteList;
	stFavoriteList = request.m_favorite_list;

	//select by feed id
	int iRet = stFavoriteList.SelectFromDB(m_table_name_prefix_feed_favorite, m_mysql_helper, strErrMsg);
	
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		return;
	}
	else if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFavoriteList.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}

	reply.m_favorite_list.m_openid_md5_list.assign( stFavoriteList.m_openid_md5_list.begin(),
								stFavoriteList.m_openid_md5_list.end() );

	reply.m_favorite_list.m_create_ts_list.assign( stFavoriteList.m_create_ts_list.begin(),
								stFavoriteList.m_create_ts_list.end() );
	
	if(reply.m_favorite_list.m_openid_md5_list.empty())
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_TRACE(logger, "not data found ");
	}
	return;
}

void ExecutorThread::process_favorite_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_favorite_add");
	std::string strErrMsg = "";
	int iRet;


	//get feed_info
	FeedInfo stFeedInfo;
	stFeedInfo.m_feed_id = request.m_favorite_info.m_feed_id;
	iRet = stFeedInfo.SelectFromDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, "not found feed_id = " << stFeedInfo.m_feed_id);
		return;
	}
	//check del
	if(stFeedInfo.m_del_ts)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FEED_DELETED;
		LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " has deleted, del_ts = " << stFeedInfo.m_del_ts);
		return;
	}
	//topic_feed can't favor
	if(stFeedInfo.m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " is TOPIC_FEED, can't favor");
		return;
	}
	//check owner
	if(stFeedInfo.m_openid_md5 != request.m_favorite_info.m_feed_owner_openid_md5)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " own by openid_md5 = " << stFeedInfo.m_openid_md5
			<< "(in db), not openid_md5 = " << request.m_follow_info.m_feed_owner_openid_md5 << "(in request)");
		return;
	}

	//get feed_index
	FeedIndexOnAppid stFeedIndexOnAppid;
	stFeedIndexOnAppid.m_feed_id = stFeedInfo.m_feed_id;
	stFeedIndexOnAppid.m_pa_appid_md5 = stFeedInfo.m_pa_appid_md5;
	iRet = stFeedIndexOnAppid.SelectFromDBByFeedid(m_table_name_prefix_feed_index_on_appid, m_mysql_helper, strErrMsg);
	if(iRet != TableBase::DB_RET_OK)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedIndexOnAppid.SelectFromDBByFeedid failed, errmsg = " << strErrMsg);
		return;
	}
	
	
	
	//record
	FavoriteInfo stFavoriteInfo = request.m_favorite_info;
	
	iRet = stFavoriteInfo.InsertToDB(m_table_name_prefix_feed_favorite, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFavoriteInfo.UpdateDB failed, errmsg = " << strErrMsg);
		return;
	}
	
	//add & save
	stFeedInfo.m_num_favorite++;
	iRet = stFeedInfo.UpdateToDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
	if(iRet != TableBase::DB_RET_OK)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.UpdateToDB failed, errmsg = " << strErrMsg);
		return;
	}

	stFeedIndexOnAppid.m_num_favorite++;
	iRet = stFeedIndexOnAppid.UpdateToDB(m_table_name_prefix_feed_index_on_appid, m_mysql_helper, strErrMsg);
	if(iRet != TableBase::DB_RET_OK)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedIndexOnAppid.UpdateToDB failed, errmsg = " << strErrMsg);
		return;
	}
	
	return;
}

void ExecutorThread::process_favorite_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_favorite_delete");
	std::string strErrMsg = "";
	FavoriteInfo stFavoriteInfo;
	stFavoriteInfo = request.m_favorite_info;
	int iRet = stFavoriteInfo.DeleteFromDB(m_table_name_prefix_feed_favorite, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFavoriteInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, "not found, feed_id = " << stFavoriteInfo.m_feed_id
						<< "openid = " << stFavoriteInfo.m_openid_md5);
		return;
	}
	//reduce num_favorite
	FeedInfo stFeedInfo;
	stFeedInfo.m_feed_id = request.m_favorite_info.m_feed_id;
	iRet = stFeedInfo.SelectFromDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, "not found feed_id = " << stFeedInfo.m_feed_id);
		return;
	}

	//get feed_index
	FeedIndexOnAppid stFeedIndexOnAppid;
	stFeedIndexOnAppid.m_feed_id = stFeedInfo.m_feed_id;
	stFeedIndexOnAppid.m_pa_appid_md5 = stFeedInfo.m_pa_appid_md5;
	iRet = stFeedIndexOnAppid.SelectFromDBByFeedid(m_table_name_prefix_feed_index_on_appid, m_mysql_helper, strErrMsg);
	if(iRet != TableBase::DB_RET_OK)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedIndexOnAppid.SelectFromDBByFeedid failed, errmsg = " << strErrMsg);
		return;
	}
	
	if(stFeedInfo.m_num_favorite == 0 || stFeedIndexOnAppid.m_num_favorite == 0)
	{
		LOG4CPLUS_ERROR(logger, "Fuck, feed_id = " << stFeedInfo.m_feed_id << ", num_favorite = " << stFeedInfo.m_num_favorite);
	}
	else
	{
		stFeedInfo.m_num_favorite--;
		iRet = stFeedInfo.UpdateToDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
		if(iRet != TableBase::DB_RET_OK)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "stFeedInfo.UpdateToDB failed, errmsg = " << strErrMsg);
			return;
		}

		stFeedIndexOnAppid.m_num_favorite--;
		iRet = stFeedIndexOnAppid.UpdateToDB(m_table_name_prefix_feed_index_on_appid, m_mysql_helper, strErrMsg);
		if(iRet != TableBase::DB_RET_OK)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "stFeedIndexOnAppid.UpdateToDB failed, errmsg = " << strErrMsg);
			return;
		}
	}	
	return;
}



void ExecutorThread::process_feed_list_collect_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_feed_list_collect_query");
	std::string strErrMsg = "";
	int iRet;
	reply.m_feed_list_collect.m_openid_md5 = request.m_feed_list_collect.m_openid_md5;
	reply.m_feed_list_collect.m_pa_appid_md5 = request.m_feed_list_collect.m_pa_appid_md5;
	reply.m_feed_list_collect.m_feed_id_list.assign( request.m_feed_list_collect.m_feed_id_list.begin(),
									request.m_feed_list_collect.m_feed_id_list.end() );
	iRet = reply.m_feed_list_collect.SelectFromDB(m_table_name_prefix_feed_collect, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		LOG4CPLUS_ERROR(logger, "request.m_feed_list_collect.SelectFromDB failed, errmsg = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		return;
	}
	reply.m_result_code = ExecutorThreadRequestType::E_OK;
	return;
}

void ExecutorThread::process_collect_query(ExecutorThreadRequestElement & request, ExecutorThreadResponseElement & reply)
{
	LOG4CPLUS_TRACE(logger, "process collect query");

	std::string strErrMsg = "";
	CollectList stCollectList;
	stCollectList = request.m_collect_list;

	//select by feed id
	int iRet = stCollectList.SelectFromDB(m_table_name_prefix_feed_collect, m_mysql_helper, strErrMsg);
	LOG4CPLUS_TRACE(logger, "sql = " << strErrMsg);
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		return;
	}
	else if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stCollectList.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	
	
	reply.m_collect_list.m_feed_id_list.assign( stCollectList.m_feed_id_list.begin(),
								stCollectList.m_feed_id_list.end() );

	reply.m_collect_list.m_create_ts_list.assign( stCollectList.m_create_ts_list.begin(),
								stCollectList.m_create_ts_list.end() );
	
	if(reply.m_collect_list.m_feed_id_list.empty())
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_TRACE(logger, "not data found ");
	}
	return;
}

void ExecutorThread::process_collect_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_collect_add");
	std::string strErrMsg = "";
	int iRet;


	//get feed_info
	FeedInfo stFeedInfo;
	stFeedInfo.m_feed_id = request.m_collect_info.m_feed_id;
	iRet = stFeedInfo.SelectFromDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, "not found feed_id = " << stFeedInfo.m_feed_id);
		return;
	}
	//check del
	if(stFeedInfo.m_del_ts)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FEED_DELETED;
		LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " has deleted, del_ts = " << stFeedInfo.m_del_ts);
		return;
	}
	//only nice_feed can collect
	if(stFeedInfo.m_feed_type != ::hoosho::commenum::FEEDS_FEED_TYPE_NICE_FEED)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		LOG4CPLUS_ERROR(logger, "feed_id = " << stFeedInfo.m_feed_id << " is NOT NICE_FEED, can't collect");
		return;
	}
	
	
	//record
	CollectInfo stCollectInfo = request.m_collect_info;
	iRet = stCollectInfo.InsertToDB(m_table_name_prefix_feed_collect, m_mysql_helper, strErrMsg);
	LOG4CPLUS_TRACE(logger, "sql = " << strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stCollectInfo.UpdateDB failed, errmsg = " << strErrMsg);
		return;
	}

	/*
	//add & save
	stFeedInfo.m_num_collect++;
	iRet = stFeedInfo.UpdateToDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
	if(iRet != TableBase::DB_RET_OK)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.UpdateToDB failed, errmsg = " << strErrMsg);
		return;
	}
	*/
	return;
}

void ExecutorThread::process_collect_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	LOG4CPLUS_TRACE(logger, "process_collect_delete");
	std::string strErrMsg = "";
	CollectInfo stCollectInfo;
	stCollectInfo = request.m_collect_info;
	int iRet = stCollectInfo.DeleteFromDB(m_table_name_prefix_feed_collect, m_mysql_helper, strErrMsg);
	LOG4CPLUS_TRACE(logger, "sql = " << strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stCollectInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, "not found, feed_id = " << stCollectInfo.m_feed_id
						<< "openid = " << stCollectInfo.m_openid_md5);
		return;
	}
	//reduce num_collect
	FeedInfo stFeedInfo;
	stFeedInfo.m_feed_id = request.m_collect_info.m_feed_id;
	iRet = stFeedInfo.SelectFromDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
	if(iRet == TableBase::DB_RET_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
		LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
		return;
	}
	if(iRet == TableBase::DB_RET_NOT_EXIST)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
		LOG4CPLUS_ERROR(logger, "not found feed_id = " << stFeedInfo.m_feed_id);
		return;
	}
	/*
	
	if(stFeedInfo.m_num_collect == 0)
	{
		LOG4CPLUS_ERROR(logger, "Fuck, feed_id = " << stFeedInfo.m_feed_id << ", num_collect = " << stFeedInfo.m_num_collect);
	}
	else
	{
		stFeedInfo.m_num_collect--;
		iRet = stFeedInfo.UpdateToDB(m_table_name_prefix_feed, m_mysql_helper, strErrMsg);
		if(iRet != TableBase::DB_RET_OK)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
			LOG4CPLUS_ERROR(logger, "stFeedInfo.UpdateToDB failed, errmsg = " << strErrMsg);
			return;
		}
	}
	*/
	return;
}

	


