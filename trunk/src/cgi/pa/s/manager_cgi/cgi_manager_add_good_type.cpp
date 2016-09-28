#include "cgi_manager_base.h"
#include "common_util.h"

class CgiManagerAddGoodType:public CgiManagerBase
{
public:
	CgiManagerAddGoodType():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		string strGoodTypeText = (string)GetInput().GetValue("good_type_text");
		string strGoodTypeIcon = (string)GetInput().GetValue("good_type_icon");
		string strExtraData = (string)GetInput().GetValue("extra_data");

		EMPTY_STR_RETURN(strGoodTypeText);

		std::ostringstream oss;

		//check  good_type_text unique
		oss.str("");
		oss << "SELECT good_type_text FROM " << m_table_name_good_type
			<< " WHERE good_type_text = '" << sql_escape(strGoodTypeText) << "'";
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			LOG4CPLUS_ERROR(logger, "good_type_text=" << strGoodTypeText << " exists");
			DoReply(CGI_RET_CODE_ALREADY_EXIST);
			return true;
		}

		uint64_t qwGoodType = time(NULL);
		uint64_t qwTS = common::util::GetCurrentMs();

		//set
		oss.str("");
		oss << "INSERT INTO " << m_table_name_good_type 
			<< " SET good_type = " << qwGoodType
			<< ", good_type_text = '" << sql_escape(strGoodTypeText) << "'"
			<< ", good_type_icon = '" << sql_escape(strGoodTypeIcon) << "'"
			<< ", extra_data = '" << sql_escape(strExtraData) << "'"
			<< ", create_ts = " << qwTS;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		LOG4CPLUS_INFO(logger, "good_type=" << qwGoodType << ", good_type_text=" << strGoodTypeText << " add by phone=" << m_login_phone);

		GetAnyValue()["good_type"] = qwGoodType;
		DoReply(CGI_RET_CODE_OK);
		return true;

	}

};

int main()
{
	CgiManagerAddGoodType cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
