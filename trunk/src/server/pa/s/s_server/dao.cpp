#include "dao.h"
#include "common_util.h"
#include <sstream>
#include <stdlib.h>
#include "jsoncpp/json.h"
#include "wx_api_download_resource.h"

IMPL_LOGGER(Dao, logger);

int DaoUserInfo::GetUserInfoByPrimaryKey(uint64_t qwUserid, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from t_user_info where user_id="<<qwUserid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetUserInfoByPrimaryKey failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		UserInfoDB2PB(mysql, m_user_info);
		return DAO_RET_OK;
	}

	LOG4CPLUS_DEBUG(logger, "user_id="<<qwUserid<<" not exists");
	return DAO_RET_NOT_EXIST;
}

int DaoUserInfo::GetUserInfoList(const std::set<uint64_t> qwUserIdSet, std::vector<hoosho::msg::s::UserInfo>& pbUserInfoList, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	for(std::set<uint64_t>::iterator it = qwUserIdSet.begin(); it != qwUserIdSet.end(); it++)
	{
		uint64_t qwUserId = *it;

		oss.str("");
		oss<<"select * from t_user_info where user_id="<<qwUserId;
		LOG4CPLUS_DEBUG(logger, oss.str());

		if(!mysql.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "GetUserInfoList failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
			return DAO_RET_FAIL;
		}

		if(0 == mysql.GetRowCount() || !mysql.Next())
		{
			LOG4CPLUS_ERROR(logger, "GetUserInfoList error, user_id="<<qwUserId<<" not exists!");
			continue;
		}

		::hoosho::msg::s::UserInfo stUserInfo;
		UserInfoDB2PB(mysql, stUserInfo);
		pbUserInfoList.push_back(stUserInfo);
	}

	return DAO_RET_OK;
}

int DaoUserInfo::SetUserPhone(uint64_t qwUserid, const std::string& strPhone, lce::cgi::CMysql& mysql)
{
	//1.update phone
	std::ostringstream oss;
	oss.str("");
	oss<<"update t_user_info"
	   <<" set phone='"<<sql_escape(strPhone)<<"'"
	   <<" where user_id="<<qwUserid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "SetUserPhone failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoUserInfo::UpdateUserSelfDesc(uint64_t qwUserid, const std::string& strDesc, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"update t_user_info"
	   <<" set self_desc='"<<sql_escape(strDesc)<<"'"
	   <<" where user_id="<<qwUserid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "UpdateUserSelfDesc failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoUserInfo::UpdateUserDefaultRecvAddr(uint64_t qwUserid, uint32_t iAddrNum, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"update t_user_info"
	   <<" set default_addr_num="<<iAddrNum
	   <<" where user_id="<<qwUserid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "UpdateUserDefaultRecvAddr failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoUserIdentityInfo::UpdateUserIdentityInfo(uint64_t qwUserid, const ::hoosho::msg::s::UserIdentityInfo& pbUserIdentityInfo, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"insert into t_user_identity_info"
	   <<" set user_id="<<qwUserid
	   <<", identity_status="<<USER_IDENTITY_STATUS_PENDING
	   <<", phone='"<<sql_escape(pbUserIdentityInfo.phone())<<"'"
	   <<", identity_num='"<<sql_escape(pbUserIdentityInfo.identity_num())<<"'"
	   <<", identity_pic1_id="<<pbUserIdentityInfo.identity_pic1_id()
	   <<", identity_pic2_id="<<pbUserIdentityInfo.identity_pic2_id()
	   <<", identity_pic3_id="<<pbUserIdentityInfo.identity_pic3_id()
	   <<", create_ts="<<timems()
	   <<" on duplicate key update"
	   <<" identity_status="<<USER_IDENTITY_STATUS_PENDING
	   <<", phone='"<<sql_escape(pbUserIdentityInfo.phone())<<"'"
	   <<", identity_num='"<<sql_escape(pbUserIdentityInfo.identity_num())<<"'"
	   <<", identity_pic1_id="<<pbUserIdentityInfo.identity_pic1_id()
	   <<", identity_pic2_id="<<pbUserIdentityInfo.identity_pic2_id()
	   <<", identity_pic3_id="<<pbUserIdentityInfo.identity_pic3_id()
	   <<", create_ts="<<timems();
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "UpdateUserIdentityInfo failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;	  
}

