#include "pay_order.h"
#include "global_var.h"
#include "common_util.h"

IMPL_LOGGER(Order, logger)

Order::Order()
{
	const lce::app::Config& stConfig = g_server->config();
	
	base_appid = stConfig.get_string_param("PA_INFO", "appid");
	base_appsecret = stConfig.get_string_param("PA_INFO", "appsecret");
	base_mch_id = stConfig.get_string_param("PA_INFO", "mch_id");
	base_api_key = stConfig.get_string_param("PA_INFO", "api_key");

	listen_price = stConfig.get_int_param("ORDER", "listen_price");
	answer_price = stConfig.get_int_param("ORDER", "answer_price");	
	base_order_expire = stConfig.get_int_param("ORDER", "order_expire");	

	base_local_ip = stConfig.get_string_param("LOCAL", "ip");

	//unifiedorder default value
	ufOrder_device_info = stConfig.get_string_param("UNIFIED_ORDER", "device_info");
	ufOrder_answer_body = stConfig.get_string_param("UNIFIED_ORDER", "answer_body");
	ufOrder_listen_body = stConfig.get_string_param("UNIFIED_ORDER", "listen_body");
	ufOrder_detail = stConfig.get_string_param("UNIFIED_ORDER", "detail");
	ufOrder_attach = stConfig.get_string_param("UNIFIED_ORDER", "attach");
	ufOrder_fee_type = stConfig.get_string_param("UNIFIED_ORDER", "fee_type");
	ufOrder_trade_type = stConfig.get_string_param("UNIFIED_ORDER", "trade_type");
	ufOrder_limit_pay = stConfig.get_string_param("UNIFIED_ORDER", "limit_pay");
	ufOrder_notify_url = stConfig.get_string_param("UNIFIED_ORDER", "notify_url");

	//sendredpack default value
	rpOrder_act_name = stConfig.get_string_param("SENDREDPACK_ORDER", "act_name");
	rpOrder_send_name = stConfig.get_string_param("SENDREDPACK_ORDER", "send_name");
	rpOrder_wishing = stConfig.get_string_param("SENDREDPACK_ORDER", "wishing");
	rpOrder_remark = stConfig.get_string_param("SENDREDPACK_ORDER", "remark");

	//transferorder default value
	tfOrder_desc = stConfig.get_string_param("TRANSFER_ORDER", "desc");
	tfOrder_check_name = stConfig.get_string_param("TRANSFER_ORDER", "check_name");
}

std::string Order::Order2Xml(const std::map<std::string, std::string>& mapParams)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"<xml>";

	for(std::map<std::string, std::string>::const_iterator iter = mapParams.begin(); iter != mapParams.end(); ++iter)
	{
		const std::string strKey = iter->first;
		const std::string strValue = iter->second;

		oss<<"<"<<strKey<<">"; 
		oss<<strValue;
		oss<<"</"<<strKey<<">";
	}
	
	oss<<"</xml>";
	std::string strRet = oss.str();

	LOG4CPLUS_DEBUG(logger, "Order2Xml: xml= \n"<<strRet);
	
	return strRet;
}


void UnifiedOrderReq::init(int type)
{
	appid = base_appid;
	mch_id = base_mch_id;
	device_info = ufOrder_device_info;
	nonce_str = genRandomString(32);	
	
	if(type == 1)
	{
		body = ufOrder_answer_body;
		total_fee = answer_price;
	}
	else if(type == 2)
	{
		body = ufOrder_listen_body;
		total_fee = listen_price;
	}

	detail = ufOrder_detail;
	attach = ufOrder_attach;
	out_trade_no = genOutTradeNoString();

	fee_type = ufOrder_fee_type;
	//spbill_create_ip = base_local_ip;

	time_start = genLocalTimeString(0);
	time_expire = genLocalTimeString(base_order_expire);

	goods_tag = "";

	notify_url = ufOrder_notify_url;
	trade_type = ufOrder_trade_type;

	product_id = "";
	limit_pay = ufOrder_limit_pay;	


	std::map<std::string, std::string> map_param;					
	map_param.insert(pair<std::string, std::string>("appid", appid));
	map_param.insert(pair<std::string, std::string>("mch_id", mch_id));
	map_param.insert(pair<std::string, std::string>("device_info", device_info));			
	map_param.insert(pair<std::string, std::string>("nonce_str", nonce_str));			
	map_param.insert(pair<std::string, std::string>("body", body));
	map_param.insert(pair<std::string, std::string>("detail", detail));
	map_param.insert(pair<std::string, std::string>("attach", attach));
	map_param.insert(pair<std::string, std::string>("out_trade_no", out_trade_no));
	
	map_param.insert(pair<std::string, std::string>("fee_type", fee_type));
	map_param.insert(pair<std::string, std::string>("total_fee", int_2_str(total_fee)));
	map_param.insert(pair<std::string, std::string>("spbill_create_ip", spbill_create_ip));

	map_param.insert(pair<std::string, std::string>("time_start", time_start));
	map_param.insert(pair<std::string, std::string>("time_expire", time_expire));
	
	//map_param.insert(pair<std::string, std::string>("goods_tag", goods_tag));
	map_param.insert(pair<std::string, std::string>("notify_url", notify_url));
	map_param.insert(pair<std::string, std::string>("trade_type", trade_type));

	if(!strcmp(trade_type.c_str(), "NATIVE"))
	{
		map_param.insert(pair<std::string, std::string>("product_id", product_id));
	}
	else if(!strcmp(trade_type.c_str(), "JSAPI"))
	{
		map_param.insert(pair<std::string, std::string>("openid", openid));
	}

	if(!limit_pay.empty())
	{
		map_param.insert(pair<std::string, std::string>("limit_pay", limit_pay));
	}	

	sign = genSignString(map_param, base_api_key);	
}

