#include "cgi_manager_base.h"

class CgiManagerGetFeedList:public CgiManagerBase
{
public:
	CgiManagerGetFeedList():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	void GetGoodInfoByGoodId(const set<uint64_t> &setGoodid, vector<hoosho::msg::s::GoodInfo> &vecGoodInfo)
	{
		ostringstream oss;
		for(set<uint64_t>::const_iterator it = setGoodid.begin(); it != setGoodid.end(); ++it)
		{
			oss.str("");
			oss << "SELECT * FROM " << m_table_name_good_info 
				<< " WHERE good_id = " << (*it);
			LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
			SQL_QUERY_FAIL_CONTINUE(m_mysql_helper, oss, logger);

			if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
			{
				hoosho::msg::s::GoodInfo stGoodInfo;
				GoodInfoDB2PB(m_mysql_helper, stGoodInfo);
				vecGoodInfo.push_back(stGoodInfo);
			}
			else
			{
				LOG4CPLUS_ERROR(logger, "good_id=" << (*it) << " not found");
			}
		}
	}

	bool InnerProcess()
	{
		uint64_t qwBeginFeedId = common::util::charToUint64_t( ((string)(GetInput().GetValue("begin_feed_id"))).c_str() );
		uint32_t dwLimit = (uint32_t) GetInput().GetValue("limit");
		uint32_t dwFeedStatus = (uint32_t) GetInput().GetValue("status");

		if(!qwBeginFeedId)
		{
			qwBeginFeedId = 0xffffffffffffffff;
		}

		if(!dwLimit || dwLimit > 50)
		{
			dwLimit = 20;
		}

		std::ostringstream oss;
		oss.str("");
		oss << "SELECT * FROM " << m_table_name_feed_info
			<< " WHERE feed_id < " << qwBeginFeedId;

		if(dwFeedStatus != 0)
		{
			oss << " AND feed_status = " << dwFeedStatus;
		}
		
		oss	<< " ORDER BY feed_id DESC"
			<< " LIMIT " << dwLimit;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		set<uint64_t> setGoodid;

		while(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			hoosho::msg::s::FeedInfo stFeedInfo;
			FeedInfoDB2PB(m_mysql_helper, stFeedInfo);
			lce::cgi::CAnyValue stAnyValue;
			stAnyValue = FeedInfoPB2Any(stFeedInfo);
			GetAnyValue()["feed_info_list"].push_back(stAnyValue);

			setGoodid.insert(stFeedInfo.good_id());
		}

		vector<hoosho::msg::s::GoodInfo> vecGoodInfo;
		GetGoodInfoByGoodId(setGoodid, vecGoodInfo);
		for(size_t i = 0; i < vecGoodInfo.size(); i++)
		{
			GetAnyValue()["good_info_list"][int_2_str(vecGoodInfo[i].good_id())] = GoodInfoPB2Any(vecGoodInfo[i]);
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiManagerGetFeedList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
