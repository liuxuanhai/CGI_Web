#ifndef _PA_Z_CGI_ORDER_BASE_H_
#define _PA_Z_CGI_ORDER_BASE_H_

#include "cgi/cgi.h"
#include "../cgi_common_util.h"
#include "common_util.h"

using namespace lce::cgi;

class CgiManagerBase: public Cgi
{
public:
	CgiManagerBase(uint64_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile, bool bCheckLogin)
		:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile), m_check_login(bCheckLogin)
	{

	}

public:
	virtual bool DerivedInit();
	virtual bool Process();
	virtual bool InnerProcess() = 0;

public:
	bool IsValidPhone(const std::string& strPhone);
	bool CheckPasswd(const std::string& strPhone, const std::string strPasswd, std::string& strErrMsg);

	bool VerifyCodeAdd(const std::string& strPhone, const std::string& strVC, std::string& strErrMsg);
	bool VerifyCodeQuery(const std::string& strPhone, std::string& strVC, std::string& strErrMsg);
	bool VerifyCodeCheck(const std::string& strPhone, const std::string& strVC, std::string& strErrMsg);

	bool SessionAdd(const std::string& strUin, const std::string& strVC, std::string& strErrMsg);
	bool SessionCheck(const std::string& strUin, const std::string& strVC, std::string& strErrMsg);

	int MakeCookie(const std::string& strUin, std::string& strCookieKeyValue, std::string& strErrMsg);
	int MakeAddReplyCookie(const string& strUin,lce::cgi::CHttpHeader& stHttpRspHeader, string& strErrMsg);
	
	int SendTemplateMessageOfInvoicePass(const hoosho::msg::z::OrderInfo &stOrderInfo);

public:
	bool m_check_login;
	std::string m_login_phone;

	uint32_t m_manager_phone_size;
	std::vector<std::string> m_manager_phone_list;
	std::vector<std::string> m_manager_passwd_list;

	std::string m_cookie_name_uin;
	std::string m_cookie_name_key;
	std::string m_cookie_domain;
	std::string m_cookie_path;

	std::string m_memcached_cache_ip;
	uint32_t m_memcached_cache_port_session;
	uint32_t m_memcached_cache_port_vc;

	uint32_t m_session_life;

	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_table_name_user_extra_info;
	std::string m_table_name_order_info;
	std::string m_table_name_user_info;
	std::string m_table_name_outcome_req;
	lce::cgi::CMysql m_mysql_helper;

	std::string m_sms_api_host;
	std::string m_sms_api_ip;
	uint16_t m_sms_api_port;
	std::string m_sms_api_account;
	std::string m_sms_api_accountpwd;
	std::string m_sms_api_appid;
	std::string m_sms_api_version;
	std::string m_sms_templateid;
	std::string m_sms_subject;
	std::string m_sms_html;

	std::string m_z_server_ip;
	uint16_t m_z_server_port;

	std::string m_pa_appid;

	std::string m_invoice_pass_template_id;
	std::string m_invoice_pass_color;
	std::string m_invoice_pass_url;
	std::string m_invoice_pass_first;
	std::string m_invoice_pass_keyword1;
	std::string m_invoice_pass_keyword2;
	std::string m_invoice_pass_keyword3;
	std::string m_invoice_pass_remark;
	
protected:
	DECL_LOGGER(logger);

};

/////////
#define DB_ORDER_INFO_OPENID_WAITING_COMPONENT_LEN 2
#define DB_ORDER_INFO_OPENID_WAITING_MAX_NUM 10

inline static void order_waiting_user_str_2_pb(const std::string& str,  hoosho::msg::z::OrderInfo& orderInfo)
{
	std::vector<std::string> vecUnits;
	std::vector<std::string> vecUsers;
	lce::util::StringOP::Split(str, ",", vecUnits);
	orderInfo.clear_order_waiting_user_list();
	for(size_t i=0; i!=vecUnits.size(); ++i)
	{
		vecUsers.clear();
		lce::util::StringOP::Split(vecUnits[i], "|", vecUsers);
		if(DB_ORDER_INFO_OPENID_WAITING_COMPONENT_LEN == vecUsers.size())
		{
			hoosho::msg::z::OrderWaitingUser* pWaitingUser = orderInfo.add_order_waiting_user_list();
			pWaitingUser->set_openid(vecUsers[0]);
			pWaitingUser->set_create_ts(strtoul(vecUsers[1].c_str(), NULL, 10));
		}
	}
}

/*
static std::string order_waiting_user_pb_2_str(const hoosho::msg::z::OrderInfo& orderInfo)
{
	std::ostringstream oss;
	oss.str("");
	for(int i=0; i!=orderInfo.order_waiting_user_list_size(); ++i)
	{
		const hoosho::msg::z::OrderWaitingUser& unit = orderInfo.order_waiting_user_list(i);
		if(i != 0)
		{
			oss<<",";
		}

		oss<<unit.openid()<<"|"<<unit.create_ts();
	}

	return oss.str();
}

static bool order_waiting_user_exists(const hoosho::msg::z::OrderInfo& orderInfo
											, const std::string& strFentchOpenid)
{
	
	for(int i=0; i!=orderInfo.order_waiting_user_list_size(); ++i)
	{
		if(strFentchOpenid == orderInfo.order_waiting_user_list(i).openid())
		{
			return true;
		}
	}

	return false;
}

static void order_waiting_user_del(hoosho::msg::z::OrderInfo& orderInfo
											, const std::string& strFentchOpenid)
{
	std::vector<hoosho::msg::z::OrderWaitingUser> vecTmp;
	for(int i=0; i!=orderInfo.order_waiting_user_list_size(); ++i)
	{
		if(strFentchOpenid != orderInfo.order_waiting_user_list(i).openid())
		{
			vecTmp.push_back(orderInfo.order_waiting_user_list(i));
		}
	}

	if((size_t)orderInfo.order_waiting_user_list_size() != vecTmp.size())
	{
		orderInfo.mutable_order_waiting_user_list()->Clear();
		for(size_t i=0; i!=vecTmp.size(); ++i)
		{
			orderInfo.add_order_waiting_user_list()->CopyFrom(vecTmp[i]);
		}
	}
}
*/