std::string UnifiedOrderReq::UnifiedOrderReqXMLbuild()
{
	std::map<std::string, std::string> map_param;					
	map_param.insert(pair<std::string, std::string>("appid", appid));
	map_param.insert(pair<std::string, std::string>("mch_id", mch_id));
	map_param.insert(pair<std::string, std::string>("device_info", device_info));			
	map_param.insert(pair<std::string, std::string>("nonce_str", nonce_str));			
	map_param.insert(pair<std::string, std::string>("sign", sign));			
	map_param.insert(pair<std::string, std::string>("body", body));
	map_param.insert(pair<std::string, std::string>("detail", detail));
	map_param.insert(pair<std::string, std::string>("attach", attach));
	map_param.insert(pair<std::string, std::string>("out_trade_no", out_trade_no));
	
	map_param.insert(pair<std::string, std::string>("fee_type", fee_type));
	map_param.insert(pair<std::string, std::string>("total_fee", int_2_str(total_fee)));
	map_param.insert(pair<std::string, std::string>("spbill_create_ip", spbill_create_ip));

	map_param.insert(pair<std::string, std::string>("time_start", time_start));
	map_param.insert(pair<std::string, std::string>("time_expire", time_expire));
	
	//map_param.insert(pair<std::string, std::string>("goods_tag", goods_tag));
	map_param.insert(pair<std::string, std::string>("notify_url", notify_url));
	map_param.insert(pair<std::string, std::string>("trade_type", trade_type));

	if(!strcmp(trade_type.c_str(), "NATIVE"))
	{
		map_param.insert(pair<std::string, std::string>("product_id", product_id));
	}

	if(!limit_pay.empty())
	{
		map_param.insert(pair<std::string, std::string>("limit_pay", limit_pay));
	}

	if(!strcmp(trade_type.c_str(), "JSAPI"))
	{
		map_param.insert(pair<std::string, std::string>("openid", openid));
	}
						
	return Order2Xml(map_param);
}

void UnifiedOrderReq::UnifiedOrderReqInsertDB(lce::cgi::CMysql& m_mysql_helper, const std::string& prepay_id, int business_type, uint64_t business_id)
{
	std::ostringstream sql;
	sql.str("");
	sql<<"INSERT INTO t_unifiedorder "
	   <<"SET openid='"<<sql_escape(openid)<<"'"
	   <<", out_trade_no='"<<sql_escape(out_trade_no)<<"'"
	   <<", prepay_id='"<<sql_escape(prepay_id)<<"'"
	   <<", body='"<<sql_escape(body)<<"'"
	   <<", detail='"<<sql_escape(detail)<<"'"
	   <<", total_fee="<<total_fee
	   <<", nonce_str='"<<sql_escape(nonce_str)<<"'"
	   <<", sign='"<<sql_escape(sign)<<"'"	   
	   <<", attach='"<<sql_escape(attach)<<"'"
	   <<", fee_type='"<<sql_escape(fee_type)<<"'"	   
	   <<", spbill_create_ip='"<<sql_escape(spbill_create_ip)<<"'"
	   <<", time_start='"<<sql_escape(time_start)<<"'"
	   <<", time_expire='"<<sql_escape(time_expire)<<"'"
	   <<", goods_tag='"<<sql_escape(goods_tag)<<"'"
	   <<", notify_url='"<<sql_escape(notify_url)<<"'"
	   <<", trade_type='"<<sql_escape(trade_type)<<"'"
	   <<", product_id='"<<sql_escape(product_id)<<"'"
	   <<", limit_pay='"<<sql_escape(limit_pay)<<"'"
	   <<", appid='"<<sql_escape(appid)<<"'"
	   <<", mch_id='"<<sql_escape(mch_id)<<"'"
	   <<", device_info='"<<sql_escape(device_info)<<"'"
	   <<", business_type="<<business_type
	   <<", business_id="<<business_id
	   <<", create_ts=unix_timestamp()";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
    {
        LOG4CPLUS_ERROR(logger, "UnifiedOrderReq::UnifiedOrderReqInsertDB failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());       
        return;
    }
	
	return;
}

int UnifiedOrderRsp::UnifiedOrderRspXMLparse(const std::string& strRspBody, std::map<std::string, std::string>& map_param, TiXmlDocument* pDoc)
{
	assert(pDoc);

	pDoc->Clear();
	pDoc->Parse(strRspBody.c_str()); 

	//root Element
	TiXmlElement* root = pDoc->RootElement();
	TiXmlElement* item = root->FirstChildElement();
	while(item)
	{
		if(!strcmp(item->Value(), "return_code"))
		{
			return_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "return_msg"))
		{
			return_msg = item->GetText();
		}
		else if(!strcmp(item->Value(), "appid"))
		{
			appid = item->GetText();
		}
		else if(!strcmp(item->Value(), "mch_id"))
		{
			mch_id= item->GetText();
		}
		else if(!strcmp(item->Value(), "device_info"))
		{
			device_info = item->GetText();
		}
		else if(!strcmp(item->Value(), "nonce_str"))
		{
			nonce_str = item->GetText();
		}
		else if(!strcmp(item->Value(), "sign"))
		{
			sign = item->GetText();
		}
		else if(!strcmp(item->Value(), "result_code"))
		{
			result_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "err_code"))
		{
			err_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "err_code_des"))
		{
			err_code_des = item->GetText();
		}
		else if(!strcmp(item->Value(), "trade_type"))
		{
			trade_type = item->GetText();
		}
		else if(!strcmp(item->Value(), "prepay_id"))
		{
			prepay_id = item->GetText();
		}
		else if(!strcmp(item->Value(), "code_url"))
		{
			code_url = item->GetText();
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "UnifiedOrderCallbackRspXMLparse don't record ChildElement key: "<<item->Value()<<", value: "<<item->GetText());
			return -1;
		}

		if(strcmp(item->Value(), "sign"))
		{
			map_param.insert(pair<std::string, std::string>(item->Value(), item->GetText()));
		}
			
		item = item->NextSiblingElement();
	}		

	return 0;
}

