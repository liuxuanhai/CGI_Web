#include "executor_thread_order.h"
#include "global_var.h"
#include "wx_api_pay.h"
#include "common_util.h"
#include "util/lce_util.h"
#include "bd_helper.h"
#include "dao.h"
#include "wx_api_message.h"
#include "wx_api_download_resource.h"

IMPL_LOGGER(ExecutorThreadOrder, logger)

ExecutorThreadOrder::ExecutorThreadOrder()
{
    m_queue = NULL;
}

ExecutorThreadOrder::~ExecutorThreadOrder()
{
}

static std::string time_2_format_str(long qwTs)
{
	long dwTs = qwTs + 60 * 60 * 8;
    struct tm *p;
    p = gmtime(&dwTs);
    char bufTs[80];
    strftime(bufTs, 80, "%Y年%m月%d日 %H:%M", p);
	return string(bufTs);
}

int ExecutorThreadOrder::init(ExecutorThreadQueue * queue)
{
    m_queue = queue;

	const lce::app::Config& stConfig = g_server->config();

	m_pa_appid = stConfig.get_string_param("PA", "appid"); 
	m_pa_pay_mchid = stConfig.get_string_param("PA", "pay_mch_id"); 
	m_pa_pay_api_key = stConfig.get_string_param("PA", "pay_api_key"); 
	m_pa_pay_ntf_url = stConfig.get_string_param("PA", "pay_ntf_url");
	m_pa_pay_reward_desc = stConfig.get_string_param("PA", "pay_reward_desc");

	m_db_ip = stConfig.get_string_param("DB", "ip"); 
	m_db_user = stConfig.get_string_param("DB", "user"); 
	m_db_passwd = stConfig.get_string_param("DB", "passwd"); 
	m_db_name = stConfig.get_string_param("DB", "db_name"); 
	m_db_table_user_info_name = stConfig.get_string_param("DB", "table_user_info_name");
	m_db_table_order_info_name = stConfig.get_string_param("DB", "table_order_info_name");
	m_db_table_order_media_info_name = stConfig.get_string_param("DB", "table_order_media_info_name");
	m_db_table_order_fetch_state_name = stConfig.get_string_param("DB", "table_order_fetch_state_name");
	m_db_table_order_reward_name = stConfig.get_string_param("DB", "table_order_reward_name");
	m_db_table_user_trust_name = stConfig.get_string_param("DB", "table_user_trust_name");
	m_db_table_wx_outtrade_name = stConfig.get_string_param("DB", "table_wx_outtrade_name");
	m_db_table_user_cash_name = stConfig.get_string_param("DB", "table_user_cash_name");
	m_db_table_cash_flow_name = stConfig.get_string_param("DB", "table_cash_flow_name");
	m_db_table_out_come_name  = stConfig.get_string_param("DB", "table_out_come_name");
	m_db_table_user_star_name = stConfig.get_string_param("DB", "table_user_star_name");
	m_db_table_order_favor_name = stConfig.get_string_param("DB", "table_order_favor_name");
	m_db_table_order_follow_name = stConfig.get_string_param("DB", "table_order_follow_name");
	m_db_table_comment_index_on_orderid_name = stConfig.get_string_param("DB", "table_comment_index_on_orderid_name");
	m_db_table_reply_index_on_origin_comment_name = stConfig.get_string_param("DB", "table_reply_index_on_origin_comment_name");

    assert(m_mysql_helper.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd));

	m_bd_api_ak = stConfig.get_string_param("BD_API", "bd_api_ak");
	m_bd_api_sk = stConfig.get_string_param("BD_API", "bd_api_sk");
	m_bd_api_geotable_id = stConfig.get_int_param("BD_API", "bd_api_geotable_id");
	m_bd_api_geosearch_radius = stConfig.get_int_param("BD_API", "bd_api_geosearch_radius");

	m_order_fetch_template_id = stConfig.get_string_param("TEMPLATE_ORDER_FETCH", "template_id");
    m_order_fetch_color = stConfig.get_string_param("TEMPLATE_ORDER_FETCH", "color");
    m_order_fetch_url = stConfig.get_string_param("TEMPLATE_ORDER_FETCH", "url");
    m_order_fetch_first = stConfig.get_string_param("TEMPLATE_ORDER_FETCH", "first");
    m_order_fetch_keyword1 = stConfig.get_string_param("TEMPLATE_ORDER_FETCH", "keyword1");
    m_order_fetch_keyword2 = stConfig.get_string_param("TEMPLATE_ORDER_FETCH", "keyword2");
    m_order_fetch_keyword3 = stConfig.get_string_param("TEMPLATE_ORDER_FETCH", "keyword3");
    m_order_fetch_remark = stConfig.get_string_param("TEMPLATE_ORDER_FETCH", "remark");
	m_order_fetch_waiting_user_size_first = stConfig.get_int_param("TEMPLATE_ORDER_FETCH", "waiting_user_size_first");
	m_order_fetch_waiting_user_size_second = stConfig.get_int_param("TEMPLATE_ORDER_FETCH", "waiting_user_size_second");
	m_order_fetch_waiting_user_size_third = stConfig.get_int_param("TEMPLATE_ORDER_FETCH", "waiting_user_size_third");

	m_order_fetch_accept_template_id = stConfig.get_string_param("TEMPLATE_ORDER_FETCH_ACCEPT", "template_id");
    m_order_fetch_accept_color = stConfig.get_string_param("TEMPLATE_ORDER_FETCH_ACCEPT", "color");
    m_order_fetch_accept_url = stConfig.get_string_param("TEMPLATE_ORDER_FETCH_ACCEPT", "url");
    m_order_fetch_accept_first = stConfig.get_string_param("TEMPLATE_ORDER_FETCH_ACCEPT", "first");
    m_order_fetch_accept_keyword1 = stConfig.get_string_param("TEMPLATE_ORDER_FETCH_ACCEPT", "keyword1");
    m_order_fetch_accept_keyword2 = stConfig.get_string_param("TEMPLATE_ORDER_FETCH_ACCEPT", "keyword2");
    m_order_fetch_accept_keyword3 = stConfig.get_string_param("TEMPLATE_ORDER_FETCH_ACCEPT", "keyword3");
    m_order_fetch_accept_remark = stConfig.get_string_param("TEMPLATE_ORDER_FETCH_ACCEPT", "remark");
	m_order_fetch_accept_order_desc_of_audio = stConfig.get_string_param("TEMPLATE_ORDER_FETCH_ACCEPT", "order_desc_of_audio");

	m_order_finish_template_id = stConfig.get_string_param("TEMPLATE_ORDER_FINISH", "template_id");
    m_order_finish_color = stConfig.get_string_param("TEMPLATE_ORDER_FINISH", "color");
    m_order_finish_url = stConfig.get_string_param("TEMPLATE_ORDER_FINISH", "url");
    m_order_finish_first = stConfig.get_string_param("TEMPLATE_ORDER_FINISH", "first");
    m_order_finish_keyword1 = stConfig.get_string_param("TEMPLATE_ORDER_FINISH", "keyword1");
    m_order_finish_keyword2 = stConfig.get_string_param("TEMPLATE_ORDER_FINISH", "keyword2");
    m_order_finish_keyword3 = stConfig.get_string_param("TEMPLATE_ORDER_FINISH", "keyword3");
    m_order_finish_remark = stConfig.get_string_param("TEMPLATE_ORDER_FINISH", "remark");

    return 0;
}


