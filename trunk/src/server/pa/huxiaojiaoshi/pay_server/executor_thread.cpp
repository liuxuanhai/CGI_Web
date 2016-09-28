#include "executor_thread.h"
#include "global_var.h"
#include "common_util.h"
#include "wx_api_pay.h"

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
	m_xml_document = new TiXmlDocument();
	const lce::app::Config& stConfig = g_server->config();
	assert(m_mysql_helper.Init(stConfig.get_string_param("DB", "ip")
				, stConfig.get_string_param("DB", "db_name")
				, stConfig.get_string_param("DB", "user")
				, stConfig.get_string_param("DB", "passwd")
				, stConfig.get_int_param("DB", "port")));
	listen_price = stConfig.get_int_param("ORDER", "listen_price");
	share_percent_default = ((float)stConfig.get_int_param("ORDER", "share_percent_default"))/100;
	share_percent_good = ((float)stConfig.get_int_param("ORDER", "share_percent_good"))/100;
	share_percent_bad = ((float)stConfig.get_int_param("ORDER", "share_percent_bad"))/100;	
	pa_appid = stConfig.get_string_param("PA_INFO", "appid");

	answer_body = stConfig.get_string_param("UNIFIED_ORDER", "answer_body");
	listen_body = stConfig.get_string_param("UNIFIED_ORDER", "listen_body");
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
			case ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_REQ:
				process_unified_order(request, reply);
				break;
			case ExecutorThreadRequestType::T_PAY_TRANSFER_REQ:
				process_transfer_order(request, reply);
				break;
			case ExecutorThreadRequestType::T_PAY_GET_USER_CASH_REQ:
				process_get_user_cash(request, reply);
				break;
			case ExecutorThreadRequestType::T_PAY_QUERY_CASH_FLOW_REQ:
				process_query_cash_flow(request, reply);
				break;
			case ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_CALLBACK_REQ:
				process_unified_order_callback(request, reply);
				break;
			case ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_QUERY_REQ:
				process_unified_order_query(request, reply);
				break;
			case ExecutorThreadRequestType::T_PAY_LISTEN_INCOME_REQ:
				process_listen_income(request, reply);
				break;
			case ExecutorThreadRequestType::T_PAY_SENDREDPACK_REQ:
				process_sendredpack(request, reply);
				break;
			case ExecutorThreadRequestType::T_PAY_OUTCOME_QUERY_REQ:
				process_outcome_req_query(request, reply);
				break;
			case ExecutorThreadRequestType::T_PAY_OUTCOME_COMMIT_REQ:
				process_outcome_req_commit(request, reply);
				break;				
			case ExecutorThreadRequestType::T_PAY_OUTCOME_CHECK_REQ:
				process_outcome_req_check(request, reply);
				break;
			case ExecutorThreadRequestType::T_PAY_OUTCOME_UPDATE_REQ:
				process_outcome_req_update(request, reply);
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

int ExecutorThread::getUnifiedOrderBusinessInfo(const std::string& openid, const std::string& out_trade_no, ExecutorThreadResponseElement& reply)
{
	std::ostringstream sql;
	sql.str("");
	sql<<"SELECT business_type,business_id FROM t_unifiedorder WHERE openid='"<<sql_escape(openid)<<"'"
	   <<" AND out_trade_no='"<<sql_escape(out_trade_no)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::getUnifiedOrderBusinessInfo failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());		
        return -1;		
	}

	
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			reply.m_business_type = strtoul(m_mysql_helper.GetRow(0), NULL, 10);
			reply.m_business_id = strtoul(m_mysql_helper.GetRow(1), NULL, 10);
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::getUnifiedOrderBusinessInfo end, openid: "<<openid<<"out_trade_no: "<<out_trade_no<<" not exists");
		return -1;
	}	

	return 0;	
}