int UnifiedOrderRsp::UnifiedOrderRspCheck(const UnifiedOrderReq & stUnifiedOrderReq, std::map<std::string, std::string>& map_param)
{
	if(strcmp(return_code.c_str(), "SUCCESS"))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrder failed, return_code: " << return_code << ", return_msg: " << return_msg);
		return -1;
	}

	if(strcmp(appid.c_str(), stUnifiedOrderReq.appid.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrder failed, req_appid: " << stUnifiedOrderReq.appid << ", rsp_appid: " << appid);
		return -1;
	}

	if(strcmp(mch_id.c_str(), stUnifiedOrderReq.mch_id.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrder failed, req_mchid: " << stUnifiedOrderReq.mch_id << ", rsp_mchid: " << mch_id);
		return -1;
	}

	if(strcmp(device_info.c_str(), stUnifiedOrderReq.device_info.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrder failed, req_deviceinfo: " << stUnifiedOrderReq.device_info << ", rsp_deviceinfo: " << device_info);
		return -1;
	}

	if(strcmp(result_code.c_str(), "SUCCESS"))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrder failed, result_code: "<< result_code << ", err_code: " << err_code << ", err_code_des: " << err_code_des);
		return -1;
	}

	if(strcmp(trade_type.c_str(), stUnifiedOrderReq.trade_type.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrder failed, req_tradetype: " << stUnifiedOrderReq.trade_type << ", rsp_tradetype: " << trade_type);
		return -1;
	}	

	std::string tmp_sign = genSignString(map_param, base_api_key);
	if(strcmp(sign.c_str(), tmp_sign.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrder failed, SIGN ERROR, rsp_sign: " << sign << ", build_sign: " << tmp_sign);
		return -1;
	}

	return 0;	
}

int UnifiedOrderCallbackRsp::UnifiedOrderCallbackRspXMLparse(const std::string& strRspBody, std::map<std::string, std::string>& map_param, std::string& strErrMsg, TiXmlDocument* pDoc)
{
	assert(pDoc);
	pDoc->Clear();
	pDoc->Parse(strRspBody.c_str()); 

	//root Element
	TiXmlElement* root = pDoc->RootElement();
	TiXmlElement* item = root->FirstChildElement();
	while(item)
	{
		if(!strcmp(item->Value(), "return_code"))
		{
			return_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "return_msg"))
		{
			return_msg = item->GetText();
		}
		else if(!strcmp(item->Value(), "appid"))
		{
			appid = item->GetText();
		}
		else if(!strcmp(item->Value(), "mch_id"))
		{
			mch_id= item->GetText();
		}
		else if(!strcmp(item->Value(), "sub_mch_id"))
		{
			sub_mch_id= item->GetText();
		}
		else if(!strcmp(item->Value(), "device_info"))
		{
			device_info = item->GetText();
		}
		else if(!strcmp(item->Value(), "nonce_str"))
		{
			nonce_str = item->GetText();
		}
		else if(!strcmp(item->Value(), "sign"))
		{
			sign = item->GetText();
		}
		else if(!strcmp(item->Value(), "result_code"))
		{
			result_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "err_code"))
		{
			err_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "err_code_des"))
		{
			err_code_des = item->GetText();
		}
		else if(!strcmp(item->Value(), "openid"))
		{
			openid = item->GetText();
		}
		else if(!strcmp(item->Value(), "is_subscribe"))
		{
			is_subscribe = item->GetText();
		}
		else if(!strcmp(item->Value(), "trade_type"))
		{
			trade_type = item->GetText();
		}
		else if(!strcmp(item->Value(), "bank_type"))
		{
			bank_type = item->GetText();
		}		
		else if(!strcmp(item->Value(), "total_fee"))
		{
			total_fee = common::util::charToUint64_t(item->GetText());
		}
		else if(!strcmp(item->Value(), "settlement_total_fee"))
		{
			settlement_total_fee = common::util::charToUint64_t(item->GetText());
		}
		else if(!strcmp(item->Value(), "fee_type"))
		{
			fee_type = item->GetText();
		}
		else if(!strcmp(item->Value(), "cash_fee"))
		{
			cash_fee = common::util::charToUint64_t(item->GetText());
		}
		else if(!strcmp(item->Value(), "cash_fee_type"))
		{
			cash_fee_type = item->GetText();
		}
		else if(!strcmp(item->Value(), "coupon_fee"))
		{
			coupon_fee = common::util::charToUint64_t(item->GetText());
		}
		else if(!strcmp(item->Value(), "coupon_count"))
		{
			coupon_count = common::util::charToUint64_t(item->GetText());
		}	
		else if(!strcmp(item->Value(), "transaction_id"))
		{
			transaction_id = item->GetText();
		}			
		else if(!strcmp(item->Value(), "out_trade_no"))
		{
			out_trade_no = item->GetText();
		}	
		else if(!strcmp(item->Value(), "attach"))
		{
			attach = item->GetText();
		}	
		else if(!strcmp(item->Value(), "time_end"))
		{
			time_end = item->GetText();
		}	
		else
		{
			LOG4CPLUS_ERROR(logger, "UnifiedOrderCallbackRspXMLparse don't record ChildElement key: "<<item->Value()<<", value: "<<item->GetText());			
		}

		if(strcmp(item->Value(), "sign"))
		{
			map_param.insert(pair<std::string, std::string>(item->Value(), item->GetText()));
		}
			
		item = item->NextSiblingElement();
	}
	return 0;
}