int DaoUserIdentityInfo::GetUserIdentityInfo(uint64_t qwUserid, ::hoosho::msg::s::UserIdentityInfo& pbUserIdentityInfo, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from t_user_identity_info where user_id="<<qwUserid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetUserIdentityInfo failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		UserIdentityInfoDB2PB(mysql, pbUserIdentityInfo);
		return DAO_RET_OK;
	}

	LOG4CPLUS_ERROR(logger, "GetUserIdentityInfo failed, user_id="<<qwUserid<<" not exist!");
	return DAO_RET_NOT_EXIST;
}

int DaoUserRecvAddrInfo::GetUserRecvAddrInfoCount(uint64_t qwUserid, uint32_t& iCount, lce::cgi::CMysql& mysql)
{
	iCount = 0;
	
	std::ostringstream oss;
	oss.str("");
	oss<<"select count(*) from t_user_recv_addr_info where user_id="<<qwUserid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetUserRecvAddrInfoCount failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		iCount = atoi(mysql.GetRow(0));
	}

	return DAO_RET_OK;
}

int DaoUserRecvAddrInfo::AddUserRecvAddrInfo(uint64_t qwUserid, const ::hoosho::msg::s::UserRecvAddrInfo& pbUserRecvAddrInfo, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"insert into t_user_recv_addr_info"
	   <<" set user_id="<<qwUserid
	   <<", addr_num="<<pbUserRecvAddrInfo.addr_num()
	   <<", recv_name='"<<sql_escape(pbUserRecvAddrInfo.recv_name())<<"'"
	   <<", identity_num='"<<sql_escape(pbUserRecvAddrInfo.identity_num())<<"'"
	   <<", phone='"<<sql_escape(pbUserRecvAddrInfo.phone())<<"'"
	   <<", post_num='"<<sql_escape(pbUserRecvAddrInfo.post_num())<<"'"
	   <<", addr_province='"<<sql_escape(pbUserRecvAddrInfo.addr_province())<<"'"
	   <<", addr_city='"<<sql_escape(pbUserRecvAddrInfo.addr_city())<<"'"
	   <<", addr_district='"<<sql_escape(pbUserRecvAddrInfo.addr_district())<<"'"
	   <<", addr_detail='"<<sql_escape(pbUserRecvAddrInfo.addr_detail())<<"'"
	   <<" on duplicate key update"
	   <<" recv_name='"<<sql_escape(pbUserRecvAddrInfo.recv_name())<<"'"
	   <<", identity_num='"<<sql_escape(pbUserRecvAddrInfo.identity_num())<<"'"
	   <<", phone='"<<sql_escape(pbUserRecvAddrInfo.phone())<<"'"
	   <<", post_num='"<<sql_escape(pbUserRecvAddrInfo.post_num())<<"'"
	   <<", addr_province='"<<sql_escape(pbUserRecvAddrInfo.addr_province())<<"'"
	   <<", addr_city='"<<sql_escape(pbUserRecvAddrInfo.addr_city())<<"'"
	   <<", addr_district='"<<sql_escape(pbUserRecvAddrInfo.addr_district())<<"'"
	   <<", addr_detail='"<<sql_escape(pbUserRecvAddrInfo.addr_detail())<<"'";
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "AddUserRecvAddrInfo failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoUserRecvAddrInfo::DelUserRecvAddrInfo(uint64_t qwUserid, uint32_t iAddrNum, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"delete from t_user_recv_addr_info where user_id="<<qwUserid
	   <<" and addr_num="<<iAddrNum;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "DelUserRecvAddrInfo failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoUserRecvAddrInfo::UpdateUserRecvAddrInfo(uint64_t qwUserid, const ::hoosho::msg::s::UserRecvAddrInfo& pbUserRecvAddrInfo, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"insert into t_user_recv_addr_info"
	   <<" set user_id="<<qwUserid
	   <<", addr_num="<<pbUserRecvAddrInfo.addr_num()
	   <<", recv_name='"<<sql_escape(pbUserRecvAddrInfo.recv_name())<<"'"
	   <<", identity_num='"<<sql_escape(pbUserRecvAddrInfo.identity_num())<<"'"
	   <<", phone='"<<sql_escape(pbUserRecvAddrInfo.phone())<<"'"
	   <<", post_num='"<<sql_escape(pbUserRecvAddrInfo.post_num())<<"'"
	   <<", addr_province='"<<sql_escape(pbUserRecvAddrInfo.addr_province())<<"'"
	   <<", addr_city='"<<sql_escape(pbUserRecvAddrInfo.addr_city())<<"'"
	   <<", addr_district='"<<sql_escape(pbUserRecvAddrInfo.addr_district())<<"'"
	   <<", addr_detail='"<<sql_escape(pbUserRecvAddrInfo.addr_detail())<<"'"
	   <<" on duplicate key update"
	   <<" recv_name='"<<sql_escape(pbUserRecvAddrInfo.recv_name())<<"'"
	   <<", identity_num='"<<sql_escape(pbUserRecvAddrInfo.identity_num())<<"'"
	   <<", phone='"<<sql_escape(pbUserRecvAddrInfo.phone())<<"'"
	   <<", post_num='"<<sql_escape(pbUserRecvAddrInfo.post_num())<<"'"
	   <<", addr_province='"<<sql_escape(pbUserRecvAddrInfo.addr_province())<<"'"
	   <<", addr_city='"<<sql_escape(pbUserRecvAddrInfo.addr_city())<<"'"
	   <<", addr_district='"<<sql_escape(pbUserRecvAddrInfo.addr_district())<<"'"
	   <<", addr_detail='"<<sql_escape(pbUserRecvAddrInfo.addr_detail())<<"'";
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "UpdateUserRecvAddrInfo failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoUserRecvAddrInfo::GetUserRecvAddrInfoList(uint64_t qwUserid, std::vector<hoosho::msg::s::UserRecvAddrInfo>& pbUserRecvAddrInfoList, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from t_user_recv_addr_info where user_id="<<qwUserid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetUserRecvAddrInfoList failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::s::UserRecvAddrInfo stUserRecvAddrInfo;
			UserRecvInfoDB2PB(mysql, stUserRecvAddrInfo);
			pbUserRecvAddrInfoList.push_back(stUserRecvAddrInfo);
		}
	}

	return DAO_RET_OK;
}

