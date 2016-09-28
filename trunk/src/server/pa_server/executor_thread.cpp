#include "executor_thread.h"
#include "global_var.h"
#include "wx_api_oauth2.h"
#include "wx_api_ticket.h"
#include "wx_api_download_resource.h"

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
	db_ip = stConfig.get_string_param("DB", "ip"); 
	db_user = stConfig.get_string_param("DB", "user"); 
	db_passwd = stConfig.get_string_param("DB", "passwd"); 
	db_name = stConfig.get_string_param("DB", "db_name"); 
	db_table_pa_info_name = stConfig.get_string_param("DB", "table_pa_info_name");
	db_table_pa_tab_name = stConfig.get_string_param("DB", "table_pa_tab_name");
	db_table_pa_expert_prefix = stConfig.get_string_param("DB", "table_pa_expert_prefix");

	assert(m_mysql_helper.Init(db_ip, db_name, db_user, db_passwd));
	
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
        ExecutorThreadResponseElement reply(request.m_request_type, request.m_fsm_type, request.m_fsm_id);

        switch(request.m_request_type)
        {					
            case ExecutorThreadRequestType::T_PREAUTH:
				process_preauth(request, reply);
            	break;

			case ExecutorThreadRequestType::T_PA_TOKEN:
				process_pa_token(request, reply);
				break;	

			case ExecutorThreadRequestType::T_RESOURCE_DOWNLOAD:
				process_resource_download(request, reply);
				break;

			case ExecutorThreadRequestType::T_PA_LIST:
				process_pa_list_get(request, reply);
				break;

			case ExecutorThreadRequestType::T_PA_TAB:
				process_pa_tab_get(request, reply);
				break;			

			case ExecutorThreadRequestType::T_PA_EXPERT_QUERY:
				process_pa_expert_query(request, reply);
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



void ExecutorThread::process_preauth(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	::common::wxapi::WXUserInfo stWXUserInfo;
	::common::wxapi::WXAPIOAuth2 stWXAPIOAuth2;
	int iRet = stWXAPIOAuth2.OAuth2GetUserInfo(request.m_pa_appid
											, request.m_pa_appsecret
											, request.m_pre_auth_code
											, stWXUserInfo);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	reply.m_wx_user_info = stWXUserInfo;
	return;
}

void ExecutorThread::process_pa_token(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strPAAccessToken = "";
	::common::wxapi::WXAPI stWXAPI;
	int iRet = stWXAPI.PAGetAccessToken(request.m_pa_appid, request.m_pa_appsecret, strPAAccessToken);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		LOG4CPLUS_ERROR(logger, "fuck, PAGetAccessToken failed!");
		return;
	}
	reply.m_pa_accesstoken = strPAAccessToken;

	std::string strJSAPITicket = "";
	::common::wxapi::WXAPITicket stWXAPITicket;
	iRet = stWXAPITicket.GetJSAPITicket(strPAAccessToken, strJSAPITicket);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		LOG4CPLUS_ERROR(logger, "fuck, GetJSAPITicket failed!");
		return;
	}
	reply.m_pa_ticket_jsapi = strJSAPITicket;
	
	return;
}

void ExecutorThread::process_resource_download(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strResource;
	::common::wxapi::WXAPIDownloadResource stWXAPIDownloadResource;
	int iRet = stWXAPIDownloadResource.DownloadResource(request.m_pa_access_token, request.m_media_id, strResource);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		LOG4CPLUS_ERROR(logger, "fuck, DownloadResource failed!");
		return;
	}

	reply.m_resource_data = strResource;
	return;
}

void ExecutorThread::process_pa_list_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strUin = request.m_uin;
	std::string strTableName = db_table_pa_info_name;

	std::ostringstream oss;
	oss.str("");
	oss<<"select appid_md5 from "<<strTableName;
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_pa_list_get failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

    if(0 == m_mysql_helper.GetRowCount())
    {
    	LOG4CPLUS_ERROR(logger, "ExecutorThread::process_pa_list_get end, appid_md5 not exists");
    	reply.m_appid_md5_list.clear();
        return;
    }

    while(m_mysql_helper.Next())
    {
		reply.m_appid_md5_list.push_back(strtoul(m_mysql_helper.GetRow(0), NULL, 10));
    }

	return;
}

void ExecutorThread::process_pa_tab_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	uint64_t stAppid_md5 = request.m_pa_appid_md5;
	std::string strTableName = db_table_pa_tab_name;

	std::ostringstream oss;
	oss.str("");
	oss<<"select * from "<<strTableName<<" where appid_md5="<<stAppid_md5;
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_pa_tab_get failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

    if(0 == m_mysql_helper.GetRowCount())
    {
    	LOG4CPLUS_ERROR(logger, "ExecutorThread::process_pa_tab_get end, appid_md5 not exists");
    	reply.m_tab_info_list.clear();
        return;
    }

	reply.m_tab_info_list.clear();
    while(m_mysql_helper.Next())
    {
    	::hoosho::commstruct::TabInfo item;
    	item.set_id(strtoul(m_mysql_helper.GetRow(1), NULL, 10));
    	item.set_name(m_mysql_helper.GetRow(2));
    	item.set_extra_data_0(m_mysql_helper.GetRow(3));
    	
		reply.m_tab_info_list.push_back(item);
    }

	return;
}

void ExecutorThread::process_pa_expert_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	uint64_t stAppid_md5 = request.m_pa_appid_md5;
	uint64_t stBegin_create_ts = request.m_begin_create_ts;
	uint64_t stLimit = request.m_limit;
	
	
	std::string strTableName = "";
	UINT64_TO_TABLENAME(db_table_pa_expert_prefix, stAppid_md5, strTableName);

	std::ostringstream oss;
	oss.str("");
	if(stBegin_create_ts == 0)
	{		
		oss<<"select openid_md5 from "<<strTableName
			<<" where appid_md5="<<stAppid_md5
			<<" order by create_ts desc limit "<< stLimit;
	}
	else
	{
		oss<<"select openid_md5 from "<<strTableName
			<<" where appid_md5="<<stAppid_md5
			<<" and create_ts < "<<stBegin_create_ts
			<<" order by create_ts desc limit "<< stLimit;
	}
	LOG4CPLUS_DEBUG(logger, oss.str());
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_pa_expert_query failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

    if(0 == m_mysql_helper.GetRowCount())
    {
    	LOG4CPLUS_ERROR(logger, "ExecutorThread::process_pa_expert_query end, appid_md5 not exists");
    	reply.m_openid_md5_list.clear();
        return;
    }

	reply.m_openid_md5_list.clear();
    while(m_mysql_helper.Next())
    {    	
		reply.m_openid_md5_list.push_back(strtoul(m_mysql_helper.GetRow(0), NULL, 10));
    }

	return;
}