int UnifiedOrderCallbackRsp::UnifiedOrderCallbackRspCheck(std::map<std::string, std::string>& map_param, std::string& strErrMsg)
{
	if(strcmp(return_code.c_str(), "SUCCESS"))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrderCallback ERR, out_trade_no: "<<out_trade_no<<", return_code: " << return_code << ", return_msg: " << return_msg);
		return -1;
	}	

	if(strcmp(result_code.c_str(), "SUCCESS"))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrderCallback ERR, out_trade_no: "<<out_trade_no<<", result_code: "<< result_code << ", err_code: " << err_code << ", err_code_des: " << err_code_des);
		return -1;
	}

	std::string tmp_sign = genSignString(map_param, base_api_key);
	if(strcmp(sign.c_str(), tmp_sign.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrderCallback failed, SIGN ERROR, rsp_sign: " << sign << ", build_sign: " << tmp_sign);
		strErrMsg = "SIGN ERROR!";
		return -1;
	}

	LOG4CPLUS_DEBUG(logger, "UnifiedOrderCallback SUCCESS, out_trade_no: "<<out_trade_no<<", time_end: "<<time_end);

	return 0;	
}

void UnifiedOrderCallbackRsp::UnifiedOrderCallbackRspUpdateDB(lce::cgi::CMysql& m_mysql_helper, int trade_state)
{
	std::ostringstream sql;
	sql.str("");
	sql<<"UPDATE t_unifiedorder SET pay_success="<<trade_state
	   <<", wx_result_code='"<<sql_escape(result_code)<<"'"
	   <<", wx_err_code='"<<sql_escape(err_code)<<"'"
	   <<", wx_err_code_des='"<<sql_escape(err_code_des)<<"'"
	   <<", create_ts=unix_timestamp()"
	   <<" WHERE openid='"<<sql_escape(openid)<<"'"
	   <<" AND out_trade_no='"<<sql_escape(out_trade_no)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
    {
        LOG4CPLUS_ERROR(logger, "UnifiedOrderCallbackRsp::UnifiedOrderCallbackRspUpdateDB, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());       
        return;
    }
	
	return;
}

void UnifiedOrderQueryReq::init()
{
	appid = base_appid;
	mch_id = base_mch_id;
	nonce_str = genRandomString(32);

	std::map<std::string, std::string> map_param;					
	map_param.insert(pair<std::string, std::string>("appid", appid));
	map_param.insert(pair<std::string, std::string>("mch_id", mch_id));
	map_param.insert(pair<std::string, std::string>("out_trade_no", out_trade_no));
	map_param.insert(pair<std::string, std::string>("nonce_str", nonce_str));

	sign = genSignString(map_param, base_api_key);	
}

std::string UnifiedOrderQueryReq::UnifiedOrderQueryReqXMLbulid()
{
	std::map<std::string, std::string> map_param;					
	map_param.insert(pair<std::string, std::string>("appid", appid));
	map_param.insert(pair<std::string, std::string>("mch_id", mch_id));
	map_param.insert(pair<std::string, std::string>("out_trade_no", out_trade_no));
	map_param.insert(pair<std::string, std::string>("nonce_str", nonce_str));			
	map_param.insert(pair<std::string, std::string>("sign", sign));				

	if(!transaction_id.empty())
	{
		map_param.insert(pair<std::string, std::string>("transaction_id", transaction_id));
	}
						
	return Order2Xml(map_param);
}

int UnifiedOrderQueryRsp::UnifiedOrderQueryRspXMLparse(const std::string& strRspBody, std::map<std::string, std::string>& map_param, TiXmlDocument* pDoc)
{
	assert(pDoc);
	pDoc->Clear();
	pDoc->Parse(strRspBody.c_str()); 

	//root Element
	TiXmlElement* root = pDoc->RootElement();
	TiXmlElement* item = root->FirstChildElement();
	while(item)
	{
		if(!strcmp(item->Value(), "return_code"))
		{
			return_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "return_msg"))
		{
			return_msg = item->GetText();
		}
		else if(!strcmp(item->Value(), "appid"))
		{
			appid = item->GetText();
		}
		else if(!strcmp(item->Value(), "mch_id"))
		{
			mch_id= item->GetText();
		}		
		else if(!strcmp(item->Value(), "nonce_str"))
		{
			nonce_str = item->GetText();
		}
		else if(!strcmp(item->Value(), "sign"))
		{
			sign = item->GetText();
		}
		else if(!strcmp(item->Value(), "result_code"))
		{
			result_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "err_code"))
		{
			err_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "err_code_des"))
		{
			err_code_des = item->GetText();
		}
		else if(!strcmp(item->Value(), "device_info"))
		{
			device_info = item->GetText();
		}
		else if(!strcmp(item->Value(), "openid"))
		{
			openid = item->GetText();
		}
		else if(!strcmp(item->Value(), "is_subscribe"))
		{
			is_subscribe = item->GetText();
		}
		else if(!strcmp(item->Value(), "trade_type"))
		{
			trade_type = item->GetText();
		}
		else if(!strcmp(item->Value(), "trade_state"))
		{
			trade_state = item->GetText();
		}	
		else if(!strcmp(item->Value(), "bank_type"))
		{
			bank_type = item->GetText();
		}
		else if(!strcmp(item->Value(), "total_fee"))
		{
			total_fee = common::util::charToUint64_t(item->GetText());
		}
		else if(!strcmp(item->Value(), "settlement_total_fee"))
		{
			settlement_total_fee = common::util::charToUint64_t(item->GetText());
		}
		else if(!strcmp(item->Value(), "fee_type"))
		{
			fee_type = item->GetText();
		}
		else if(!strcmp(item->Value(), "cash_fee"))
		{
			cash_fee = common::util::charToUint64_t(item->GetText());
		}
		else if(!strcmp(item->Value(), "cash_fee_type"))
		{
			cash_fee_type = item->GetText();
		}
		else if(!strcmp(item->Value(), "coupon_fee"))
		{
			coupon_fee = common::util::charToUint64_t(item->GetText());
		}
		else if(!strcmp(item->Value(), "coupon_count"))
		{
			coupon_count = common::util::charToUint64_t(item->GetText());
		}	
		else if(!strcmp(item->Value(), "transaction_id"))
		{
			transaction_id = item->GetText();
		}			
		else if(!strcmp(item->Value(), "out_trade_no"))
		{
			out_trade_no = item->GetText();
		}	
		else if(!strcmp(item->Value(), "attach"))
		{
			attach = item->GetText();
		}	
		else if(!strcmp(item->Value(), "time_end"))
		{
			time_end = item->GetText();
		}	
		else if(!strcmp(item->Value(), "trade_state_desc"))
		{
			trade_state_desc = item->GetText();
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "UnifiedOrderCallbackRspXMLparse don't record ChildElement key: "<<item->Value()<<", value: "<<item->GetText());				
		}

		if(strcmp(item->Value(), "sign"))
		{
			map_param.insert(pair<std::string, std::string>(item->Value(), item->GetText()));
		}
			
		item = item->NextSiblingElement();
	}
	return 0;
}

