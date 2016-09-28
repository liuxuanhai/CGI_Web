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
        ExecutorThreadResponseElement reply(request.m_request_type, request.m_fsm_type, request.m_fsm_id, request.m_self_user_id);

        switch(request.m_request_type)
        {
			case ExecutorThreadRequestType::T_USER_LOGIN:
				process_user_login(request, reply);
				break;				

        	case ExecutorThreadRequestType::T_GET_PHONE_VC:
        		process_get_phone_vc(request, reply);
        		break;

        	case ExecutorThreadRequestType::T_SET_PHONE:
        		process_set_phone(request, reply);
        		break;

        	case ExecutorThreadRequestType::T_UPDATE_USER_INFO:
        		process_user_info_update(request, reply);
        		break;

			case ExecutorThreadRequestType::T_GET_USER_INFO:
				process_user_info_get(request, reply);
        		break;

			case ExecutorThreadRequestType::T_UPDATE_USER_IDENTITY_INFO:
				process_user_identity_info_update(request, reply);
        		break;

			case ExecutorThreadRequestType::T_GET_USER_IDENTITY_INFO:
				process_user_identity_info_get(request, reply);
        		break;

			case ExecutorThreadRequestType::T_ADD_USER_RECV_ADDR_INFO:
				process_user_recv_info_add(request, reply);
        		break;

			case ExecutorThreadRequestType::T_DEL_USER_RECV_ADDR_INFO:
				process_user_recv_info_del(request, reply);
        		break;

			case ExecutorThreadRequestType::T_UPDATE_USER_RECV_ADDR_INFO:
				process_user_recv_info_update(request, reply);
        		break;

			case ExecutorThreadRequestType::T_GET_USER_RECV_ADDR_INFO:
				process_user_recv_info_get(request, reply);
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
    
	uint64_t qwUserId = 0;
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
    	//add info for phone && self_desc
    	UserInfoDB2PB(m_mysql_helper, reply.m_user_info);
    	qwUserId = reply.m_user_info.user_id();
    }    
    else
    {
	    qwUserId = ++max_user_id;	    
    }

	//5. update newest UserInfo to DB
	oss.str("");
	oss<<"insert into "<<m_db_table_user_info_name
		<<" set user_id="<<qwUserId
		<<", openid='"<<sql_escape(strOpenid)<<"'"		
		<<", nickname='"<<sql_escape(strNickName)<<"'"
		<<", sex="<<dwSex		
		<<", headimgurl='"<<sql_escape(strHeadImgurl)<<"'"
		<<" on duplicate key update"		
		<<" nickname='"<<sql_escape(strNickName)<<"'"
		<<", sex="<<dwSex		
		<<", headimgurl='"<<sql_escape(strHeadImgurl)<<"'";
		
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThreadUser::process_user_login failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

	//6. return to main thread
	reply.m_user_info.set_user_id(qwUserId);
	reply.m_user_info.set_openid(strOpenid);	
	reply.m_user_info.set_nickname(strNickName);
	reply.m_user_info.set_sex(dwSex);	
	reply.m_user_info.set_headimgurl(strHeadImgurl);
	
	reply.m_auth_code = request.m_snsapi_base_pre_auth_code;
	return;
}

void ExecutorThreadUser::process_get_phone_vc(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
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
	LOG4CPLUS_DEBUG(logger, "phone_vc: "<<request.m_phone<<", "<<strVC);
	
	reply.m_phone = request.m_phone;
	reply.m_phone_vc = strVC;	

	return;
}