inline static void order_info_db_2_pb(lce::cgi::CMysql& mysql, hoosho::msg::z::OrderInfo& orderInfo)
{
	orderInfo.set_order_id(strtoul(mysql.GetRow(0), NULL, 10));
	orderInfo.set_openid_master(mysql.GetRow(1));
	orderInfo.set_openid_slave(mysql.GetRow(2));
	orderInfo.set_order_visible(strtoul(mysql.GetRow(3), NULL, 10));
	orderInfo.set_order_status(strtoul(mysql.GetRow(4), NULL, 10));
	orderInfo.set_pay_status(strtoul(mysql.GetRow(5), NULL, 10));
	orderInfo.set_addr_to_longitude(strtoul(mysql.GetRow(6), NULL, 10));
	orderInfo.set_addr_to_latitude(strtoul(mysql.GetRow(7), NULL, 10));
	orderInfo.set_addr_to(mysql.GetRow(8));
	orderInfo.set_addr_to_detail(mysql.GetRow(9));
	orderInfo.set_order_desc(mysql.GetRow(10));
	orderInfo.set_order_receiver_name(mysql.GetRow(11));
	orderInfo.set_order_receiver_phone(mysql.GetRow(12));
	orderInfo.set_price(strtoul(mysql.GetRow(13), NULL, 10));
	orderInfo.set_expect_from_ts(strtoul(mysql.GetRow(14), NULL, 10));
	orderInfo.set_expect_to_ts(strtoul(mysql.GetRow(15), NULL, 10));
	orderInfo.set_pay_ts(strtoul(mysql.GetRow(16), NULL, 10));
	orderInfo.set_create_ts(strtoul(mysql.GetRow(17), NULL, 10));
	orderInfo.set_finish_ts(strtoul(mysql.GetRow(18), NULL, 10));
	orderInfo.set_fetch_ts(strtoul(mysql.GetRow(19), NULL, 10));
	orderInfo.set_cancel_ts(strtoul(mysql.GetRow(20), NULL, 10));
	orderInfo.set_star_from_master(strtoul(mysql.GetRow(21), NULL, 10));
	orderInfo.set_star_from_slave(strtoul(mysql.GetRow(22), NULL, 10));
	order_waiting_user_str_2_pb(mysql.GetRow(23), orderInfo);
	orderInfo.set_extra_data(mysql.GetRow(24));
	orderInfo.set_order_type(strtoul(mysql.GetRow(25), NULL, 10));
	orderInfo.set_media_type(strtoul(mysql.GetRow(26), NULL, 10));
}

inline static void user_info_db_2_pb(lce::cgi::CMysql& mysql, hoosho::msg::z::UserInfo& userInfo)
{
	userInfo.set_openid(mysql.GetRow(0));
	userInfo.set_nickname(mysql.GetRow(1));
	userInfo.set_sex(strtoul(mysql.GetRow(2), NULL, 10));
	userInfo.set_headimgurl(mysql.GetRow(3));
	userInfo.set_self_desc(mysql.GetRow(4));
	userInfo.set_phone(mysql.GetRow(5));
	userInfo.set_user_type(atoi(mysql.GetRow(6)));	
	userInfo.set_user_flag(strtoul(mysql.GetRow(7), NULL, 10));
	userInfo.set_user_score(strtoul(mysql.GetRow(8), NULL, 10));
}

inline static void outcome_req_info_db_2_pb(lce::cgi::CMysql& mysql, hoosho::msg::z::OutcomeReqInfo& outcomeReqInfo)
{
	outcomeReqInfo.set_table_id(strtoul(mysql.GetRow(0), NULL, 10));
	outcomeReqInfo.set_openid(mysql.GetRow(1));
	outcomeReqInfo.set_amount(strtoul(mysql.GetRow(2), NULL, 10));
	outcomeReqInfo.set_state(strtoul(mysql.GetRow(3), NULL, 10));
	outcomeReqInfo.set_create_ts(strtoul(mysql.GetRow(4), NULL, 10));
}

inline static void cash_flow_info_db_2_pb(lce::cgi::CMysql& mysql, hoosho::msg::z::CashFlowInfo& cashFlowInfo)
{
	cashFlowInfo.set_table_id(strtoul(mysql.GetRow(0), NULL, 10));
	cashFlowInfo.set_openid(mysql.GetRow(1));
	cashFlowInfo.set_out_trade_no(mysql.GetRow(2));
	cashFlowInfo.set_amount(strtoul(mysql.GetRow(3), NULL, 10));
	cashFlowInfo.set_add_or_reduce(strtoul(mysql.GetRow(4), NULL, 10));
	cashFlowInfo.set_type(strtoul(mysql.GetRow(5), NULL, 10));
	cashFlowInfo.set_balance(strtoul(mysql.GetRow(6), NULL, 10));
	cashFlowInfo.set_create_ts(strtoul(mysql.GetRow(7), NULL, 10));
}


#endif