int UnifiedOrderQueryRsp::UnifiedOrderQueryRspCheck(const std::string& reqOpenid, const UnifiedOrderQueryReq& stUnifiedOrderQueryReq, std::map<std::string, std::string>& map_param)
{
	if(strcmp(return_code.c_str(), "SUCCESS"))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrderQueryRspCheck ERROR, return_code: " << return_code << ", return_msg: " << return_msg);
		return -1;
	}

	if(strcmp(appid.c_str(), stUnifiedOrderQueryReq.appid.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrderQueryRspCheck ERROR, req_appid: " << stUnifiedOrderQueryReq.appid << ", rsp_appid: " << appid);
		return -1;
	}

	if(strcmp(mch_id.c_str(), stUnifiedOrderQueryReq.mch_id.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrderQueryRspCheck ERROR, req_mchid: " << stUnifiedOrderQueryReq.mch_id << ", rsp_mchid: " << mch_id);
		return -1;
	}		

	std::string tmp_sign = genSignString(map_param, base_api_key);
	if(strcmp(sign.c_str(), tmp_sign.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrderQueryRspCheck SIGN ERROR, rsp_sign: " << sign << ", build_sign: " << tmp_sign);
		return -1;
	}

	if(strcmp(result_code.c_str(), "SUCCESS"))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrderQueryRspCheck ERROR, result_code: "<< result_code << ", err_code: " << err_code << ", err_code_des: " << err_code_des);
		return -1;
	}

	if(strcmp(out_trade_no.c_str(), stUnifiedOrderQueryReq.out_trade_no.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrderQueryRspCheck ERROR, req_sign: " << stUnifiedOrderQueryReq.out_trade_no << ", rsp_sign: " << sign);
		return -1;
	}	

	if(strcmp(openid.c_str(), reqOpenid.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "UnifiedOrderQueryRspCheck ERROR, req_openid: " << reqOpenid << ", rsp_openid: " << openid);
		return -1;
	}

	LOG4CPLUS_DEBUG(logger, "UnifiedOrderQueryRspCheck trade_state: " << trade_state);
	if(!strcmp(trade_state.c_str(), "SUCCESS"))
	{		
		return TRADE_STATE_SUCCESS;
	}
	else 
	{		
		return TRADE_STATE_FAIL;
	}	
	
	return 0;	
}

void TransferOrderReq::init()
{
	appid = base_appid;
	mch_id = base_mch_id;
	desc = tfOrder_desc;
	nonce_str = genRandomString(32);	
	partner_trade_no = genOutTradeNoString();
	check_name = tfOrder_check_name;	

	std::map<std::string, std::string> map_param;
	map_param.insert(pair<std::string, std::string>("openid", openid));
	map_param.insert(pair<std::string, std::string>("appid", appid));
	map_param.insert(pair<std::string, std::string>("mch_id", mch_id));
	map_param.insert(pair<std::string, std::string>("device_info", device_info));			
	map_param.insert(pair<std::string, std::string>("nonce_str", nonce_str));			
	map_param.insert(pair<std::string, std::string>("sign", sign));
	map_param.insert(pair<std::string, std::string>("check_name", check_name));
	map_param.insert(pair<std::string, std::string>("amount", int_2_str(amount)));
	map_param.insert(pair<std::string, std::string>("desc", desc));
	map_param.insert(pair<std::string, std::string>("partner_trade_no", partner_trade_no));	
	map_param.insert(pair<std::string, std::string>("spbill_create_ip", spbill_create_ip));

	sign = genSignString(map_param, base_api_key);
}

bool TransferOrderReq::TransferOrderCheckAmount(lce::cgi::CMysql& m_mysql_helper, const std::string& m_openid, uint64_t m_amount)
{
	std::ostringstream sql;
	sql.str("");
	sql<<"SELECT * FROM t_user_cash WHERE openid='"<<sql_escape(m_openid)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "TransferOrderCheckAmount failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());		
        return false;		
	}
	
	int64_t user_balance = 0;
    if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{			
			uint64_t total_income = strtoul(m_mysql_helper.GetRow(1), NULL, 10);
    		uint64_t total_outcome = strtoul(m_mysql_helper.GetRow(2), NULL, 10);

    		user_balance = total_income - total_outcome;
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "TransferOrderCheckAmount end, openid: "<<m_openid<<" not exists");
        return false;
	}

	if((user_balance < 0) || ((uint64_t)user_balance < m_amount))
	{
		LOG4CPLUS_ERROR(logger, "TransferOrderCheckAmount fail, openid: "<<m_openid<<", balance: "<<user_balance<<" < req.amount: "<<amount);		
		return false;
	}	

	return true;
}


