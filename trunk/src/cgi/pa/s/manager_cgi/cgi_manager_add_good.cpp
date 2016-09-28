#include "cgi_manager_base.h"
#include "common_util.h"

class CgiManagerAddGood:public CgiManagerBase
{
public:
	CgiManagerAddGood():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		string strGoodIdText = (string) GetInput().GetValue("good_id_text");
		uint64_t qwGoodType = strtoul(((string) GetInput().GetValue("good_type")).c_str(), NULL, 10);
		string strTitle = (string) GetInput().GetValue("title");
		string strDesc = (string) GetInput().GetValue("desc");
		string strShowPics = (string) GetInput().GetValue("show_pics");
		string strDetailPicId = (string) GetInput().GetValue("detail_pic_id");
		uint32_t dwTotalJoinNum  = (uint32_t) GetInput().GetValue("total_join_num") ;

		EMPTY_STR_RETURN(strGoodIdText);
		ZERO_INT_RETURN(qwGoodType);

		std::ostringstream oss;

		//check good_id_text unique
		oss.str("");
		oss << "SELECT good_id_text FROM " << m_table_name_good_info
			<< " WHERE good_id_text = '" << sql_escape(strGoodIdText) << "'";
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			LOG4CPLUS_ERROR(logger, "good_id_text=" << strGoodIdText << " exists");
			DoReply(CGI_RET_CODE_ALREADY_EXIST);
			return true;
		}

		uint64_t qwGoodId = time(NULL);
		uint64_t qwTS = common::util::GetCurrentMs();

		//add good
		oss.str("");
		oss << "INSERT INTO " << m_table_name_good_info 
			<< " SET good_id = " << qwGoodId
			<< ", good_id_text = '" << sql_escape(strGoodIdText) << "'"
			<< ", good_type = " << qwGoodType
			<< ", title = '" << sql_escape(strTitle) << "'"
			<< ", good_desc = '" << sql_escape(strDesc) << "'"
			<< ", show_pics = '" << sql_escape(strShowPics) << "'"
			<< ", detail_pic_id = '" << sql_escape(strDetailPicId) << "'"
			<< ", total_join_num = " << dwTotalJoinNum
			<< ", create_ts = " << qwTS;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		LOG4CPLUS_INFO(logger, "good_id=" << qwGoodId << ", good_id_text=" << strGoodIdText << " add by phone=" << m_login_phone);

		GetAnyValue()["good_id"] = qwGoodId;
		DoReply(CGI_RET_CODE_OK);
		return true;

	}

};

int main()
{
	CgiManagerAddGood cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
