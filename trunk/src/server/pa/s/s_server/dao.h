#ifndef _DAO_H_
#define _DAO_H_

#include "msg.pb.h"
#include <vector>
#include <cgi/cgi_mysql.h>
#include "global_var.h"

#define SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg) \
	if(!mysql.Query(oss.str())) \
	{ \
		strErrMsg = "sql query failed, sql = "; \
		strErrMsg += oss.str(); \
		strErrMsg += ", errmsg = "; \
		strErrMsg += mysql.GetErrMsg(); \
		return DAO_RET_FAIL; \
	}

enum DAO_RET_CODE
{
	DAO_RET_OK = 0,
	DAO_RET_NOT_EXIST = 1,
	DAO_RET_FAIL = 2,
	DAO_RET_INVALID = 3,
	DAO_RET_LOGIC_TOO_MANY = 4,
	DAO_RET_LOGIC_REPEATED = 5,
	DAO_RET_UNSHOWED = 6,
};

class Dao
{
public:
	Dao(){};
	~Dao(){};
public:
	DECL_LOGGER(logger);
};

class DaoUserInfo: public Dao
{
public:
	DaoUserInfo(){};
	~DaoUserInfo(){};

public:
	int GetUserInfoByPrimaryKey(uint64_t qwUserid, lce::cgi::CMysql& mysql);
	int GetUserInfoList(const std::set<uint64_t> qwUserIdSet, std::vector<hoosho::msg::s::UserInfo>& pbUserInfoList, lce::cgi::CMysql& mysql);
	int SetUserPhone(uint64_t qwUserid, const std::string& strPhone, lce::cgi::CMysql& mysql);	
	int UpdateUserSelfDesc(uint64_t qwUserid, const std::string& strDesc, lce::cgi::CMysql& mysql);
	int UpdateUserDefaultRecvAddr(uint64_t qwUserid, uint32_t iAddrNum, lce::cgi::CMysql& mysql);

public:
	::hoosho::msg::s::UserInfo m_user_info;
};

class DaoUserIdentityInfo: public Dao
{
public:
	DaoUserIdentityInfo(){}
	~DaoUserIdentityInfo(){}

public:
	int UpdateUserIdentityInfo(uint64_t qwUserid, const ::hoosho::msg::s::UserIdentityInfo& pbUserIdentityInfo, lce::cgi::CMysql& mysql);
	int GetUserIdentityInfo(uint64_t qwUserid, ::hoosho::msg::s::UserIdentityInfo& pbUserIdentityInfo, lce::cgi::CMysql& mysql);
};

class DaoUserRecvAddrInfo: public Dao
{
public:
	DaoUserRecvAddrInfo(){};
	~DaoUserRecvAddrInfo(){};
	
public:
	int GetUserRecvAddrInfoCount(uint64_t qwUserid, uint32_t& iCount, lce::cgi::CMysql& mysql);
	int AddUserRecvAddrInfo(uint64_t qwUserid, const ::hoosho::msg::s::UserRecvAddrInfo& pbUserRecvAddrInfo, lce::cgi::CMysql& mysql);
	int DelUserRecvAddrInfo(uint64_t qwUserid, uint32_t iAddrNum, lce::cgi::CMysql& mysql);
	int UpdateUserRecvAddrInfo(uint64_t qwUserid, const ::hoosho::msg::s::UserRecvAddrInfo& pbUserRecvAddrInfo, lce::cgi::CMysql& mysql);
	int GetUserRecvAddrInfoList(uint64_t qwUserid, std::vector<hoosho::msg::s::UserRecvAddrInfo>& pbUserRecvAddrInfoList, lce::cgi::CMysql& mysql);
};

class DaoGoodType: public Dao
{
public:
	DaoGoodType(){}
	~DaoGoodType(){}
	
public:
	int GetGoodTypeList(std::vector<hoosho::msg::s::GoodType>& pbGoodTypeList, lce::cgi::CMysql& mysql);
};

class DaoGoodInfo: public Dao
{
public:
	DaoGoodInfo(){}
	~DaoGoodInfo(){}

public:
	int GetGoodInfoList(std::vector<hoosho::msg::s::GoodInfo>& pbGoodInfoList, lce::cgi::CMysql& mysql);
	int GetGoodInfoListByGoodid(const std::set<uint64_t>& qwGoodidList, std::vector<hoosho::msg::s::GoodInfo>& pbGoodInfoList, lce::cgi::CMysql& mysql);
};