std::string TransferOrderReq::TransferOrderReqXMLbuild()
{
	std::map<std::string, std::string> map_param;
	map_param.insert(pair<std::string, std::string>("openid", openid));
	map_param.insert(pair<std::string, std::string>("appid", appid));
	map_param.insert(pair<std::string, std::string>("mch_id", mch_id));
	map_param.insert(pair<std::string, std::string>("device_info", device_info));			
	map_param.insert(pair<std::string, std::string>("nonce_str", nonce_str));			
	map_param.insert(pair<std::string, std::string>("sign", sign));
	map_param.insert(pair<std::string, std::string>("check_name", check_name));
	map_param.insert(pair<std::string, std::string>("amount", int_2_str(amount)));
	map_param.insert(pair<std::string, std::string>("desc", desc));
	map_param.insert(pair<std::string, std::string>("partner_trade_no", partner_trade_no));	
	map_param.insert(pair<std::string, std::string>("spbill_create_ip", spbill_create_ip));

	if(strcmp(check_name.c_str(), "NO_CHECK"))	
	{
		map_param.insert(pair<std::string, std::string>("re_user_name", re_user_name));
	}
						
	return Order2Xml(map_param);
}

void TransferOrderReq::TransferOrderReqInsertDB(lce::cgi::CMysql& m_mysql_helper)
{
	std::ostringstream sql;
	sql.str("");
	sql<<"INSERT INTO t_transferorder_req "
	   <<"SET openid='"<<sql_escape(openid)<<"'"
	   <<", partner_trade_no='"<<sql_escape(partner_trade_no)<<"'"
	   <<", appid='"<<sql_escape(appid)<<"'"
	   <<", mch_id='"<<sql_escape(mch_id)<<"'"
	   <<", device_info='"<<sql_escape(device_info)<<"'"
	   <<", nonce_str='"<<sql_escape(nonce_str)<<"'"
	   <<", sign='"<<sql_escape(sign)<<"'"
	   <<", check_name='"<<sql_escape(check_name)<<"'"
	   <<", re_user_name='"<<sql_escape(re_user_name)<<"'"
	   <<", amount="<<amount
	   <<", des='"<<sql_escape(desc)<<"'"
	   <<", spbill_create_ip='"<<sql_escape(spbill_create_ip)<<"'";	   

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
    {
        LOG4CPLUS_ERROR(logger, "TransferOrderReq::TransferOrderReqInsertDB failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());       
        return;
    }
	
	return;
}

int TransferOrderRsp::TransferOrderRspXMLparse(const std::string& strRspBody, std::map<std::string, std::string>& map_param, TiXmlDocument* pDoc)
{
	assert(pDoc);
	pDoc->Clear();
	pDoc->Parse(strRspBody.c_str()); 

	//root Element
	TiXmlElement* root = pDoc->RootElement();
	TiXmlElement* item = root->FirstChildElement();
	while(item)
	{
		if(!strcmp(item->Value(), "return_code"))
		{
			return_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "return_msg"))
		{
			return_msg = item->GetText();
		}
		else if(!strcmp(item->Value(), "appid"))
		{
			appid = item->GetText();
		}
		else if(!strcmp(item->Value(), "mch_id"))
		{
			mch_id= item->GetText();
		}
		else if(!strcmp(item->Value(), "device_info"))
		{
			device_info = item->GetText();
		}
		else if(!strcmp(item->Value(), "nonce_str"))
		{
			nonce_str = item->GetText();
		}		
		else if(!strcmp(item->Value(), "result_code"))
		{
			result_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "err_code"))
		{
			err_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "err_code_des"))
		{
			err_code_des = item->GetText();
		}
		else if(!strcmp(item->Value(), "partner_trade_no"))
		{
			partner_trade_no = item->GetText();
		}
		else if(!strcmp(item->Value(), "payment_no"))
		{
			payment_no = item->GetText();
		}
		else if(!strcmp(item->Value(), "payment_time"))
		{
			payment_time = item->GetText();
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "UnifiedOrderCallbackRspXMLparse don't record ChildElement key: "<<item->Value()<<", value: "<<item->GetText());
			return -1;
		}

		if(strcmp(item->Value(), "sign"))
		{
			map_param.insert(pair<std::string, std::string>(item->Value(), item->GetText()));
		}
			
		item = item->NextSiblingElement();
	}		

	return 0;
}

int TransferOrderRsp::TransferOrderRspCheck(const TransferOrderReq& stTransferOrderReq, std::map<std::string, std::string>& map_param)
{
	if(strcmp(return_code.c_str(), "SUCCESS"))
	{
		LOG4CPLUS_ERROR(logger, "TransferOrder failed, return_code: " << return_code << ", return_msg: " << return_msg);
		return -1;
	}

	if(strcmp(appid.c_str(), stTransferOrderReq.appid.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "TransferOrder failed, req_appid: " << stTransferOrderReq.appid << ", rsp_appid: " << appid);
		return -1;
	}

	if(strcmp(mch_id.c_str(), stTransferOrderReq.mch_id.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "TransferOrder failed, req_mchid: " << stTransferOrderReq.mch_id << ", rsp_mchid: " << mch_id);
		return -1;
	}

	if(strcmp(device_info.c_str(), stTransferOrderReq.device_info.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "TransferOrder failed, req_deviceinfo: " << stTransferOrderReq.device_info << ", rsp_deviceinfo: " << device_info);
		return -1;
	}

	if(strcmp(result_code.c_str(), "SUCCESS"))
	{
		LOG4CPLUS_ERROR(logger, "TransferOrder failed, result_code: "<< result_code << ", err_code: " << err_code << ", err_code_des: " << err_code_des);
		return -1;
	}	

	if(strcmp(partner_trade_no.c_str(), stTransferOrderReq.partner_trade_no.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "TransferOrder failed, req_partner_trade_no: " << stTransferOrderReq.partner_trade_no << ", rsp_partner_trade_no: " << partner_trade_no);
		return -1;
	}

	return 0;	
}