int DaoGoodType::GetGoodTypeList(std::vector<hoosho::msg::s::GoodType>& pbGoodTypeList, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from t_good_type";
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetGoodTypeList failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::s::GoodType stGoodType;
			GoodTypeDB2PB(mysql, stGoodType);			
			pbGoodTypeList.push_back(stGoodType);
		}
	}

	return DAO_RET_OK;
}

int DaoGoodInfo::GetGoodInfoList(std::vector<hoosho::msg::s::GoodInfo>& pbGoodInfoList, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from t_good_info";
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetGoodInfoList failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::s::GoodInfo stGoodInfo;
			GoodInfoDB2PB(mysql, stGoodInfo);
			pbGoodInfoList.push_back(stGoodInfo);
		}
	}

	return DAO_RET_OK;
}

int DaoGoodInfo::GetGoodInfoListByGoodid(const std::set<uint64_t>& qwGoodidList, std::vector<hoosho::msg::s::GoodInfo>& pbGoodInfoList, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	
	std::set<uint64_t>::iterator it = qwGoodidList.begin();
	for(; it != qwGoodidList.end(); it++)
	{
		uint64_t qwGoodid = *it;
		oss.str("");	
		oss<<"select * from t_good_info where good_id="<<qwGoodid;
		LOG4CPLUS_DEBUG(logger, oss.str());

		if(!mysql.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "GetGoodInfoListByGoodid failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
			return DAO_RET_FAIL;
		}

		if(mysql.GetRowCount() && mysql.Next())
		{
			::hoosho::msg::s::GoodInfo stGoodInfo;
			GoodInfoDB2PB(mysql, stGoodInfo);
			pbGoodInfoList.push_back(stGoodInfo);
		}
	}

	return DAO_RET_OK;
}

