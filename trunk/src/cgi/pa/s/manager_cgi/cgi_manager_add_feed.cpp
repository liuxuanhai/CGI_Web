#include "cgi_manager_base.h"
#include "common_util.h"

class CgiManagerAddFeed:public CgiManagerBase
{
public:
	CgiManagerAddFeed():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		uint64_t qwGoodId = strtoul(((string) GetInput().GetValue("good_id")).c_str(), NULL, 10);
		string strExtraData = (string) GetInput().GetValue("extra_data");		
		
		ZERO_INT_RETURN(qwGoodId);

		std::ostringstream oss;

		//get good_info
		uint32_t dwTotalJoinNum = 0;
		oss.str("");
		oss << "SELECT total_join_num FROM " << m_table_name_good_info
			<< " WHERE good_id = " << qwGoodId;
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			dwTotalJoinNum = atoi(m_mysql_helper.GetRow(0));
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "invalid good_id=" << qwGoodId << " not found");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		//make sure not running feed
		oss.str("");
		oss << "SELECT count(*) FROM " << m_table_name_feed_info
			<< " WHERE good_id = " << qwGoodId
			<< " AND feed_status != " << hoosho::msg::s::FEED_STATUS_FINISH;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		m_mysql_helper.Next();
		if(atoi(m_mysql_helper.GetRow(0)) != 0)
		{
			LOG4CPLUS_ERROR(logger, "good_id=" << qwGoodId << " still has feed running");
			DoReply(CGI_RET_CODE_ALREADY_EXIST);
			return true;
		}

		//get feed_num of the good
		oss.str("");
		oss << "SELECT count(*) FROM " << m_table_name_feed_info
			<< " WHERE good_id = " << qwGoodId;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		m_mysql_helper.Next();
		uint32_t dwFeedNum = atoi(m_mysql_helper.GetRow(0));
		dwFeedNum++;

		const uint32_t dwMaxFeedNum = 1000000;
		uint64_t qwFeedId = qwGoodId * dwMaxFeedNum + dwFeedNum;
		uint64_t qwTS = common::util::GetCurrentMs();

		//add feed
		oss.str("");
		oss << "INSERT INTO " << m_table_name_feed_info 
			<< " SET feed_id = " << qwFeedId
			<< ", good_id = " << qwGoodId
			<< ", total_join_num = " << dwTotalJoinNum
			<< ", create_ts = " << qwTS
			<< ", feed_status = " << hoosho::msg::s::FEED_STATUS_INIT
			<< ", extra_data = '" << sql_escape(strExtraData) << "'";

		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		LOG4CPLUS_INFO(logger, "feed_id=" << qwFeedId << " add by phone=" << m_login_phone);

		GetAnyValue()["feed_id"] = int_2_str(qwFeedId);
		DoReply(CGI_RET_CODE_OK);
		return true;

	}

};

int main()
{
	CgiManagerAddFeed cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
