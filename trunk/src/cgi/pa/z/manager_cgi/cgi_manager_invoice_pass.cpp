#include "cgi_manager_base.h"

class CgiManagerInvoicePass:public CgiManagerBase
{
public:
	CgiManagerInvoicePass():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		uint64_t qwOrderid = common::util::charToUint64_t( ((string)(GetInput().GetValue("orderid"))).c_str() );
		uint32_t dwOrderStatus = (uint32_t) GetInput().GetValue("status");

		ZERO_INT_RETURN(qwOrderid);
		if(dwOrderStatus != 1 && dwOrderStatus != 2)
		{
			LOG4CPLUS_ERROR(logger, "invalid status = " << dwOrderStatus);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		std::ostringstream oss;
		std::string strErrMsg;
		std::string strOpenidMaster;

		//check 
		oss.str("");
		oss << "SELECT order_status, order_type, openid_master FROM " << m_table_name_order_info
			<< " WHERE orderid = " << qwOrderid;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			uint32_t dwOrderStatus = atoi(m_mysql_helper.GetRow(0));
			uint32_t dwOrderType = atoi(m_mysql_helper.GetRow(1));
			strOpenidMaster = m_mysql_helper.GetRow(2);
			if(dwOrderStatus != ORDER_STATUS_INVOICE_WAITING_CHECK)
			{
				LOG4CPLUS_ERROR(logger, "orderid=" << qwOrderid << " has been set order_status=" << dwOrderStatus);
				DoReply(CGI_RET_CODE_INVALID_OP);
				return true;
			}
			if(dwOrderType != ORDER_TYPE_INVOICE)
			{
				LOG4CPLUS_ERROR(logger, "orderid=" << qwOrderid << " has order_type=" << dwOrderType << ", not involice");
				DoReply(CGI_RET_CODE_INVALID_OP);
				return true;
			}
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "orderid=" << qwOrderid << " not found");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		if(dwOrderStatus == 1)
		{
			//set
			oss.str("");
			oss << "UPDATE " << m_table_name_order_info
				<< " SET order_status = " << ORDER_STATUS_INVOICE_CHECK_SUCC
				<< " , finish_ts = unix_timestamp()"
				<< " WHERE orderid = " << qwOrderid;
			LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
			SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

			LOG4CPLUS_INFO(logger, "orderid=" << qwOrderid << " pass by phone=" << m_login_phone);

			//add
			oss.str("");
			oss << "SELECT available_lottery_time FROM " << m_table_name_user_extra_info
				<< " WHERE openid = '" << sql_escape(strOpenidMaster) << "'";
			SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
			
			uint32_t dwAvailableLotteryTime = 0;
			if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
			{
				dwAvailableLotteryTime = atoi(m_mysql_helper.GetRow(0));
				LOG4CPLUS_INFO(logger, "openid=" << strOpenidMaster << ", old available_lottery_time=" << dwAvailableLotteryTime);
				dwAvailableLotteryTime++;

				oss.str("");
				oss << "UPDATE " << m_table_name_user_extra_info
					<< " SET available_lottery_time = " << dwAvailableLotteryTime
					<< " WHERE openid = '" << sql_escape(strOpenidMaster) << "'";
				SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
			}
			else
			{
				LOG4CPLUS_INFO(logger, "openid=" << strOpenidMaster << ",  old available_lottery_time=0");
				dwAvailableLotteryTime++;
				oss.str("");
				oss << "INSERT INTO " << m_table_name_user_extra_info
					<< " SET openid = '" << sql_escape(strOpenidMaster) << "'"
					<< ", available_lottery_time = 1";
				SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
			}

			LOG4CPLUS_INFO(logger, "openid=" << strOpenidMaster << " available_lottery_time add to " << dwAvailableLotteryTime << ", by phone=" << m_login_phone);

			//
			oss.str("");
			oss << "UPDATE " << m_table_name_user_info
				<< " SET user_score = user_score + " << SCORE_TYPE_TICKET_UPLOAD 
				<< " WHERE openid = '" << sql_escape(strOpenidMaster) << "'";
			SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
			LOG4CPLUS_INFO(logger, "openid=" << strOpenidMaster << " user_score add " << SCORE_TYPE_TICKET_UPLOAD << ", by phone=" << m_login_phone);
		}
		else if(dwOrderStatus == 2)
		{
			//set
			oss.str("");
			oss << "UPDATE " << m_table_name_order_info
				<< " SET order_status = " << ORDER_STATUS_INVOICE_CHECK_FAIL
				<< " , finish_ts = unix_timestamp()"
				<< " WHERE orderid = " << qwOrderid;
			LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
			SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

			LOG4CPLUS_INFO(logger, "orderid=" << qwOrderid << " not pass by phone=" << m_login_phone);
		}

		//send template message
		oss.str("");
		oss << "SELECT * FROM " << m_table_name_order_info 
			<< " WHERE orderid = " << qwOrderid;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		hoosho::msg::z::OrderInfo stOrderInfo;
		order_info_db_2_pb(m_mysql_helper, stOrderInfo);
		SendTemplateMessageOfInvoicePass(stOrderInfo);

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiManagerInvoicePass cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