void TransferOrderRsp::TransferOrderRspInsertDB(lce::cgi::CMysql& m_mysql_helper, const std::string& openid)
{
	std::ostringstream sql;
	sql.str("");
	sql<<"INSERT INTO t_transferorder_rsp "
	   <<"SET openid='"<<sql_escape(openid)<<"'"
	   <<", partner_trade_no='"<<sql_escape(partner_trade_no)<<"'"
	   <<", payment_no='"<<sql_escape(payment_no)<<"'"
	   <<", return_code='"<<sql_escape(return_code)<<"'"
	   <<", return_msg='"<<sql_escape(return_msg)<<"'"
	   <<", appid='"<<sql_escape(appid)<<"'"
	   <<", mch_id='"<<sql_escape(mch_id)<<"'"
	   <<", device_info='"<<sql_escape(device_info)<<"'"
	   <<", nonce_str='"<<sql_escape(nonce_str)<<"'"
	   <<", result_code='"<<sql_escape(result_code)<<"'"
	   <<", err_code='"<<sql_escape(err_code)<<"'"
	   <<", err_code_des='"<<sql_escape(err_code_des)<<"'"
	   <<", payment_time='"<<sql_escape(payment_time)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
    {
        LOG4CPLUS_ERROR(logger, "TransferOrderRsp::TransferOrderRspInsertDB failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());       
        return;
    }
	
	return;
}

void SendRedPackReq::init()
{
	wxappid = base_appid;
	mch_id = base_mch_id;
	act_name = rpOrder_act_name;
	nonce_str = genRandomString(32);
	
	mch_billno = genRedPackBillNoString(mch_id);
	send_name = rpOrder_send_name;
	wishing = rpOrder_wishing;	
	remark = rpOrder_remark;
	total_num = 1;

	std::map<std::string, std::string> map_param;
	map_param.insert(pair<std::string, std::string>("nonce_str", nonce_str));	
	map_param.insert(pair<std::string, std::string>("mch_billno", mch_billno));
	map_param.insert(pair<std::string, std::string>("mch_id", mch_id));			
	map_param.insert(pair<std::string, std::string>("wxappid", wxappid));			
	map_param.insert(pair<std::string, std::string>("send_name", send_name));
	map_param.insert(pair<std::string, std::string>("re_openid", re_openid));
	map_param.insert(pair<std::string, std::string>("total_amount", int_2_str(total_amount)));
	map_param.insert(pair<std::string, std::string>("total_num", int_2_str(total_num)));
	map_param.insert(pair<std::string, std::string>("wishing", wishing));
	map_param.insert(pair<std::string, std::string>("client_ip", client_ip));	
	map_param.insert(pair<std::string, std::string>("act_name", act_name));
	map_param.insert(pair<std::string, std::string>("remark", remark));	
	
	sign = genSignString(map_param, base_api_key);
}

bool SendRedPackReq::SendRedPackReqCheckAmount(lce::cgi::CMysql& m_mysql_helper, const std::string& m_openid, uint64_t m_amount)
{
	std::ostringstream sql;
	sql.str("");
	sql<<"SELECT * FROM t_user_cash WHERE openid='"<<sql_escape(m_openid)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackReqCheckAmount failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());		
        return false;		
	}
	
	int64_t user_balance = 0;
    if(m_mysql_helper.GetRowCount())
	{
		while(m_mysql_helper.Next())
		{			
			uint64_t total_income = strtoul(m_mysql_helper.GetRow(1), NULL, 10);
    		uint64_t total_outcome = strtoul(m_mysql_helper.GetRow(2), NULL, 10);

    		user_balance = total_income - total_outcome;
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackReqCheckAmount end, openid: "<<m_openid<<" not exists");
        return false;
	}

	if((user_balance < 0) || ((uint64_t)user_balance < m_amount))
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackReqCheckAmount fail, openid: "<<m_openid<<", balance: "<<user_balance<<" < req.amount: "<<m_amount);		
		return false;
	}	

	return true;
}

std::string SendRedPackReq::SendRedPackReqXMLbuild()
{
	std::map<std::string, std::string> map_param;
	map_param.insert(pair<std::string, std::string>("nonce_str", nonce_str));
	map_param.insert(pair<std::string, std::string>("sign", sign));
	map_param.insert(pair<std::string, std::string>("mch_billno", mch_billno));
	map_param.insert(pair<std::string, std::string>("mch_id", mch_id));			
	map_param.insert(pair<std::string, std::string>("wxappid", wxappid));			
	map_param.insert(pair<std::string, std::string>("send_name", send_name));
	map_param.insert(pair<std::string, std::string>("re_openid", re_openid));
	map_param.insert(pair<std::string, std::string>("total_amount", int_2_str(total_amount)));
	map_param.insert(pair<std::string, std::string>("total_num", int_2_str(total_num)));
	map_param.insert(pair<std::string, std::string>("wishing", wishing));
	map_param.insert(pair<std::string, std::string>("client_ip", client_ip));	
	map_param.insert(pair<std::string, std::string>("act_name", act_name));
	map_param.insert(pair<std::string, std::string>("remark", remark));	
						
	return Order2Xml(map_param);
}

