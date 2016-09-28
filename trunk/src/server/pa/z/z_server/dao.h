#ifndef _Z_SERVER_DAO_H_
#define _Z_SERVER_DAO_H_

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

#define MIN_OPENID(stra, strb) stra > strb ? strb : stra
#define MAX_OPENID(stra, strb) stra > strb ? stra : strb

enum DAO_RET_CODE
{
	DAO_RET_OK = 0,
	DAO_RET_NOT_EXIST = 1,
	DAO_RET_FAIL = 2,
	DAO_RET_INVALID = 3,
	DAO_RET_LOGIC_TOO_MANY = 4,
	DAO_RET_LOGIC_REPEATED = 5,
};

class DaoOrderInfo
{
public:
	DaoOrderInfo(const std::string& strTableName)
		:m_table_name(strTableName)
	{

	}

public:
	int AddNew(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int Payed(uint64_t qwOrderId, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int FetchAccepted(uint64_t qwOrderId
							, const std::string& strMasterOpenid
							, const std::string& strSlaveOpenid
							, lce::cgi::CMysql& mysql
							, std::string& strErrMsg);
	int FetchCanceled(uint64_t qwOrderId
							, const std::string& strFentchOpenid
							, lce::cgi::CMysql& mysql
							, std::string& strErrMsg);
	int FetchTry(uint64_t qwOrderId
							, const std::string& strFentchOpenid
							, lce::cgi::CMysql& mysql
							, std::string& strErrMsg);
	int Finished(uint64_t qwOrderId, const std::string& strMasterOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int Canceled(uint64_t qwOrderId, const std::string& strMasterOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int Commented(uint64_t qwOrderId, const std::string& strCommentOpenid, std::string& strAnoOpenid, uint32_t dwStar, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetByPrimaryKey(uint64_t qwOrderId, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	
	int DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);

	uint64_t CreateRandomRewardFeeForNewUser(const std::string& strOpenid, const std::string& strTableUserInfoName, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int FinishRewardFeeForNewUser(const std::string& strOpenid, 
											const std::string& strTableUserInfoName, 
											const std::string& m_db_table_user_cash_name, 
											const std::string& m_db_table_cash_flow_name, 
											lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::string m_table_name;
	::hoosho::msg::z::OrderInfo m_order_info;
};

class DaoOrderMediaInfo
{
public:
	DaoOrderMediaInfo(const std::string& strTableName)
		:m_table_name(strTableName)
	{

	}

public:
	int AddOrUpdate(uint64_t qwOrderId
						, uint64_t qwMediaType
						, const std::string& strMediaContent
						, lce::cgi::CMysql& mysql
						, std::string& strErrMsg);
public:
	std::string m_table_name;
	uint64_t m_orderId;
	uint64_t m_media_type;
	std::string m_media_content;
};

class DaoOrderFetchState
{
public:
	DaoOrderFetchState(const std::string& strTableName)
		:m_table_name(strTableName)
	{

	}

public:
	int GetByPrimaryKey(uint64_t qwOrderId, const std::string& strFetchOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int AddNewWaiting(uint64_t qwOrderId, const std::string& strFetchOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int UpdateToAccepted(uint64_t qwOrderId, const std::string& strFetchOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int DeleteByPrimaryKey(uint64_t qwOrderId, const std::string& strFetchOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg);
public:
	std::string m_table_name;
	hoosho::msg::z::OrderOpenidFetchState m_order_openid_fetch_state;
}; 

class DaoOrderFetchStateList
{
public:
	DaoOrderFetchStateList(const std::string& strTableName)
		:m_table_name(strTableName)
	{

	}

public:
	int GetFentchedOrderList(const std::string& strOpenid
											, uint64_t qwBeforeOrderId
											, uint32_t dwNeedNum
											, lce::cgi::CMysql& mysql
											, std::string& strErrMsg);
	
public:
	std::string m_table_name;
	std::vector<hoosho::msg::z::OrderOpenidFetchState> m_vec_order_fetch_state_list;
}; 



class DaoOrderInfoList: public DaoOrderInfo
{
public:
	DaoOrderInfoList(const std::string& strTableName)
			: DaoOrderInfo(strTableName)
	{
	}

	int GetCreatedOrderList(const std::string& strOpenid
										, uint64_t qwBeforeOrderId
										, uint32_t dwNeedNum
										, lce::cgi::CMysql& mysql
										, std::string& strErrMsg);
	
	std::vector<::hoosho::msg::z::OrderInfo>  m_order_info_list;
};

class DaoOrderReward
{
public:
	DaoOrderReward(const std::string& strTableName):m_table_name(strTableName)
	{

	}

	int AddNew(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int Payed(const std::string& strOutTradeNo, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetByPrimaryKey(const std::string& strOutTradeNo, lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::string m_table_name;

	std::string m_out_trade_no;
	uint64_t m_orderid;
	std::string m_openid_from;
	std::string m_openid_to;
	uint64_t m_reward_ts;
	uint64_t m_amount;
	uint16_t m_pay_status;
	uint64_t m_pay_ts;
};

class DaoUserInfo
{
public:
	DaoUserInfo(const std::string& strTableName)
		: m_table_name(strTableName)
	{
	}

	int GetByPrimaryKey(const std::string& strOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int UpdateUserInfo(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int	IncreaseUserScore(const std::string& strOpenid, int iScore, lce::cgi::CMysql& mysql, std::string& strErrMsg); 
	int GetRankingList(uint32_t iLen, uint32_t iPage, std::vector<hoosho::msg::z::UserInfo>& vecPBUserInfoList, lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::string m_table_name;

	hoosho::msg::z::UserInfo m_user_info;
};

class DaoUserTrust
{
public:
	DaoUserTrust(const std::string& strTableName)
			:m_table_name(strTableName)
	{

	}

public:
	int SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::string m_table_name;
	std::string m_openid_from;
	std::string m_openid_to;

};

class DaoUserExtraInfo
{
public:
	DaoUserExtraInfo(const std::string& strTableUserStarName, const std::string& strTableOrderInfoName)
			: m_table_user_star_name(strTableUserStarName)
			, m_table_order_info_name(strTableOrderInfoName)
			, m_openid("")
			, m_order_id(0)
			, m_star(0)
			, m_create_ts(0)
	{
	}

	int AddNewUserStar(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int CountAverageStar(const std::string& strOpenid, float& fAvgStar, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int CountUserLevel(const std::string& strOpenid, uint32_t& iLevel, lce::cgi::CMysql& mysql, std::string& strErrMsg);	
	int GetUserExtraInfo(const std::string& strOpenid, ::hoosho::msg::z::UserExtraInfo& stUserExtraInfo, lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::string m_table_user_star_name;
	std::string m_table_order_info_name;
	std::string m_openid;  //pri
	uint64_t    m_order_id; //pri
	uint16_t    m_star;
	uint16_t    m_create_ts;
};

class DaoUserActivityInfo
{
public:
	DaoUserActivityInfo(const std::string& strTableName):
			m_table_name(strTableName)
	{
	}

public:
	int GetByPrimaryKey(const std::string& strInfoId, const uint32_t dwActivityId, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int AddNew(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int Update(lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::string m_table_name;
	hoosho::msg::z::UserActivityInfo m_user_activity_info;
};

class DAOWXOuttrade
{
public:
	DAOWXOuttrade(const std::string& strTableName)
			:m_table_name(strTableName)
			, m_total_fee(0)
			, m_pay_state(0)
	{

	}

public:
	int AddNew(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int UpdatePayState(const std::string& strOpenid, const std::string& strOuttradeNO, uint16_t wPayState, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetByPrimaryKey(const std::string& strOpenid, const std::string& strOuttradeNO, lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::string m_table_name;
	
	std::string m_openid;  //pri
	std::string m_outtrade_no; //pri
	std::string m_prepay_id;
	std::string m_body;
	std::string m_detail;
	std::string m_attach;
	std::string m_goods_tag;
	std::string m_spbill_create_ip;
	std::string m_time_start;
	std::string m_time_expire;
	std::string m_trade_type;
	std::string m_product_id;
	std::string m_limit_pay;
	std::string m_appid;
	std::string m_mch_id;
	std::string m_device_info;
	std::string m_fee_type;
	uint64_t m_total_fee;
	uint16_t m_pay_state;
	uint16_t m_pay_reason;
};

class DAOCashFlow
{
public:
	DAOCashFlow(const std::string& strTableName)
		:m_table_name(strTableName)
		, m_openid("")
		, m_table_id(0)
		, m_outrade_no("")
		, m_add_or_reduce(0)
		, m_amount(0)
		, m_flow_type(0)
		, m_balance(0)
		, m_create_ts(0)
	{

	}

public:
	int AddNew(lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::string m_table_name;
	std::string m_openid;
	uint64_t m_table_id;
	std::string m_outrade_no;
	uint16_t m_add_or_reduce; //1.add, 2, reduce
	uint16_t m_amount;
	uint16_t m_flow_type;
	uint64_t m_balance;
	uint64_t m_create_ts;
};

class DaoCashFlowList : public DAOCashFlow
{
public:
	DaoCashFlowList(const std::string& strTableName)
		: DAOCashFlow(strTableName)
	{
		
	}
	int GetByTsOrder(const std::string& strOpenid, uint64_t qwBeforeTS, uint32_t dwNeedNum, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetByTsOrder(const std::string& strOpenid, uint64_t qwBeforeTS, uint32_t dwNeedNum
						, lce::cgi::CMysql& mysql, std::string& strErrMsg, std::vector<hoosho::msg::z::CashFlowInfo>& vecPBList);
public:
	std::vector<DAOCashFlow> m_cash_flow_list;
};

class DAOUserCash
{
public:
	DAOUserCash(const std::string& strUserCashTableName, const std::string& strCashFlowTableName)
			: m_table_name_user_cash(strUserCashTableName)
			, m_table_name_cash_flow(strCashFlowTableName)
			, m_balance(0)
			, m_pending(0)
	{
		
	}

public:
	int GetByPrimaryKey(const std::string& strOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int AddBalance(const std::string& strOpenid, uint64_t qwAmount, int iReason, lce::cgi::CMysql& mysql, std::string& strErrMsg, const std::string& strOuttradeNO = "");
	int ReduceBalance(const std::string& strOpenid, uint64_t qwAmount, int iReason, lce::cgi::CMysql& mysql, std::string& strErrMsg, const std::string& strOutTradeNo = "");
	int ReducePending(const std::string& strOpenid, uint64_t qwAmount, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	
public:
	std::string m_table_name_user_cash;
	std::string m_table_name_cash_flow;
	std::string m_openid;
	uint64_t m_balance;
	uint64_t m_pending;
};

class DaoOutcome
{
public:
	DaoOutcome(const std::string& strTableNameOutCome, const std::string& strTableNameUserCash, const std::string& strTableNameCashFlow)
		: m_table_name_outcome(strTableNameOutCome)
		, m_table_name_usercash(strTableNameUserCash)
		, m_table_name_cashflow(strTableNameCashFlow)
		, m_table_id(0)
		, m_openid("")
		, m_amount(0)
		, m_state(0)
		, m_create_ts(0)
	{
	}

public:
	int GetByPrimaryKey(uint64_t qwtableId, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int Commit(const std::string& strOpenid, uint64_t qwAmount, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int Finish(uint64_t qwTableId, bool succ, lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::string m_table_name_outcome;
	std::string m_table_name_usercash;
	std::string m_table_name_cashflow;
	uint64_t m_table_id;
	std::string m_openid;
	uint64_t m_amount;
	uint32_t m_state;
	uint64_t m_create_ts;
};

class DaoOutcomeList: public DaoOutcome
{
public:
	DaoOutcomeList(const std::string& strTableNameOutCome, const std::string& strTableNameUserCash, const std::string& strTableNameCashFlow)
		: DaoOutcome(strTableNameOutCome, strTableNameUserCash, strTableNameCashFlow)
	{
	}

public:
	int GetByStateWithTsOrder(const std::string& strOpenid
										, uint32_t dwState
										, uint64_t qwBeforeTS
										, uint32_t dwNeedNum
										, lce::cgi::CMysql& mysql
										, std::string& strErrMsg);

	int GetByStateWithTsOrder(const std::string& strOpenid
										, uint32_t dwState
										, uint64_t qwBeforeTS
										, uint32_t dwNeedNum
										, lce::cgi::CMysql& mysql
										, std::string& strErrMsg
										, std::vector<hoosho::msg::z::OutcomeReqInfo>& vecPBList);

public:
	std::vector<DaoOutcome> m_outcome_list;
};

class DaoOrderFavorInfo
{
public:
	DaoOrderFavorInfo(const std::string& strTableName): m_table_name(strTableName)
	{
	}

public:
	int AddFavor(const std::string& strOpenid, uint64_t qwOrderid, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int DelFavor(const std::string& strOpenid, uint64_t qwOrderid, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetByPrimaryKey(const std::string& strOpenid, uint64_t qwOrderid, hoosho::msg::z::OrderFavorInfo &stFavorInfo, lce::cgi::CMysql& mysql, std::string& strErrMsg);


public:
	std::string m_table_name;
};

class DaoOrderFavorInfoList
{
public:
	DaoOrderFavorInfoList(const std::string& strTableName): m_table_name(strTableName)
	{
	}

public:
	int GetFavorList(uint64_t qwOrderid, uint64_t qwBeginTs, uint32_t dwNeedNum, lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::vector<hoosho::msg::z::OrderFavorInfo> m_favor_list;
	std::string m_table_name;
};

class DaoOrderFollowInfo
{
public:
	DaoOrderFollowInfo(const std::string& strTableNameFollowInfo, const std::string& strTableNameCommentIndex, const std::string& strTableNameReplyIndex)
			: m_table_name_follow_info(strTableNameFollowInfo), m_table_name_comment_index(strTableNameCommentIndex), m_table_name_reply_index(strTableNameReplyIndex)
	{
	}

public:
	int AddComment(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int AddReply(lce::cgi::CMysql& mysql, std::string& strErrMsg, std::string& strOriginCommentOpenid);
	int AddFollow(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int DelFollow(const std::string& strOpenid, uint64_t qwFollowId, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetByPrimaryKey(uint64_t qwFollowId, lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::string m_table_name_follow_info;
	std::string m_table_name_comment_index;
	std::string m_table_name_reply_index;
	hoosho::msg::z::OrderFollowInfo m_follow_info;
};

class DaoOrderCommentInfoList: public DaoOrderFollowInfo
{
public:
	DaoOrderCommentInfoList(const std::string& strTableNameFollowInfo, const std::string& strTableNameCommentIndex, const std::string& strTableNameReplyIndex)
			: DaoOrderFollowInfo(strTableNameFollowInfo, strTableNameCommentIndex, strTableNameReplyIndex)
	{
	}

public:
	int GetCommentList(uint64_t qwOrderid, uint64_t qwBeginCommentid, uint32_t dwNeedNum, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetCommentDetail(std::vector<uint64_t> vecCommentid, lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	std::vector<hoosho::msg::z::OrderCommentInfo> m_comment_list;
};

class DaoMsg
{
public:
	DaoMsg()
	{
	}

public:
	int AddNewMsg(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetMsgList(uint64_t qwSessionId, uint64_t qwBeginMsgId, uint32_t iLen, int iDelStatuss, std::vector<hoosho::msg::z::MsgContent>& vecMsgContent, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetMsgDetail(std::vector<uint64_t>& vecMsgId, std::vector<hoosho::msg::z::MsgContent>& vecMsgContent, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int ClearUserSessionNewMsgStatus(const std::string& strOpenidFrom, const std::string& strOpenidTo, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetSessionList(const std::string& strOpenid, uint64_t qwBeginTs, uint32_t iLen, 
						std::set<std::string>& strOpenidSet,
						std::vector<hoosho::msg::z::Session>& vecSession, 
						lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int DelSession(const std::string& strOpenidFrom, const std::string& strOpenidTo, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int DelMsg(uint64_t qwSessionId, int iDelStatus, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetNewMsgStatus(const std::string& strOpenid, uint32_t& iNewMsgStatus, lce::cgi::CMysql& mysql, std::string& strErrMsg);

public:
	::hoosho::msg::z::MsgContent m_msg_content;	
};


class DaoRedPoint
{
public:
	DaoRedPoint()
	{
	}
	
public:
	int Add(const std::string& strOpenid, uint32_t type, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int Get(const std::string& strOpenid, std::vector<hoosho::msg::z::RedPointInfo>& vecRedPointInfo, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int ClearValue(const std::string& strOpenid, uint32_t type, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int Reduce(const std::string& strOpenid, uint32_t type, lce::cgi::CMysql& mysql, std::string& strErrMsg);
};

class DaoNoticeInfo
{
public:
	DaoNoticeInfo()
	{
	}
public:
	int Add(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int Del(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int DelNoticeFavorite(uint64_t qwOrderId, std::string& strOpenidFavor, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int DelNoticeFollow(uint64_t qwOrderId, uint64_t qwFollowId, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int ClearStatus(const std::string& strOpenid, uint32_t type, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetByType(const std::string& strOpenid, uint32_t type, uint64_t qwBeginTs, uint32_t iLen, std::vector<hoosho::msg::z::NoticeInfo>& vecNoticeInfo, lce::cgi::CMysql& mysql, std::string& strErrMsg);	

public:
	::hoosho::msg::z::NoticeInfo m_notice_info;
};

class DaoLotteryInfo
{
public:
	DaoLotteryInfo(){}
public:
	int AddNewLotteryInfo(const std::string& strOpenid, uint32_t iPrize, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetUserAvailableLotteryTime(const std::string& strOpenid, uint32_t& times, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetUserLotteryRecordList(const std::string& strOpenid, uint64_t qwBeginTs, uint32_t iLen, std::vector<hoosho::msg::z::LotteryInfo>& vecLotteryInfoList, lce::cgi::CMysql& mysql, std::string& strErrMsg);
};

#endif