class DaoTicketInfo: public Dao
{
public:
	DaoTicketInfo(){}
	~DaoTicketInfo(){}

public:
	int AddTicket(const ::hoosho::msg::s::TicketInfo& pbTicketInfo, lce::cgi::CMysql& mysql);
	int GetUserFeedIdList(uint64_t qwUserid, uint32_t iOffset, uint32_t iLimit, std::set<uint64_t>& qwFeedIdList, lce::cgi::CMysql& mysql);
	int GetUserUploadTimeByFeedId(uint64_t qwUserid, uint64_t qwFeedId, uint32_t& iCount, lce::cgi::CMysql& mysql);
	int GetContendListByFeedid(uint64_t qwFeedid, uint64_t qwBeginTicketid, uint32_t iLimit, std::vector<hoosho::msg::s::ContendInfo>& pbContendInfoList, lce::cgi::CMysql& mysql);
	int GetUserContendIdListByFeedid(uint64_t qwUserid, uint64_t qwFeedid, std::set<uint64_t>& qwContendidList, lce::cgi::CMysql& mysql);
	int GetTicketInfoListByUserid(uint64_t qwUserid, uint64_t qwBeginTicketid, uint32_t dwLimit, std::vector<hoosho::msg::s::TicketInfo>& pbTicketInfoList, lce::cgi::CMysql& mysql);
};

class DaoFeedInfo: public Dao
{
public:
	DaoFeedInfo(){}
	~DaoFeedInfo(){}

public:
	int GetUserFeedInfoList(const std::set<uint64_t>& qwFeedIdList, std::vector<hoosho::msg::s::FeedInfo>& pbFeedInfoList, lce::cgi::CMysql& mysql);
	int GetFeedInfoListByGoodId(uint64_t qwGoodid, uint64_t qwBeginFeedId, uint32_t iLimit, std::vector<hoosho::msg::s::FeedInfo>& pbFeedInfoList, lce::cgi::CMysql& mysql);
	int CheckShowStatus(uint64_t qwFeedId, lce::cgi::CMysql& mysql);
	int UpdateShowStatus(uint64_t qwFeedId, uint32_t iShowStatus, lce::cgi::CMysql& mysql);
	int GetAvailableUploadNumByFeedId(uint64_t qwFeedId, uint64_t& qwAvailableNum, lce::cgi::CMysql& mysql);
	int GetFeedInfoByFeedid(uint64_t qwFeedid, ::hoosho::msg::s::FeedInfo& stFeedInfo, lce::cgi::CMysql& mysql);
	int UpdateFeedInfoByAddTicket(uint64_t qwFeedid, lce::cgi::CMysql& mysql);
	int GetUserLuckFeedList(uint64_t qwUserid, uint64_t qwBeginFeedid, uint32_t dwLimit, std::vector<hoosho::msg::s::FeedInfo>& pbFeedInfoList, lce::cgi::CMysql& mysql);
};

class DaoShowInfo: public Dao
{
public:
	DaoShowInfo(){}
	~DaoShowInfo(){}

public:
	int AddShowInfo(const ::hoosho::msg::s::ShowInfo& pbShowInfo, lce::cgi::CMysql& mysql);
	int GetShowInfoList(uint64_t qwUserid, uint64_t qwBeginShowId, uint32_t iLimit, std::vector<hoosho::msg::s::ShowInfo>& pbShowInfoList, lce::cgi::CMysql& mysql);	
	int GetSHowDetailInfoByShowid(uint64_t qwShowid, ::hoosho::msg::s::ShowInfo& stShowInfo, lce::cgi::CMysql& mysql);
};

class DaoCollectInfo: public Dao
{
public:
	DaoCollectInfo(){}
	~DaoCollectInfo(){}

public:
	int AddCollectInfo(const ::hoosho::msg::s::CollectInfo& pbCollectInfo, lce::cgi::CMysql& mysql);
	int GetCollectInfoList(uint64_t qwUserid, uint64_t qwBeginTs, uint32_t iLimit, std::vector<hoosho::msg::s::CollectInfo>& pbCollectInfoList, lce::cgi::CMysql& mysql);
	int DelCollect(uint64_t qwUserid, uint64_t qwFeedid, lce::cgi::CMysql& mysql);
	int CheckUserCollect(uint64_t qwUserid, std::set<uint64_t>& qwFeedidList, std::vector<hoosho::msg::s::UserCollectFlag>& pbUserCollectFlagList, lce::cgi::CMysql& mysql);

};

class DaoBannerInfo: public Dao
{
public:
	DaoBannerInfo(){}
	~DaoBannerInfo(){}
public:
	int GetBannerInfoList(uint64_t qwBeginBannerid, uint32_t dwLimit, std::vector<hoosho::msg::s::BannerInfo>& pbBannerInfoList, lce::cgi::CMysql& mysql);
};

class DaoPicInfo: public Dao
{
public:
	DaoPicInfo(std::string strPAAppid)
	{
		m_pa_appid = strPAAppid;
	}
	~DaoPicInfo(){}
public:
	int AddPic(uint64_t qwPicid, const std::string& strContent, uint64_t qwUserid, uint32_t iType, lce::cgi::CMysql& mysql);
	int DownLoadAddPics(std::string& strPics, uint64_t qwUserid, uint32_t iType, lce::cgi::CMysql& mysql);
private:
	std::string m_pa_appid;
};

#endif
