#ifndef _PAY_SERVER_ORDER_H_
#define _PAY_SERVER_ORDER_H_

#include <string>
#include "util/logger.h"
#include "cgi/cgi_mysql.h"
#include <tinyxml/tinyxml.h>
#include <tinyxml/tinystr.h>

class Order
{
public:
	Order();
	~Order(){};

public:
	enum TRADE_STATE
	{
		
		TRADE_STATE_SUCCESS = 1,		//支付成功
		TRADE_STATE_FAIL = 2,			//支付失败
		
	};

	//现金流水类型
	enum CASH_FLOW_TYPE
	{
		CASH_FLOW_TYPE_IN = 1,			//进
		CASH_FLOW_TYPE_OUT = 2,			//出
	};

	//现金流水详情
	enum CASH_FLOW_REASON
	{
		CASH_FLOW_REASON_ANSWER = 1,		//回答支出
		CASH_FLOW_REASON_LISTEN = 2,		//偷听支出
		CASH_FLOW_REASON_LISTEN_INCOME = 3,	//偷听分成收益
		CASH_FLOW_REASON_OUTCOME = 4,		//提现
	};

	//提现处理状态
	enum OUTCOME_STATE
	{
		OUTCOME_STATE_PENDING = 1,			//等待处理
		OUTCOME_STATE_PASS = 2,				//审核成功,处理完成
		OUTCOME_STATE_FAIL = 3,				//审核失败
	};

	static const std::string type_string(int type)
    {
        switch(type)
        {				
			case TRADE_STATE_SUCCESS:
				return "Order::TRADE_STATE_SUCCESS";
			case TRADE_STATE_FAIL:
				return "Order::TRADE_STATE_FAIL";					

            default:
                return "Order::T_UNKNOWN";
        }
    }
	
public:
	std::string base_appid;
	std::string base_appsecret;
	std::string base_mch_id;
	std::string base_api_key;
	std::string base_local_ip;	

	uint32_t listen_price;
	uint32_t answer_price;
	//订单失效时间(单位:秒)
	uint32_t base_order_expire;			

	//unifiedorder default value
	std::string ufOrder_device_info;
	std::string ufOrder_answer_body;
	std::string ufOrder_listen_body;
	std::string ufOrder_detail;
	std::string ufOrder_attach;
	std::string ufOrder_fee_type;
	std::string ufOrder_trade_type;
	std::string ufOrder_limit_pay;
	std::string ufOrder_notify_url;

	//sendredpack default value
	std::string rpOrder_act_name;
	std::string rpOrder_send_name;
	std::string rpOrder_wishing;
	std::string rpOrder_remark;	

	//transferorder default value
	std::string tfOrder_desc;
	std::string tfOrder_check_name;

	static std::string Order2Xml(const std::map<std::string, std::string>& mapParams);

	DECL_LOGGER(logger);
};

class UnifiedOrderReq: public Order
{
public:
	UnifiedOrderReq(){};
	~UnifiedOrderReq(){};

	void init(int type);
	std::string UnifiedOrderReqXMLbuild();
	void UnifiedOrderReqInsertDB(lce::cgi::CMysql& m_mysql_helper, const std::string& prepay_id, int business_type, uint64_t business_id);

public:
	std::string appid;
	std::string mch_id;
	std::string device_info;
	std::string nonce_str;
	std::string sign;
	std::string body;
	std::string detail;
	std::string attach;
	std::string out_trade_no;
	std::string fee_type;
	uint64_t total_fee;
	std::string spbill_create_ip;
	std::string time_start;
	std::string time_expire;
	std::string goods_tag;
	std::string notify_url;
	std::string trade_type;	
	std::string product_id;
	std::string limit_pay;										
	std::string openid;

};

class UnifiedOrderRsp: public Order
{
public:
	UnifiedOrderRsp(){};
	~UnifiedOrderRsp(){};

	int UnifiedOrderRspXMLparse(const std::string& strRspBody, std::map<std::string, std::string>& map_param, TiXmlDocument* pDoc);
	int UnifiedOrderRspCheck(const UnifiedOrderReq& strReqBody, std::map<std::string, std::string>& map_param);	

public:
	std::string return_code;
	std::string return_msg;
	std::string appid;
	std::string mch_id;
	std::string device_info;
	std::string nonce_str;
	std::string sign;
	std::string result_code;
	std::string err_code;
	std::string err_code_des;
	std::string trade_type;
	std::string prepay_id;
	std::string code_url;

};

class UnifiedOrderCallbackRsp: public Order
{
public:
	UnifiedOrderCallbackRsp(){};
	~UnifiedOrderCallbackRsp(){};

	int UnifiedOrderCallbackRspXMLparse(const std::string& strRspBody, std::map<std::string, std::string>& map_param, std::string& strErrMsg, TiXmlDocument* pDoc);
	int UnifiedOrderCallbackRspCheck(std::map<std::string, std::string>& map_param, std::string& strErrMsg);
	void UnifiedOrderCallbackRspUpdateDB(lce::cgi::CMysql& m_mysql_helper, int trade_state);

public:
	std::string return_code;
	std::string return_msg;
	std::string appid;
	std::string mch_id;
	std::string sub_mch_id;
	std::string device_info;
	std::string nonce_str;
	std::string sign;
	std::string result_code;
	std::string err_code;
	std::string err_code_des;
	std::string openid;
	std::string is_subscribe;
	std::string trade_type;
	std::string bank_type;
	uint64_t total_fee;
	uint64_t settlement_total_fee;
	std::string fee_type;
	uint64_t cash_fee;
	std::string cash_fee_type;
	uint64_t coupon_fee;
	uint64_t coupon_count;
	std::string transaction_id;
	std::string out_trade_no;
	std::string attach;
	std::string time_end;
	
	
};