int DaoTicketInfo::AddTicket(const ::hoosho::msg::s::TicketInfo& pbTicketInfo, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"insert into t_ticket_info"
	   <<" set ticket_id="<<pbTicketInfo.ticket_id()
	   <<", ticket_pics='"<<sql_escape(pbTicketInfo.ticket_pics())<<"'"
	   <<", user_id="<<pbTicketInfo.user_id()
	   <<", user_ip='"<<sql_escape(pbTicketInfo.user_ip())<<"'"
	   <<", feed_id="<<pbTicketInfo.feed_id()	   
	   <<", good_id="<<pbTicketInfo.good_id()
	   <<", create_ts="<<pbTicketInfo.create_ts()	   
	   <<", check_status="<<TICKET_CHECK_STATUS_PENDING;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "AddTicket failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	DaoFeedInfo stDaoFeedInfo;	
	return stDaoFeedInfo.UpdateFeedInfoByAddTicket(pbTicketInfo.feed_id(), mysql);
}

int DaoTicketInfo::GetUserFeedIdList(uint64_t qwUserid, uint32_t iOffset, uint32_t iLimit, std::set<uint64_t>& qwFeedIdList, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select distinct (feed_id) from t_ticket_info"
	   <<" where user_id="<<qwUserid
	   <<" and check_status="<<TICKET_CHECK_STATUS_PASS
	   <<" order by create_ts desc limit "<<iOffset<<", "<<iLimit;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetUserFeedIdList failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			qwFeedIdList.insert(strtoul(mysql.GetRow(0), NULL, 10));
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "user_id="<<qwUserid<<" not exist");
	}

	return DAO_RET_OK;
}

int DaoTicketInfo::GetUserUploadTimeByFeedId(uint64_t qwUserid, uint64_t qwFeedId, uint32_t& iCount,lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss<<"select count(*) from t_ticket_info"
	   <<" where feed_id="<<qwFeedId
	   <<" and user_id="<<qwUserid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetUserUploadTimeByFeedId failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		iCount = atoi(mysql.GetRow(0));			
	}
	else
	{
		iCount = 0;
	}

	return DAO_RET_OK;
}

int DaoTicketInfo::GetContendListByFeedid(uint64_t qwFeedid, uint64_t qwBeginTicketid, uint32_t iLimit, std::vector<hoosho::msg::s::ContendInfo>& pbContendInfoList, lce::cgi::CMysql& mysql)
{
	if(0 == qwBeginTicketid)
		qwBeginTicketid = 0xFFFFFFFFFFFFFFFF;
		
	std::ostringstream oss;
	oss<<"select ticket_id, user_id, user_ip, feed_id, contend_id, create_ts from t_ticket_info"
	   <<" where feed_id="<<qwFeedid
	   <<" and ticket_id<"<<qwBeginTicketid
	   <<" and check_status="<<TICKET_CHECK_STATUS_PASS
	   <<" order by create_ts desc limit "<<iLimit;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetUserUploadTimeByFeedId failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::s::ContendInfo stContendInfo;
			ContentdInfoDB2PB(mysql, stContendInfo);
			pbContendInfoList.push_back(stContendInfo);
		}
	}

	return DAO_RET_OK;
}

