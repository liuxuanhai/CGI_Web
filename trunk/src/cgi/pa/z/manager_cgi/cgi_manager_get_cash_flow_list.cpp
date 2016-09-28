#include "cgi_manager_base.h"

class CgiManagerGetCashFlowList:public CgiManagerBase
{
public:
	CgiManagerGetCashFlowList():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{		
		//1.Get HTTP Params
		std::string strOpenid = (std::string)GetInput().GetValue("openid");				
		uint64_t qwBeginTs = strtoul(((std::string)GetInput().GetValue("begin_ts")).c_str(), NULL, 10);
		uint32_t iLimit = (int)GetInput().GetValue("limit");

		if(iLimit > 20)
		{
			LOG4CPLUS_ERROR(logger, "invalid limit");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		if(0 == qwBeginTs)
		{
			qwBeginTs = 0xFFFFFFFFFFFFFFFF;
		}

		//2.Get cash_flow_info_list
		std::ostringstream oss;
		std::string strErrMsg;
		oss.str("");
		oss << "SELECT * FROM t_cash_flow"
			<< " WHERE openid='"<<sql_escape(strOpenid)<<"'"
			<< " AND create_ts<" << qwBeginTs
			<< " LIMIT " << iLimit;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		
		if(m_mysql_helper.GetRowCount())
		{
			while(m_mysql_helper.Next())
			{
				::hoosho::msg::z::CashFlowInfo stCashFlowInfo;				
				cash_flow_info_db_2_pb(m_mysql_helper, stCashFlowInfo);				
				lce::cgi::CAnyValue stAnyValue;
				CashFlowInfoPB2Any(stCashFlowInfo, stAnyValue);
				GetAnyValue()["cash_flow_info_list"].push_back(stAnyValue);
			}
		}		

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiManagerGetCashFlowList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

