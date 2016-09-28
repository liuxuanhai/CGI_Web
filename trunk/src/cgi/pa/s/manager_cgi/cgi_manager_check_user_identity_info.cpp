#include "cgi_manager_base.h"

class CgiManagerCheckUserIdentityInfo:public CgiManagerBase
{
public:
	CgiManagerCheckUserIdentityInfo():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		uint64_t qwUserId = common::util::charToUint64_t( ((string)(GetInput().GetValue("user_id"))).c_str() );
		uint32_t dwStatus = (uint32_t) GetInput().GetValue("status");

		ZERO_INT_RETURN(qwUserId);
		if(dwStatus != hoosho::msg::s::USER_IDENTITY_STATUS_PASS && dwStatus != hoosho::msg::s::USER_IDENTITY_STATUS_FAIL)
		{
			LOG4CPLUS_ERROR(logger, "invalid status = " << dwStatus);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		std::ostringstream oss;

		//get user identity info, check status
		oss.str("");
		oss << "SELECT identity_status FROM " << m_table_name_user_identity_info
			<< " WHERE user_id = " << qwUserId;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			uint32_t dwIdentityStatus = atoi(m_mysql_helper.GetRow(0));
			if(dwIdentityStatus != hoosho::msg::s::USER_IDENTITY_STATUS_PENDING)
			{
				LOG4CPLUS_ERROR(logger, "user_id=" << qwUserId << " has been set identity_status=" << dwIdentityStatus);
				DoReply(CGI_RET_CODE_INVALID_OP);
				return true;
			}
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "user_id=" << qwUserId << " not found");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		uint64_t qwTS = common::util::GetCurrentMs();
		//update user identity info
		oss.str("");
		oss << "UPDATE " << m_table_name_user_identity_info 
			<< " SET identity_status = " << dwStatus
			<< ", check_ts = " << qwTS
			<< ", check_mgr = '" << sql_escape(m_login_phone) << "'"
			<< " WHERE user_id = " << qwUserId;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		LOG4CPLUS_INFO(logger, "user_id=" << qwUserId << " set to identity_status=" << dwStatus << " by phone=" << m_login_phone);

		DoReply(CGI_RET_CODE_OK);
		return true;

	}

};

int main()
{
	CgiManagerCheckUserIdentityInfo cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