class UnifiedOrderQueryReq: public Order
{
public:
	UnifiedOrderQueryReq(){};
	~UnifiedOrderQueryReq(){};

	void init();
    int UnifiedOrderQueryFromDB(lce::cgi::CMysql& m_mysql_helper);
	std::string UnifiedOrderQueryReqXMLbulid();	

public:
	std::string appid;
	std::string mch_id;
	std::string transaction_id;
	std::string out_trade_no;
	std::string nonce_str;
	std::string sign;
	
};

class UnifiedOrderQueryRsp: public Order
{
public:
	UnifiedOrderQueryRsp(){};
	~UnifiedOrderQueryRsp(){};

	int UnifiedOrderQueryRspXMLparse(const std::string& strRspBody, std::map<std::string, std::string>& map_param, TiXmlDocument* pDoc);
	int UnifiedOrderQueryRspCheck(const std::string& reqOpenid, const UnifiedOrderQueryReq& stUnifiedOrderQueryReq, std::map<std::string, std::string>& map_param);	

public:
	std::string return_code;
	std::string return_msg;
	std::string appid;
	std::string mch_id; 
	std::string nonce_str;
	std::string sign;
	std::string result_code;
	std::string err_code;
	std::string err_code_des;
	std::string device_info;
	std::string openid;
	std::string is_subscribe;
	std::string trade_type;
	std::string trade_state;
	std::string bank_type;
	uint64_t total_fee;
	uint64_t settlement_total_fee;
	std::string fee_type;
	uint64_t cash_fee;
	std::string cash_fee_type;
	uint64_t coupon_fee;
	uint64_t coupon_count;
	std::string transaction_id;
	std::string out_trade_no;
	std::string attach;
	std::string time_end;
	std::string trade_state_desc;
};

class TransferOrderReq: public Order
{
public:
	TransferOrderReq(){};
	~TransferOrderReq(){};

	void init();
	bool TransferOrderCheckAmount(lce::cgi::CMysql& m_mysql_helper, const std::string& openid, uint64_t m_amount);
	std::string TransferOrderReqXMLbuild();
	void TransferOrderReqInsertDB(lce::cgi::CMysql& m_mysql_helper);

public:
	std::string appid;
	std::string mch_id;
	std::string device_info;
	std::string nonce_str;
	std::string sign;
	std::string partner_trade_no;
	std::string openid;
	std::string check_name;
	std::string re_user_name;
	uint64_t amount;
	std::string desc;
	std::string spbill_create_ip;	
	
};

class TransferOrderRsp: public Order
{
public:
	TransferOrderRsp(){};
	~TransferOrderRsp(){};

	int TransferOrderRspXMLparse(const std::string& strRspBody, std::map<std::string, std::string>& map_param, TiXmlDocument* pDoc);
	int TransferOrderRspCheck(const TransferOrderReq& strReqBody, std::map<std::string, std::string>& map_param);
	void TransferOrderRspInsertDB(lce::cgi::CMysql& m_mysql_helper, const std::string& openid);

public:
	std::string return_code;
	std::string return_msg;
	std::string appid;
	std::string mch_id;
	std::string device_info;
	std::string nonce_str;
	std::string result_code;
	std::string err_code;
	std::string err_code_des;
	std::string partner_trade_no;
	std::string payment_no;
	std::string payment_time;
};

class SendRedPackReq: public Order
{
public:
	SendRedPackReq(){};
	~SendRedPackReq(){};

	void init();
	bool SendRedPackReqCheckAmount(lce::cgi::CMysql& m_mysql_helper, const std::string& openid, uint64_t m_amount);
	std::string SendRedPackReqXMLbuild();
	void SendRedPackReqInsertDB(lce::cgi::CMysql& m_mysql_helper);

public:
	std::string nonce_str;
	std::string sign;
	std::string mch_billno;
	std::string mch_id;
	std::string wxappid;
	std::string send_name;
	std::string re_openid;
	uint64_t total_amount;
	uint64_t total_num;
	std::string wishing;
	std::string client_ip;
	std::string act_name;
	std::string remark;
	
};

class SendRedPackRsp: public Order
{
public:
	SendRedPackRsp(){};
	~SendRedPackRsp(){};

	int SendRedPackRspXMLparse(const std::string& strRspBody, std::map<std::string, std::string>& map_param, TiXmlDocument* pDoc);
	int SendRedPackRspCheck(const SendRedPackReq& stSendRedPackReq, std::map<std::string, std::string>& map_param);
	void SendRedPackRspInsertDB(lce::cgi::CMysql& m_mysql_helper);
	
public:
	std::string return_code;
	std::string return_msg;
	std::string sign;
	std::string result_code;
	std::string err_code;
	std::string err_code_des;
	std::string mch_billno;
	std::string mch_id;
	std::string wxappid;
	std::string re_openid;
	uint64_t total_amount;
	std::string send_time;
	std::string send_listid;
	
};


#endif