void ExecutorThreadOrder::run()
{
    LOG4CPLUS_TRACE(logger, "ExecutorThreadOrder "<<pthread_self()<<" running ...");

    while(true)
    {
        if(m_queue->request_empty())
        {
            usleep(10);
            continue;
        }

        ExecutorThreadRequestElement request = m_queue->get_request();
        m_queue->pop_request();
		
        LOG4CPLUS_TRACE(logger, "ExecutorThreadOrder get request "<<request.ToString());
        ExecutorThreadResponseElement reply(request.m_request_type, request.m_fsm_type, request.m_fsm_id, request.m_self_openid);

        switch(request.m_request_type)
        {
			case ExecutorThreadRequestType::T_CASH_FLOW_QUERY:
				process_cash_flow_query(request, reply);
				break;

			case ExecutorThreadRequestType::T_OUTCOME_COMMIT:
				process_outcome_commit(request, reply);
				break;

			case ExecutorThreadRequestType::T_OUTCOME_LIST:
				process_outcome_list(request, reply);
				break;



			case ExecutorThreadRequestType::T_ORDER_CREATE:
				process_order_create(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_PAY_SUCC:
				process_order_payed(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_FETCH:
				process_order_fetch(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_FETCH_CANCEL:
				process_order_fetch_canceled(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_FETCH_ACCEPT:
				process_order_fetch_accepted(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_SEARCH:
				process_order_search(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_CANCEL:
				process_order_cancel(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_FINISH:
				process_order_finish(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_ORDER_COMMENT:
				process_order_comment(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_ORDER_INFO_QUERY:
				process_order_info_query(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_ORDER_LIST_USER_FETCHED:
				process_order_list_user_fetched(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_LIST_USER_CREATED:
				process_order_list_user_created(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_REWARD:
				process_order_reward(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_FAVOR_CHECK:
				process_order_favor_check(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_FAVOR_UPDATE:
				process_order_favor_update(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_FAVOR_LIST:
				process_order_favor_list(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_FOLLOW_ADD:
				process_order_follow_add(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_FOLLOW_DEL:
				process_order_follow_del(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_COMMENT_LIST:
				process_order_comment_list(request, reply);
				break;

			case ExecutorThreadRequestType::T_ORDER_COMMENT_DETAIL:
				process_order_comment_detail(request, reply);
				break;

			default:
				LOG4CPLUS_ERROR(logger, "ExecutorThreadOrder fail, request_type: "<<request.m_request_type);
				reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
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


void ExecutorThreadOrder::process_cash_flow_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoCashFlowList stDaoCashFlowList(m_db_table_cash_flow_name);
	std::vector<hoosho::msg::z::CashFlowInfo> vecCashFlowInfo;
	int iRet = stDaoCashFlowList.GetByTsOrder(request.m_self_openid, request.m_offset, request.m_len, m_mysql_helper, strErrMsg, vecCashFlowInfo);
	if(iRet != DAO_RET_OK && iRet != DAO_RET_NOT_EXIST)
	{
		LOG4CPLUS_ERROR(logger, "process_cash_flow_query failed, DaoCashFlowList.GetByTsOrder failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	reply.m_cash_flow_list.clear();
	for(size_t i=0; i!=vecCashFlowInfo.size(); ++i)
	{
		reply.m_cash_flow_list.push_back(vecCashFlowInfo[i]);
	}

	return;
}

void ExecutorThreadOrder::process_outcome_commit(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoOutcome stDAOOutCome(m_db_table_out_come_name, m_db_table_user_cash_name, m_db_table_cash_flow_name);
	if(stDAOOutCome.Commit(request.m_self_openid, request.m_amount, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_outcome_commit failed, DAOOutCome.Commit failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	DAOUserCash stDAOUserCash(m_db_table_user_cash_name, m_db_table_cash_flow_name);
	if(stDAOUserCash.GetByPrimaryKey(request.m_self_openid, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_outcome_commit failed, DAOUserCash.GetByPrimaryKey failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	reply.m_balance = stDAOUserCash.m_balance;
	reply.m_pending = stDAOUserCash.m_pending;
	return;
}

void ExecutorThreadOrder::process_outcome_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoOutcomeList stDaoOutcomeList(m_db_table_out_come_name, m_db_table_user_cash_name, m_db_table_cash_flow_name);
	std::vector<hoosho::msg::z::OutcomeReqInfo> vecOutcomeReqInfo;
	int iRet = stDaoOutcomeList.GetByStateWithTsOrder(request.m_self_openid
											, request.m_outcome_state
											, request.m_offset
											, request.m_len
											, m_mysql_helper
											, strErrMsg
											, vecOutcomeReqInfo);
	if(iRet != DAO_RET_OK && iRet != DAO_RET_NOT_EXIST)
	{
		LOG4CPLUS_ERROR(logger, "process_outcome_list failed, DaoOutcomeList.GetByStateWithTsOrder failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	reply.m_outcome_req_list.clear();
	for(size_t i=0; i!=vecOutcomeReqInfo.size(); ++i)
	{
		reply.m_outcome_req_list.push_back(vecOutcomeReqInfo[i]);
	}

	return;
}

void ExecutorThreadOrder::process_order_create(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	time_t now = time(0);
	
	std::string strErrMsg = "";
	uint64_t qwOrderId = common::util::generate_unique_id();

	reply.m_order_info.set_order_id(qwOrderId);

	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	stDaoOrderInfo.m_order_info.CopyFrom(request.m_order_info);
	stDaoOrderInfo.m_order_info.set_order_id(qwOrderId);
	stDaoOrderInfo.m_order_info.set_openid_master(request.m_self_openid);
	stDaoOrderInfo.m_order_info.set_create_ts(now);	
	stDaoOrderInfo.m_order_info.set_finish_ts(0);
	stDaoOrderInfo.m_order_info.set_fetch_ts(0);
	stDaoOrderInfo.m_order_info.set_cancel_ts(0);

	reply.m_reward_fee = stDaoOrderInfo.CreateRandomRewardFeeForNewUser(request.m_self_openid , m_db_table_user_info_name, m_mysql_helper, strErrMsg);
	if(!strErrMsg.empty())
	{
		LOG4CPLUS_ERROR(logger, "process_order_create error, DaoOrderInfo.CreateRandomRewardFeeForNewUser strErrMsg="<<strErrMsg);
	}
	
	if(ORDER_MEDIA_TYPE_AUDIO == stDaoOrderInfo.m_order_info.media_type())
	{
		std::string strAppid = g_server->config().get_string_param("PA", "appid");
		std::string strBaseAccessToken = "";
		std::string strJSAPITicket = "";
		if(g_token_server_processor->get_pa_info(strAppid, strBaseAccessToken, strJSAPITicket) < 0)
		{
			LOG4CPLUS_FATAL(logger, "process_order_create failed, get_pa_info failed, strAppid="<<strAppid);
			reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
			return;
		}

		std::string strResource = "";
		common::wxapi::WXAPIDownloadResource stWXAPIDownloadResource;
		if(stWXAPIDownloadResource.DownloadResource(strBaseAccessToken, request.m_media_server_id, strResource) < 0)
		{
			LOG4CPLUS_ERROR(logger, "process_order_create failed, DownloadResource failed, media_server_id="<<request.m_media_server_id);
			reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
			return;
		}

		DaoOrderMediaInfo stDaoOrderMediaInfo(m_db_table_order_media_info_name);
		stDaoOrderMediaInfo.m_orderId = qwOrderId;
		stDaoOrderMediaInfo.m_media_type = ORDER_MEDIA_TYPE_AUDIO;
		stDaoOrderMediaInfo.m_media_content = strResource;
		if(DAO_RET_OK != stDaoOrderMediaInfo.AddOrUpdate(qwOrderId
											, ORDER_MEDIA_TYPE_AUDIO
											, strResource
											, m_mysql_helper
											, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "process_order_create failed, DaoOrderMediaInfo.AddOrUpdate failed, errmsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}
	else if(ORDER_MEDIA_TYPE_PICTURE == stDaoOrderInfo.m_order_info.media_type())
	{
		std::string strAppid = g_server->config().get_string_param("PA", "appid");
		std::string strBaseAccessToken = "";
		std::string strJSAPITicket = "";
		if(g_token_server_processor->get_pa_info(strAppid, strBaseAccessToken, strJSAPITicket) < 0)
		{
			LOG4CPLUS_FATAL(logger, "process_order_create failed, get_pa_info failed, strAppid="<<strAppid);
			reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
			return;
		}

		std::string strResource = "";
		common::wxapi::WXAPIDownloadResource stWXAPIDownloadResource;
		if(stWXAPIDownloadResource.DownloadResource(strBaseAccessToken, request.m_media_server_id, strResource) < 0)
		{
			LOG4CPLUS_ERROR(logger, "process_order_create failed, DownloadResource failed, media_server_id="<<request.m_media_server_id);
			reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
			return;
		}

		DaoOrderMediaInfo stDaoOrderMediaInfo(m_db_table_order_media_info_name);
		stDaoOrderMediaInfo.m_orderId = qwOrderId;
		stDaoOrderMediaInfo.m_media_type = ORDER_MEDIA_TYPE_PICTURE;
		stDaoOrderMediaInfo.m_media_content = strResource;
		if(DAO_RET_OK != stDaoOrderMediaInfo.AddOrUpdate(qwOrderId
											, ORDER_MEDIA_TYPE_PICTURE
											, strResource
											, m_mysql_helper
											, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "process_order_create failed, DaoOrderMediaInfo.AddOrUpdate failed, errmsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}

	
	DAOUserCash stDaoUserCash(m_db_table_user_cash_name, m_db_table_cash_flow_name);
	if(DAO_RET_OK == stDaoUserCash.ReduceBalance(request.m_self_openid
							, request.m_order_info.price()
							, CASH_FLOW_TYPE_CONSUME_ORDER_CREATE
							, m_mysql_helper
							, strErrMsg
							, int_2_str(stDaoOrderInfo.m_order_info.order_id())))
	{
		//type A, user cash
		reply.m_cost_type = ORDER_PAY_COST_TYPE_USER_CASH;

		//add new order to db
		stDaoOrderInfo.m_order_info.set_order_status(ORDER_STATUS_WAITING_FETCH);
		stDaoOrderInfo.m_order_info.set_pay_status(ORDER_PAY_STATUS_PAYED);
		stDaoOrderInfo.m_order_info.set_pay_ts(now);
		if(stDaoOrderInfo.AddNew(m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_create failed, DaoOrderInfo.AddNew failed, strErrMsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}

		//add poi to baidu
		if(stDaoOrderInfo.m_order_info.order_type() != ORDER_TYPE_INVOICE)
		{
			double ffLongi= stDaoOrderInfo.m_order_info.addr_to_longitude();
			double ffLati = stDaoOrderInfo.m_order_info.addr_to_latitude();
			BDHelper stBDHelper(m_bd_api_ak, m_bd_api_sk, m_bd_api_geotable_id);
			if(stBDHelper.CreatePoi(ffLongi / 1000000, ffLati / 1000000, qwOrderId, stDaoOrderInfo.m_order_info.order_type()) < 0)
			{
				LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_create failed, BDHelper.CreatePoi failed");
				reply.m_result_code = ExecutorThreadRequestType::E_BAIDU_ERR;
				return;
			}
		}

		//add user_score
		DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
		if(stDaoUserInfo.IncreaseUserScore(request.m_self_openid, SCORE_TYPE_ORDER_CREATE + request.m_order_info.price(), m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "process_order_create failed, DaoUserInfo.IncreaseUserScore failed, strErrMsg="<<strErrMsg);
		}

		return;
	}
	
	//type B, wx pay again
	reply.m_cost_type = ORDER_PAY_COST_TYPE_WX_PAY;

	//B1. unified order
	std::string strPrepayId = "";
	common::wxapi::WXAPIPAY stWXAPIPAY;
	int iRet = stWXAPIPAY.PaPayUnifiedOrder(m_pa_appid
						, m_pa_pay_mchid
						, m_pa_pay_api_key
						, request.m_order_info.order_desc()
						, lce::util::StringOP::TypeToStr(qwOrderId)
						, request.m_order_info.price()
						, request.m_user_ip
						, m_pa_pay_ntf_url
						, request.m_self_openid
						, strPrepayId);
	if(iRet < 0)
	{
		LOG4CPLUS_ERROR(logger, "process_order_create failed, PaPayUnifiedOrder failed");
		reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
		return;
	}

	DAOWXOuttrade stDAOWXOuttrade(m_db_table_wx_outtrade_name);
	stDAOWXOuttrade.m_openid = request.m_self_openid;
	stDAOWXOuttrade.m_outtrade_no = int_2_str(qwOrderId);
	stDAOWXOuttrade.m_prepay_id = strPrepayId;
	stDAOWXOuttrade.m_body = request.m_order_info.order_desc();
	stDAOWXOuttrade.m_detail = "";
	stDAOWXOuttrade.m_attach = "";
	stDAOWXOuttrade.m_goods_tag = "";
	stDAOWXOuttrade.m_spbill_create_ip = request.m_user_ip;
	stDAOWXOuttrade.m_time_start = common::wxapi::WXAPIPAY::GenTimeStringFromNow_yyyyMMddHHmmss(0);
	stDAOWXOuttrade.m_time_expire = common::wxapi::WXAPIPAY::GenTimeStringFromNow_yyyyMMddHHmmss(600);
	stDAOWXOuttrade.m_trade_type = "JSAPI";
	stDAOWXOuttrade.m_product_id = "";
	stDAOWXOuttrade.m_limit_pay = "no_credit";
	stDAOWXOuttrade.m_appid = m_pa_appid;
	stDAOWXOuttrade.m_mch_id = m_pa_pay_mchid;
	stDAOWXOuttrade.m_device_info = "WEB";
	stDAOWXOuttrade.m_fee_type = "CNY";
	stDAOWXOuttrade.m_total_fee = request.m_order_info.price();
	stDAOWXOuttrade.m_pay_state = WX_UNINFIED_ORDER_PAY_STATE_WAITING_PAY;
	stDAOWXOuttrade.m_pay_reason = WX_UNINFIED_ORDER_PAY_REASON_CREATE_ORDER;
	if(stDAOWXOuttrade.AddNew(m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_create failed, DAOWXOuttrade.AddNew failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2. add new order to DB
	stDaoOrderInfo.m_order_info.set_order_status(ORDER_STATUS_INIT);
	stDaoOrderInfo.m_order_info.set_pay_status(ORDER_PAY_STATUS_NOT_PAYED_YET);
	stDaoOrderInfo.m_order_info.set_pay_ts(0);
	if(stDaoOrderInfo.AddNew(m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_create failed, DaoOrderInfo.AddNew failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	
	reply.m_wx_unified_order_param.set_appid(m_pa_appid);
	reply.m_wx_unified_order_param.set_timestamp(int_2_str(now));
	reply.m_wx_unified_order_param.set_noncestr(common::wxapi::WXAPIPAY::GenRandomString(32));
	reply.m_wx_unified_order_param.set_package("prepay_id=" + strPrepayId);
	reply.m_wx_unified_order_param.set_signtype("MD5");
	
	std::map<std::string, std::string> tmp_map;						
	tmp_map.insert(pair<std::string, std::string>("appId", reply.m_wx_unified_order_param.appid()));
	tmp_map.insert(pair<std::string, std::string>("timeStamp", reply.m_wx_unified_order_param.timestamp()));
	tmp_map.insert(pair<std::string, std::string>("nonceStr", reply.m_wx_unified_order_param.noncestr()));
	tmp_map.insert(pair<std::string, std::string>("package",  reply.m_wx_unified_order_param.package()));
	tmp_map.insert(pair<std::string, std::string>("signType", reply.m_wx_unified_order_param.signtype()));
	reply.m_wx_unified_order_param.set_paysign(common::wxapi::WXAPIPAY::Params2Sign(tmp_map, m_pa_pay_api_key));
	
	return;
}

void ExecutorThreadOrder::process_order_payed(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strPaAppid = "";
	std::string strMachId = "";
	std::string strUserOpenid = "";
	std::string strOuttradeNO = "";
	std::string strEndTime = "";
	int iTotalFee = 0;

	//1, parse && check
	common::wxapi::WXAPIPAY stWXAPIPAY;
	if(stWXAPIPAY.PaPayResultCallBackParse(request.m_wx_pay_callback_xml
								, m_pa_pay_api_key
								, strPaAppid
								, strMachId
								, strUserOpenid
								, iTotalFee
								, strOuttradeNO
								, strEndTime) < 0)
	{
		LOG4CPLUS_ERROR(logger, "process_order_payed failed, WXAPIPAY.PaPayResultCallBackParse");
		reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
		return;
	}

	//2, compare with our outtrade
	DAOWXOuttrade stDAOWXOuttrade(m_db_table_wx_outtrade_name);
	std::string strErrMsg = "";
	if(stDAOWXOuttrade.GetByPrimaryKey(strUserOpenid, strOuttradeNO, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_payed failed, DAOWXOuttrade.GetByPrimaryKey failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	if(strPaAppid != stDAOWXOuttrade.m_appid
		|| strMachId != stDAOWXOuttrade.m_mch_id
		|| strUserOpenid != stDAOWXOuttrade.m_openid
		|| strOuttradeNO != stDAOWXOuttrade.m_outtrade_no
		|| (uint64_t)iTotalFee != stDAOWXOuttrade.m_total_fee)
	{
		LOG4CPLUS_ERROR(logger, "process_order_payed failed, callback.params != unifiedorder.params");
		reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
		return;
	}

	//3. update uninified order 
	if(stDAOWXOuttrade.UpdatePayState(strUserOpenid, strOuttradeNO, WX_UNINFIED_ORDER_PAY_STATE_SUCC_PAY, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_payed failed, DAOWXOuttrade.UpdatePayState failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	if(stDAOWXOuttrade.m_pay_reason == WX_UNINFIED_ORDER_PAY_REASON_CREATE_ORDER)
	{
		uint64_t qwOrderId = strtoul(strOuttradeNO.c_str(), NULL, 10);
		LOG4CPLUS_DEBUG(logger, "order_id="<<qwOrderId<<", succ payed on "<<strEndTime);
		
		//4. update order info
		DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
		if(stDaoOrderInfo.Payed(qwOrderId, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "process_order_payed failed, DaoOrderInfo.Payed failed, strErrMsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}

		//5. add,reduce user cash
		DAOUserCash stDAOUserCash(m_db_table_user_cash_name, m_db_table_cash_flow_name);
		if(stDAOUserCash.AddBalance(strUserOpenid, iTotalFee, CASH_FLOW_TYPE_RECHARGE, m_mysql_helper, strErrMsg, strOuttradeNO) != DAO_RET_OK)
		{
			LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_payed, DAOUserCash.AddBalance failed, strErrMsg="<<strErrMsg);
		}
		if(stDAOUserCash.ReduceBalance(strUserOpenid, iTotalFee, CASH_FLOW_TYPE_CONSUME_ORDER_CREATE, m_mysql_helper, strErrMsg))
		{
			LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_payed, DAOUserCash.ReduceBalance failed, strErrMsg="<<strErrMsg);
		}

		//6. if newuser pay for first time, finish reward fee 
		if(stDaoOrderInfo.FinishRewardFeeForNewUser(strUserOpenid, m_db_table_user_info_name, m_db_table_user_cash_name, m_db_table_cash_flow_name, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_payed, DaoOrderInfo.FinishRewardFeeForNewUser failed, strErrMsg="<<strErrMsg);
		}

		//7. add POI to BAIDU Map
		if(stDaoOrderInfo.GetByPrimaryKey(qwOrderId, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "process_order_payed failed, DaoOrderInfo.GetByPrimaryKey failed, strErrMsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}

		if(stDaoOrderInfo.m_order_info.order_type() != ORDER_TYPE_INVOICE)
		{
			double ffLati = stDaoOrderInfo.m_order_info.addr_to_latitude();
			double ffLongi = stDaoOrderInfo.m_order_info.addr_to_longitude();
			BDHelper stBDHelper(m_bd_api_ak, m_bd_api_sk, m_bd_api_geotable_id);
			if(stBDHelper.CreatePoi(ffLongi / 1000000, ffLati / 1000000, qwOrderId, stDaoOrderInfo.m_order_info.order_type()) < 0)
			{
				LOG4CPLUS_ERROR(logger, "process_order_payed failed, BDHelper.CreatePoi failed");
				reply.m_result_code = ExecutorThreadRequestType::E_BAIDU_ERR;
				return;
			}
		}

		//8. add user_score
		DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
		if(stDaoUserInfo.IncreaseUserScore(strUserOpenid, SCORE_TYPE_ORDER_CREATE + iTotalFee, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoUserInfo.IncreaseUserScore failed, strErrMsg="<<strErrMsg);
		}
	}
	else if(stDAOWXOuttrade.m_pay_reason == WX_UNINFIED_ORDER_PAY_REASON_REWARD_ORDER)
	{
		LOG4CPLUS_DEBUG(logger, "outtrade_no="<<stDAOWXOuttrade.m_outtrade_no<<", succ payed on "<<strEndTime);

		//4. update OrderReward pay_status
		DaoOrderReward stDaoOrderReward(m_db_table_order_reward_name);
		if(stDaoOrderReward.Payed(stDAOWXOuttrade.m_outtrade_no, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "process_order_payed failed, DaoOrderReward.Payed failed, strErrMsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}

		//5. add,reduce user cash
		DAOUserCash stDAOUserCash(m_db_table_user_cash_name, m_db_table_cash_flow_name);
		if(stDAOUserCash.AddBalance(strUserOpenid, iTotalFee, CASH_FLOW_TYPE_RECHARGE, m_mysql_helper, strErrMsg, strOuttradeNO) != DAO_RET_OK)
		{
			LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_payed, DAOUserCash.AddBalance failed, strErrMsg="<<strErrMsg);
		}
		if(stDAOUserCash.ReduceBalance(strUserOpenid, iTotalFee, CASH_FLOW_TYPE_CONSUME_ORDER_REWARD, m_mysql_helper, strErrMsg))
		{
			LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_payed, DAOUserCash.ReduceBalance failed, strErrMsg="<<strErrMsg);
		}

		//add user cash
		if(stDAOUserCash.AddBalance(stDaoOrderReward.m_openid_to, iTotalFee, CASH_FLOW_TYPE_INCOME_ORDER_REWARD_BY_USER, m_mysql_helper, strErrMsg, strOuttradeNO) != DAO_RET_OK)
		{
			LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_payed, DAOUserCash.AddBalance failed, strErrMsg="<<strErrMsg);
		}
	}

	return;
}


void ExecutorThreadOrder::process_order_fetch(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	int iRet = stDaoOrderInfo.FetchTry(request.m_order_id, request.m_self_openid, m_mysql_helper, strErrMsg);
	if(iRet == DAO_RET_LOGIC_TOO_MANY)
	{
		LOG4CPLUS_ERROR(logger, "process_order_fetch failed, errMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_ORDER_CANNOT_FETCH_TOO_MANY_WAITING;
		return;
	}

	if(iRet == DAO_RET_LOGIC_REPEATED)
	{
		LOG4CPLUS_ERROR(logger, "process_order_fetch failed, errMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_ORDER_CANNOT_FETCH_REPEATED;
		return;
	}
	
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_fetch failed, DaoOrderInfo.Fetched failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//add new waiting to t_order_fetch_state;
	DaoOrderFetchState stDaoOrderFetchState(m_db_table_order_fetch_state_name);
	if(stDaoOrderFetchState.AddNewWaiting(request.m_order_id, request.m_self_openid, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		//just fatal log  temply;
		LOG4CPLUS_FATAL(logger, "fuck, in process_order_fetch, DaoOrderFetchState.AddNewWaiting failed, strErrMsg="<<strErrMsg);
	}

	//add user_score
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	if(stDaoUserInfo.IncreaseUserScore(request.m_self_openid, SCORE_TYPE_ORDER_FETCH_COMMIT, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.IncreaseUserScore failed, strErrMsg="<<strErrMsg);
	}

	//notify
	if(stDaoOrderInfo.GetByPrimaryKey(request.m_order_id, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoOrderInfo.GetByPrimaryKey failed, errmsg = " << strErrMsg);
		return;
	}
	const uint32_t dwOrderWaitingUserSize = stDaoOrderInfo.m_order_info.order_waiting_user_list_size();
	LOG4CPLUS_DEBUG(logger, "orderid = " << stDaoOrderInfo.m_order_info.order_id()
							<< " order_waiting_user_list_size = " << dwOrderWaitingUserSize);
	if(dwOrderWaitingUserSize == m_order_fetch_waiting_user_size_first
		|| dwOrderWaitingUserSize == m_order_fetch_waiting_user_size_second
		||dwOrderWaitingUserSize == m_order_fetch_waiting_user_size_third)
	{
		send_template_message_of_order_fetch(stDaoOrderInfo);
	}
	else
	{
		LOG4CPLUS_DEBUG(logger, "not need to send template message");
	}

	return;
}


void ExecutorThreadOrder::process_order_fetch_canceled(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	int iRet = stDaoOrderInfo.FetchCanceled(request.m_order_id, request.m_self_openid, m_mysql_helper, strErrMsg);

	if(iRet == DAO_RET_INVALID)
	{
		LOG4CPLUS_ERROR(logger, "process_order_fetch_canceled failed, errMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_ORDER_CANNOT_FETCH_CANCEL_AFTER_CHOSEN;
		return;
	}
	
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_fetch_canceled failed, DaoOrderInfo.FetchCanceled failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//delete order openid in t_order_fetch_state
	DaoOrderFetchState stDaoOrderFetchState(m_db_table_order_fetch_state_name);
	if(stDaoOrderFetchState.DeleteByPrimaryKey(request.m_order_id, request.m_self_openid, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_fetch_canceled failed, DaoOrderFetchState.DeleteByPrimaryKey failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadOrder::process_order_fetch_accepted(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	int iRet = stDaoOrderInfo.FetchAccepted(request.m_order_id, request.m_self_openid, request.m_slave_openid, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_fetch_accepted failed, DaoOrderInfo.FetchAccepted failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//update t_order_fetch_state;
	DaoOrderFetchState stDaoOrderFetchState(m_db_table_order_fetch_state_name);
	if(stDaoOrderFetchState.UpdateToAccepted(request.m_order_id, request.m_slave_openid, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		//just fatal log  temply;
		LOG4CPLUS_FATAL(logger, "fuck, in process_order_fetch_accepted, DaoOrderFetchState.UpdateToAccepted failed, strErrMsg="<<strErrMsg);
	}

	//add user_score
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	if(stDaoUserInfo.IncreaseUserScore(request.m_slave_openid, SCORE_TYPE_ORDER_FETCH_ACCEPT, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.IncreaseUserScore failed, strErrMsg="<<strErrMsg);
	}

	//notify
	if(stDaoOrderInfo.GetByPrimaryKey(request.m_order_id, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoOrderInfo.GetByPrimaryKey failed, errmsg = " << strErrMsg);
		return;
	}
	send_template_message_of_order_fetch_accept(stDaoOrderInfo);
	return;
}

void ExecutorThreadOrder::process_order_search(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	reply.m_order_info_list.clear();

	std::vector<uint64_t> vecOrderidList;
	uint32_t dwSize, dwTotal, dwPageIndex = 0, dwPageSize = 50;

	BDHelper stBDHelper(m_bd_api_ak, m_bd_api_sk, m_bd_api_geotable_id);
	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	DaoUserTrust stDaoUserTrust(m_db_table_user_trust_name);
	std::string strErrMsg;
	int iRet;

	while(1)
	{
		if(stBDHelper.Geosearch(request.m_longitude / 1000000.0, request.m_latitude / 1000000.0, m_bd_api_geosearch_radius,
		dwPageIndex, dwPageSize, vecOrderidList, dwSize, dwTotal) < 0)
		{
			LOG4CPLUS_ERROR(logger, "BDHelper.Geosearch failed");
			reply.m_result_code = ExecutorThreadRequestType::E_BAIDU_ERR;

			return;
		}

		if(!dwSize || !vecOrderidList.size())
		{
			LOG4CPLUS_DEBUG(logger, "rsp size: 0");
			break;
		}

		for(size_t i = 0; i < vecOrderidList.size(); i++)
		{
			//get order_info
			iRet = stDaoOrderInfo.GetByPrimaryKey(vecOrderidList[i], m_mysql_helper, strErrMsg);

			if(iRet == DAO_RET_NOT_EXIST)
			{
				LOG4CPLUS_ERROR(logger, strErrMsg << "\n, delete poi");

				iRet = stBDHelper.DeletePoi(stDaoOrderInfo.m_order_info.order_id());
				if(iRet < 0)
				{
					LOG4CPLUS_ERROR(logger, "BDHelper.DeletePoi failed");
				}
				continue;
			}
			if(iRet != DAO_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, strErrMsg);
				continue;
			}

			//- 过滤自己下的单
			if(stDaoOrderInfo.m_order_info.openid_master() == request.m_self_openid)
			{
				LOG4CPLUS_DEBUG(logger, "skip self order");
				continue;
			}

			//check trust
			if(stDaoOrderInfo.m_order_info.order_visible() == ORDER_VISIBLE_ONLY_TRUST)
			{

				stDaoUserTrust.m_openid_to = request.m_self_openid;
				stDaoUserTrust.m_openid_from = stDaoOrderInfo.m_order_info.openid_master();
				iRet = stDaoUserTrust.SelectFromDB(m_mysql_helper, strErrMsg);
				if(iRet == DAO_RET_FAIL)
				{
					LOG4CPLUS_ERROR(logger, strErrMsg);
					continue;
				}
				if(iRet != DAO_RET_OK)
				{
					LOG4CPLUS_DEBUG(logger, strErrMsg);
					continue;
				}
			}

			LOG4CPLUS_DEBUG(logger, stDaoOrderInfo.m_order_info.Utf8DebugString());
			reply.m_order_info_list.push_back(stDaoOrderInfo.m_order_info);
		}

		dwPageIndex += 1;
		if(dwSize < dwPageSize)
		{
			LOG4CPLUS_DEBUG(logger, "rsp size: " << dwSize << " < req page_size:" << dwPageSize << ", not more data");
			break;
		}
	}


	//get user_info & user_star
	func_get_user_info_by_order_info(reply, true);

}

void ExecutorThreadOrder::process_order_cancel(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	if(stDaoOrderInfo.Canceled(request.m_order_id, request.m_self_openid, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_cancel failed, DaoOrderInfo.Canceled failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	LOG4CPLUS_DEBUG(logger, "order_info.order_status()="<<stDaoOrderInfo.m_order_info.order_status());

	if(ORDER_STATUS_WAITING_FETCH == stDaoOrderInfo.m_order_info.order_status())
	{
		LOG4CPLUS_DEBUG(logger, "order_info.order_status()==ORDER_STATUS_WAITING_FETCH, need to pay back to user cash!!"); 
		DAOUserCash stDAOUserCash(m_db_table_user_cash_name, m_db_table_cash_flow_name);
		if(stDAOUserCash.AddBalance(request.m_self_openid
							, stDaoOrderInfo.m_order_info.price()
							, CASH_FLOW_TYPE_INCOME_ORDER_CANCELED
							, m_mysql_helper
							, strErrMsg
							, "") != DAO_RET_OK)
		{
			LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_cancel, pay back to user cash failed.");
		}
	}
	else
	{
		LOG4CPLUS_DEBUG(logger, "nothing about user cash to do, ignore");
	}

	BDHelper stBdHelper(m_bd_api_ak, m_bd_api_sk, m_bd_api_geotable_id);
	if(stBdHelper.DeletePoi(request.m_order_id) < 0)
	{
		LOG4CPLUS_ERROR(logger, "BDHlper.DeletePoi failed");
	}
	return;
}

void ExecutorThreadOrder::process_order_finish(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	if(stDaoOrderInfo.Finished(request.m_order_id, request.m_self_openid, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_finish failed, DaoOrderInfo.Finished failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	LOG4CPLUS_DEBUG(logger, "order finished succ, order_info.slave_openid()="<<stDaoOrderInfo.m_order_info.openid_slave()
						<<", need to add user cash to this SB!!");

	DAOUserCash stDAOUserCash(m_db_table_user_cash_name, m_db_table_cash_flow_name);
	if(stDAOUserCash.AddBalance(stDaoOrderInfo.m_order_info.openid_slave()
						, stDaoOrderInfo.m_order_info.price()
						, CASH_FLOW_TYPE_INCOME_FETCH
						, m_mysql_helper
						, strErrMsg
						, "") != DAO_RET_OK)
	{
		LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_finish, add user cash to other SB failed.");
	}	

	//notify
	if(stDaoOrderInfo.GetByPrimaryKey(request.m_order_id, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoOrderInfo.GetByPrimaryKey failed, errmsg = " << strErrMsg);
		return;
	}
	send_template_message_of_order_finish(stDaoOrderInfo);

	//add user_score
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	if(stDaoUserInfo.IncreaseUserScore(stDaoOrderInfo.m_order_info.openid_slave(), SCORE_TYPE_ORDER_FINISH, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.IncreaseUserScore failed, strErrMsg="<<strErrMsg);
	}

	return;
}

void ExecutorThreadOrder::process_order_comment(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	std::string strAnoOpenid;
	int iRet = stDaoOrderInfo.Commented(request.m_order_id, request.m_self_openid, strAnoOpenid, request.m_order_star, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_comment failed, DaoOrderInfo.Commented failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = iRet == DAO_RET_INVALID? ExecutorThreadRequestType::E_PARAM_ERR : ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	LOG4CPLUS_DEBUG(logger, "order commented succ, order_info.slave_openid()="<<stDaoOrderInfo.m_order_info.openid_slave()
						<<", need to add user star to this SB!!");

	DaoUserExtraInfo stDaoUserExtraInfo(m_db_table_user_star_name, m_db_table_order_info_name);
	stDaoUserExtraInfo.m_openid = strAnoOpenid;
	stDaoUserExtraInfo.m_order_id = request.m_order_id;
	stDaoUserExtraInfo.m_star = request.m_order_star;
	if(stDaoUserExtraInfo.AddNewUserStar(m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_comment, add user star to other SB failed.");
	}

	//add user_score
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	if(stDaoUserInfo.IncreaseUserScore(request.m_self_openid, SCORE_TYPE_ORDER_COMMENT, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.IncreaseUserScore failed, strErrMsg="<<strErrMsg);
	}

	return;
}

void ExecutorThreadOrder::process_order_info_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";

	//order info
	for(size_t i=0; i!=request.m_order_id_list.size(); ++i)
	{
		DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
		if(stDaoOrderInfo.GetByPrimaryKey(request.m_order_id_list[i], m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoOrderInfo.GetByPrimaryKey failed, order_id="<<request.m_order_id_list[i]<<", strErrMsg="<<strErrMsg);
			continue;
		}

		reply.m_order_info_list.push_back(stDaoOrderInfo.m_order_info);
	}

	//user info
	func_get_user_info_by_order_info(reply, true);

	return;
}

void ExecutorThreadOrder::process_order_list_user_created(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoOrderInfoList stDaoOrderInfoList(m_db_table_order_info_name);
	std::string strOpenid;
	if(request.m_third_openid.empty())
	{
		strOpenid = request.m_self_openid;
	}
	else
	{
		strOpenid = request.m_third_openid;
	}
	int iRet = stDaoOrderInfoList.GetCreatedOrderList(strOpenid, request.m_offset, request.m_len, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK && iRet != DAO_RET_NOT_EXIST)
	{
		LOG4CPLUS_ERROR(logger, "process_order_list_user_created failed, DaoOrderInfoList.GetCreatedOrderList failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	for(size_t i=0; i!=stDaoOrderInfoList.m_order_info_list.size(); ++i)
	{
		reply.m_order_info_list.push_back(stDaoOrderInfoList.m_order_info_list[i]);
	}

	func_get_user_info_by_order_info(reply, true);
	return;
}

void ExecutorThreadOrder::process_order_list_user_fetched(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strErrMsg = "";
	DaoOrderFetchStateList stDaoOrderFetchStateList(m_db_table_order_fetch_state_name);
	std::string strOpenid;
	if(request.m_third_openid.empty())
	{
		strOpenid = request.m_self_openid;
	}
	else
	{
		strOpenid = request.m_third_openid;
	}
	int iRet = stDaoOrderFetchStateList.GetFentchedOrderList(strOpenid, request.m_offset, request.m_len, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK && iRet != DAO_RET_NOT_EXIST)
	{
		LOG4CPLUS_ERROR(logger, "process_order_list_user_created failed, DaoOrderInfoList.GetCreatedOrderList failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	std::set<uint64_t> orderIds;
	for(size_t i=0; i!=stDaoOrderFetchStateList.m_vec_order_fetch_state_list.size(); ++i)
	{
		orderIds.insert(stDaoOrderFetchStateList.m_vec_order_fetch_state_list[i].order_id());
		reply.m_order_openid_fetch_state_list.push_back(stDaoOrderFetchStateList.m_vec_order_fetch_state_list[i]);
	}

	if(!orderIds.empty())
	{
		func_get_order_info(reply, orderIds);
	}

	func_get_user_info_by_order_info(reply, true);
	return;
}

void ExecutorThreadOrder::process_order_reward(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
	std::string strErrMsg;
	int iRet;
	time_t now = time(0);
	std::string strRewardDesc = m_pa_pay_reward_desc;

	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	iRet = stDaoOrderInfo.GetByPrimaryKey(request.m_order_id, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_reward failed, DaoOrderInfo.GetByPrimaryKey failed, errmsg=" << strErrMsg);
		if(iRet == DAO_RET_NOT_EXIST) 
		{
			reply.m_result_code = ExecutorThreadRequestType::E_PARAM_ERR;
		}
		else
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		}

		return;
	}

	DaoOrderReward stDaoOrderReward(m_db_table_order_reward_name);
	stDaoOrderReward.m_out_trade_no = int_2_str(common::util::generate_unique_id());
	stDaoOrderReward.m_orderid = request.m_order_id;
	stDaoOrderReward.m_openid_from = request.m_self_openid;
	stDaoOrderReward.m_openid_to = stDaoOrderInfo.m_order_info.openid_master();
	stDaoOrderReward.m_reward_ts = now;
	stDaoOrderReward.m_amount = request.m_amount;

	DAOUserCash stDaoUserCash(m_db_table_user_cash_name, m_db_table_cash_flow_name);
	if(DAO_RET_OK == stDaoUserCash.ReduceBalance(request.m_self_openid
							, request.m_amount
							, CASH_FLOW_TYPE_CONSUME_ORDER_REWARD
							, m_mysql_helper
							, strErrMsg
							, stDaoOrderReward.m_out_trade_no))
	{
		//type A, user cash
		reply.m_cost_type = ORDER_PAY_COST_TYPE_USER_CASH;

		//add new order_reward to db
		stDaoOrderReward.m_pay_status = ORDER_PAY_STATUS_PAYED;
		stDaoOrderReward.m_pay_ts = now;
		if(stDaoOrderReward.AddNew(m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_reward failed, DaoOrderReward.AddNew failed, strErrMsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}

		// add user cash
		if(stDaoUserCash.AddBalance(stDaoOrderReward.m_openid_to, request.m_amount, CASH_FLOW_TYPE_INCOME_ORDER_REWARD_BY_USER, m_mysql_helper, strErrMsg, stDaoOrderReward.m_out_trade_no) != DAO_RET_OK)
		{
			LOG4CPLUS_FATAL(logger, "++ ++ ++, process_order_payed, DAOUserCash.AddBalance failed, strErrMsg="<<strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
		return;
	}

	//type B, wx pay again
	reply.m_cost_type = ORDER_PAY_COST_TYPE_WX_PAY;

	//B1. unified order
	std::string strPrepayId = "";
	common::wxapi::WXAPIPAY stWXAPIPAY;
	iRet = stWXAPIPAY.PaPayUnifiedOrder(m_pa_appid
						, m_pa_pay_mchid
						, m_pa_pay_api_key
						, strRewardDesc
						, stDaoOrderReward.m_out_trade_no 
						, request.m_amount
						, request.m_user_ip
						, m_pa_pay_ntf_url
						, request.m_self_openid
						, strPrepayId);
	if(iRet < 0)
	{
		LOG4CPLUS_ERROR(logger, "process_order_create failed, PaPayUnifiedOrder failed");
		reply.m_result_code = ExecutorThreadRequestType::E_WX_ERR;
		return;
	}

	DAOWXOuttrade stDAOWXOuttrade(m_db_table_wx_outtrade_name);
	stDAOWXOuttrade.m_openid = request.m_self_openid;
	stDAOWXOuttrade.m_outtrade_no = stDaoOrderReward.m_out_trade_no;
	stDAOWXOuttrade.m_prepay_id = strPrepayId;
	stDAOWXOuttrade.m_body = strRewardDesc;
	stDAOWXOuttrade.m_detail = "";
	stDAOWXOuttrade.m_attach = "";
	stDAOWXOuttrade.m_goods_tag = "";
	stDAOWXOuttrade.m_spbill_create_ip = request.m_user_ip;
	stDAOWXOuttrade.m_time_start = common::wxapi::WXAPIPAY::GenTimeStringFromNow_yyyyMMddHHmmss(0);
	stDAOWXOuttrade.m_time_expire = common::wxapi::WXAPIPAY::GenTimeStringFromNow_yyyyMMddHHmmss(600);
	stDAOWXOuttrade.m_trade_type = "JSAPI";
	stDAOWXOuttrade.m_product_id = "";
	stDAOWXOuttrade.m_limit_pay = "no_credit";
	stDAOWXOuttrade.m_appid = m_pa_appid;
	stDAOWXOuttrade.m_mch_id = m_pa_pay_mchid;
	stDAOWXOuttrade.m_device_info = "WEB";
	stDAOWXOuttrade.m_fee_type = "CNY";
	stDAOWXOuttrade.m_total_fee = request.m_amount;
	stDAOWXOuttrade.m_pay_state = WX_UNINFIED_ORDER_PAY_STATE_WAITING_PAY;
	stDAOWXOuttrade.m_pay_reason = WX_UNINFIED_ORDER_PAY_REASON_REWARD_ORDER;
	if(stDAOWXOuttrade.AddNew(m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_create failed, DAOWXOuttrade.AddNew failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2. add new order_reward to DB

	stDaoOrderReward.m_pay_status = ORDER_PAY_STATUS_NOT_PAYED_YET;
	stDaoOrderReward.m_pay_ts = 0;

	if(stDaoOrderReward.AddNew(m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "process_order_reward failed, DaoOrderReward.AddNew failed, strErrMsg="<<strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}


	reply.m_wx_unified_order_param.set_appid(m_pa_appid);
	reply.m_wx_unified_order_param.set_timestamp(int_2_str(now));
	reply.m_wx_unified_order_param.set_noncestr(common::wxapi::WXAPIPAY::GenRandomString(32));
	reply.m_wx_unified_order_param.set_package("prepay_id=" + strPrepayId);
	reply.m_wx_unified_order_param.set_signtype("MD5");

	std::map<std::string, std::string> tmp_map;
	tmp_map.insert(pair<std::string, std::string>("appId", reply.m_wx_unified_order_param.appid()));
	tmp_map.insert(pair<std::string, std::string>("timeStamp", reply.m_wx_unified_order_param.timestamp()));
	tmp_map.insert(pair<std::string, std::string>("nonceStr", reply.m_wx_unified_order_param.noncestr()));
	tmp_map.insert(pair<std::string, std::string>("package",  reply.m_wx_unified_order_param.package()));
	tmp_map.insert(pair<std::string, std::string>("signType", reply.m_wx_unified_order_param.signtype()));
	reply.m_wx_unified_order_param.set_paysign(common::wxapi::WXAPIPAY::Params2Sign(tmp_map, m_pa_pay_api_key));

	return;
}

void ExecutorThreadOrder::process_order_favor_check(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
	DaoOrderFavorInfo stDaoOrderFavorInfo(m_db_table_order_favor_name);
	int iRet;
	std::string strErrMsg;

	reply.m_order_favor_info_list.clear();
	hoosho::msg::z::OrderFavorInfo stOrderFavorInfo;
	stOrderFavorInfo.set_openid(request.m_self_openid);
	for(size_t i = 0; i < request.m_order_id_list.size(); i++)
	{
		stOrderFavorInfo.set_orderid(request.m_order_id_list[i]);
		iRet = stDaoOrderFavorInfo.GetByPrimaryKey(request.m_self_openid, request.m_order_id_list[i], stOrderFavorInfo, m_mysql_helper, strErrMsg);
		if(iRet != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoOrderFavorInfo.GetByPrimayKey failed, errmsg = " << strErrMsg);
			stOrderFavorInfo.set_create_ts(0);
		}
		reply.m_order_favor_info_list.push_back(stOrderFavorInfo);
	}
	return;
}

void ExecutorThreadOrder::process_order_favor_update(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	DaoOrderFavorInfo stDaoOrderFavorInfo(m_db_table_order_favor_name);
	DaoNoticeInfo stDaoNoticeInfo;
	int iRet;
	std::string strErrMsg;

	iRet = stDaoOrderInfo.GetByPrimaryKey(request.m_order_id, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoOrderInfo.GetByPrimaryKey failed, errmsg  = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	if(request.m_oper == ORDER_FAVOR_OPER_TYPE_ADD)
	{
		iRet = stDaoOrderFavorInfo.AddFavor(request.m_self_openid, request.m_order_id, m_mysql_helper, strErrMsg);	
		if(iRet != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoOrderFavorInfo.AddFavor failed, errmsg = " << strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}

		//notify
		stDaoNoticeInfo.m_notice_info.set_table_id(::common::util::generate_unique_id());
		stDaoNoticeInfo.m_notice_info.set_openid(stDaoOrderInfo.m_order_info.openid_master());
		stDaoNoticeInfo.m_notice_info.set_type(NOTICE_TYPE_FAVORITE);
		stDaoNoticeInfo.m_notice_info.set_status(NOTICE_STATUS_UNREAD);
		stDaoNoticeInfo.m_notice_info.set_extra_data_0(int_2_str(stDaoOrderInfo.m_order_info.order_id()));
		stDaoNoticeInfo.m_notice_info.set_extra_data_1(request.m_self_openid);
		stDaoNoticeInfo.m_notice_info.set_extra_data_2("");
		stDaoNoticeInfo.m_notice_info.set_create_ts(time(0));
		if(stDaoNoticeInfo.Add(m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoNoticeInfo.Add failed, errmsg = " << strErrMsg);
		}
	}
	else if(request.m_oper == ORDER_FAVOR_OPER_TYPE_DEL)
	{
		iRet = stDaoOrderFavorInfo.DelFavor(request.m_self_openid, request.m_order_id, m_mysql_helper, strErrMsg);	
		if(iRet != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoOrderFavorInfo.DelFavor failed, errmsg = " << strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}

		iRet = stDaoNoticeInfo.DelNoticeFavorite(request.m_order_id, request.m_self_openid, m_mysql_helper, strErrMsg);
		if(iRet != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoNoticeInfo.DelNoticeFavorite failed, errmsg = " << strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "invalid oper = " << request.m_oper);
		reply.m_result_code = ExecutorThreadRequestType::E_PARAM_ERR;
		return;
	}
}

void ExecutorThreadOrder::process_order_favor_list(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
	DaoOrderFavorInfoList stDaoOrderFavorInfoList(m_db_table_order_favor_name);
	std::string strErrMsg;
	int iRet = stDaoOrderFavorInfoList.GetFavorList(request.m_order_id, request.m_begin_ts, request.m_len, m_mysql_helper, strErrMsg);
	if(iRet == DAO_RET_FAIL)
	{
		LOG4CPLUS_ERROR(logger, "DaoOrderFavorInfoList.GetFavorList failed, errmsg = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	reply.m_order_favor_info_list.clear();
	reply.m_order_favor_info_list.assign(stDaoOrderFavorInfoList.m_favor_list.begin(), stDaoOrderFavorInfoList.m_favor_list.end());
	func_get_user_info_by_favor_info(reply);
}

void ExecutorThreadOrder::process_order_follow_add(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
	DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
	DaoOrderFollowInfo stDaoOrderFollowInfo(m_db_table_order_follow_name, m_db_table_comment_index_on_orderid_name, m_db_table_reply_index_on_origin_comment_name);
	
	int iRet;
	std::string strErrMsg;
	std::string strOriginCommentOpenid;
	DaoNoticeInfo stDaoNoticeInfo;
	iRet = stDaoOrderInfo.GetByPrimaryKey(request.m_order_follow_info.orderid(), m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoOrderInfo.GetByPrimaryKey failed, errmsg = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	
	stDaoOrderFollowInfo.m_follow_info.CopyFrom(request.m_order_follow_info);
	if(request.m_order_follow_info.type() == ORDER_FOLLOW_TYPE_COMMMENT)
	{
		stDaoOrderFollowInfo.m_follow_info.set_follow_id(::common::util::generate_unique_id());
		stDaoOrderFollowInfo.m_follow_info.set_origin_comment_id(0);
		stDaoOrderFollowInfo.m_follow_info.set_openid_from(request.m_self_openid);
		stDaoOrderFollowInfo.m_follow_info.set_openid_to("");
		stDaoOrderFollowInfo.m_follow_info.set_create_ts(time(0));
		stDaoOrderFollowInfo.m_follow_info.set_del_ts(0);

		iRet = stDaoOrderFollowInfo.AddComment(m_mysql_helper, strErrMsg);
		if(iRet != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoOrderFollowInfo.AddComment failed, errmsg = " << strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}

		//notify
		stDaoNoticeInfo.m_notice_info.set_table_id(::common::util::generate_unique_id());
		stDaoNoticeInfo.m_notice_info.set_openid(stDaoOrderInfo.m_order_info.openid_master());
		stDaoNoticeInfo.m_notice_info.set_type(NOTICE_TYPE_COMMENT);
		stDaoNoticeInfo.m_notice_info.set_status(NOTICE_STATUS_UNREAD);
		stDaoNoticeInfo.m_notice_info.set_extra_data_0(int_2_str(stDaoOrderFollowInfo.m_follow_info.orderid()));
		stDaoNoticeInfo.m_notice_info.set_extra_data_1("");
		stDaoNoticeInfo.m_notice_info.set_extra_data_2(int_2_str(stDaoOrderFollowInfo.m_follow_info.follow_id()));
		stDaoNoticeInfo.m_notice_info.set_create_ts(stDaoOrderFollowInfo.m_follow_info.create_ts());
		if(stDaoNoticeInfo.Add(m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoNoticeInfo.Add failed, errmsg = " << strErrMsg);
		}
	}
	else if(request.m_order_follow_info.type() == ORDER_FOLLOW_TYPE_REPLY)
	{
		stDaoOrderFollowInfo.m_follow_info.set_follow_id(::common::util::generate_unique_id());
		LOG4CPLUS_DEBUG(logger, "follow_id = " << stDaoOrderFollowInfo.m_follow_info.follow_id());
		stDaoOrderFollowInfo.m_follow_info.set_openid_from(request.m_self_openid);
		stDaoOrderFollowInfo.m_follow_info.set_create_ts(time(0));
		stDaoOrderFollowInfo.m_follow_info.set_del_ts(0);

		iRet = stDaoOrderFollowInfo.AddReply(m_mysql_helper, strErrMsg, strOriginCommentOpenid);
		if(iRet != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoOrderFollowInfo.AddReply failed, errmsg = " << strErrMsg);
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}

		//notify
		//通知被回复者
		stDaoNoticeInfo.m_notice_info.set_table_id(::common::util::generate_unique_id());
		stDaoNoticeInfo.m_notice_info.set_openid(stDaoOrderFollowInfo.m_follow_info.openid_to());
		stDaoNoticeInfo.m_notice_info.set_type(NOTICE_TYPE_COMMENT);
		stDaoNoticeInfo.m_notice_info.set_status(NOTICE_STATUS_UNREAD);
		stDaoNoticeInfo.m_notice_info.set_extra_data_0(int_2_str(stDaoOrderFollowInfo.m_follow_info.orderid()));
		stDaoNoticeInfo.m_notice_info.set_extra_data_1(int_2_str(stDaoOrderFollowInfo.m_follow_info.origin_comment_id()));
		stDaoNoticeInfo.m_notice_info.set_extra_data_2(int_2_str(stDaoOrderFollowInfo.m_follow_info.follow_id()));
		stDaoNoticeInfo.m_notice_info.set_create_ts(stDaoOrderFollowInfo.m_follow_info.create_ts());
		if(stDaoNoticeInfo.Add(m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoNoticeInfo.Add failed, errmsg = " << strErrMsg);
		}

		//通知原评论者
		if(strOriginCommentOpenid != stDaoOrderFollowInfo.m_follow_info.openid_to())
		{
			stDaoNoticeInfo.m_notice_info.set_table_id(::common::util::generate_unique_id());
			stDaoNoticeInfo.m_notice_info.set_openid(strOriginCommentOpenid);
			stDaoNoticeInfo.m_notice_info.set_type(NOTICE_TYPE_COMMENT);
			stDaoNoticeInfo.m_notice_info.set_status(NOTICE_STATUS_UNREAD);
			stDaoNoticeInfo.m_notice_info.set_extra_data_0(int_2_str(stDaoOrderFollowInfo.m_follow_info.orderid()));
			stDaoNoticeInfo.m_notice_info.set_extra_data_1(int_2_str(stDaoOrderFollowInfo.m_follow_info.origin_comment_id()));
			stDaoNoticeInfo.m_notice_info.set_extra_data_2(int_2_str(stDaoOrderFollowInfo.m_follow_info.follow_id()));
			stDaoNoticeInfo.m_notice_info.set_create_ts(stDaoOrderFollowInfo.m_follow_info.create_ts());
			if(stDaoNoticeInfo.Add(m_mysql_helper, strErrMsg) != DAO_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "DaoNoticeInfo.Add failed, errmsg = " << strErrMsg);
			}
		}

		//通知下单者
		if(stDaoOrderInfo.m_order_info.openid_master() != strOriginCommentOpenid
		   && stDaoOrderInfo.m_order_info.openid_master() != stDaoOrderFollowInfo.m_follow_info.openid_to())
		{
			stDaoNoticeInfo.m_notice_info.set_table_id(::common::util::generate_unique_id());
			stDaoNoticeInfo.m_notice_info.set_openid(stDaoOrderInfo.m_order_info.openid_master());
			stDaoNoticeInfo.m_notice_info.set_type(NOTICE_TYPE_COMMENT);
			stDaoNoticeInfo.m_notice_info.set_status(NOTICE_STATUS_UNREAD);
			stDaoNoticeInfo.m_notice_info.set_extra_data_0(int_2_str(stDaoOrderFollowInfo.m_follow_info.orderid()));
			stDaoNoticeInfo.m_notice_info.set_extra_data_1(int_2_str(stDaoOrderFollowInfo.m_follow_info.origin_comment_id()));
			stDaoNoticeInfo.m_notice_info.set_extra_data_2(int_2_str(stDaoOrderFollowInfo.m_follow_info.follow_id()));
			stDaoNoticeInfo.m_notice_info.set_create_ts(stDaoOrderFollowInfo.m_follow_info.create_ts());
			if(stDaoNoticeInfo.Add(m_mysql_helper, strErrMsg) != DAO_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "DaoNoticeInfo.Add failed, errmsg = " << strErrMsg);
			}
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "invalid follow.type = " << request.m_order_follow_info.type());
		reply.m_result_code = ExecutorThreadRequestType::E_PARAM_ERR;
		return;
	}
	reply.m_follow_id = stDaoOrderFollowInfo.m_follow_info.follow_id();
}

void ExecutorThreadOrder::process_order_comment_list(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
	DaoOrderCommentInfoList stDaoCommentInfoList(m_db_table_order_follow_name, m_db_table_comment_index_on_orderid_name, m_db_table_reply_index_on_origin_comment_name);
	int iRet;
	std::string strErrMsg;
	
	iRet = stDaoCommentInfoList.GetCommentList(request.m_order_id, request.m_begin_comment_id, request.m_len, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK && iRet != DAO_RET_NOT_EXIST)
	{
		LOG4CPLUS_ERROR(logger, "DaoCommentInfoList.GetCommentList failed, errmsg = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	reply.m_order_comment_info_list.assign(stDaoCommentInfoList.m_comment_list.begin(), stDaoCommentInfoList.m_comment_list.end());
	func_get_user_info_by_comment_info(reply);
}

void ExecutorThreadOrder::process_order_comment_detail(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
	DaoOrderCommentInfoList stDaoCommentInfoList(m_db_table_order_follow_name, m_db_table_comment_index_on_orderid_name, m_db_table_reply_index_on_origin_comment_name);
	int iRet;
	std::string strErrMsg;
	
	iRet = stDaoCommentInfoList.GetCommentDetail(request.m_origin_comment_id_list, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK && iRet != DAO_RET_NOT_EXIST)
	{
		LOG4CPLUS_ERROR(logger, "DaoCommentInfoList.GetCommentDetail failed, errmsg = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	reply.m_order_comment_info_list.assign(stDaoCommentInfoList.m_comment_list.begin(), stDaoCommentInfoList.m_comment_list.end());
	func_get_user_info_by_comment_info(reply);
}

void ExecutorThreadOrder::process_order_follow_del(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
	DaoOrderFollowInfo stDaoOrderFollowInfo(m_db_table_order_follow_name, m_db_table_comment_index_on_orderid_name, m_db_table_reply_index_on_origin_comment_name);
	int iRet;
	std::string strErrMsg;
	iRet = stDaoOrderFollowInfo.DelFollow(request.m_self_openid, request.m_follow_id, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoOrderFollowInfo.DelFollow failed, errmsg = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	DaoNoticeInfo stDaoNoticeInfo;
	iRet = stDaoNoticeInfo.DelNoticeFollow(stDaoOrderFollowInfo.m_follow_info.orderid(), request.m_follow_id, m_mysql_helper, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoNoticeInfo.DelNoticeFollow failed, errmsg = " << strErrMsg);
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
}

void ExecutorThreadOrder::func_get_order_info(ExecutorThreadResponseElement &reply, const std::set<uint64_t>& orderIdSet)
{
	for(std::set<uint64_t>::const_iterator iter = orderIdSet.begin(); iter != orderIdSet.end(); ++iter)
	{
		std::string strErrMsg = "";
		uint64_t qwOrderId = *iter;
		DaoOrderInfo stDaoOrderInfo(m_db_table_order_info_name);
		if(stDaoOrderInfo.GetByPrimaryKey(qwOrderId, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "in func_get_order_info, DaoOrderInfo.GetByPrimaryKey failed, strErrMsg="<<strErrMsg);
			continue;
		}

		reply.m_order_info_list.push_back(stDaoOrderInfo.m_order_info);
	}
}



void ExecutorThreadOrder::func_get_user_info_by_order_info(ExecutorThreadResponseElement &reply, bool bGetUserStar)
{
	//user info
	set<string> setOpenid;
	std::string strErrMsg;

	for(size_t i = 0; i< reply.m_order_info_list.size(); i++)
	{
		setOpenid.insert(reply.m_order_info_list[i].openid_master());
		if(!reply.m_order_info_list[i].openid_slave().empty())
		{
			setOpenid.insert(reply.m_order_info_list[i].openid_slave());
		}
		for(int j = 0; j < reply.m_order_info_list[i].order_waiting_user_list_size(); j++)
		{
			setOpenid.insert(reply.m_order_info_list[i].order_waiting_user_list(j).openid());
		}
	}
	reply.m_user_info_list.clear();
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	for(std::set<std::string>::iterator iter = setOpenid.begin(); iter != setOpenid.end(); ++iter)
	{
		if(stDaoUserInfo.GetByPrimaryKey(*iter, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoUserInfo.GetByPrimaryKey failed, openid="<<(*iter)<<", strErrMsg="<<strErrMsg);
			continue;
		}

		reply.m_user_info_list.push_back(stDaoUserInfo.m_user_info);
	}

	if(bGetUserStar)
	{
		//user star
		reply.m_user_extra_info_list.clear();
		DaoUserExtraInfo stDaoUserExtraInfo(m_db_table_user_star_name, m_db_table_order_info_name);
		::hoosho::msg::z::UserExtraInfo stUserExtraInfo;		
		for(std::set<std::string>::iterator iter = setOpenid.begin(); iter != setOpenid.end(); ++iter)
		{
			if(stDaoUserExtraInfo.GetUserExtraInfo(*iter, stUserExtraInfo, m_mysql_helper, strErrMsg) != DAO_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "DaoUserExtraInfo.GetUserExtraInfo failed, openid=" << (*iter) << ", msg = " << strErrMsg);
				continue;
			}
			
			reply.m_user_extra_info_list.push_back(stUserExtraInfo);
		}
	}

}

void ExecutorThreadOrder::func_get_user_info_by_favor_info(ExecutorThreadResponseElement &reply)
{
	//user info
	set<string> setOpenid;
	std::string strErrMsg;

	for(size_t i = 0; i< reply.m_order_favor_info_list.size(); i++)
	{
		setOpenid.insert(reply.m_order_favor_info_list[i].openid());
	}
	reply.m_user_info_list.clear();
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	for(std::set<std::string>::iterator iter = setOpenid.begin(); iter != setOpenid.end(); ++iter)
	{
		if(stDaoUserInfo.GetByPrimaryKey(*iter, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoUserInfo.GetByPrimaryKey failed, openid="<<(*iter)<<", strErrMsg="<<strErrMsg);
			continue;
		}

		reply.m_user_info_list.push_back(stDaoUserInfo.m_user_info);
	}
}

void ExecutorThreadOrder::func_get_user_info_by_comment_info(ExecutorThreadResponseElement &reply)
{
	//user info
	set<string> setOpenid;
	std::string strErrMsg;

	for(size_t i = 0; i< reply.m_order_comment_info_list.size(); i++)
	{
		const hoosho::msg::z::OrderCommentInfo stOrderCommentInfo = reply.m_order_comment_info_list[i];
		setOpenid.insert(stOrderCommentInfo.comment().openid_from());

		for(int j = 0; j < stOrderCommentInfo.reply_list_size(); j++)
		{
			setOpenid.insert(stOrderCommentInfo.reply_list(j).openid_from());
		}
	}
	reply.m_user_info_list.clear();
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	for(std::set<std::string>::iterator iter = setOpenid.begin(); iter != setOpenid.end(); ++iter)
	{
		if(stDaoUserInfo.GetByPrimaryKey(*iter, m_mysql_helper, strErrMsg) != DAO_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "DaoUserInfo.GetByPrimaryKey failed, openid="<<(*iter)<<", strErrMsg="<<strErrMsg);
			continue;
		}

		reply.m_user_info_list.push_back(stDaoUserInfo.m_user_info);
	}
}

int ExecutorThreadOrder::send_template_message_of_order_fetch(const DaoOrderInfo& stDaoOrderInfo)
{
	LOG4CPLUS_DEBUG(logger, "send template message of order fetch");

	uint32_t dwOrderWaitingUserSize = stDaoOrderInfo.m_order_info.order_waiting_user_list_size();
	if(dwOrderWaitingUserSize == 0)
	{
		LOG4CPLUS_ERROR(logger, "invalid waiting user list size = 0, orderid = " << stDaoOrderInfo.m_order_info.order_id());
		return -1;
	}

	const std::string& strFetchOpenid = stDaoOrderInfo.m_order_info.order_waiting_user_list(dwOrderWaitingUserSize - 1).openid();
	uint64_t qwFetchTs = stDaoOrderInfo.m_order_info.order_waiting_user_list(dwOrderWaitingUserSize - 1).create_ts();
	string strErrMsg;
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	if(stDaoUserInfo.GetByPrimaryKey(strFetchOpenid, m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.GetByPrimaryKey failed, msg = " << strErrMsg);
		return -1;
	}

	char buf[100];
    Json::Value any;
    any["touser"] = stDaoOrderInfo.m_order_info.openid_master();
    any["template_id"] = m_order_fetch_template_id;

	std::string strUrl = m_order_fetch_url;
	strUrl.replace(strUrl.find("{{ORDERID}}"), strlen("{{ORDERID}}"), int_2_str(stDaoOrderInfo.m_order_info.order_id()));
    any["url"] = strUrl;
    any["data"]["first"]["value"] = m_order_fetch_first;
    any["data"]["first"]["color"] = m_order_fetch_color;
    any["data"][m_order_fetch_keyword1]["value"] = int_2_str(stDaoOrderInfo.m_order_info.order_id());
    any["data"][m_order_fetch_keyword1]["color"] = m_order_fetch_color;
    any["data"][m_order_fetch_keyword2]["value"] = stDaoUserInfo.m_user_info.nickname();
    any["data"][m_order_fetch_keyword2]["color"] = m_order_fetch_color;
    any["data"][m_order_fetch_keyword3]["value"] = time_2_format_str(qwFetchTs);
    any["data"][m_order_fetch_keyword3]["color"] = m_order_fetch_color;

	sprintf(buf, m_order_fetch_remark.c_str(), dwOrderWaitingUserSize,
			DB_ORDER_INFO_OPENID_WAITING_MAX_NUM - dwOrderWaitingUserSize);
    any["data"]["remark"]["value"] = buf;
    any["data"]["remark"]["color"] = m_order_fetch_color;

    std::string strPostData = any.toJsonString();
    LOG4CPLUS_DEBUG(logger, "postdata = " << strPostData);

    //调用微信api
    common::wxapi::WXAPIMessage wxapiMessage;
    std::string strBaseAccessToken;
    std::string strJSAPITicket;
	g_token_server_processor->get_pa_info(m_pa_appid, strBaseAccessToken, strJSAPITicket);

    int iRet = 0;

    if(wxapiMessage.SendTemplateMessage(strBaseAccessToken, strPostData, iRet) < 0)
    {
        LOG4CPLUS_ERROR(logger, "WXAPI.SendTemplateMessage failed, errcode = " << iRet);
        return -1;
    }
    return 0;
}

int ExecutorThreadOrder::send_template_message_of_order_fetch_accept(const DaoOrderInfo& stDaoOrderInfo)
{
	LOG4CPLUS_DEBUG(logger, "send template message of order fetch accept");
	string strErrMsg;
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	if(stDaoUserInfo.GetByPrimaryKey(stDaoOrderInfo.m_order_info.openid_master(), m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.GetByPrimaryKey failed, msg = " << strErrMsg);
		return -1;
	}

	char buf[100];
    Json::Value any;
    any["touser"] = stDaoOrderInfo.m_order_info.openid_slave();
    any["template_id"] = m_order_fetch_accept_template_id;

	std::string strUrl = m_order_fetch_accept_url;
	strUrl.replace(strUrl.find("{{ORDERID}}"), strlen("{{ORDERID}}"), int_2_str(stDaoOrderInfo.m_order_info.order_id()));
	any["url"] = strUrl;
    any["data"]["first"]["value"] = m_order_fetch_accept_first;
    any["data"]["first"]["color"] = m_order_fetch_accept_color;

	sprintf(buf, "%.2f元", stDaoOrderInfo.m_order_info.price() / 100.0);
    any["data"][m_order_fetch_accept_keyword1]["value"] = buf;
    any["data"][m_order_fetch_accept_keyword1]["color"] = m_order_fetch_accept_color;
	
	std::string strOrderDesc;
	if(stDaoOrderInfo.m_order_info.media_type() == ORDER_MEDIA_TYPE_AUDIO)
	{
		strOrderDesc = m_order_fetch_accept_order_desc_of_audio;
	}
	else
	{
		strOrderDesc = stDaoOrderInfo.m_order_info.order_desc();
	}
    any["data"][m_order_fetch_accept_keyword2]["value"] = strOrderDesc;
    any["data"][m_order_fetch_accept_keyword2]["color"] = m_order_fetch_accept_color;
    any["data"][m_order_fetch_accept_keyword3]["value"] = stDaoOrderInfo.m_order_info.addr_to();
    any["data"][m_order_fetch_accept_keyword3]["color"] = m_order_fetch_accept_color;
   	any["data"]["remark"]["value"] = m_order_fetch_accept_remark;
    any["data"]["remark"]["color"] = m_order_fetch_accept_color;

    std::string strPostData = any.toJsonString();
    LOG4CPLUS_DEBUG(logger, "postdata = " << strPostData);

    //调用微信api
    common::wxapi::WXAPIMessage wxapiMessage;
    std::string strBaseAccessToken;
    std::string strJSAPITicket;
	g_token_server_processor->get_pa_info(m_pa_appid, strBaseAccessToken, strJSAPITicket);

    int iRet = 0;

    if(wxapiMessage.SendTemplateMessage(strBaseAccessToken, strPostData, iRet) < 0)
    {
        LOG4CPLUS_ERROR(logger, "WXAPI.SendTemplateMessage failed, errcode = " << iRet);
        return -1;
    }
    return 0;
}

int ExecutorThreadOrder::send_template_message_of_order_finish(const DaoOrderInfo& stDaoOrderInfo)
{
	LOG4CPLUS_DEBUG(logger, "send template message of order finish");
	string strErrMsg;
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	if(stDaoUserInfo.GetByPrimaryKey(stDaoOrderInfo.m_order_info.openid_master(), m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.GetByPrimaryKey failed, msg = " << strErrMsg);
		return -1;
	}

	char buf[100];
    Json::Value any;
    any["touser"] = stDaoOrderInfo.m_order_info.openid_slave();
    any["template_id"] = m_order_finish_template_id;

	std::string strUrl = m_order_finish_url;
	strUrl.replace(strUrl.find("{{ORDERID}}"), strlen("{{ORDERID}}"), int_2_str(stDaoOrderInfo.m_order_info.order_id()));
	any["url"] = strUrl;
    any["data"]["first"]["value"] = m_order_finish_first;
    any["data"]["first"]["color"] = m_order_finish_color;
    any["data"][m_order_finish_keyword1]["value"] = int_2_str(stDaoOrderInfo.m_order_info.order_id());
    any["data"][m_order_finish_keyword1]["color"] = m_order_finish_color;
    any["data"][m_order_finish_keyword2]["value"] = stDaoUserInfo.m_user_info.nickname();
    any["data"][m_order_finish_keyword2]["color"] = m_order_finish_color;

	sprintf(buf, "%.2f元", stDaoOrderInfo.m_order_info.price() / 100.0);
    any["data"][m_order_finish_keyword3]["value"] = buf;
    any["data"][m_order_finish_keyword3]["color"] = m_order_finish_color;
    any["data"]["remark"]["value"] = m_order_finish_remark;
    any["data"]["remark"]["color"] = m_order_finish_color;

    std::string strPostData = any.toJsonString();
    LOG4CPLUS_DEBUG(logger, "postdata = " << strPostData);

    //调用微信api
    common::wxapi::WXAPIMessage wxapiMessage;
    std::string strBaseAccessToken;
    std::string strJSAPITicket;
	g_token_server_processor->get_pa_info(m_pa_appid, strBaseAccessToken, strJSAPITicket);

    int iRet = 0;

    if(wxapiMessage.SendTemplateMessage(strBaseAccessToken, strPostData, iRet) < 0)
    {
        LOG4CPLUS_ERROR(logger, "WXAPI.SendTemplateMessage failed, errcode = " << iRet);
        return -1;
    }
    return 0;
}

int ExecutorThreadOrder::send_message_of_order_finish(const DaoOrderInfo& stDaoOrderInfo)
{
   	uint64_t qwTs = stDaoOrderInfo.m_order_info.finish_ts();
    //8 hour
    long dwTs = qwTs + 60 * 60 * 8;
    struct tm *p;
    p = gmtime(&dwTs);
    char bufTs[80];
    strftime(bufTs, 80, "%Y年%m月%d日 %H:%M", p);
    LOG4CPLUS_DEBUG(logger, "ts_str = " << bufTs);


	char chrPrice[100];
	sprintf(chrPrice, "%.2f元", stDaoOrderInfo.m_order_info.price() / 100.0);

	string strErrMsg;
	DaoUserInfo stDaoUserInfo(m_db_table_user_info_name);
	if(stDaoUserInfo.GetByPrimaryKey(stDaoOrderInfo.m_order_info.openid_master(), m_mysql_helper, strErrMsg) != DAO_RET_OK)
	{
		LOG4CPLUS_ERROR(logger, "DaoUserInfo.GetByPrimaryKey failed, msg = " << strErrMsg);
		return -1;
	}

	ostringstream oss;
	oss.str("");
	oss << "订单确认通知\n"
		<< "\n"
		<< "您抢的订单已被对方确认\n"
		<< "单号：" << stDaoOrderInfo.m_order_info.order_id() << "\n"
		<< "下单人：" << stDaoUserInfo.m_user_info.nickname() << "\n"
		<< "确认时间：" << bufTs << "\n"
		<< "如需提现，请前往个人中心处理（可参考下方ziga手册-怎样提现）";

	LOG4CPLUS_DEBUG(logger, "content=" << oss.str());

    Json::Value any;
	//微信限制群发openid至少为2
    any["touser"].append(stDaoOrderInfo.m_order_info.openid_slave());
	any["touser"].append(stDaoOrderInfo.m_order_info.openid_slave());
	any["msgtype"] = "text";
	any["text"]["content"] = oss.str();


    std::string strPostData = any.toJsonString();
    LOG4CPLUS_DEBUG(logger, "postdata = " << strPostData);

    //调用微信api
    common::wxapi::WXAPIMessage wxapiMessage;
    std::string strBaseAccessToken;
    std::string strJSAPITicket;
	g_token_server_processor->get_pa_info(m_pa_appid, strBaseAccessToken, strJSAPITicket);

    int iRet = 0;

    if(wxapiMessage.SendMessage(strBaseAccessToken, strPostData, iRet) < 0)
    {
        LOG4CPLUS_ERROR(logger, "WXAPI.SendMessage failed, errcode = " << iRet);
        return -1;
    }
	return 0;
}
