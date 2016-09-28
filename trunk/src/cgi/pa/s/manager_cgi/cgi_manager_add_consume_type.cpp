#include "cgi_manager_base.h"
#include "common_util.h"

class CgiManagerAddConsumeType:public CgiManagerBase
{
public:
	CgiManagerAddConsumeType():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		string strConsumeTypeText = (string)GetInput().GetValue("consume_type_text");

		EMPTY_STR_RETURN(strConsumeTypeText);

		std::ostringstream oss;

		//check text unique
		oss.str("");
		oss << "SELECT consume_type_text FROM " << m_table_name_consume_type
			<< " WHERE consume_type_text = '" << sql_escape(strConsumeTypeText) << "'";
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			LOG4CPLUS_ERROR(logger, "consume_type_text=" << strConsumeTypeText << " exists");
			DoReply(CGI_RET_CODE_ALREADY_EXIST);
			return true;
		}

		uint64_t qwConsumeType = time(NULL);
		uint64_t qwTS = common::util::GetCurrentMs();
		//add consume_type
		oss.str("");
		oss << "INSERT INTO " << m_table_name_consume_type 
			<< " SET consume_type = " << qwConsumeType
			<< ", consume_type_text = '" << sql_escape(strConsumeTypeText) << "'"
			<< ", create_ts = " << qwTS;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		LOG4CPLUS_INFO(logger, "consumte_type=" << qwConsumeType << ", consume_type_text=" << strConsumeTypeText << " add by phone=" << m_login_phone);

		GetAnyValue()["consumte_type"] = qwConsumeType;
		DoReply(CGI_RET_CODE_OK);
		return true;

	}

};

int main()
{
	CgiManagerAddConsumeType cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
