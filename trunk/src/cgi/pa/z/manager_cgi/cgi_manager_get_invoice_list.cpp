#include "cgi_manager_base.h"

class CgiManagerGetInvoiceList:public CgiManagerBase
{
public:
	CgiManagerGetInvoiceList():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		uint64_t qwBeginOrderid = common::util::charToUint64_t( ((string)(GetInput().GetValue("begin_orderid"))).c_str() );
		uint32_t dwLimit = (uint32_t) GetInput().GetValue("limit");
		uint32_t dwOrderStatus = (uint32_t) GetInput().GetValue("status");

		if(!qwBeginOrderid)
		{
			qwBeginOrderid = 0xffffffffffffffff;
		}

		if(!dwLimit || dwLimit > 50)
		{
			dwLimit = 20;
		}

		std::ostringstream oss;
		std::string strErrMsg;
		oss.str("");
		oss << "SELECT * FROM " << m_table_name_order_info
			<< " WHERE order_type = " << ORDER_TYPE_INVOICE
			<< " AND orderid < " << qwBeginOrderid;

		if(dwOrderStatus == 1)
		{
			oss << " AND order_status = " << ORDER_STATUS_INVOICE_CHECK_SUCC;
		}
		else if(dwOrderStatus == 2)
		{
			oss << " AND order_status = " << ORDER_STATUS_INVOICE_CHECK_FAIL;
		}
		else if(dwOrderStatus == 3)
		{
			oss << " AND order_status = " << ORDER_STATUS_INVOICE_WAITING_CHECK;
		}
		oss	<< " ORDER BY orderid DESC"
			<< " LIMIT " << dwLimit;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		std::set<std::string> setOpenid;
		while(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			hoosho::msg::z::OrderInfo stOrderInfo;
			order_info_db_2_pb(m_mysql_helper, stOrderInfo);
			setOpenid.insert(stOrderInfo.openid_master());
			lce::cgi::CAnyValue stAnyValue;
			OrderInfoPB2Any(stOrderInfo, stAnyValue);
			GetAnyValue()["order_list"].push_back(stAnyValue);
		}

		for(std::set<std::string>::iterator iter = setOpenid.begin(); iter != setOpenid.end(); ++iter)
		{
			oss.str("");
			oss << "SELECT * FROM " << m_table_name_user_info
				<< " WHERE openid = '" << sql_escape(*iter) << "'";
			LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
			SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

			m_mysql_helper.Next();
			hoosho::msg::z::UserInfo stUserInfo;
			user_info_db_2_pb(m_mysql_helper, stUserInfo);
			lce::cgi::CAnyValue stAnyValue;
			UserInfoPB2Any(stUserInfo, stAnyValue);
			GetAnyValue()["user_info"][*iter] = stAnyValue;
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiManagerGetInvoiceList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