bool ExecutorThread::getUserCashInfo(const std::string& openid, UserCash& tUserCash)
{
	tUserCash.total_income = 0;
	tUserCash.total_outcome = 0;
	tUserCash.total_outcome_pending = 0;
	
	std::ostringstream sql;
	sql.str("");
	sql<<"SELECT * FROM t_user_cash WHERE openid='"<<sql_escape(openid)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::getUserCashInfo failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());		
        return false;		
	}

	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			tUserCash.total_income = strtoul(m_mysql_helper.GetRow(1), NULL, 10);
    		tUserCash.total_outcome = strtoul(m_mysql_helper.GetRow(2), NULL, 10);
    		tUserCash.total_outcome_pending = strtoul(m_mysql_helper.GetRow(3), NULL, 10);    		
		}
	}
	else
	{
		LOG4CPLUS_DEBUG(logger, "ExecutorThread::getUserCashInfo end, openid: "<<openid<<" not exists, add new user cash!!!");
		sql.str("");
		sql<<"INSERT INTO t_user_cash SET openid='"<<sql_escape(openid)<<"'"
		   <<", total_income=0, total_outcome=0, total_outcome_pending=0";

    	LOG4CPLUS_DEBUG(logger, sql.str());
    	if(!m_mysql_helper.Query(sql.str()))
		{
			LOG4CPLUS_ERROR(logger, "insert failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());			
	        return false;		
		}				
	}

	return true;	
}

int ExecutorThread::checkOutTradeNoFromWX(const std::string& out_trade_no, const std::string& openid)
{
	UnifiedOrderQueryReq stUnifiedOrderQueryReq;
	stUnifiedOrderQueryReq.out_trade_no = out_trade_no;
	stUnifiedOrderQueryReq.init();

	std::string strReqBody = stUnifiedOrderQueryReq.UnifiedOrderQueryReqXMLbulid();	
	
	::common::wxapi::WXAPIPAY stWXAPIPay;	
	std::string strRspBody = "";
	int iRet = stWXAPIPay.PaPayUnifiedOrderQuery(strReqBody, strRspBody);

	if(iRet < 0)
	{		
		return -1;
	}

	std::map<std::string, std::string> map_param;
	
	UnifiedOrderQueryRsp stUnifiedOrderQueryRsp;
	if(stUnifiedOrderQueryRsp.UnifiedOrderQueryRspXMLparse(strRspBody, map_param, m_xml_document) < 0)
	{		
		return -1;
	}

	int type = stUnifiedOrderQueryRsp.UnifiedOrderQueryRspCheck(openid, stUnifiedOrderQueryReq, map_param);	
	if(type != Order::TRADE_STATE_SUCCESS)
	{
		//说明该out_trade_no支付有问题		
		return Order::TRADE_STATE_FAIL;
	}		
	
	return Order::TRADE_STATE_SUCCESS;
}



void ExecutorThread::process_unified_order(ExecutorThreadRequestElement & request,ExecutorThreadResponseElement & reply)
{	
	UnifiedOrderReq stUnifiedOrderReq;
	stUnifiedOrderReq.openid = request.m_openid;
	stUnifiedOrderReq.spbill_create_ip = request.m_client_ip;
	stUnifiedOrderReq.init(request.m_business_type);
	
	std::string strReqBody = stUnifiedOrderReq.UnifiedOrderReqXMLbuild();	
	
	::common::wxapi::WXAPIPAY stWXAPIPay;	
	std::string strRspBody = "";
	int iRet = stWXAPIPay.PaPayUnifiedOrder(strReqBody, strRspBody);

	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}
	
	std::map<std::string, std::string> map_param;
	UnifiedOrderRsp stUnifiedorderRsp;
	if(stUnifiedorderRsp.UnifiedOrderRspXMLparse(strRspBody, map_param, m_xml_document) < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}	

	if(stUnifiedorderRsp.UnifiedOrderRspCheck(stUnifiedOrderReq, map_param) < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	//insert t_unifiedorder
	stUnifiedOrderReq.UnifiedOrderReqInsertDB(m_mysql_helper, stUnifiedorderRsp.prepay_id, request.m_business_type, request.m_business_id); 

	//gen return code
	reply.m_appid = stUnifiedOrderReq.appid;
	reply.m_time_stamp = int_2_str(time(NULL));
	reply.m_nonce_str = genRandomString(30);	
	reply.m_package = "prepay_id=" + stUnifiedorderRsp.prepay_id;
	reply.m_sign_type = "MD5";

	std::map<std::string, std::string> tmp_map;						
	tmp_map.insert(pair<std::string, std::string>("appId", reply.m_appid));
	tmp_map.insert(pair<std::string, std::string>("timeStamp", reply.m_time_stamp));
	tmp_map.insert(pair<std::string, std::string>("nonceStr", reply.m_nonce_str));
	tmp_map.insert(pair<std::string, std::string>("package",  reply.m_package));
	tmp_map.insert(pair<std::string, std::string>("signType", reply.m_sign_type));
	
	reply.m_pay_sign = genSignString(tmp_map, stUnifiedOrderReq.base_api_key);
	reply.m_out_trade_no = stUnifiedOrderReq.out_trade_no;
	
}