int DaoTicketInfo::GetUserContendIdListByFeedid(uint64_t qwUserid, uint64_t qwFeedid, std::set<uint64_t>& qwContendidList, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss<<"select contend_id from t_ticket_info"
	   <<" where user_id="<<qwUserid
	   <<" and feed_id="<<qwFeedid
	   <<" and check_status="<<TICKET_CHECK_STATUS_PASS
	   <<" order by create_ts";
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetUserContendIdListByFeedid failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			qwContendidList.insert(strtoul(mysql.GetRow(0), NULL, 10));
		}
	}

	return DAO_RET_OK;
}

int DaoTicketInfo::GetTicketInfoListByUserid(uint64_t qwUserid, uint64_t qwBeginTicketid, uint32_t dwLimit, std::vector<hoosho::msg::s::TicketInfo>& pbTicketInfoList, lce::cgi::CMysql& mysql)
{
	if(0 == qwBeginTicketid)
		qwBeginTicketid = 0xFFFFFFFFFFFFFFFF;
		
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from t_ticket_info"
	   <<" where ticket_id<"<<qwBeginTicketid
	   <<" and user_id="<<qwUserid
	   <<" order by ticket_id desc limit "<<dwLimit;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetTicketInfoListByUserid failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::s::TicketInfo stTicketInfo;
			TicketInfoDB2PB(mysql, stTicketInfo);
			pbTicketInfoList.push_back(stTicketInfo);
		}
	}

	return DAO_RET_OK;
}



int DaoFeedInfo::GetUserFeedInfoList(const std::set<uint64_t>& qwFeedIdList, std::vector<hoosho::msg::s::FeedInfo>& pbFeedInfoList, lce::cgi::CMysql& mysql)
{
	if(qwFeedIdList.empty())
		return DAO_RET_OK;

	std::ostringstream oss;
	std::set<uint64_t>::iterator it = qwFeedIdList.begin();
	for(; it != qwFeedIdList.end(); it++)
	{
		uint64_t qwFeedId = *it;

		oss.str("");
		oss<<"select * from t_feed_info where feed_id="<<qwFeedId;
		LOG4CPLUS_DEBUG(logger, oss.str());

		if(!mysql.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "GetUserFeedInfoList failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
			return DAO_RET_FAIL;
		}

		if(mysql.GetRowCount())
		{
			while(mysql.Next())
			{
				::hoosho::msg::s::FeedInfo stFeedInfo;
				FeedInfoDB2PB(mysql, stFeedInfo);
				pbFeedInfoList.push_back(stFeedInfo);
			}
		}
	}

	return DAO_RET_OK;
}

int DaoFeedInfo::GetFeedInfoListByGoodId(uint64_t qwGoodid, uint64_t qwBeginFeedId, uint32_t iLimit, std::vector<hoosho::msg::s::FeedInfo>& pbFeedInfoList, lce::cgi::CMysql& mysql)
{
	if(0 == qwBeginFeedId)
		qwBeginFeedId = 0xFFFFFFFFFFFFFFFF;
		
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from t_feed_info where";
	if(qwGoodid)
	{
		oss << " good_id="<<qwGoodid << " and ";
	}
	oss<<" feed_id<"<<qwBeginFeedId
	   <<" limit "<<iLimit;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetFeedInfoListByGoodId failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::s::FeedInfo stFeedInfo;
			FeedInfoDB2PB(mysql, stFeedInfo);
			pbFeedInfoList.push_back(stFeedInfo);
		}
	}

	return DAO_RET_OK;
}

int DaoFeedInfo::CheckShowStatus(uint64_t qwFeedId, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select show_status from t_feed_info where feed_id="<<qwFeedId;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "CheckShowStatus failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		if(SHOW_STATUS_UNDO == atoi(mysql.GetRow(0)))
			return DAO_RET_UNSHOWED;			
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "feed_id="<<qwFeedId<<" not exist");
		return DAO_RET_NOT_EXIST;
	}
	   
	return DAO_RET_OK;
}

