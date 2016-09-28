#include "executor_thread_user.h"
#include "global_var.h"
#include "wx_api_fans.h"
#include "wx_api_oauth2.h"
#include "sms_https_req.h"
#include "dao.h"
#include "common_util.h"

IMPL_LOGGER(ExecutorThreadUser, logger)

ExecutorThreadUser::ExecutorThreadUser()
{
    m_queue = NULL;
}

ExecutorThreadUser::~ExecutorThreadUser()
{
}

int ExecutorThreadUser::init(ExecutorThreadQueue * queue)
{
    m_queue = queue;

	const lce::app::Config& stConfig = g_server->config();
	m_db_ip = stConfig.get_string_param("DB", "ip"); 
	m_db_user = stConfig.get_string_param("DB", "user"); 
	m_db_passwd = stConfig.get_string_param("DB", "passwd"); 
	m_db_name = stConfig.get_string_param("DB", "db_name"); 
	m_db_table_user_info_name = stConfig.get_string_param("DB", "table_user_info_name"); 
	m_db_table_user_cash_name = stConfig.get_string_param("DB", "table_user_cash_name");
	m_db_table_user_trust_name = stConfig.get_string_param("DB", "table_user_trust_name");
	m_db_table_user_star_name = stConfig.get_string_param("DB", "table_user_star_name");
	m_db_table_order_info_name = stConfig.get_string_param("DB", "table_order_info_name");
	m_db_table_order_follow_name = stConfig.get_string_param("DB", "table_order_follow_name");
	m_db_table_comment_index_on_orderid_name = stConfig.get_string_param("DB", "table_comment_index_on_orderid_name");
	m_db_table_reply_index_on_origin_comment_name = stConfig.get_string_param("DB", "table_reply_index_on_origin_comment_name");
	m_db_table_user_activity_info_name = stConfig.get_string_param("DB", "table_user_activity_info_name");

	m_pa_appid = stConfig.get_string_param("PA", "appid");  
	m_pa_appsecret = stConfig.get_string_param("PA", "appsecret");	

	//sms param
	m_sms_api_host = stConfig.get_string_param("SMS_API", "sms_api_host");
	m_sms_api_port = stConfig.get_int_param("SMS_API", "sms_api_port");
	m_sms_api_account = stConfig.get_string_param("SMS_API", "sms_api_account");
	m_sms_api_accountpwd = stConfig.get_string_param("SMS_API", "sms_api_accountpwd");
	m_sms_api_appid = stConfig.get_string_param("SMS_API", "sms_api_appid");
	m_sms_api_version = stConfig.get_string_param("SMS_API", "sms_api_version");
	m_sms_templateid = stConfig.get_string_param("SMS_API", "sms_templateid");

	//lottery param
	m_percent_0 = stConfig.get_int_param("LOTTERY", "percent_0");
	m_percent_1 = stConfig.get_int_param("LOTTERY", "percent_1");
	m_percent_2 = stConfig.get_int_param("LOTTERY", "percent_2");

	m_prize1_count_max = stConfig.get_int_param("LOTTERY", "prize1_count_max");
	m_prize2_count_max = stConfig.get_int_param("LOTTERY", "prize2_count_max");
		
    assert(m_mysql_helper.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd));
    return 0;
}