void ExecutorThread::process_transfer_order(ExecutorThreadRequestElement & request,ExecutorThreadResponseElement & reply)
{
	//1. TransferOrderReq init;
	TransferOrderReq stTransferOrderReq;
	stTransferOrderReq.openid = request.m_openid;
	stTransferOrderReq.amount = request.m_amount;
	stTransferOrderReq.spbill_create_ip = request.m_client_ip;
	stTransferOrderReq.init();
	
	//2. check user's total_outcome_pending from t_user_cash 	
	if(!getUserCashInfo(request.m_openid, reply.m_userCash))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}	
	
	if(request.m_amount > reply.m_userCash.total_outcome_pending)
	{
		//提现金额大于用户申请的提现金额
		reply.m_result_code = ExecutorThreadRequestType::E_BALANCE_NOT_ENOUGH;
		return;
	}
	

	//3. do req
	std::string strReqBody = stTransferOrderReq.TransferOrderReqXMLbuild();	
	
	::common::wxapi::WXAPIPAY stWXAPIPay;	
	std::string strRspBody = "";
	int iRet = stWXAPIPay.PaPayTransferOrder(strReqBody, strRspBody);

	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	//4. insert transferorder req into DB
	stTransferOrderReq.TransferOrderReqInsertDB(m_mysql_helper);	

	std::map<std::string, std::string> map_param;
	
	TransferOrderRsp stTransferorderRsp;
	if(stTransferorderRsp.TransferOrderRspXMLparse(strRspBody, map_param, m_xml_document) < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	if(stTransferorderRsp.TransferOrderRspCheck(stTransferOrderReq, map_param) < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	//5. insert transferorder rsp into DB
	stTransferorderRsp.TransferOrderRspInsertDB(m_mysql_helper, stTransferOrderReq.openid);	

	//6. update t_user_cash
	std::ostringstream sql;
	sql.str("");
	sql<<"UPDATE t_user_cash SET total_outcome=total_outcome+"<<stTransferOrderReq.amount
	   <<", total_outcome_pending=total_outcome_pending-"<<stTransferOrderReq.amount
	   <<" WHERE openid='"<<sql_escape(stTransferOrderReq.openid)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_transfer_order update t_user_cash failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}

	//7. get user cash	
	reply.m_userCash.total_outcome += stTransferOrderReq.amount;
	reply.m_userCash.total_outcome_pending -= stTransferOrderReq.amount;		
	
	//8. insert t_cash_flow
	int64_t balance = reply.m_userCash.total_income - reply.m_userCash.total_outcome;
	if(balance < 0)
	{
		LOG4CPLUS_ERROR(logger, "openid: "<<stTransferOrderReq.openid<<" balance: "<<balance<<" < 0");
		return;
	}
	uint64_t table_id = ::common::util::generate_unique_id();	
	sql.str("");
	sql<<"INSERT t_cash_flow SET table_id="<<table_id
	   <<", openid='"<<sql_escape(stTransferOrderReq.openid)<<"'"
	   <<", out_trade_no='"<<sql_escape(stTransferOrderReq.partner_trade_no)<<"'"
	   <<", out_trade_openid='"<<sql_escape(stTransferOrderReq.openid)<<"'"
	   <<", amount="<<stTransferOrderReq.amount
	   <<", type="<<Order::CASH_FLOW_TYPE_OUT
	   <<", reason="<<Order::CASH_FLOW_REASON_OUTCOME
	   <<", balance="<<balance
	   <<", create_ts=unix_timestamp()";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_transfer_order insert t_cash_flow failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}

	//9. update t_outcome_req
	sql.str("");
	sql<<"UPDATE t_outcome_req SET state="<<Order::OUTCOME_STATE_PASS
	   <<" WHERE table_id="<<request.m_table_id
	   <<" AND openid='"<<sql_escape(request.m_openid)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_transfer_order update t_outcome_req failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}	
	 
	return;	
}