int DaoFeedInfo::UpdateShowStatus(uint64_t qwFeedId, uint32_t iShowStatus, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"update t_feed_info set show_status="<<iShowStatus
	   <<" where feed_id="<<qwFeedId;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "UpdateShowStatus failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoFeedInfo::GetAvailableUploadNumByFeedId(uint64_t qwFeedId, uint64_t& qwAvailableNum, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select total_join_num, approved_num, pending_num from t_feed_info where feed_id="<<qwFeedId;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetAvailableUploadNumByFeedId failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		uint64_t qwTotalJionNum = strtoul(mysql.GetRow(0), NULL, 10);
		uint64_t qwApprovedNum = strtoul(mysql.GetRow(1), NULL, 10);
		uint64_t qwPendingNum = strtoul(mysql.GetRow(2), NULL, 10);

		qwAvailableNum = qwTotalJionNum - qwApprovedNum - qwPendingNum;
	}	
	else
	{
		LOG4CPLUS_ERROR(logger, "feed_id="<<qwFeedId<<"not exist");
		return DAO_RET_NOT_EXIST;
	}

	return DAO_RET_OK;
}

int DaoFeedInfo::GetFeedInfoByFeedid(uint64_t qwFeedid, ::hoosho::msg::s::FeedInfo& stFeedInfo, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from t_feed_info where feed_id="<<qwFeedid;
	LOG4CPLUS_DEBUG(logger, oss.str());
	
	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetFeedInfoByFeedid failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		FeedInfoDB2PB(mysql, stFeedInfo);
	}	
	else
	{
		LOG4CPLUS_ERROR(logger, "feed_id="<<qwFeedid<<"not exist");
		return DAO_RET_NOT_EXIST;
	}	

	return DAO_RET_OK;
}

int DaoFeedInfo::UpdateFeedInfoByAddTicket(uint64_t qwFeedid, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"update t_feed_info set current_join_num=current_join_num+1, pending_num=pending_num+1 where feed_id="<<qwFeedid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "UpdateFeedInfoByAddTicket failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoFeedInfo::GetUserLuckFeedList(uint64_t qwUserid, uint64_t qwBeginFeedid, uint32_t dwLimit, std::vector<hoosho::msg::s::FeedInfo>& pbFeedInfoList, lce::cgi::CMysql& mysql)
{
	if(0 == qwBeginFeedid)
		qwBeginFeedid = 0xFFFFFFFFFFFFFFFF;
		
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from t_feed_info where feed_id<"<<qwBeginFeedid
	   <<" and luck_user_id="<<qwUserid
	   <<" order by create_ts desc limit "<<dwLimit;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetUserLuckFeedList failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::s::FeedInfo stFeedInfo;
			FeedInfoDB2PB(mysql, stFeedInfo);
			pbFeedInfoList.push_back(stFeedInfo);
		}
	}

	return DAO_RET_OK;
}


int DaoShowInfo::AddShowInfo(const ::hoosho::msg::s::ShowInfo& pbShowInfo, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"insert into t_show_info"
	   <<" set show_id="<<pbShowInfo.show_id()
	   <<", user_id="<<pbShowInfo.user_id()
	   <<", feed_id="<<pbShowInfo.feed_id()
	   <<", title='"<<sql_escape(pbShowInfo.title())<<"'"
	   <<", show_desc='"<<sql_escape(pbShowInfo.show_desc())<<"'"
	   <<", show_pics='"<<sql_escape(pbShowInfo.show_pics())<<"'"
	   <<", create_ts="<<pbShowInfo.create_ts()
	   <<" on duplicate key update"
	   <<" user_id="<<pbShowInfo.user_id()
	   <<", feed_id="<<pbShowInfo.feed_id()
	   <<", title='"<<sql_escape(pbShowInfo.title())<<"'"
	   <<", show_desc='"<<sql_escape(pbShowInfo.show_desc())<<"'"
	   <<", show_pics='"<<sql_escape(pbShowInfo.show_pics())<<"'"
	   <<", create_ts="<<pbShowInfo.create_ts();
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "AddShowInfo failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	//update t_feed_info field "show_status"
	DaoFeedInfo stDaoFeedInfo;	
	return stDaoFeedInfo.UpdateShowStatus(pbShowInfo.feed_id(), SHOW_STATUS_DONE,mysql);
}