void ExecutorThreadUser::run()
{
    LOG4CPLUS_TRACE(logger, "ExecutorThreadUser "<<pthread_self()<<" running ...");

    while(true)
    {
        if(m_queue->request_empty())
        {
            usleep(10);
            continue;
        }

        ExecutorThreadRequestElement request = m_queue->get_request();
        m_queue->pop_request();
		
        LOG4CPLUS_TRACE(logger, "ExecutorThreadUser get request "<<request.ToString());
        ExecutorThreadResponseElement reply(request.m_request_type, request.m_fsm_type, request.m_fsm_id, request.m_self_openid);

        switch(request.m_request_type)
        {
			case ExecutorThreadRequestType::T_USER_LOGIN:
				process_user_login(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_GET_PHONE_VC:
				process_user_get_phone_vc(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_SET_PHONE:
				process_user_update_phone(request, reply);
				break;

			case ExecutorThreadRequestType::T_USERINFO_UPDATE:
				process_user_info_update(request, reply);
				break;

			case ExecutorThreadRequestType::T_USERINFO_QUERY:
				process_user_info_query(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_GET_CASH:
				process_user_get_cash(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_RECOMMEND_LIST:
				process_user_recommend_list(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_TRUST:
				process_user_trust(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_TRUST_SB_CHECK:
				process_user_trust_sb_check(request, reply);
				break;				

			case ExecutorThreadRequestType::T_MY_TRUST_LIST_QUERY:
				process_my_trust_list(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_TRUST_ME_LIST_QUERY:
				process_trust_me_list(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_GET_RANKING_LIST:
				process_user_get_ranking_list(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_ACTIVITY_INFO_UPDATE:
				process_user_activity_info_update(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_ACTIVITY_INFO_QUERY:
				process_user_activity_info_query(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_GET_AVAILABLE_LOTTERY_TIME:
				process_user_get_available_lottery_time(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_DO_LOTTERY:
				process_user_do_lottery(request, reply);
				break;

			case ExecutorThreadRequestType::T_USER_GET_LOTTERY_RECORD_LIST:
				process_user_get_lottery_record_list(request, reply);
				break;

			/***************************MSG**************************/
			case ExecutorThreadRequestType::T_MSG_ADD:
				process_msg_add_new(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_MSG_GET_LIST:
				process_msg_get_list(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_MSG_DEL:
				process_msg_del(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_MSG_GET_SESSION_LIST:
				process_msg_get_session_list(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_MSG_DEL_SESSION:
				process_msg_del_session(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_MSG_GET_NEWMSG_STATUS:
				process_msg_get_newmsg_status(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_REDPOINT_GET:
				process_redpoint_get(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_REDPOINT_UPDATE:
				process_redpoint_update(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_NOTICE_ADD:
				process_notice_add(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_NOTICE_GET:
				process_notice_get(request, reply);
				break;			
				
				
			default:
				LOG4CPLUS_ERROR(logger, "ExecutorThreadUser fail, request_type: "<<request.m_request_type);
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

static void user_info_db_2_pb(::hoosho::msg::z::UserInfo& stUserInfo, ::lce::cgi::CMysql& mysql)
{
	stUserInfo.set_openid(mysql.GetRow(0));	
	stUserInfo.set_nickname(mysql.GetRow(1));
	stUserInfo.set_sex(atoi(mysql.GetRow(2)));	
	stUserInfo.set_headimgurl(mysql.GetRow(3));
	stUserInfo.set_self_desc(mysql.GetRow(4));
	stUserInfo.set_phone(mysql.GetRow(5));
	stUserInfo.set_user_type(atoi(mysql.GetRow(6)));	
	stUserInfo.set_user_flag(strtoul(mysql.GetRow(7), NULL, 10));
	stUserInfo.set_user_score(strtoul(mysql.GetRow(8), NULL, 10));
	/*
	mysql> desc t_user_info;	
	+------------+---------------------+------+-----+---------+-------+
	| Field 	 | Type 			   | Null | Key | Default | Extra |
	+------------+---------------------+------+-----+---------+-------+
	| openid	 | char(128)		   | NO   | PRI |		  | 	  |
	| nickname	 | tinyblob 		   | NO   | 	| NULL	  | 	  |
	| sex		 | tinyint(3) unsigned | NO   | 	| 0 	  | 	  |
	| headimgurl | blob 			   | NO   | 	| NULL	  | 	  |
	| self_desc  | blob 			   | NO   | 	| NULL	  | 	  |
	| phone 	 | char(32) 		   | NO   | 	|		  | 	  |
	| user_type  | tinyint(3) unsigned | NO   |     | 1       |       |
	| user_flag  | bigint(20) unsigned | NO   |     | 0       |       |
	+------------+---------------------+------+-----+---------+-------+	
	*/
}

static bool sort_by_user_score(const ::hoosho::msg::z::UserInfo& stUserInfo_1, const ::hoosho::msg::z::UserInfo& stUserInfo_2)
{
	return stUserInfo_1.user_score() > stUserInfo_2.user_score();
}

static std::string generate_yyyymmdd()
{
	struct tm *_tm;
	time_t _ts = time(NULL);
	_tm = localtime(&_ts);

	char tmp[16];
	sprintf(tmp, "%d%d%d", _tm->tm_year+1900, _tm->tm_mon, _tm->tm_mday);
	
	return tmp;
}

uint32_t ExecutorThreadUser::generate_prize(const std::string strOpenid, std::string& strLotteryNum)
{
	uint32_t iRand = common::util::GetRandNum(0, 10000);
	std::string date = generate_yyyymmdd();
	
	uint32_t iPrize = 0;
	if(iRand > 100*m_percent_0)
	{
		iPrize = 1;
	}
	if(iRand > 100*m_percent_1)
	{
		iPrize = 2;
	}
	if(iRand > 100*m_percent_2)
	{
		iPrize = 3;
	}

	//check prize available
	std::ostringstream oss;
	if(iPrize > 0)
	{		
		oss.str("");
		oss<<"select count from t_lottery_count"
		   <<" where date='"<<sql_escape(date)<<"'"
		   <<" and prize="<<iPrize;

		if(!m_mysql_helper.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "get count from t_lottery_count failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
			iPrize = 0;
		}

		uint32_t iResult = 0;
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			iResult = common::util::charToUint64_t(m_mysql_helper.GetRow(0));
			if(iPrize == 1 && iResult >= m_prize1_count_max)
			{
				LOG4CPLUS_ERROR(logger, "prize="<<iPrize<<" && lottery count="<<iResult<<" use up");
				iPrize = 0;
			}
			else if(iPrize == 2 && iResult >= m_prize2_count_max)
			{
				LOG4CPLUS_ERROR(logger, "prize="<<iPrize<<" && lottery count="<<iResult<<" use up");
				iPrize = 0;
			}
		}
		else
		{
			iPrize = 0;
		}
	}

	if(iPrize == 1)
	{		
		oss.str("");
		oss<<"select lottery_num from t_lottery_pool"
		   <<" where date='"<<sql_escape(date)<<"'"
		   <<" and status=0 limit 1";

		if(!m_mysql_helper.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "get lottery_num from t_lottery_pool failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
			iPrize = 0;
		}

		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			strLotteryNum = m_mysql_helper.GetRow(0);
		}
		else
		{
			LOG4CPLUS_DEBUG(logger, "Can not find available lottery num");
			iPrize = 0;	
		}

		if(iPrize)
		{
			oss.str("");
			oss<<"update t_lottery_pool set status=1, openid='"<<sql_escape(strOpenid)<<"'"
			   <<", create_ts=unix_timestamp() where lottery_num='"<<sql_escape(strLotteryNum)<<"'";

			if(!m_mysql_helper.Query(oss.str()))
			{
				LOG4CPLUS_ERROR(logger, "update t_lottery_pool failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
				iPrize = 0;
			}
		}
	}

	oss.str("");
	oss<<"insert into t_lottery_count"
	   <<" set date='"<<sql_escape(date)<<"'"
	   <<", prize="<<iPrize
	   <<", count=1"
	   <<" on duplicate key update"
	   <<" count=count+1";

	if(!m_mysql_helper.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "insert into t_lottery_count failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
	}

	return iPrize;
}

int ExecutorThreadUser::get_user_detail_info_list(std::set<std::string>& strOpenidSet, std::vector<hoosho::msg::z::UserInfo>& vecPBUserInfoList)
{
	std::ostringstream oss;	
	
	for(std::set<std::string>::iterator it = strOpenidSet.begin(); it != strOpenidSet.end(); it++)
	{
		std::string strOneOpenid = *it;	
		
		oss.str("");
		oss<<"select * from "<<m_db_table_user_info_name
		   <<" where openid='"<<sql_escape(strOneOpenid)<<"'";

		LOG4CPLUS_DEBUG(logger, oss.str());

		if(!m_mysql_helper.Query(oss.str()))
	    {
	        LOG4CPLUS_ERROR(logger, "ExecutorThreadUser::process_user_info_query failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
	        continue;
	    }

	    if(0 == m_mysql_helper.GetRowCount() || !m_mysql_helper.Next())
	    {
	    	LOG4CPLUS_ERROR(logger, "ExecutorThreadUser::process_user_info_query failed, userinfo not exists for strOneOpenid="<<strOneOpenid);
	    	continue;
	    }		

	    ::hoosho::msg::z::UserInfo stUserInfo;
		user_info_db_2_pb(stUserInfo, m_mysql_helper);		
		vecPBUserInfoList.push_back(stUserInfo);		    
	}

	return 0;
}

int ExecutorThreadUser::get_user_extra_info_list(std::set<std::string>& strOpenidSet, std::vector<hoosho::msg::z::UserExtraInfo>& vecPBUserExtraInfoList)
{   
	std::string strErrMsg = ""; 
	
    for(std::set<std::string>::iterator it = strOpenidSet.begin(); it != strOpenidSet.end(); it++)
    {
    	std::string strOneOpenid = *it;

    	::hoosho::msg::z::UserExtraInfo stUserExtraInfo;
		DaoUserExtraInfo stDaoUserExtraInfo(m_db_table_user_star_name, m_db_table_order_info_name);		
		if(stDaoUserExtraInfo.GetUserExtraInfo(strOneOpenid, stUserExtraInfo, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			continue;
		}	    

	    vecPBUserExtraInfoList.push_back(stUserExtraInfo);	    
    }   

    return 0;
}

int ExecutorThreadUser::get_me_trust_sb_status_list(std::string strSelfOpenid, std::set<std::string>& strThirdOpenidSet, std::vector<hoosho::msg::z::UserTrustSBStatus>& vecPBTrustStatusList)
{
	std::ostringstream oss;	
	for(std::set<std::string>::iterator it = strThirdOpenidSet.begin(); it != strThirdOpenidSet.end(); it++)
	{
		std::string strOneOpenid = *it;

		oss.str("");
		oss<<"select * from "<<m_db_table_user_trust_name
		   <<" where openid_from='"<<sql_escape(strSelfOpenid)<<"'"
		   <<" and openid_to='"<<strOneOpenid<<"'";

		LOG4CPLUS_DEBUG(logger, oss.str());

		if(!m_mysql_helper.Query(oss.str()))
	    {
	        LOG4CPLUS_ERROR(logger, "UserProcess::get_me_trust_sb_status_list failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());	        
	        continue;
	    }	

		::hoosho::msg::z::UserTrustSBStatus stUserTrustSBStatus;
		stUserTrustSBStatus.set_openid(strOneOpenid);
	    if(0 == m_mysql_helper.GetRowCount())
	    {			
			stUserTrustSBStatus.set_status(USER_TRUST_SB_STATUS_NO);	    	
		}
	    else
	    {
	    	stUserTrustSBStatus.set_status(USER_TRUST_SB_STATUS_YES);			
	    }

	    vecPBTrustStatusList.push_back(stUserTrustSBStatus);
	}

	return 0;
}

void ExecutorThreadUser::process_user_info_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.Get detail user info
	reply.m_user_info_list.clear();
	get_user_detail_info_list(request.m_openid_set, reply.m_user_info_list);

	//2.Get recommand user star
	reply.m_user_extra_info_list.clear();
	get_user_extra_info_list(request.m_openid_set, reply.m_user_extra_info_list);
	
	return;	
}

void ExecutorThreadUser::process_user_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.Update user info
	const ::hoosho::msg::z::UserInfo& stUserInfo = request.m_user_info;	
	
	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_db_table_user_info_name
		<<" set self_desc='"<<sql_escape(stUserInfo.self_desc())<<"'"
		<<" where openid='"<<sql_escape(stUserInfo.openid())<<"'";
	LOG4CPLUS_DEBUG(logger, "sql="<<oss.str());
	
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadUser::process_user_info_update failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }	
	
	//2.Get userinfo
	std::string strErrMsg = "";
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);	
	if(stDaoUserInfo.GetByPrimaryKey(stUserInfo.openid(), m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.GetByPrimaryKey failed, error: "<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
	}
	
	reply.m_user_info.CopyFrom(stDaoUserInfo.m_user_info);
	reply.m_auth_code = request.m_snsapi_base_pre_auth_code;	

	return;
}

void ExecutorThreadUser::process_user_login(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strAccessToken = "";
	std::string strRefreshToken = "";
	std::string strOpenid = "";
	std::string strScope = "";

	//1. get openid
	common::wxapi::WXAPIOAuth2 stWXAPIOAuth2;
	int iRet = stWXAPIOAuth2.OAuth2GetAccessToken(m_pa_appid
							 , m_pa_appsecret
							 , request.m_snsapi_base_pre_auth_code
							 , strAccessToken
							 , strRefreshToken
							 , strOpenid
							 , strScope);
	if(iRet < 0)
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadUser::process_user_login failed, OAuth2GetAccessToken failed !");
        reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
        return;
	}

	//2. get pa access token
	std::string strBaseAccessToken = "";
	std::string strJSAPITiket = "";
	if(g_token_server_processor->get_pa_info(m_pa_appid, strBaseAccessToken, strJSAPITiket) < 0)
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThreadUser::process_user_login failed, PAToken not found for strAppid="<<m_pa_appid );
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
		LOG4CPLUS_ERROR(logger, "ExecutorThreadUser::process_user_login failed, GetFansInfo return failed ");
        reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
        return;
	}

	bool bFans = (iRet > 0);
	if(!bFans)
	{
		//not pa fans, so try another oauth2.0(snapi_scope=user_info)
		LOG4CPLUS_DEBUG(logger, "!!not pa fans, so try to GetUserInfo using 'snapi_scope=user_info', strOpenid="<<strOpenid<<", pa_appid="<<m_pa_appid);
		if(stWXAPIOAuth2.OAuth2GetUserInfo(strOpenid, strAccessToken, stWXUserInfo) < 0)
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThreadUser::process_user_login failed, OAuth2GetUserInfo return failed ");
	        reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
	        return;
		}
	}

	std::string strNickName = bFans?stWXFansInfo._nick:stWXUserInfo._nick;
	uint32_t dwSex = bFans?stWXFansInfo._sex:stWXUserInfo._sex;	
	std::string strHeadImgurl = bFans?stWXFansInfo._portrait_url:stWXUserInfo._portrait_url;	

	//4. select old userInfo From DB	
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from "<<m_db_table_user_info_name
		<<" where openid='"<<sql_escape(strOpenid)<<"'";
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_login failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
	    return;
    }
	
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
    	//add info for phone && self_desc
    	user_info_db_2_pb(reply.m_user_info, m_mysql_helper);
    }    

	//5. update newest UserInfo to DB
	oss.str("");
	oss<<"insert into "<<m_db_table_user_info_name
		<<" set openid='"<<sql_escape(strOpenid)<<"'"		
		<<", nickname='"<<sql_escape(strNickName)<<"'"
		<<", sex="<<dwSex		
		<<", headimgurl='"<<sql_escape(strHeadImgurl)<<"'"
		<<", user_type="<<USER_TYPE_COMMON
		<<", user_flag="<<USER_FLAG_NEWUSER
		<<" on duplicate key update"		
		<<" nickname='"<<sql_escape(strNickName)<<"'"
		<<", sex="<<dwSex		
		<<", headimgurl='"<<sql_escape(strHeadImgurl)<<"'"
		<<", user_type="<<USER_TYPE_COMMON;
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadUser::process_user_login failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

	//6. return to main thread
	reply.m_user_info.set_openid(strOpenid);	
	reply.m_user_info.set_nickname(strNickName);
	reply.m_user_info.set_sex(dwSex);	
	reply.m_user_info.set_headimgurl(strHeadImgurl);
	
	reply.m_auth_code = request.m_snsapi_base_pre_auth_code;
	return;
}

void ExecutorThreadUser::process_user_get_phone_vc(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	if(request.m_phone.empty())
	{
		reply.m_result_code = ExecutorThreadRequestType::E_PARAM_ERR;
		return;
	}

	uint32_t dwNewVc = common::util::GetRandNum(100000, 999999);
	std::string strVC = int_2_str(dwNewVc);
	std::string strErrMsg = "";

	common::smsapi::SMSHttpsReq smsSend(m_sms_api_host
	 				, m_sms_api_port
	 				, m_sms_api_account
	 				, m_sms_api_accountpwd
	 				, m_sms_api_appid
	 				, m_sms_api_version
	 				, m_sms_templateid);
	if(smsSend.SMSUnifiedPost(request.m_phone, strVC, strErrMsg)< 0)
	{	      
	  LOG4CPLUS_ERROR(logger, "SendRegisterSMS failed, errmsg="<<strErrMsg);
	  reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
	  return;
	}

	reply.m_phone = request.m_phone;
	reply.m_phone_vc = strVC;

	return;
}

void ExecutorThreadUser::process_user_update_phone(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.Update phone
	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_db_table_user_info_name
		<<" set phone='"<<sql_escape(request.m_phone)<<"'"
		<<" where openid='"<<sql_escape(request.m_self_openid)<<"'";
	LOG4CPLUS_DEBUG(logger, "sql="<<oss.str());
	
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "UserProcess::process_user_update_phone failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }	

    //add user_score
	std::string strErrMsg = "";
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);	
	if(stDaoUserInfo.IncreaseUserScore(request.m_self_openid, SCORE_TYPE_PHONE_COMPLETE, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.IncreaseUserScore failed, strErrMsg="<<strErrMsg);
	}

    //2.Get userinfo	
	if(stDaoUserInfo.GetByPrimaryKey(request.m_self_openid, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.GetByPrimaryKey failed, error: "<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
	}
	
	reply.m_user_info.CopyFrom(stDaoUserInfo.m_user_info);

	reply.m_phone = request.m_phone;
    reply.m_auth_code = request.m_snsapi_base_pre_auth_code;

	return;
}

void ExecutorThreadUser::process_user_get_cash(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from "<<m_db_table_user_cash_name
	   <<" where openid='"<<sql_escape(request.m_self_openid)<<"'";

	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "UserProcess::process_user_get_cash failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }	

    if(0 == m_mysql_helper.GetRowCount() || !m_mysql_helper.Next())
    {
		//openid not exist, add new 
		oss.str("");
		oss<<"insert into "<<m_db_table_user_cash_name
		   <<" set openid='"<<sql_escape(request.m_self_openid)<<"'";

		LOG4CPLUS_DEBUG(logger, oss.str());

		if(!m_mysql_helper.Query(oss.str()))
	    {
	        LOG4CPLUS_ERROR(logger, "UserProcess::process_user_get_cash failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
	        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
	        return;
	    }	

	    reply.m_balance = 0;
	    reply.m_pending = 0;
	    return;
    }

	reply.m_balance = strtoul(m_mysql_helper.GetRow(1), NULL, 10);
	reply.m_pending = strtoul(m_mysql_helper.GetRow(2), NULL, 10);    

	return;
	
}

void ExecutorThreadUser::process_user_recommend_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{		
	//1.Get me trust openid list
	std::ostringstream oss;
	oss.str("");
	oss<<"select openid_to from "<<m_db_table_user_trust_name
	   <<" where openid_from='"<<sql_escape(request.m_self_openid)<<"'";

	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!m_mysql_helper.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "UserProcess::process_my_trust_list failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}	

	std::set<std::string> setMeTrustOpenidList;
	if(0 != m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			setMeTrustOpenidList.insert(m_mysql_helper.GetRow(0));
		}
	}

	//2.Get recommand openid list	
	reply.m_user_info_list.clear();
	std::set<std::string> strRecommandOpenidSet;	
	std::string strTempOffsetOpenid = request.m_offset_openid;
	while(reply.m_user_info_list.size() < request.m_len)
	{		
		oss.str("");
		oss<<"select * from "<<m_db_table_user_info_name
		   <<" where openid > '"<<sql_escape(strTempOffsetOpenid)<<"'"
		   <<" limit "<<request.m_len;

		LOG4CPLUS_DEBUG(logger, oss.str());

		if(!m_mysql_helper.Query(oss.str()))
	    {
	        LOG4CPLUS_ERROR(logger, "UserProcess::process_user_recommend_list failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
	        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
	        return;
	    }			
		
	    if(0 != m_mysql_helper.GetRowCount())
	    {
			while(m_mysql_helper.Next() && reply.m_user_info_list.size() < request.m_len)
			{			
				::hoosho::msg::z::UserInfo stUserInfo;
				user_info_db_2_pb(stUserInfo, m_mysql_helper);	
				strTempOffsetOpenid = stUserInfo.openid();
				
				//filter self_openid & my_trust_openid
				{
					if(stUserInfo.openid() == request.m_self_openid)
					{
						//self
						continue;
					}

					std::set<std::string>::const_iterator iter = setMeTrustOpenidList.find(stUserInfo.openid());
					if(iter != setMeTrustOpenidList.end())
					{
						//already trust					
						continue;
					}				
				}

				strRecommandOpenidSet.insert(stUserInfo.openid());	
				reply.m_user_info_list.push_back(stUserInfo);
			}
	    }    
	    else
	    {
			break;
	    }
    }
    
    //2.Get recommand user star
    reply.m_user_extra_info_list.clear();
	get_user_extra_info_list(strRecommandOpenidSet, reply.m_user_extra_info_list);   

	//3.Get recommand trust_sb_check_list
	reply.m_user_trust_sb_status_list.clear();
	get_me_trust_sb_status_list(request.m_self_openid, strRecommandOpenidSet, reply.m_user_trust_sb_status_list);

    return;
}

void ExecutorThreadUser::process_user_trust(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::ostringstream oss;

	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	std::string strErrMsg;
	int iRet = stDaoUserInfo.GetByPrimaryKey(request.m_third_openid, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, strErrMsg);
		reply.m_result_code = iRet == DAO_RET_NOT_EXIST? ExecutorThreadRequestType::E_PARAM_ERR : ExecutorThreadRequestType::E_INNER_ERR;
		return;
	}
	if(USER_TRUST_TYPE_ADD == request.m_trust_opt)
	{
		oss.str("");
		oss<<"insert into "<<m_db_table_user_trust_name
		   <<" set openid_from='"<<sql_escape(request.m_self_openid)<<"'"
		   <<", openid_to='"<<sql_escape(request.m_third_openid)<<"'"
		   <<", create_ts=unix_timestamp()"
		   <<" on duplicate key update"
		   <<" create_ts=unix_timestamp()";
	}
	else if(USER_TRUST_TYPE_CANCEL == request.m_trust_opt)
	{
		oss.str("");
		oss<<"delete from "<<m_db_table_user_trust_name
		   <<" where openid_from='"<<sql_escape(request.m_self_openid)<<"'"
		   <<" and openid_to='"<<sql_escape(request.m_third_openid)<<"'";
	}	

	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "UserProcess::process_user_trust failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }	

	//set redpoint
	DaoRedPoint stDaoRedPoint;
	if(USER_TRUST_TYPE_ADD == request.m_trust_opt)
	{
		if(stDaoRedPoint.Add(request.m_third_openid, RED_POINT_TYPE_TRUST, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "UserProcess::process_user_trust error, DaoRedPoint.Add failed, strErrMsg="<<strErrMsg);
		}
	}
	else if(USER_TRUST_TYPE_CANCEL == request.m_trust_opt)
	{
		if(stDaoRedPoint.Reduce(request.m_third_openid, RED_POINT_TYPE_TRUST, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "UserProcess::process_user_trust error, DaoRedPoint.Reduce failed, strErrMsg="<<strErrMsg);
		}
	}    

    return;
}

void ExecutorThreadUser::process_user_trust_sb_check(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	reply.m_user_trust_sb_status_list.clear();
	get_me_trust_sb_status_list(request.m_self_openid, request.m_openid_set, reply.m_user_trust_sb_status_list);		
}

void ExecutorThreadUser::process_my_trust_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strSearchOpenid = request.m_self_openid;
	if(!request.m_third_openid.empty())
		strSearchOpenid = request.m_third_openid;
		
	//1.Get me trust openid set
	std::ostringstream oss;
	oss.str("");
	oss<<"select openid_to from "<<m_db_table_user_trust_name
	   <<" where openid_from='"<<sql_escape(strSearchOpenid)<<"'"
	   <<" and openid_to>'"<<sql_escape(request.m_offset_openid)<<"'"
	   <<" limit "<<request.m_len;

	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "UserProcess::process_my_trust_list failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }	

	std::set<std::string> strMeTrustOpenidSet;
    if(0 != m_mysql_helper.GetRowCount())
    {
		while(m_mysql_helper.Next())
		{
			strMeTrustOpenidSet.insert(m_mysql_helper.GetRow(0));			
		}
	}

	//2.Get detail user info
	reply.m_user_info_list.clear();
	get_user_detail_info_list(strMeTrustOpenidSet, reply.m_user_info_list);

	//3.Get user star
    reply.m_user_extra_info_list.clear();
	get_user_extra_info_list(strMeTrustOpenidSet, reply.m_user_extra_info_list);	

	return;
}

void ExecutorThreadUser::process_trust_me_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strSearchOpenid = request.m_self_openid;
	if(!request.m_third_openid.empty())
		strSearchOpenid = request.m_third_openid;
		
	//1.Get me trust openid set
	std::ostringstream oss;
	oss.str("");
	oss<<"select openid_from from "<<m_db_table_user_trust_name
	   <<" where openid_to='"<<sql_escape(strSearchOpenid)<<"'"
	   <<" and openid_from>'"<<sql_escape(request.m_offset_openid)<<"'"
	   <<" limit "<<request.m_len;

	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "UserProcess::process_trust_me_list failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }	

	std::set<std::string> setTrustMeOpenidSet;
    if(0 != m_mysql_helper.GetRowCount())
    {
		while(m_mysql_helper.Next())
		{
			setTrustMeOpenidSet.insert(m_mysql_helper.GetRow(0));
		}
	}

	//2.Get detail user info
	reply.m_user_info_list.clear();
	get_user_detail_info_list(setTrustMeOpenidSet, reply.m_user_info_list);

	//3.Get user star
    reply.m_user_extra_info_list.clear();
	get_user_extra_info_list(setTrustMeOpenidSet, reply.m_user_extra_info_list);

	//4.�����ѯ���� "�����ҵ���"�����ѯ"��"�����Ƿ�Ҳ�Ѿ����ι��Է����������ֶ�ֵ���ظ�ǰ��
	//�����ѯ���� "����������"���򲻿���
	if(strSearchOpenid == request.m_self_openid)
	{
		reply.m_user_trust_sb_status_list.clear();
		get_me_trust_sb_status_list(request.m_self_openid, setTrustMeOpenidSet, reply.m_user_trust_sb_status_list);

		//5.clear redpoint
		DaoRedPoint stDaoRedPoint;
		std::string strErrMsg = "";
		if(stDaoRedPoint.ClearValue(request.m_self_openid, RED_POINT_TYPE_TRUST, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "UserProcess::process_trust_me_list error, DaoRedPoint.ClearValue failed, strErrMsg="<<strErrMsg);
		}
	}	

	return;
}

void ExecutorThreadUser::process_user_get_ranking_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	reply.m_user_info_list.clear();
	std::string strErrMsg;
	if(request.m_ranking_type == RANKING_TYPE_ALL)
	{
		DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
		if(stDaoUserInfo.GetRankingList(request.m_len, request.m_ranking_page, reply.m_user_info_list, m_mysql_helper, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "DaoUserInfo.GetRankingList failed, strErrMsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}
	else if(request.m_ranking_type == RANKING_TYPE_TRUST)
	{		
		//1.Get me trust openid set
		std::ostringstream oss;
		oss.str("");
		oss<<"select openid_to from "<<m_db_table_user_trust_name
		   <<" where openid_from='"<<sql_escape(request.m_self_openid)<<"'";

		LOG4CPLUS_DEBUG(logger, oss.str());

		if(!m_mysql_helper.Query(oss.str()))
	    {
	        LOG4CPLUS_ERROR(logger, "Get me trust openid set failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
	        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
	        return;
	    }	

		std::set<std::string> strMeTrustOpenidSet;
	    if(0 != m_mysql_helper.GetRowCount())
	    {
			while(m_mysql_helper.Next())
			{
				strMeTrustOpenidSet.insert(m_mysql_helper.GetRow(0));			
			}
		}

		//2.Get detail user info
		std::vector<hoosho::msg::z::UserInfo> stTmpVector;
		get_user_detail_info_list(strMeTrustOpenidSet, stTmpVector);
		sort(stTmpVector.begin(), stTmpVector.end(), sort_by_user_score);

		reply.m_user_info_list.clear();
		for(size_t i=request.m_ranking_page*request.m_len; i<stTmpVector.size(); i++)
		{
			if(i >= request.m_len)
				break;

			reply.m_user_info_list.push_back(stTmpVector[i]);
		}				
	}

	return ;
}
void ExecutorThreadUser::process_user_activity_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	request.m_user_activity_info.set_openid(request.m_self_openid);
	DaoUserActivityInfo stDaoUserActivityInfo(m_db_table_user_activity_info_name);
	int iRet;
	std::string strErrMsg;
	iRet = stDaoUserActivityInfo.GetByPrimaryKey(request.m_user_activity_info.info_id(), request.m_user_activity_info.activity_type(), m_mysql_helper, strErrMsg);
	if(iRet == DAO_RET_NOT_EXIST)
	{
		LOG4CPLUS_DEBUG(logger, "add new activity_info=" << request.m_user_activity_info.Utf8DebugString());
		stDaoUserActivityInfo.m_user_activity_info.CopyFrom(request.m_user_activity_info);
		iRet = stDaoUserActivityInfo.AddNew(m_mysql_helper, strErrMsg);
		if(iRet != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "process_user_activity_info_update failed, msg = " << strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}
	else if(iRet == DAO_RET_OK)
	{
		LOG4CPLUS_DEBUG(logger, "update activity_info=" << request.m_user_activity_info.Utf8DebugString());
		stDaoUserActivityInfo.m_user_activity_info.CopyFrom(request.m_user_activity_info);
		iRet = stDaoUserActivityInfo.Update(m_mysql_helper, strErrMsg);
		if(iRet != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "process_user_activity_info_update failed, msg = " << strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "process_user_activity_info_update failed, msg = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadUser::process_user_activity_info_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{

	DaoUserActivityInfo stDaoUserActivityInfo(m_db_table_user_activity_info_name);
	int iRet;
	std::string strErrMsg;
	iRet = stDaoUserActivityInfo.GetByPrimaryKey(request.m_user_activity_info.info_id(), request.m_user_activity_info.activity_type(), m_mysql_helper, strErrMsg);
	if(iRet == DAO_RET_NOT_EXIST)
	{
		LOG4CPLUS_ERROR(logger, strErrMsg);
		return;
	}
	else if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_user_activity_info_query failed, msg = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	reply.m_user_activity_info.CopyFrom(stDaoUserActivityInfo.m_user_activity_info);
	return;
}

void ExecutorThreadUser::process_user_get_available_lottery_time(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoLotteryInfo stDaoLotteryInfo;

	std::string strErrMsg;
	if(stDaoLotteryInfo.GetUserAvailableLotteryTime(request.m_self_openid, reply.m_available_lottery_times, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_user_get_available_lottery_time failed, errmsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadUser::process_user_do_lottery(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoLotteryInfo stDaoLotteryInfo;

	std::string strErrMsg;
	uint32_t iAvailableTime = 0;
	if(stDaoLotteryInfo.GetUserAvailableLotteryTime(request.m_self_openid, iAvailableTime, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_user_get_available_lottery_time failed, errmsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	if(iAvailableTime == 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_AVAILABLE_LOTTERY_TIME_NOT_ENOUGH;
		return;
	}

	std::string strLotteryNum;
	uint32_t iPrize = generate_prize(request.m_self_openid, strLotteryNum);
	
	if(stDaoLotteryInfo.AddNewLotteryInfo(request.m_self_openid, iPrize, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_user_do_lottery failed, errmsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	reply.m_prize = iPrize;
	reply.m_prize_desc = strLotteryNum;
	return;
}

void ExecutorThreadUser::process_user_get_lottery_record_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoLotteryInfo stDaoLotteryInfo;

	std::string strErrMsg;
	reply.m_lottery_info_list.clear();
	if(stDaoLotteryInfo.GetUserLotteryRecordList(request.m_self_openid, request.m_begin_ts, request.m_len, reply.m_lottery_info_list, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_user_get_lottery_record_list failed, errmsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadUser::process_msg_add_new(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoMsg stDaoMsg;
	stDaoMsg.m_msg_content.CopyFrom(request.m_msg_content);
	stDaoMsg.m_msg_content.set_openid_from(request.m_self_openid);
	stDaoMsg.m_msg_content.set_msg_id(common::util::generate_unique_id());	
	stDaoMsg.m_msg_content.set_create_ts(time(NULL));

	std::string strErrMsg = "";
	if(stDaoMsg.AddNewMsg(m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_msg_add_new failed, DaoMsg.AddNewMsg failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	reply.m_msg_id = stDaoMsg.m_msg_content.msg_id();
	reply.m_msg_create_ts = stDaoMsg.m_msg_content.create_ts();

	return ;
}

void ExecutorThreadUser::process_msg_get_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	uint64_t qwSessionId = common::util::generate_session_id_bystr(request.m_self_openid, request.m_msg_openid_to);
	int iDelStatus = request.m_self_openid > request.m_msg_openid_to ? MSG_DEL_STATUS_BY_MAX : MSG_DEL_STATUS_BY_MIN;

	DaoMsg stDaoMsg;
	std::string strErrMsg = "";
	// 1.get msg content list
	reply.m_msg_content_list.clear();
	int iRet = stDaoMsg.GetMsgList(qwSessionId, request.m_begin_msg_id, request.m_len, iDelStatus, reply.m_msg_content_list, m_mysql_helper, strErrMsg);
	if( iRet != DAO_RET_OK && iRet != DAO_RET_NOT_EXIST)
	{
		LOG4CPLUS_ERROR(logger, "process_msg_get_list failed, DaoMsg.GetMsgList failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	// 2.update t_user_session
	if(stDaoMsg.ClearUserSessionNewMsgStatus(request.m_self_openid, request.m_msg_openid_to, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_msg_get_list failed, DaoMsg.ClearUserSessionNewMsgStatus failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}	

	// 3.get userinfo	
	reply.m_user_info_list.clear();
	get_user_detail_info_list(request.m_openid_set, reply.m_user_info_list);
	
	return;
}

void ExecutorThreadUser::process_msg_del(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
}

void ExecutorThreadUser::process_msg_get_session_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoMsg stDaoMsg;
	std::string strErrMsg = "";

	std::set<std::string> strOpenidSet;
	// 1.get session list
	reply.m_msg_session_list.clear();
	int iRet = stDaoMsg.GetSessionList(request.m_self_openid, request.m_begin_ts, request.m_len, strOpenidSet, reply.m_msg_session_list, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK && iRet != DAO_RET_NOT_EXIST)
	{
		LOG4CPLUS_ERROR(logger, "process_msg_get_session_list failed, DaoMsg.GetSessionList failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	// 2.get userinfo
	reply.m_user_info_list.clear();
	get_user_detail_info_list(strOpenidSet, reply.m_user_info_list);

	// 3.user_extra_info_list
	reply.m_user_extra_info_list.clear();
	get_user_extra_info_list(strOpenidSet, reply.m_user_extra_info_list);

	return;
}

void ExecutorThreadUser::process_msg_del_session(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoMsg stDaoMsg;
	std::string strErrMsg = "";

	if(stDaoMsg.DelSession(request.m_self_openid, request.m_msg_openid_to, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_msg_del_session failed, DaoMsg.DelSession failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadUser::process_msg_get_newmsg_status(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoMsg stDaoMsg;
	std::string strErrMsg = "";

	if(stDaoMsg.GetNewMsgStatus(request.m_self_openid, reply.m_newmsg_status, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_msg_get_newmsg_status failed, DaoMsg.GetNewMsgStatus failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadUser::process_redpoint_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoRedPoint stDaoRedPoint;
	std::string strErrMsg = "";

	reply.m_red_point_info_list.clear();
	if(stDaoRedPoint.Get(request.m_self_openid, reply.m_red_point_info_list, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_redpoint_get failed, DaoMsg.GetRedPoint failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadUser::process_redpoint_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoRedPoint stDaoRedPoint;
	std::string strErrMsg = "";

	if(request.m_redpoint_opt == RED_POINT_OPT_TYPE_REDUCE)
	{
		if(stDaoRedPoint.Add(request.m_self_openid, request.m_redpoint_type, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "process_redpoint_update failed, DaoRedPoint.Add failed, strErrMsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}
	else if(request.m_redpoint_opt == RED_POINT_OPT_TYPE_INCREASE)
	{
		if(stDaoRedPoint.Add(request.m_self_openid, request.m_redpoint_type, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "process_redpoint_update failed, DaoRedPoint.Increase failed, strErrMsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}
	else if(request.m_redpoint_opt == RED_POINT_OPT_TYPE_CLEAR)
	{
		if(stDaoRedPoint.ClearValue(request.m_self_openid, request.m_redpoint_type, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "process_redpoint_update failed, DaoRedPoint.Clear failed, strErrMsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}

	return ;
}

void ExecutorThreadUser::process_notice_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoNoticeInfo stDaoNoticeInfo;
	stDaoNoticeInfo.m_notice_info.CopyFrom(request.m_notice_info);

	uint64_t qwTableId = common::util::generate_unique_id();
	stDaoNoticeInfo.m_notice_info.set_table_id(qwTableId);
	stDaoNoticeInfo.m_notice_info.set_status(0);		//unread
	stDaoNoticeInfo.m_notice_info.set_create_ts(time(NULL));

	std::string strErrMsg = "";
	if(stDaoNoticeInfo.Add(m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_notice_add failed, DaoMsg.Add failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return ;
}

void ExecutorThreadUser::process_notice_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoNoticeInfo stDaoNoticeInfo;
	DaoRedPoint stDaoRedPoint;

	std::string strErrMsg = "";
	// 1.get notice_info list, only support type: favorite & comment
	reply.m_notice_info_list.clear();
	int iRet = stDaoNoticeInfo.GetByType(request.m_self_openid, request.m_notice_type, request.m_begin_ts, request.m_len, 
										reply.m_notice_info_list, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_notice_get failed, DaoMsg.GetByType failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	// 2.clear red point
	if(stDaoRedPoint.ClearValue(request.m_self_openid, request.m_notice_type, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_notice_get failed, DaoRedPoint.Clear failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	/*
	 * 3.	a)type=favorite, fetch order_info_list & user_info_list
	 * 		b)type=comment,  fetch order_info_list & user_info_list & order_follow_info_list
	 */
	std::vector<uint64_t> vecOrderId;	
	for(size_t i=0; i<reply.m_notice_info_list.size(); i++)
	{
		vecOrderId.push_back(strtoul(reply.m_notice_info_list[i].extra_data_0().c_str(), NULL, 10));
	}
	reply.m_order_info_list.clear();
	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	for(size_t i=0; i<vecOrderId.size(); i++)
	{
		if(stDaoOrderInfo.GetByPrimaryKey(vecOrderId[i], m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "process_notice_get failed, DaoOrderInfo.GetByPrimaryKey failed, strErrMsg="<<strErrMsg);
			continue;
		}

		reply.m_order_info_list.push_back(stDaoOrderInfo.m_order_info);
	}
	
	if(request.m_notice_type == RED_POINT_TYPE_FAVORITE)
	{
		// 1.user_info_list
		std::set<std::string> strFavoriteOpenidSet;
		for(size_t i=0; i<reply.m_notice_info_list.size(); i++)
		{
			strFavoriteOpenidSet.insert(reply.m_notice_info_list[i].extra_data_1());
		}

		reply.m_user_info_list.clear();
		get_user_detail_info_list(strFavoriteOpenidSet, reply.m_user_info_list);

		// 2.user_extra_info_list
		reply.m_user_extra_info_list.clear();
		get_user_extra_info_list(strFavoriteOpenidSet, reply.m_user_extra_info_list);
	}
	else if(request.m_notice_type == RED_POINT_TYPE_COMMENT)
	{
		std::vector<uint64_t> vecOrderFollowId;
		for(size_t i=0; i<reply.m_notice_info_list.size(); i++)
		{
			vecOrderFollowId.push_back(strtoul(reply.m_notice_info_list[i].extra_data_2().c_str(), NULL, 10));
		}

		// 1.order_follow_info_list
		std::set<std::string> strFollowOpenidSet;
		DaoOrderFollowInfo stDaoOrderFollowInfo(m_db_table_order_follow_name, m_db_table_comment_index_on_orderid_name, m_db_table_reply_index_on_origin_comment_name);
		reply.m_order_follow_info_list.clear();
		for(size_t i=0; i<vecOrderFollowId.size(); i++)
		{
			if(stDaoOrderFollowInfo.GetByPrimaryKey(vecOrderFollowId[i], m_mysql_helper, strErrMsg) != DAO_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "process_notice_get failed, DaoOrderFollowInfo.GetByPrimaryKey failed, strErrMsg="<<strErrMsg);
				continue;
			}

			reply.m_order_follow_info_list.push_back(stDaoOrderFollowInfo.m_follow_info);
			strFollowOpenidSet.insert(stDaoOrderFollowInfo.m_follow_info.openid_from());
		}

		// 2.user_info_list
		reply.m_user_info_list.clear();
		get_user_detail_info_list(strFollowOpenidSet, reply.m_user_info_list);

		// 3.user_extra_info_list
		reply.m_user_extra_info_list.clear();
		get_user_extra_info_list(strFollowOpenidSet, reply.m_user_extra_info_list);
		
	}	

	return;
}