void SendRedPackReq::SendRedPackReqInsertDB(lce::cgi::CMysql& m_mysql_helper)
{
	std::ostringstream sql;
	sql.str("");
	sql<<"INSERT INTO t_sendredpack_req "
	   <<"SET re_openid='"<<sql_escape(re_openid)<<"'"
	   <<", mch_billno='"<<sql_escape(mch_billno)<<"'"
	   <<", wxappid='"<<sql_escape(wxappid)<<"'"
	   <<", mch_id='"<<sql_escape(mch_id)<<"'"	   
	   <<", nonce_str='"<<sql_escape(nonce_str)<<"'"
	   <<", sign='"<<sql_escape(sign)<<"'"
	   <<", send_name='"<<sql_escape(send_name)<<"'"
	   <<", wishing='"<<sql_escape(wishing)<<"'"
	   <<", total_amount="<<total_amount
	   <<", total_num="<<total_num
	   <<", act_name='"<<sql_escape(act_name)<<"'"
	   <<", remark='"<<sql_escape(remark)<<"'"
	   <<", client_ip='"<<sql_escape(client_ip)<<"'";	   

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
    {
        LOG4CPLUS_ERROR(logger, "SendRedPackReq::SendRedPackReqInsertDB failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());       
        return;
    }
	
	return;
}

int SendRedPackRsp::SendRedPackRspXMLparse(const std::string& strRspBody, std::map<std::string, std::string>& map_param, TiXmlDocument* pDoc)
{
	assert(pDoc);
	pDoc->Clear();
	pDoc->Parse(strRspBody.c_str()); 

	//root Element
	TiXmlElement* root = pDoc->RootElement();
	TiXmlElement* item = root->FirstChildElement();
	while(item)
	{
		if(!strcmp(item->Value(), "return_code"))
		{
			return_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "return_msg"))
		{
			return_msg = item->GetText();
		}
		else if(!strcmp(item->Value(), "sign"))
		{
			sign = item->GetText();
		}
		else if(!strcmp(item->Value(), "result_code"))
		{
			result_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "err_code"))
		{
			err_code = item->GetText();
		}
		else if(!strcmp(item->Value(), "err_code_des"))
		{
			err_code_des = item->GetText();
		}
		else if(!strcmp(item->Value(), "mch_billno"))
		{
			mch_billno = item->GetText();
		}
		else if(!strcmp(item->Value(), "mch_id"))
		{
			mch_id= item->GetText();
		}	
		else if(!strcmp(item->Value(), "wxappid"))
		{
			wxappid = item->GetText();
		}
		else if(!strcmp(item->Value(), "re_openid"))
		{
			re_openid = item->GetText();
		}
		else if(!strcmp(item->Value(), "total_amount"))
		{
			total_amount = common::util::charToUint64_t(item->GetText());
		}
		else if(!strcmp(item->Value(), "send_time"))
		{
			send_time = item->GetText();
		}
		else if(!strcmp(item->Value(), "send_listid"))
		{
			send_listid = item->GetText();
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "UnifiedOrderCallbackRspXMLparse don't record ChildElement key: "<<item->Value()<<", value: "<<item->GetText());
			return -1;
		}

		if(strcmp(item->Value(), "sign"))
		{
			map_param.insert(pair<std::string, std::string>(item->Value(), item->GetText()));
		}
			
		item = item->NextSiblingElement();
	}		

	return 0;
}

int SendRedPackRsp::SendRedPackRspCheck(const SendRedPackReq& stSendRedPackReq, std::map<std::string, std::string>& map_param)
{
	if(strcmp(return_code.c_str(), "SUCCESS"))
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackRspCheck ERROR, return_code: " << return_code << ", return_msg: " << return_msg);
		return -1;
	}	
	
	std::string tmp_sign = genSignString(map_param, base_api_key);
	if(strcmp(sign.c_str(), tmp_sign.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackRspCheck SIGN ERROR, rsp_sign: " << sign << ", build_sign: " << tmp_sign);
		return -1;
	}

	if(strcmp(result_code.c_str(), "SUCCESS"))
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackRspCheck ERROR, result_code: "<< result_code << ", err_code: " << err_code << ", err_code_des: " << err_code_des);
		return -1;
	}	

	if(strcmp(mch_billno.c_str(), stSendRedPackReq.mch_billno.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackRspCheck ERROR, req_mch_billno: " << stSendRedPackReq.mch_billno << ", rsp_mch_billno: " << mch_billno);
		return -1;
	}

	if(strcmp(wxappid.c_str(), stSendRedPackReq.wxappid.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackRspCheck ERROR, req_wxappid: " << stSendRedPackReq.wxappid << ", rsp_wxappid: " << wxappid);
		return -1;
	}

	if(strcmp(mch_id.c_str(), stSendRedPackReq.mch_id.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackRspCheck ERROR, req_mchid: " << stSendRedPackReq.mch_id << ", rsp_mchid: " << mch_id);
		return -1;
	}

	if(strcmp(re_openid.c_str(), stSendRedPackReq.re_openid.c_str()))
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackRspCheck ERROR, req_re_openid: " << stSendRedPackReq.re_openid << ", rsp_re_openid: " << re_openid);
		return -1;
	}

	if(total_amount != stSendRedPackReq.total_amount)
	{
		LOG4CPLUS_ERROR(logger, "SendRedPackRspCheck ERROR, req_total_amount: " << stSendRedPackReq.total_amount << ", rsp_total_amount: " << total_amount);
		return -1;
	}	
	
	return 0;	
}

void SendRedPackRsp::SendRedPackRspInsertDB(lce::cgi::CMysql& m_mysql_helper)
{
	std::ostringstream sql;
	sql.str("");
	sql<<"INSERT INTO t_sendredpack_rsp "
	   <<"SET re_openid='"<<sql_escape(re_openid)<<"'"
	   <<", mch_billno='"<<sql_escape(mch_billno)<<"'"
	   <<", send_listid='"<<sql_escape(send_listid)<<"'"
	   <<", return_code='"<<sql_escape(return_code)<<"'"
	   <<", return_msg='"<<sql_escape(return_msg)<<"'"
	   <<", sign='"<<sql_escape(sign)<<"'"	   
	   <<", result_code='"<<sql_escape(result_code)<<"'"
	   <<", err_code='"<<sql_escape(err_code)<<"'"
	   <<", err_code_des='"<<sql_escape(err_code_des)<<"'"
	   <<", mch_id='"<<sql_escape(mch_id)<<"'"
	   <<", wxappid='"<<sql_escape(wxappid)<<"'"
	   <<", total_amount="<<total_amount
	   <<", send_time='"<<sql_escape(send_time)<<"'";

	LOG4CPLUS_DEBUG(logger, sql.str());
	if(!m_mysql_helper.Query(sql.str()))
    {
        LOG4CPLUS_ERROR(logger, "SendRedPackRsp::SendRedPackRspInsertDB failed, sql="<<sql.str()<<", msg="<<m_mysql_helper.GetErrMsg());       
        return;
    }
	
	return;
}
