#include "cgi_manager_base.h"

class CgiManagerGetTicketList:public CgiManagerBase
{
public:
	CgiManagerGetTicketList():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		uint64_t qwBeginTicketId = common::util::charToUint64_t( ((string)(GetInput().GetValue("begin_ticket_id"))).c_str() );
		uint32_t dwLimit = (uint32_t) GetInput().GetValue("limit");
		uint32_t dwCheckStatus = (uint32_t) GetInput().GetValue("status");

		if(!qwBeginTicketId)
		{
			qwBeginTicketId = 0xffffffffffffffff;
		}

		if(!dwLimit || dwLimit > 50)
		{
			dwLimit = 20;
		}

		std::ostringstream oss;
		std::string strErrMsg;
		oss.str("");
		oss << "SELECT * FROM " << m_table_name_ticket_info
			<< " WHERE ticket_id < " << qwBeginTicketId;

		if(dwCheckStatus != 0)
		{
			oss << " AND check_status = " << dwCheckStatus;
		}
		
		oss	<< " ORDER BY ticket_id DESC"
			<< " LIMIT " << dwLimit;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		std::set<uint64_t> setOpenid;
		while(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			hoosho::msg::s::TicketInfo stTicketInfo;
			TicketInfoDB2PB(m_mysql_helper, stTicketInfo);
			setOpenid.insert(stTicketInfo.user_id());
			lce::cgi::CAnyValue stAnyValue;
			stAnyValue = TicketInfoPB2Any(stTicketInfo);
			GetAnyValue()["ticket_info_list"].push_back(stAnyValue);
		}

		for(std::set<uint64_t>::iterator iter = setOpenid.begin(); iter != setOpenid.end(); ++iter)
		{
			oss.str("");
			oss << "SELECT * FROM " << m_table_name_user_info
				<< " WHERE user_id = " << *iter;
			LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
			SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

			m_mysql_helper.Next();
			hoosho::msg::s::UserInfo stUserInfo;
			UserInfoDB2PB(m_mysql_helper, stUserInfo);
			lce::cgi::CAnyValue stAnyValue;
			stAnyValue = UserInfoPB2Any(stUserInfo);
			GetAnyValue()["user_info"][*iter] = stAnyValue;
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiManagerGetTicketList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
