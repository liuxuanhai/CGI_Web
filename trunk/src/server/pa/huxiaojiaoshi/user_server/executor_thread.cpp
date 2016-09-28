#include "executor_thread.h"
#include "global_var.h"
#include "wx_api_fans.h"
#include "wx_api_oauth2.h"
#include "proto_io_tcp_client.h"

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
	db_table_user_info_name = stConfig.get_string_param("DB", "table_user_info_name"); 

	pa_appid = stConfig.get_string_param("PA", "appid");  
	pa_appsecret = stConfig.get_string_param("PA", "appsecret");  
	
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
            case ExecutorThreadRequestType::T_USERINFO_QUERY:
				process_user_info_query(request, reply);
            	break;

			case ExecutorThreadRequestType::T_USERINFO_UPDATE:
				process_user_info_update(request, reply);
				break;	

			case ExecutorThreadRequestType::T_USER_LOGIN:
				process_user_login(request, reply);
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

static void user_info_mem_2_pb(::hoosho::j::commstruct::UserInfo& stUserInfo, const common::wxapi::WXFansInfo& stWXFansInfo)
{				
	stUserInfo.set_openid(stWXFansInfo._openid);
	stUserInfo.set_subscribe(1);
	stUserInfo.set_subscribe_time(stWXFansInfo._qwSubscribeTime);
	stUserInfo.set_nickname(stWXFansInfo._nick);
	stUserInfo.set_sex(stWXFansInfo._sex);
	stUserInfo.set_country(stWXFansInfo._country);
	stUserInfo.set_province(stWXFansInfo._province);
	stUserInfo.set_city(stWXFansInfo._city);
	stUserInfo.set_headimgurl(stWXFansInfo._portrait_url);
	stUserInfo.set_self_desc("");
}

static void user_info_db_2_pb(::hoosho::j::commstruct::UserInfo& stUserInfo, ::lce::cgi::CMysql& mysql)
{
	stUserInfo.set_openid(mysql.GetRow(0));
	stUserInfo.set_subscribe(atoi(mysql.GetRow(1)));
	stUserInfo.set_subscribe_time(strtoll(mysql.GetRow(2), NULL, 10));
	stUserInfo.set_nickname(mysql.GetRow(3));
	stUserInfo.set_sex(atoi(mysql.GetRow(4)));
	stUserInfo.set_country(mysql.GetRow(5));
	stUserInfo.set_province(mysql.GetRow(6));
	stUserInfo.set_city(mysql.GetRow(7));
	stUserInfo.set_headimgurl(mysql.GetRow(8));
	stUserInfo.set_self_desc(mysql.GetRow(9));
	/*
	mysql> desc t_user_info;
	+----------------+---------------------+------+-----+---------+-------+
	| Field          | Type                | Null | Key | Default | Extra |
	+----------------+---------------------+------+-----+---------+-------+
	| openid         | char(128)           | NO   | PRI |         |       |
	| subscribe      | bigint(20) unsigned | NO   |     | 0       |       |
	| subscribe_time | bigint(20) unsigned | NO   |     | 0       |       |
	| nickname       | tinyblob            | NO   |     | NULL    |       |
	| sex            | tinyint(3) unsigned | NO   |     | 0       |       |
	| country        | char(128)           | NO   |     |         |       |
	| province       | char(128)           | NO   |     |         |       |
	| city           | char(128)           | NO   |     |         |       |
	| headimgurl     | blob                | NO   |     | NULL    |       |
	| self_desc      | blob                | NO   |     | NULL    |       |
	+----------------+---------------------+------+-----+---------+-------+
	*/
}

void ExecutorThread::process_user_info_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	reply.m_user_info_list.clear();
	
	for(size_t i=0; i<request.m_openid_list.size(); ++i)
	{
		std::string strOneOpenid = request.m_openid_list[i];
		
		std::ostringstream oss;
		oss.str("");
		oss<<"select * from "<<db_table_user_info_name<<" where openid='"<<sql_escape(strOneOpenid)<<"'";
		
		if(!m_mysql_helper.Query(oss.str()))
	    {
	        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_info_query failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
	        continue;
	    }

	    if(0 == m_mysql_helper.GetRowCount() || !m_mysql_helper.Next())
	    {
	    	LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_info_query failed, userinfo not exists for strOneOpenid="<<strOneOpenid);
	    	continue;
	    }

		::hoosho::j::commstruct::UserInfo stUserInfo;
		user_info_db_2_pb(stUserInfo, m_mysql_helper);
		reply.m_user_info_list.push_back(stUserInfo);
	}
	
	return;	
}

void ExecutorThread::process_user_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	const ::hoosho::j::commstruct::UserInfo& stUserInfo = request.m_user_info;
	
	
	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<db_table_user_info_name
		<<" set self_desc='"<<sql_escape(stUserInfo.self_desc())<<"'"
		<<" where openid='"<<sql_escape(stUserInfo.openid())<<"'";
	LOG4CPLUS_DEBUG(logger, "sql="<<oss.str());
	
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_info_update failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

	reply.m_user_info.set_self_desc(stUserInfo.self_desc());

	return;
}