void ExecutorThread::process_get_user_cash(ExecutorThreadRequestElement & request,ExecutorThreadResponseElement & reply)
{
	if(!getUserCashInfo(request.m_openid, reply.m_userCash))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;	
	}
    
	return;	
}

void ExecutorThread::process_query_cash_flow(ExecutorThreadRequestElement & request,ExecutorThreadResponseElement & reply)
{
	std::ostringstream sql;
	sql.str("");
	if(request.m_begin_ts == 0)
	{
		sql<<"SELECT * FROM t_cash_flow WHERE openid='"<<sql_escape(request.m_openid)<<"'"
		   <<" ORDER BY create_ts DESC LIMIT "<<request.m_limit;			
	}
	else
	{		
		sql<<"SELECT * FROM t_cash_flow WHERE openid='"<<sql_escape(request.m_openid)<<"'"
		   <<" AND create_ts < "<<request.m_begin_ts
		   <<" ORDER BY create_ts DESC LIMIT "<<request.m_limit;			
	}

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_query_cash_flow failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}

	reply.vecCashFlowInfo.clear();
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			hoosho::j::commstruct::CashFlowInfo item;
			item.set_openid(m_mysql_helper.GetRow(1));
			item.set_out_trade_no(m_mysql_helper.GetRow(2));
			item.set_out_trade_openid(m_mysql_helper.GetRow(3));
			item.set_amount(strtoul(m_mysql_helper.GetRow(4), NULL, 10));
			item.set_type(strtoul(m_mysql_helper.GetRow(5), NULL, 10));
			item.set_reason(strtoul(m_mysql_helper.GetRow(6), NULL, 10));
			item.set_balance(strtoul(m_mysql_helper.GetRow(7), NULL, 10));
			item.set_create_ts(strtoul(m_mysql_helper.GetRow(8), NULL, 10));

			reply.vecCashFlowInfo.push_back(item);
		}
	}
    

	return;		
}

void ExecutorThread::process_unified_order_callback(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	UnifiedOrderCallbackRsp rsp;
	std::string strErrMsg = "";

	std::map<std::string, std::string> map_param;
	if(rsp.UnifiedOrderCallbackRspXMLparse(request.m_callback_xml, map_param, strErrMsg, m_xml_document) < 0)
	{
		reply.m_return_code = "FAIL";
		reply.m_return_msg = strErrMsg;
		return;
	}		

	strErrMsg = "";
	if(rsp.UnifiedOrderCallbackRspCheck(map_param, strErrMsg) < 0)
	{
		reply.m_return_code = "FAIL";
		reply.m_return_msg = strErrMsg;

		//update t_unifiedorder
		rsp.UnifiedOrderCallbackRspUpdateDB(m_mysql_helper, Order::TRADE_STATE_FAIL);
		return;
	}	

	//update t_unifiedorder
	rsp.UnifiedOrderCallbackRspUpdateDB(m_mysql_helper, Order::TRADE_STATE_SUCCESS);

	reply.m_return_code = "SUCCESS";
	reply.m_return_msg = "OK";

	//fetch order info from t_unifiedorder
	if(getUnifiedOrderBusinessInfo(rsp.openid, rsp.out_trade_no, reply) < 0)
	{
		return;
	}
	reply.m_out_trade_no = rsp.out_trade_no;
	
	//get user balance
	UserCash stUserCash;
	if(!getUserCashInfo(rsp.openid, stUserCash))
	{
		return;
	}
	
	int64_t balance = stUserCash.total_income - stUserCash.total_outcome;
	if(balance < 0)
	{
		LOG4CPLUS_ERROR(logger, "FetchUser balance ERROR, balance: "<<balance<<" < 0");
		return;
	}
	
	//insert t_cash_flow
	uint64_t table_id = ::common::util::generate_unique_id();
	std::ostringstream sql;
	sql.str("");
	sql<<"INSERT t_cash_flow SET table_id="<<table_id
	   <<", openid='"<<sql_escape(rsp.openid)<<"'"
	   <<", out_trade_no='"<<sql_escape(rsp.out_trade_no)<<"'"
	   <<", out_trade_openid='"<<sql_escape(rsp.openid)<<"'"
	   <<", amount="<<rsp.total_fee
	   <<", type="<<Order::CASH_FLOW_TYPE_OUT
	   <<", reason="<<reply.m_business_type
	   <<", balance="<<balance
	   <<", create_ts=unix_timestamp()";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_unified_order_callback insert t_cash_flow failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        return;		
	}
	
}

