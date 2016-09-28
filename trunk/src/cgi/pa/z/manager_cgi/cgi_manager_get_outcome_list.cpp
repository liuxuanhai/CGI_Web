#include "cgi_manager_base.h"

class CgiManagerGetOutcomeList:public CgiManagerBase
{
public:
	CgiManagerGetOutcomeList():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{	
		//1.Get HTTP Params
		uint32_t iState = (int)GetInput().GetValue("state");
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

		//2.Get outcome_list
		std::ostringstream oss;
		std::string strErrMsg;
		oss.str("");
		oss << "SELECT * FROM " << m_table_name_outcome_req
			<< " WHERE state=" << iState
			<< " AND create_ts<" << qwBeginTs
			<< " LIMIT " << iLimit;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		std::set<std::string> setOpenid;
		if(m_mysql_helper.GetRowCount())
		{
			while(m_mysql_helper.Next())
			{
				::hoosho::msg::z::OutcomeReqInfo stOutcomeReqInfo;
				
				outcome_req_info_db_2_pb(m_mysql_helper, stOutcomeReqInfo);
				setOpenid.insert(stOutcomeReqInfo.openid());
				lce::cgi::CAnyValue stAnyValue;
				OutComeReqInfoPB2Any(stOutcomeReqInfo, stAnyValue);
				GetAnyValue()["outcome_list"].push_back(stAnyValue);
			}
		}

		//3.Get userinfo_map
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
			GetAnyValue()["userinfo_map"][*iter] = stAnyValue;
		}		

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiManagerGetOutcomeList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