void ExecutorThread::process_user_login(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strAccessToken = "";
	std::string strRefreshToken = "";
	std::string strOpenid = "";
	std::string strScope = "";

	//1. get openid
	common::wxapi::WXAPIOAuth2 stWXAPIOAuth2;
	int iRet = stWXAPIOAuth2.OAuth2GetAccessToken(pa_appid
							 , pa_appsecret
							 , request.m_snsapi_base_pre_auth_code
							 , strAccessToken
							 , strRefreshToken
							 , strOpenid
							 , strScope);
	if(iRet < 0)
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_login failed, OAuth2GetAccessToken failed !");
        reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
        return;
	}

	//2. get pa access token
	std::string strBaseAccessToken = "";
	std::string strJSAPITiket = "";
	if(g_token_server_processor->get_pa_info(pa_appid, strBaseAccessToken, strJSAPITiket) < 0)
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_login failed, PAToken not found for strAppid="<<pa_appid );
        reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
        return;
	}

	//3. get fans info
	common::wxapi::WXAPIFans stWXAPIFans;
	common::wxapi::WXFansInfo stWXFansInfo;
	common::wxapi::WXUserInfo stWXUserInfo;
	iRet = stWXAPIFans.GetFansInfo(strBaseAccessToken, strOpenid, stWXFansInfo);
	if(iRet < 0)
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_login failed, GetFansInfo return failed ");
        reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
        return;
	}

	bool bFans = (iRet > 0);
	if(!bFans)
	{
		//not pa fans, so try another oauth2.0(snapi_scope=user_info)
		LOG4CPLUS_DEBUG(logger, "!!not pa fans, so try to GetUserInfo using 'snapi_scope=user_info', strOpenid="<<strOpenid<<", pa_appid="<<pa_appid);
		if(stWXAPIOAuth2.OAuth2GetUserInfo(strOpenid, strAccessToken, stWXUserInfo) < 0)
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_login failed, OAuth2GetUserInfo return failed ");
	        reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
	        return;
		}
	}

	std::string strNickName = bFans?stWXFansInfo._nick:stWXUserInfo._nick;
	uint32_t dwSex = bFans?stWXFansInfo._sex:stWXUserInfo._sex;
	std::string strCountry = bFans?stWXFansInfo._country:stWXUserInfo._country;
	std::string strProvince = bFans?stWXFansInfo._province:stWXUserInfo._province;
	std::string strCity = bFans?stWXFansInfo._city:stWXUserInfo._city;
	std::string strHeadImgurl = bFans?stWXFansInfo._portrait_url:stWXUserInfo._portrait_url;
	int iSubscribe = bFans? 1 : 0;
	uint64_t qwSubscribeTime = bFans? stWXFansInfo._qwSubscribeTime : 0;

	//4. update FansInfo to DB
	std::ostringstream oss;
	oss.str("");
	oss<<"insert into "<<db_table_user_info_name
		<<" set openid='"<<sql_escape(strOpenid)<<"'"
		<<", subscribe="<<iSubscribe
		<<", subscribe_time="<<qwSubscribeTime
		<<", nickname='"<<sql_escape(strNickName)<<"'"
		<<", sex="<<dwSex
		<<", country='"<<sql_escape(strCountry)<<"'"
		<<", province='"<<sql_escape(strProvince)<<"'"
		<<", city='"<<sql_escape(strCity)<<"'"
		<<", headimgurl='"<<sql_escape(strHeadImgurl)<<"'"
		<<" on duplicate key update "
		<<" subscribe="<<iSubscribe
		<<", subscribe_time="<<qwSubscribeTime
		<<", nickname='"<<sql_escape(strNickName)<<"'"
		<<", sex="<<dwSex
		<<", country='"<<sql_escape(strCountry)<<"'"
		<<", province='"<<sql_escape(strProvince)<<"'"
		<<", city='"<<sql_escape(strCity)<<"'"
		<<", headimgurl='"<<sql_escape(strHeadImgurl)<<"'";
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_login failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

	//5. return to main thread
	user_info_mem_2_pb(reply.m_user_info, stWXFansInfo);
	reply.m_user_info.set_openid(strOpenid);
	reply.m_user_info.set_subscribe(iSubscribe);
	reply.m_user_info.set_subscribe_time(qwSubscribeTime);
	reply.m_user_info.set_nickname(strNickName);
	reply.m_user_info.set_sex(dwSex);
	reply.m_user_info.set_country(strCountry);
	reply.m_user_info.set_province(strProvince);
	reply.m_user_info.set_city(strCity);
	reply.m_user_info.set_headimgurl(strHeadImgurl);
	reply.m_user_info.set_self_desc("");
	return;
}