void ExecutorThread::process_unified_order_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{	
	//do req
	reply.vecFaultOutTradeNo.clear();
	for(size_t i=0; i<request.vecOutTradeNo.size(); i++)
	{
		std::string tmpOutTradeNo = request.vecOutTradeNo[i];
		int iRet = checkOutTradeNoFromWX(tmpOutTradeNo, request.m_openid);
		if(iRet < 0)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
			return;
		}
		else if(iRet == Order::TRADE_STATE_FAIL)
		{
			reply.m_result_code = ExecutorThreadRequestType::E_PAY_FAIL;
			reply.vecFaultOutTradeNo.push_back(request.vecOutTradeNo[i]);			
		}		
	}

	return;	
}

void ExecutorThread::process_listen_income(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	if(request.m_openid.empty() || request.m_out_trade_openid.empty())
	{
		LOG4CPLUS_ERROR(logger, "invalid openid: empty!");
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}
	
	//首先去微信那边校验该out_trade_no
	int iRet = checkOutTradeNoFromWX(request.m_out_trade_no, request.m_out_trade_openid);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}
	else if(iRet == Order::TRADE_STATE_FAIL)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_PAY_FAIL;
		return;
	}
	
	
	//根据out_trade_no, 查询用户现金流水表, 如果该out_trade_no对应的偷听分成记录已经存在，
	//说明该out_trade_no已经对用户产生了偷听分成收益,则不能再次产生收益, 防止用同一out_trade_no刷收入
	//check from t_cash_flow first
	std::ostringstream sql;
	sql.str("");
	sql<<"SELECT * FROM t_cash_flow WHERE openid='"<<sql_escape(request.m_openid)<<"'"
	   <<" AND out_trade_no='"<<sql_escape(request.m_out_trade_no)<<"'"
	   <<" AND out_trade_openid='"<<sql_escape(request.m_out_trade_openid)<<"'"
	   <<" AND type="<<Order::CASH_FLOW_TYPE_IN
	   <<" AND reason="<<Order::CASH_FLOW_REASON_LISTEN_INCOME;

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_listen_income failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return; 	
	}

	if(m_mysql_helper.GetRowCount() != 0)
	{
		//说明已有记录, 直接返回失败
		reply.m_result_code = ExecutorThreadRequestType::E_REPEAT_LISTEN_INCOME;
		return; 
	}	
	
	//get user balance
	UserCash stUserCash;
	if(!getUserCashInfo(request.m_openid, stUserCash))
	{
		return;
	}
	
	int64_t balance = stUserCash.total_income - stUserCash.total_outcome;	
	if(balance < 0)
	{
		LOG4CPLUS_ERROR(logger, "Fetch User balance ERROR, balance: "<<balance<<" < 0");
		return;
	}

	
	//update t_user_cash
	uint64_t user_increase = 0;
	
	sql.str("");
	if(request.m_record_type == 1) //new
	{
		if(request.m_share_type == ::hoosho::j::commenum::COMMENT_TYPE_DEFAULT) //default
		{
			user_increase = listen_price * share_percent_default;	
		}
		else if(request.m_share_type == ::hoosho::j::commenum::COMMENT_TYPE_GOOD) //good
		{
			user_increase = listen_price * share_percent_good;			
		}
		else if(request.m_share_type == ::hoosho::j::commenum::COMMENT_TYPE_BAD) //bad
		{
			user_increase = listen_price * share_percent_bad;	
		}		
		else
		{
			LOG4CPLUS_ERROR(logger,"process_listen_income param err, m_share_type="<<request.m_share_type);
			reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
			return;
		}
	
		sql<<"UPDATE t_user_cash SET total_income=total_income+"<<user_increase
		   <<" WHERE openid='"<<sql_escape(request.m_openid)<<"'";
	}	
	else
	{
		LOG4CPLUS_ERROR(logger,"process_listen_income param err, m_record_type="<<request.m_record_type);
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_listen_income failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return; 	
	}	

	balance += user_increase; 
	
	//update t_cash_flow
	uint64_t table_id = ::common::util::generate_unique_id();

	sql.str("");
	sql<<"INSERT INTO t_cash_flow SET table_id="<<table_id
		   <<", openid='"<<sql_escape(request.m_openid)<<"'"
		   <<", out_trade_no='"<<sql_escape(request.m_out_trade_no)<<"'"
		   <<", out_trade_openid='"<<sql_escape(request.m_out_trade_openid)<<"'"
		   <<", amount="<<user_increase
		   <<", type="<<Order::CASH_FLOW_TYPE_IN
		   <<", reason="<<Order::CASH_FLOW_REASON_LISTEN_INCOME
		   <<", balance="<<balance
		   <<", create_ts=unix_timestamp()";			

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_listen_income failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return; 	
	}

	return;	
}

