#include "cgi_manager_base.h"

class CgiManagerCheckTicket:public CgiManagerBase
{
public:
	CgiManagerCheckTicket():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
		uint64_t qwTicketId = common::util::charToUint64_t( ((string)(GetInput().GetValue("ticket_id"))).c_str() );
		uint32_t dwReqCheckStatus = (uint32_t) GetInput().GetValue("status");
		uint64_t qwTicketPrice = common::util::charToUint64_t( ((string)(GetInput().GetValue("ticket_price"))).c_str() );
		uint32_t dwConsumeType = (uint32_t) GetInput().GetValue("consume_type");
		string strExtraData = (string) GetInput().GetValue("extra_data");

		ZERO_INT_RETURN(qwTicketId);
		if(dwReqCheckStatus != hoosho::msg::s::TICKET_CHECK_STATUS_PASS && dwReqCheckStatus != hoosho::msg::s::TICKET_CHECK_STATUS_FAIL)
		{
			LOG4CPLUS_ERROR(logger, "invalid status = " << dwReqCheckStatus);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		std::ostringstream oss;

		//get ticket info, check status
		uint64_t qwFeedId = 0;
		uint64_t qwUserId = 0;
		oss.str("");
		oss << "SELECT check_status, feed_id, user_id FROM " << m_table_name_ticket_info
			<< " WHERE ticket_id = " << qwTicketId;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			uint32_t dwCheckStatus = atoi(m_mysql_helper.GetRow(0));
			qwFeedId = strtoul(m_mysql_helper.GetRow(1), NULL, 10);
			qwUserId = strtoul(m_mysql_helper.GetRow(2), NULL, 10);
			if(dwCheckStatus != hoosho::msg::s::TICKET_CHECK_STATUS_PENDING)
			{
				LOG4CPLUS_ERROR(logger, "ticket_id=" << qwTicketId << " has been set check_status=" << dwCheckStatus);
				DoReply(CGI_RET_CODE_INVALID_OP);
				return true;
			}
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "ticket_id=" << qwTicketId << " not found");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		const uint32_t dwBeginContendId = 1000000;
		uint32_t dwTotalJoinNum = 0;
		uint32_t dwApprovedNum = 0;
		uint32_t dwPendginNum = 0;
		uint64_t qwContendId = 0;
		uint32_t dwFeedStatus = 0;

		//get feed info
		oss.str("");
		oss << "SELECT total_join_num, approved_num, pending_num, feed_status FROM " << m_table_name_feed_info
			<< " WHERE feed_id = " << qwFeedId;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			dwTotalJoinNum = atoi(m_mysql_helper.GetRow(0));
			dwApprovedNum = atoi(m_mysql_helper.GetRow(1));
			dwPendginNum = atoi(m_mysql_helper.GetRow(2));
			dwFeedStatus = atoi(m_mysql_helper.GetRow(3));
			LOG4CPLUS_DEBUG(logger, "total_join_num=" << dwTotalJoinNum
								<< ", approved_num=" << dwApprovedNum
								<< ", pending_num=" << dwPendginNum
								<< ", feed_status=" << dwFeedStatus);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "feed_id=" << qwFeedId << " not found");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		if(dwFeedStatus != hoosho::msg::s::FEED_STATUS_INIT)
		{
			LOG4CPLUS_ERROR(logger, "feed_id=" << qwFeedId << " status=" << dwFeedStatus << ", invalid");
			DoReply(CGI_RET_CODE_INVALID_OP);
			return true;
		}

		if(dwPendginNum == 0)
		{
			LOG4CPLUS_ERROR(logger, "invalid pending_num=0 with feed_id=" << qwFeedId);
		}
		else
		{
			dwPendginNum--;
		}

		uint64_t qwLuckts = 0;
		uint64_t qwLuckContendid = 0;
		uint64_t qwLuckUserid = 0;
		if(dwReqCheckStatus == hoosho::msg::s::TICKET_CHECK_STATUS_PASS)
		{
			dwApprovedNum++;
			qwContendId = dwBeginContendId + dwApprovedNum;
			if(dwApprovedNum == dwTotalJoinNum)
			{
				dwFeedStatus = hoosho::msg::s::FEED_STATUS_PENDING;
				LOG4CPLUS_INFO(logger, "feed_id=" << qwFeedId << " is pending");

				//10 minuters later
				dwFeedStatus = hoosho::msg::s::FEED_STATUS_FINISH;
				qwLuckts = ::common::util::GetCurrentMs() + 10 * 60 * 1000;
				qwLuckContendid = ::common::util::GetRandNum(1, dwTotalJoinNum) + dwBeginContendId;

				//get luck userid
				if(qwLuckContendid == qwContendId)
				{
					qwLuckUserid = qwUserId;
					LOG4CPLUS_DEBUG(logger, "current userid=" << qwUserId);
				}
				else
				{
					oss.str("");
					oss << "SELECT user_id FROM " << m_table_name_ticket_info
						<< " WHERE contend_id = " << qwLuckContendid
						<< " AND feed_id = " << qwFeedId;
					LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
					SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
					if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
					{
						qwLuckUserid = strtoul(m_mysql_helper.GetRow(0), NULL, 10);
					}
					else
					{
						LOG4CPLUS_ERROR(logger, "contend_id=" << qwLuckContendid << " not found");
						DoReply(CGI_RET_CODE_SERVER_BUSY);
						return true;
					}
				}

				LOG4CPLUS_INFO(logger, "feed_id=" << qwFeedId << ", luck_ts=" << qwLuckts 
						<< ", luck_contend_id=" << qwLuckContendid << ", luck_user_id=" << qwLuckUserid);
			}
		}

		uint64_t qwTS = common::util::GetCurrentMs();

		//update ticket info
		oss.str("");
		oss << "UPDATE " << m_table_name_ticket_info 
			<< " SET check_status = " << dwReqCheckStatus
			<< ", contend_id = " << qwContendId
			<< ", ticket_price = " << qwTicketPrice
			<< ", consume_type = " << dwConsumeType
			<< ", extra_data = '" << sql_escape(strExtraData) << "'"
			<< ", check_ts = " << qwTS
			<< ", check_mgr = '" << sql_escape(m_login_phone) << "'"
			<< " WHERE ticket_id = " << qwTicketId;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		LOG4CPLUS_INFO(logger, "ticket_id=" << qwTicketId << " set to check_status=" << dwReqCheckStatus << ", contend_id=" << qwContendId << " by phone=" << m_login_phone);

		//update feed info
		oss.str("");
		oss << "UPDATE " << m_table_name_feed_info
			<< " SET approved_num = " << dwApprovedNum
			<< ", pending_num = " << dwPendginNum
			<< ", feed_status = " << dwFeedStatus
			<< ", luck_ts = " << qwLuckts
			<< ", luck_contend_id = " << qwLuckContendid
			<< ", luck_user_id = " << qwLuckUserid;
		LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);
		

		DoReply(CGI_RET_CODE_OK);
		return true;

	}

};

int main()
{
	CgiManagerCheckTicket cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