int DaoShowInfo::GetShowInfoList(uint64_t qwUserid, uint64_t qwBeginShowId, uint32_t iLimit, std::vector<hoosho::msg::s::ShowInfo>& pbShowInfoList, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");	
	if(0 == qwBeginShowId)
	{
		qwBeginShowId = 0xFFFFFFFFFFFFFFFF;
	}
	
	if(0 == qwUserid)
	{
		oss<<"select * from t_show_info where show_id<"<<qwBeginShowId
		   <<" order by show_id desc limit "<<iLimit;
	}
	else
	{
		oss<<"select * from t_show_info where user_id="<<qwUserid
		   <<" and show_id<"<<qwBeginShowId
		   <<" order by show_id desc limit "<<iLimit;
	}
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetShowInfoList failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::s::ShowInfo stShowInfo;
			ShowInfoDB2PB(mysql, stShowInfo);
			pbShowInfoList.push_back(stShowInfo);
		}
	}

	return DAO_RET_OK;	
}

int DaoShowInfo::GetSHowDetailInfoByShowid(uint64_t qwShowid, ::hoosho::msg::s::ShowInfo& stShowInfo, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from t_show_info where show_id="<<qwShowid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetSHowDetailInfoByShowid failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		ShowInfoDB2PB(mysql, stShowInfo);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "show_id="<<qwShowid<<"not exist");
		return DAO_RET_NOT_EXIST;
	}

	return DAO_RET_OK;
}

int DaoCollectInfo::AddCollectInfo(const ::hoosho::msg::s::CollectInfo& pbCollectInfo, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");	
	oss<<"insert into t_collect_info"
	   <<" set user_id="<<pbCollectInfo.user_id()
	   <<", feed_id="<<pbCollectInfo.feed_id()
	   <<", create_ts="<<pbCollectInfo.create_ts()
	   <<" on duplicate key update"
	   <<" create_ts="<<pbCollectInfo.create_ts();
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "AddCollectInfo failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoCollectInfo::GetCollectInfoList(uint64_t qwUserid, uint64_t qwBeginTs, uint32_t iLimit, std::vector<hoosho::msg::s::CollectInfo>& pbCollectInfoList, lce::cgi::CMysql& mysql)
{
	if(0 == qwBeginTs)
		qwBeginTs = 0xFFFFFFFFFFFFFFFF;
		
	std::ostringstream oss;
	oss.str("");	
	oss<<"select * from t_collect_info where user_id="<<qwUserid
	   <<" and create_ts<"<<qwBeginTs
	   <<" order by create_ts desc limit "<<iLimit;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "AddCollectInfo failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::s::CollectInfo stCollectInfo;
			CollectInfoDB2PB(mysql, stCollectInfo);
			pbCollectInfoList.push_back(stCollectInfo);
		}
	}

	return DAO_RET_OK;	
}

int DaoCollectInfo::DelCollect(uint64_t qwUserid, uint64_t qwFeedid, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"delete from t_collect_info where user_id="<<qwUserid
	   <<" and feed_id="<<qwFeedid;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "DelCollect failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoCollectInfo::CheckUserCollect(uint64_t qwUserid, std::set<uint64_t>& qwFeedidList, std::vector<hoosho::msg::s::UserCollectFlag>& pbUserCollectFlagList, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	std::set<uint64_t>::iterator it = qwFeedidList.begin();
	for(; it != qwFeedidList.end(); it++)
	{
		uint64_t qwFeedid = *it;
		uint32_t dwFlag = 0;

		oss.str("");
		oss<<"select * from t_collect_info where user_id="<<qwUserid
		   <<" and feed_id="<<qwFeedid;
		LOG4CPLUS_DEBUG(logger, oss.str());

		if(!mysql.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "CheckUserCollect failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
			return DAO_RET_FAIL;
		}

		if(mysql.GetRowCount())
			dwFlag = 1;

		::hoosho::msg::s::UserCollectFlag stUserCollectFlag;
		stUserCollectFlag.set_feed_id(qwFeedid);
		stUserCollectFlag.set_flag(dwFlag);
		pbUserCollectFlagList.push_back(stUserCollectFlag);
	}

	return DAO_RET_OK;
}