void ExecutorThread::process_sendredpack(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1. SendRedPackReq init;
	SendRedPackReq stSendRedPackReq;
	stSendRedPackReq.re_openid = request.m_openid;
	stSendRedPackReq.total_amount = request.m_amount;
	stSendRedPackReq.client_ip = request.m_client_ip;
	stSendRedPackReq.init();

	//2. check user's total_outcome_pending from t_user_cash 	
	if(!getUserCashInfo(request.m_openid, reply.m_userCash))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}	
	
	if(request.m_amount > reply.m_userCash.total_outcome_pending)
	{
		//提现金额大于用户申请的提现金额
		reply.m_result_code = ExecutorThreadRequestType::E_BALANCE_NOT_ENOUGH;
		return;
	}

	//3. do req
	std::string strReqBody = stSendRedPackReq.SendRedPackReqXMLbuild();	
	
	::common::wxapi::WXAPIPAY stWXAPIPay;	
	std::string strRspBody = "";
	int iRet = stWXAPIPay.PaPaySendRedPack(strReqBody, strRspBody);

	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	//4. insert sendredpack req into DB
	stSendRedPackReq.SendRedPackReqInsertDB(m_mysql_helper);	

	std::map<std::string, std::string> map_param;
	
	SendRedPackRsp stSendRedPackRsp;
	if(stSendRedPackRsp.SendRedPackRspXMLparse(strRspBody, map_param, m_xml_document) < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	//5. insert sendredpack rsp into DB
	stSendRedPackRsp.SendRedPackRspInsertDB(m_mysql_helper);	
	
	if(stSendRedPackRsp.SendRedPackRspCheck(stSendRedPackReq, map_param) < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_PAY_FAIL;
		return;
	}	

	//6. update t_user_cash
	std::ostringstream sql;
	sql.str("");
	sql<<"UPDATE t_user_cash SET total_outcome=total_outcome+"<<stSendRedPackRsp.total_amount
	   <<", total_outcome_pending=total_outcome_pending-"<<stSendRedPackRsp.total_amount
	   <<" WHERE openid='"<<sql_escape(stSendRedPackRsp.re_openid)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_sendredpack update t_user_cash failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}

	//7. get user cash
	reply.m_userCash.total_outcome += stSendRedPackRsp.total_amount;
	reply.m_userCash.total_outcome_pending -= stSendRedPackRsp.total_amount;	
	
	//8. insert t_cash_flow
	int64_t balance = reply.m_userCash.total_income - reply.m_userCash.total_outcome;
	if(balance < 0)
	{
		LOG4CPLUS_ERROR(logger, "openid: "<<stSendRedPackRsp.re_openid<<" balance: "<<balance<<" < 0");
		return;
	}
	
	uint64_t table_id = ::common::util::generate_unique_id();	
	sql.str("");
	sql<<"INSERT t_cash_flow SET table_id="<<table_id
	   <<", openid='"<<sql_escape(stSendRedPackRsp.re_openid)<<"'"
	   <<", out_trade_no='"<<sql_escape(stSendRedPackRsp.mch_billno)<<"'"
	   <<", out_trade_openid='"<<sql_escape(stSendRedPackRsp.re_openid)<<"'"
	   <<", amount="<<stSendRedPackRsp.total_amount
	   <<", type="<<Order::CASH_FLOW_TYPE_OUT
	   <<", reason="<<Order::CASH_FLOW_REASON_OUTCOME
	   <<", balance="<<balance
	   <<", create_ts=unix_timestamp()";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_sendredpack insert t_cash_flow failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}

	//9. update t_outcome_req
	sql.str("");
	sql<<"UPDATE t_outcome_req SET state="<<Order::OUTCOME_STATE_PASS
	   <<" WHERE table_id="<<request.m_table_id
	   <<" AND openid='"<<sql_escape(stSendRedPackRsp.re_openid)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_sendredpack update t_outcome_req failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}	
	 
	return;	
}