void ExecutorThreadUser::process_set_phone(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoUserInfo stDaoUserInfo;
	if(DAO_RET_OK != stDaoUserInfo.SetUserPhone(request.m_self_user_id, request.m_phone, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	if(DAO_RET_OK != stDaoUserInfo.GetUserInfoByPrimaryKey(request.m_self_user_id, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	reply.m_user_info.CopyFrom(stDaoUserInfo.m_user_info);
	reply.m_phone = request.m_phone;
	reply.m_auth_code = request.m_snsapi_base_pre_auth_code;

	return;
}

void ExecutorThreadUser::process_user_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoUserInfo stDaoUserInfo;
	if(DAO_RET_OK != stDaoUserInfo.UpdateUserSelfDesc(request.m_self_user_id, request.m_self_desc, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	if(DAO_RET_OK != stDaoUserInfo.GetUserInfoByPrimaryKey(request.m_self_user_id, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	reply.m_auth_code = request.m_snsapi_base_pre_auth_code;
	reply.m_user_info.CopyFrom(stDaoUserInfo.m_user_info);
	return;
}

void ExecutorThreadUser::process_user_info_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoUserInfo stDaoUserInfo;
	reply.m_user_info_list.clear();
	if(DAO_RET_OK != stDaoUserInfo.GetUserInfoList(request.m_user_id_set, reply.m_user_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadUser::process_user_identity_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.down pic from wx, insert into t_pic_info
	DaoPicInfo stDaoPicInfo(m_pa_appid);
	std::string strPicid = request.m_user_identity_info.identity_pic1_id();	
	if(DAO_RET_OK != stDaoPicInfo.DownLoadAddPics(strPicid, request.m_self_user_id, PIC_TYPE_PRIVATE, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
		return;
	}	
	request.m_user_identity_info.set_identity_pic1_id(strPicid);

	strPicid = request.m_user_identity_info.identity_pic2_id();	
	if(DAO_RET_OK != stDaoPicInfo.DownLoadAddPics(strPicid, request.m_self_user_id, PIC_TYPE_PRIVATE, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
		return;
	}	
	request.m_user_identity_info.set_identity_pic2_id(strPicid);

	strPicid = request.m_user_identity_info.identity_pic3_id();	
	if(DAO_RET_OK != stDaoPicInfo.DownLoadAddPics(strPicid, request.m_self_user_id, PIC_TYPE_PRIVATE, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
		return;
	}	
	request.m_user_identity_info.set_identity_pic3_id(strPicid);

	//
	DaoUserIdentityInfo stDaoUserIdentityInfo;
	if(DAO_RET_OK != stDaoUserIdentityInfo.UpdateUserIdentityInfo(request.m_self_user_id, request.m_user_identity_info, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadUser::process_user_identity_info_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoUserIdentityInfo stDaoUserIdentityInfo;
	int iRet = stDaoUserIdentityInfo.GetUserIdentityInfo(request.m_self_user_id, reply.m_user_identity_info, m_mysql_helper);
	if(DAO_RET_NOT_EXIST == iRet)
	{
		LOG4CPLUS_DEBUG(logger, "return empty user_identity_info");
		return;
	}
	if(DAO_RET_OK != iRet)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadUser::process_user_recv_info_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	request.m_user_recv_addr_info.set_user_id(request.m_self_user_id);

	//0.get user_recv_addr_info count
	uint32_t iCount = 0;
	DaoUserRecvAddrInfo stDaoUserRecvInfo;	
	if(DAO_RET_OK != stDaoUserRecvInfo.GetUserRecvAddrInfoCount(request.m_self_user_id, iCount, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	if(iCount >=16)
	{
		LOG4CPLUS_ERROR(logger, "user_recv_addr_info of user_id=" << request.m_self_user_id << " has reach " << iCount << " too much");
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//1.insert into t_user_recv_info
	request.m_user_recv_addr_info.set_addr_num(time(NULL));
	if(DAO_RET_OK != stDaoUserRecvInfo.AddUserRecvAddrInfo(request.m_self_user_id, request.m_user_recv_addr_info, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2.set default_addr 
	if(request.m_set_default_addr)
	{
		DaoUserInfo stDaoUserInfo;
		if(DAO_RET_OK != stDaoUserInfo.UpdateUserDefaultRecvAddr(request.m_self_user_id, request.m_user_recv_addr_info.addr_num(), m_mysql_helper))
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}

	reply.m_addr_num = request.m_user_recv_addr_info.addr_num();	
	return;
}

void ExecutorThreadUser::process_user_recv_info_del(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{	
	DaoUserRecvAddrInfo stDaoUserRecvInfo;
	//1.delete t_user_recv_info
	if(DAO_RET_OK != stDaoUserRecvInfo.DelUserRecvAddrInfo(request.m_self_user_id, request.m_addr_num, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2.update t_user_info
	DaoUserInfo stDaoUserInfo;
	if(DAO_RET_OK != stDaoUserInfo.GetUserInfoByPrimaryKey(request.m_self_user_id, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	if(stDaoUserInfo.m_user_info.default_addr_num() == request.m_addr_num)
	{
		if(DAO_RET_OK != stDaoUserInfo.UpdateUserDefaultRecvAddr(request.m_self_user_id, 0, m_mysql_helper))
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}

	return;
}

void ExecutorThreadUser::process_user_recv_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	request.m_user_recv_addr_info.set_user_id(request.m_self_user_id);
	
	//1.update t_user_recv_info
	DaoUserRecvAddrInfo stDaoUserRecvInfo;
	if(DAO_RET_OK != stDaoUserRecvInfo.UpdateUserRecvAddrInfo(request.m_self_user_id, request.m_user_recv_addr_info, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	
	//3.update t_user_info
	if(request.m_set_default_addr)
	{
		DaoUserInfo stDaoUserInfo;
		if(DAO_RET_OK != stDaoUserInfo.UpdateUserDefaultRecvAddr(request.m_self_user_id, request.m_user_recv_addr_info.addr_num(), m_mysql_helper))
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}
}

void ExecutorThreadUser::process_user_recv_info_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoUserRecvAddrInfo stDaoUserRecvInfo;
	reply.m_user_recv_addr_info_list.clear();
	if(DAO_RET_OK != stDaoUserRecvInfo.GetUserRecvAddrInfoList(request.m_self_user_id, reply.m_user_recv_addr_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