int DaoBannerInfo::GetBannerInfoList(uint64_t qwBeginBannerid, uint32_t dwLimit, std::vector<hoosho::msg::s::BannerInfo>& pbBannerInfoList, lce::cgi::CMysql& mysql)
{
	if(0 == qwBeginBannerid)
		qwBeginBannerid = 0xFFFFFFFFFFFFFFFF;
		
	std::stringstream oss;
	oss.str("");
	oss<<"select * from t_banner_info where banner_id<"<<qwBeginBannerid
	   <<"order by banner_id desc limit "<<dwLimit;
	LOG4CPLUS_DEBUG(logger, oss.str());

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "GetBannerInfoList failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::s::BannerInfo stBannerInfo;
			BannerInfoDB2PB(mysql, stBannerInfo);
			pbBannerInfoList.push_back(stBannerInfo);
		}
	}

	return DAO_RET_OK;		
}


int DaoPicInfo::AddPic(uint64_t qwPicid, const std::string& strContent, uint64_t qwUserid, uint32_t iType, lce::cgi::CMysql& mysql)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"insert into t_pic_info"
	   <<" set pic_id="<<qwPicid
	   <<", content='"<<sql_escape(strContent)<<"'"
	   <<", user_id="<<qwUserid
	   <<", type="<<iType;

	if(!mysql.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "AddPic failed, sql="<<oss.str()<<", ErrMsg="<<mysql.GetErrMsg());
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoPicInfo::DownLoadAddPics(std::string& strPics, uint64_t qwUserid, uint32_t iType, lce::cgi::CMysql& mysql)
{
	//1.down pic from wx, insert into t_pic_info	
	std::vector<std::string> strPicList;
	lce::cgi::Split(strPics, "|", strPicList);

	//get pa access token
	std::string strBaseAccessToken = "";
	std::string strJSAPITiket = "";
	if(g_token_server_processor->get_pa_info(m_pa_appid, strBaseAccessToken, strJSAPITiket) < 0)
	{
		LOG4CPLUS_ERROR(logger, "DaoPicInfo::DownLoadAddPics failed, PAToken not found for strAppid="<<m_pa_appid );        
        return DAO_RET_FAIL;
	}

	std::string strMediaId = "";
	std::string strResource = "";	
	::common::wxapi::WXAPIDownloadResource stWXAPIDownloadResource;
	std::ostringstream ossPicidMd5;
	ossPicidMd5.str("");
	uint64_t qwPicId = 0;
	md5 _md5;
	for(size_t i = 0; i < strPicList.size(); i++)
	{
		strMediaId = strPicList[i];
		strResource = "";			

		//down resource
		int iRet = stWXAPIDownloadResource.DownloadResource(strBaseAccessToken, strMediaId, strResource);
		if(iRet < 0)
		{			
	        return DAO_RET_FAIL;
		}

		qwPicId = _md5.hash64(strMediaId.c_str(), strMediaId.size());		
		if(DAO_RET_OK != AddPic(qwPicId, strResource, qwUserid, iType, mysql))
		{			
			return DAO_RET_FAIL;
		}

		if(i == 0)
		{
			ossPicidMd5 << qwPicId;
		}
		else
		{
			ossPicidMd5 << "|" << qwPicId;
		}
	}

	strPics = ossPicidMd5.str();
	return DAO_RET_OK;
}