void ExecutorThread::process_outcome_req_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::ostringstream sql;
	sql.str("");
	if(request.m_openid.empty())	//query by manager
	{
		if(request.m_begin_ts == 0)
		{
			sql<<"SELECT * FROM t_outcome_req WHERE state="<<request.m_state
			   <<" ORDER BY create_ts DESC LIMIT "<<request.m_limit;			
		}
		else
		{		
			sql<<"SELECT * FROM t_outcome_req WHERE state="<<request.m_state
			   <<" AND create_ts < "<<request.m_begin_ts
			   <<" ORDER BY create_ts DESC LIMIT "<<request.m_limit;			
		}	
	}
	else	//query by user
	{
		if(request.m_begin_ts == 0)
		{
			sql<<"SELECT * FROM t_outcome_req WHERE openid='"<<sql_escape(request.m_openid)<<"'"
			   <<" ORDER BY create_ts DESC LIMIT "<<request.m_limit;			
		}
		else
		{		
			sql<<"SELECT * FROM t_outcome_req WHERE openid='"<<sql_escape(request.m_openid)<<"'"
			   <<" AND create_ts < "<<request.m_begin_ts
			   <<" ORDER BY create_ts DESC LIMIT "<<request.m_limit;			
		}	
	}
	

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_outcome_req_query failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}

	reply.vecOutcomeReqInfo.clear();
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{
			hoosho::j::commstruct::OutcomeReqInfo item;
			item.set_table_id(strtoul(m_mysql_helper.GetRow(0), NULL, 10));
			item.set_openid(m_mysql_helper.GetRow(1));			
			item.set_amount(strtoul(m_mysql_helper.GetRow(2), NULL, 10));
			item.set_state(strtoul(m_mysql_helper.GetRow(3), NULL, 10));
			item.set_create_ts(strtoul(m_mysql_helper.GetRow(4), NULL, 10));

			reply.vecOutcomeReqInfo.push_back(item);
		}
	}
    

	return;		
}

void ExecutorThread::process_outcome_req_commit(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//get t_user_cash	
	if(!getUserCashInfo(request.m_openid, reply.m_userCash))
	{		
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	
	int64_t outcome_balance = reply.m_userCash.total_income - reply.m_userCash.total_outcome - reply.m_userCash.total_outcome_pending;	
	if((outcome_balance < 0) || ((uint64_t)outcome_balance < request.m_amount))
	{
		LOG4CPLUS_ERROR(logger, "INVALID OUTCOME REQ, balance: "<<outcome_balance<<" < req.amount:"<<request.m_amount);
		reply.m_result_code = ExecutorThreadRequestType::E_BALANCE_NOT_ENOUGH;
		return;
	}

	//insert t_outcome_req
	uint64_t table_id = ::common::util::generate_unique_id();
	std::ostringstream sql;
	sql.str("");
	sql<<"INSERT INTO t_outcome_req SET table_id="<<table_id
	   <<", openid='"<<sql_escape(request.m_openid)<<"'"
	   <<", amount="<<request.m_amount
	   <<", state="<<Order::OUTCOME_STATE_PENDING
	   <<", create_ts=unix_timestamp()";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_outcome_req_commit failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}

	//update t_user_cash
	sql.str("");
	sql<<"UPDATE t_user_cash SET total_outcome_pending=total_outcome_pending+"<<request.m_amount
	   <<" WHERE openid='"<<sql_escape(request.m_openid)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_outcome_req_commit failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}

	reply.m_userCash.total_outcome_pending += request.m_amount;

	return;
}


