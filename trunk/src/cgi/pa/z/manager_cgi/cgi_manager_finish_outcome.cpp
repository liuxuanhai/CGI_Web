#include "cgi_manager_base.h"
#include "wx_api_pay.h"

class CgiManagerFinishOutcome:public CgiManagerBase
{
public:
	CgiManagerFinishOutcome():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{		
		//1.Get HTTP Params
		std::string strOpenid = (std::string)GetInput().GetValue("openid");
		std::string strTableid = (std::string)GetInput().GetValue("table_id");
		uint64_t qwAmount = strtoul(((std::string)GetInput().GetValue("amount")).c_str(), NULL, 10);
		std::string strClientIP = std::string(GetInput().GetClientIPStr());

		if(strOpenid.empty() || strTableid.empty())
		{
			LOG4CPLUS_ERROR(logger, "empty openid or tableid");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		//2.Check outcome record
		std::ostringstream oss;
		std::string strErrMsg;
		oss.str("");
		oss << "SELECT * FROM " << m_table_name_outcome_req
			<< " WHERE table_id=" << strTableid
			<< " AND openid='" << sql_escape(strOpenid) << "'"
			<< " AND amount=" << qwAmount
			<< " AND state!=2";
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		if(0 == m_mysql_helper.GetRowCount())
		{
			LOG4CPLUS_ERROR(logger, "Invalid outcome record!");
			DoReply(CGI_RET_CODE_OUTCOME_CHECK_FAIL);
			return true;
		}

		//3.PaPaySendRedPack 
		std::string strMchBillNo;
		common::wxapi::WXAPIPAY stWXAPIPAY;

		int iRet = 0;
		iRet = stWXAPIPAY.PaPaySendRedPack("wx41c62067410a82f6"
									, "1375010002"								
									, "2c43a21c0fa9c8a905e7bba922c6bb8c"
									, "ziga"
									, strOpenid
									, qwAmount
									, strClientIP
									, 1
									, "outcome"
									, "outcome"
									, "outcome"
									, "/etc/ssl/wxpay/ziga/rootca.pem"
									, "/etc/ssl/wxpay/ziga/apiclient_cert.pem"
									, "/etc/ssl/wxpay/ziga/apiclient_key.pem"
									, strMchBillNo
									, strErrMsg
									);

		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "WXAPIPAY.PaPaySendRedPack failed");
			DoReply(6666, strErrMsg);
			return true;
		}		

		LOG4CPLUS_DEBUG(logger, "WXAPIPAY.PaPaySendRedPack success");
		
		//4.update t_user_cash
		oss.str("");
		oss<<"UPDATE t_user_cash SET pending=pending-"<<qwAmount		   
		   <<" WHERE openid='"<<sql_escape(strOpenid)<<"'";
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());		
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		//5.insert t_cash_flow
		oss.str("");
		oss<<"SELECT balance FROM t_user_cash WHERE openid='"<<sql_escape(strOpenid)<<"'";
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());	
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		int64_t qwBalance = 0;
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			qwBalance = strtoul(m_mysql_helper.GetRow(0), NULL, 10);
		}

		uint64_t table_id = ::common::util::generate_unique_id();	
		oss.str("");
		oss<<"INSERT t_cash_flow SET table_id="<<table_id
		   <<", openid='"<<sql_escape(strOpenid)<<"'"
		   <<", out_trade_no='"<<sql_escape(strMchBillNo)<<"'"
		   <<", amount="<<qwAmount
		   <<", add_or_reduce=2"
		   <<", type=7"
		   <<", balance="<<qwBalance
		   <<", create_ts=unix_timestamp()";
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());	
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		//6.update t_outcome_req
		oss.str("");
		oss << "UPDATE " << m_table_name_outcome_req
			<< " SET state=2"
			<< " WHERE table_id=" << strTableid
			<< " AND openid='" << sql_escape(strOpenid) << "'"
			<< " AND amount=" << qwAmount;			
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());		
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiManagerFinishOutcome cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