void ExecutorThread::process_outcome_req_check(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1. check record from t_outcome_req
	std::ostringstream sql;
	sql.str("");
	sql<<"SELECT amount,state FROM t_outcome_req WHERE table_id="<<request.m_table_id
	   <<" AND openid='"<<sql_escape(request.m_openid)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_outcome_req_check failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());	
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return ;		
	}
	
	uint64_t tmpAmount;
	uint64_t tmpState;
	if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{			
    		tmpAmount = strtoul(m_mysql_helper.GetRow(0), NULL, 10);
    		tmpState = strtoul(m_mysql_helper.GetRow(1), NULL, 10);
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_outcome_req_check failed, table_id:"<<request.m_table_id<<" not exist!!!");
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	//提现金额不符
	if(tmpAmount != request.m_amount)
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_outcome_req_check failed, req.amount: "<<request.m_amount<<" not equal db.amount: "<<tmpAmount);
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}

	//该记录已经被成功处理
	if(tmpState == Order::OUTCOME_STATE_PASS)
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_outcome_req_check failed, this record already handle! db.state: OUTCOME_STATE_PASS");
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		return;
	}
	
	//2. check user's total_outcome_pending from t_user_cash 	
	UserCash stUserCash;
	if(!getUserCashInfo(request.m_openid, stUserCash))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}	
	
	if(request.m_amount > stUserCash.total_outcome_pending)
	{
		//提现金额大于用户申请的提现金额
		reply.m_result_code = ExecutorThreadRequestType::E_BALANCE_NOT_ENOUGH;		
		return;
	}

	return;
}

void ExecutorThread::process_outcome_req_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{	
	//只有当提现处理传递 "处理成功" 状态时，才更新 t_user_cash, 并记录现金流水
	std::ostringstream sql;
	if(request.m_state == Order::OUTCOME_STATE_PASS)
	{
		//1. update t_user_cash		
		sql.str("");
		sql<<"UPDATE t_user_cash SET total_outcome=total_outcome+"<<request.m_amount
		   <<", total_outcome_pending=total_outcome_pending-"<<request.m_amount
		   <<" WHERE openid='"<<sql_escape(request.m_openid)<<"'";

		LOG4CPLUS_DEBUG(logger, sql.str());
		if(!m_mysql_helper.Query(sql.str()))
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThread::process_outcome_req_update t_user_cash failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
	        return;		
		}

		//2. get user cash
		UserCash stUserCash;
		if(!getUserCashInfo(request.m_openid, stUserCash))
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}		
		
		//3. insert t_cash_flow
		int64_t balance = stUserCash.total_income - stUserCash.total_outcome;
		if(balance < 0)
		{
			LOG4CPLUS_ERROR(logger, "openid: "<<request.m_openid<<" balance: "<<balance<<" < 0");
			return;
		}
		uint64_t table_id = ::common::util::generate_unique_id();	
		sql.str("");
		sql<<"INSERT t_cash_flow SET table_id="<<table_id
		   <<", openid='"<<sql_escape(request.m_openid)<<"'"
		   <<", out_trade_no='"<<sql_escape(request.m_out_trade_no)<<"'"
		   <<", out_trade_openid='"<<sql_escape(request.m_openid)<<"'"
		   <<", amount="<<request.m_amount
		   <<", type="<<Order::CASH_FLOW_TYPE_OUT
		   <<", reason="<<Order::CASH_FLOW_REASON_OUTCOME
		   <<", balance="<<balance
		   <<", create_ts=unix_timestamp()";

		LOG4CPLUS_DEBUG(logger, sql.str());
		if(!m_mysql_helper.Query(sql.str()))
		{
			LOG4CPLUS_ERROR(logger, "ExecutorThread::process_outcome_req_update insert t_cash_flow failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
	        return;		
		}
	}	

	//4. update t_outcome_req
	sql.str("");
	sql<<"UPDATE t_outcome_req SET state="<<request.m_state
	   <<" WHERE table_id="<<request.m_table_id
	   <<" AND openid='"<<sql_escape(request.m_openid)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_outcome_req_update update t_outcome_req failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;		
	}	
	 
	return;		
}



