#include "dao.h"
#include "common_util.h"
#include <sstream>
#include <stdlib.h>
#include "jsoncpp/json.h"

static void user_info_db_2_pb(lce::cgi::CMysql& mysql, hoosho::msg::z::UserInfo& userInfo)
{
	userInfo.set_openid(mysql.GetRow(0));
	userInfo.set_nickname(mysql.GetRow(1));
	userInfo.set_sex(strtoul(mysql.GetRow(2), NULL, 10));
	userInfo.set_headimgurl(mysql.GetRow(3));
	userInfo.set_self_desc(mysql.GetRow(4));
	userInfo.set_phone(mysql.GetRow(5));
	userInfo.set_user_type(atoi(mysql.GetRow(6)));	
	userInfo.set_user_flag(strtoul(mysql.GetRow(7), NULL, 10));
	userInfo.set_user_score(strtoul(mysql.GetRow(8), NULL, 10));

	/*
	mysql> desc t_user_info;	
	+------------+---------------------+------+-----+---------+-------+
	| Field 	 | Type 			   | Null | Key | Default | Extra |
	+------------+---------------------+------+-----+---------+-------+
	| openid	 | char(128)		   | NO   | PRI |		  | 	  |
	| nickname	 | tinyblob 		   | NO   | 	| NULL	  | 	  |
	| sex		 | tinyint(3) unsigned | NO   | 	| 0 	  | 	  |
	| headimgurl | blob 			   | NO   | 	| NULL	  | 	  |
	| self_desc  | blob 			   | NO   | 	| NULL	  | 	  |
	| phone 	 | char(32) 		   | NO   | 	|		  | 	  |
	| user_type  | tinyint(3) unsigned | NO   |     | 1       |       |
	| user_flag  | bigint(20) unsigned | NO   |     | 0       |       |
	+------------+---------------------+------+-----+---------+-------+	
	*/
}

static void order_waiting_user_str_2_pb(const std::string& str,  hoosho::msg::z::OrderInfo& orderInfo)
{
	std::vector<std::string> vecUnits;
	std::vector<std::string> vecUsers;
	lce::util::StringOP::Split(str, ",", vecUnits);
	orderInfo.clear_order_waiting_user_list();
	for(size_t i=0; i!=vecUnits.size(); ++i)
	{
		vecUsers.clear();
		lce::util::StringOP::Split(vecUnits[i], "|", vecUsers);
		if(DB_ORDER_INFO_OPENID_WAITING_COMPONENT_LEN == vecUsers.size())
		{
			hoosho::msg::z::OrderWaitingUser* pWaitingUser = orderInfo.add_order_waiting_user_list();
			pWaitingUser->set_openid(vecUsers[0]);
			pWaitingUser->set_create_ts(strtoul(vecUsers[1].c_str(), NULL, 10));
		}
	}
}

static std::string order_waiting_user_pb_2_str(const hoosho::msg::z::OrderInfo& orderInfo)
{
	std::ostringstream oss;
	oss.str("");
	for(int i=0; i!=orderInfo.order_waiting_user_list_size(); ++i)
	{
		const hoosho::msg::z::OrderWaitingUser& unit = orderInfo.order_waiting_user_list(i);
		if(i != 0)
		{
			oss<<",";
		}

		oss<<unit.openid()<<"|"<<unit.create_ts();
	}

	return oss.str();
}

static bool order_waiting_user_exists(const hoosho::msg::z::OrderInfo& orderInfo
											, const std::string& strFentchOpenid)
{
	
	for(int i=0; i!=orderInfo.order_waiting_user_list_size(); ++i)
	{
		if(strFentchOpenid == orderInfo.order_waiting_user_list(i).openid())
		{
			return true;
		}
	}

	return false;
}

static void order_waiting_user_del(hoosho::msg::z::OrderInfo& orderInfo
											, const std::string& strFentchOpenid)
{
	std::vector<hoosho::msg::z::OrderWaitingUser> vecTmp;
	for(int i=0; i!=orderInfo.order_waiting_user_list_size(); ++i)
	{
		if(strFentchOpenid != orderInfo.order_waiting_user_list(i).openid())
		{
			vecTmp.push_back(orderInfo.order_waiting_user_list(i));
		}
	}

	if((size_t)orderInfo.order_waiting_user_list_size() != vecTmp.size())
	{
		orderInfo.mutable_order_waiting_user_list()->Clear();
		for(size_t i=0; i!=vecTmp.size(); ++i)
		{
			orderInfo.add_order_waiting_user_list()->CopyFrom(vecTmp[i]);
		}
	}
}



static void order_info_db_2_pb(lce::cgi::CMysql& mysql, hoosho::msg::z::OrderInfo& orderInfo)
{
	orderInfo.set_order_id(strtoul(mysql.GetRow(0), NULL, 10));
	orderInfo.set_openid_master(mysql.GetRow(1));
	orderInfo.set_openid_slave(mysql.GetRow(2));
	orderInfo.set_order_visible(strtoul(mysql.GetRow(3), NULL, 10));
	orderInfo.set_order_status(strtoul(mysql.GetRow(4), NULL, 10));
	orderInfo.set_pay_status(strtoul(mysql.GetRow(5), NULL, 10));
	orderInfo.set_addr_to_longitude(strtoul(mysql.GetRow(6), NULL, 10));
	orderInfo.set_addr_to_latitude(strtoul(mysql.GetRow(7), NULL, 10));
	orderInfo.set_addr_to(mysql.GetRow(8));
	orderInfo.set_addr_to_detail(mysql.GetRow(9));
	orderInfo.set_order_desc(mysql.GetRow(10));
	orderInfo.set_order_receiver_name(mysql.GetRow(11));
	orderInfo.set_order_receiver_phone(mysql.GetRow(12));
	orderInfo.set_price(strtoul(mysql.GetRow(13), NULL, 10));
	orderInfo.set_expect_from_ts(strtoul(mysql.GetRow(14), NULL, 10));
	orderInfo.set_expect_to_ts(strtoul(mysql.GetRow(15), NULL, 10));
	orderInfo.set_pay_ts(strtoul(mysql.GetRow(16), NULL, 10));
	orderInfo.set_create_ts(strtoul(mysql.GetRow(17), NULL, 10));
	orderInfo.set_finish_ts(strtoul(mysql.GetRow(18), NULL, 10));
	orderInfo.set_fetch_ts(strtoul(mysql.GetRow(19), NULL, 10));
	orderInfo.set_cancel_ts(strtoul(mysql.GetRow(20), NULL, 10));
	orderInfo.set_star_from_master(strtoul(mysql.GetRow(21), NULL, 10));
	orderInfo.set_star_from_slave(strtoul(mysql.GetRow(22), NULL, 10));
	order_waiting_user_str_2_pb(mysql.GetRow(23), orderInfo);
	orderInfo.set_extra_data(mysql.GetRow(24));
	orderInfo.set_order_type(strtoul(mysql.GetRow(25), NULL, 10));
	orderInfo.set_media_type(strtoul(mysql.GetRow(26), NULL, 10));
}

static void order_openid_fetch_state_db_2_pb(lce::cgi::CMysql& mysql, hoosho::msg::z::OrderOpenidFetchState& pb)
{
	pb.set_order_id(strtoul(mysql.GetRow(0), NULL, 10));
	pb.set_fetch_openid(mysql.GetRow(1));
	pb.set_fetch_state(atoi(mysql.GetRow(2)));
	pb.set_fetch_ts(strtoul(mysql.GetRow(3), NULL, 10));
	pb.set_accept_ts(strtoul(mysql.GetRow(4), NULL, 10));
}


static void cash_flow_info_mem_2_pb(const DAOCashFlow& src, hoosho::msg::z::CashFlowInfo& cashFlowInfo)
{
	cashFlowInfo.set_table_id(src.m_table_id);
	cashFlowInfo.set_openid(src.m_openid);
	cashFlowInfo.set_out_trade_no(src.m_outrade_no);
	cashFlowInfo.set_amount(src.m_amount);
	cashFlowInfo.set_add_or_reduce(src.m_add_or_reduce);
	cashFlowInfo.set_type(src.m_flow_type);
	cashFlowInfo.set_balance(src.m_balance);
	cashFlowInfo.set_create_ts(src.m_create_ts);
}

static void out_come_req_info_mem_2_pb(const DaoOutcome& src, hoosho::msg::z::OutcomeReqInfo& outcomeReqInfo)
{
	outcomeReqInfo.set_table_id(src.m_table_id);
	outcomeReqInfo.set_openid(src.m_openid);
	outcomeReqInfo.set_amount(src.m_amount);
	outcomeReqInfo.set_state(src.m_state);
	outcomeReqInfo.set_create_ts(src.m_create_ts);
}

static void order_favor_info_db_2_pb(lce::cgi::CMysql &mysql, hoosho::msg::z::OrderFavorInfo &stOrderFavorInfo)
{
	stOrderFavorInfo.set_orderid(strtoul(mysql.GetRow(0), NULL, 10));
	stOrderFavorInfo.set_openid(mysql.GetRow(1));
	stOrderFavorInfo.set_create_ts(strtoul(mysql.GetRow(2), NULL, 10));
}

static void order_follow_info_db_2_pb(lce::cgi::CMysql &mysql, hoosho::msg::z::OrderFollowInfo &stOrderFollowInfo)
{
	stOrderFollowInfo.set_follow_id(strtoul(mysql.GetRow(0), NULL, 10));
	stOrderFollowInfo.set_content(mysql.GetRow(1));
	stOrderFollowInfo.set_orderid(strtoul(mysql.GetRow(2), NULL, 10));
	stOrderFollowInfo.set_type(atoi(mysql.GetRow(3)));
	stOrderFollowInfo.set_origin_comment_id(strtoul(mysql.GetRow(4), NULL, 10));

	stOrderFollowInfo.set_openid_from(mysql.GetRow(5));
	stOrderFollowInfo.set_openid_to(mysql.GetRow(6));
	stOrderFollowInfo.set_create_ts(strtoul(mysql.GetRow(7), NULL, 10));

	stOrderFollowInfo.set_del_ts(strtoul(mysql.GetRow(8), NULL, 10));

}

static void notice_info_db_2_pb(lce::cgi::CMysql &mysql, hoosho::msg::z::NoticeInfo &stNoticeInfo)
{
	stNoticeInfo.set_table_id(common::util::charToUint64_t(mysql.GetRow(0)));
	stNoticeInfo.set_openid(mysql.GetRow(1));
	stNoticeInfo.set_type(common::util::charToUint64_t(mysql.GetRow(2)));
	stNoticeInfo.set_status(common::util::charToUint64_t(mysql.GetRow(3)));
	stNoticeInfo.set_extra_data_0(mysql.GetRow(4));
	stNoticeInfo.set_extra_data_1(mysql.GetRow(5));
	stNoticeInfo.set_extra_data_2(mysql.GetRow(6));
	stNoticeInfo.set_create_ts(common::util::charToUint64_t(mysql.GetRow(7)));	
}

static void user_activity_info_db_2_pb(lce::cgi::CMysql &mysql, hoosho::msg::z::UserActivityInfo &stUserActivityInfo)
{
	stUserActivityInfo.set_openid(mysql.GetRow(0));
	stUserActivityInfo.set_info_id(mysql.GetRow(1));
	stUserActivityInfo.set_activity_type(atoi(mysql.GetRow(2)));
	stUserActivityInfo.set_activity_info(mysql.GetRow(3));
}

static void lottery_info_db_2_pb(lce::cgi::CMysql &mysql, hoosho::msg::z::LotteryInfo &stLotteryInfo)
{
	stLotteryInfo.set_prize(common::util::charToUint64_t(mysql.GetRow(0)));
	stLotteryInfo.set_prize_desc(mysql.GetRow(1));
	stLotteryInfo.set_create_ts(common::util::charToUint64_t(mysql.GetRow(2)));
}

int DaoOrderInfo::AddNew(lce::cgi::CMysql &mysql, std::string &strErrMsg)
{
	std::ostringstream oss;
	oss.str("");

	oss << "insert into " << m_table_name
			<< " set orderid = " << m_order_info.order_id()
			<< ", openid_master = '" << sql_escape(m_order_info.openid_master()) << "'"
			<< ", openid_slave = ''"
			<< ", order_visible = " << m_order_info.order_visible()
			<< ", order_status = " << m_order_info.order_status()
			<< ", pay_status = " << m_order_info.pay_status()
			<< ", addr_to_longitude = " << m_order_info.addr_to_longitude()
			<< ", addr_to_latitude = " << m_order_info.addr_to_latitude()
			<< ", addr_to = '" << sql_escape(m_order_info.addr_to()) << "'"
			<< ", addr_to_detail = '" << sql_escape(m_order_info.addr_to_detail()) << "'"
			<< ", order_desc = '" << sql_escape(m_order_info.order_desc()) << "'"
			<< ", order_receiver_name = '" << sql_escape(m_order_info.order_receiver_name()) << "'"
			<< ", order_receiver_phone = '" << sql_escape(m_order_info.order_receiver_phone()) << "'"
			<< ", price = " << m_order_info.price()
			<< ", expect_from_ts = " << m_order_info.expect_from_ts()
			<< ", expect_to_ts = " << m_order_info.expect_to_ts()
			<< ", pay_ts = " << m_order_info.pay_ts()
			<< ", create_ts = unix_timestamp()"
			<< ", finish_ts = 0"
			<< ", fentch_ts = 0"
			<< ", cancel_ts = 0"
			<< ", star_from_master = 0"
			<< ", star_from_slave = 0"
			<< ", order_type = "<<m_order_info.order_type()
			<< ", media_type = "<<m_order_info.media_type()
			<< ", openid_waiting = ''"
			<< ", extra_data = '"<<sql_escape(m_order_info.extra_data())<<"'";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoOrderInfo::Payed(uint64_t qwOrderId, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_order_info.set_order_id(qwOrderId);

	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(qwOrderId, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "order payed failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	if(ORDER_PAY_STATUS_PAYED == m_order_info.pay_status())
	{
		return DAO_RET_OK;
	}
	
	
	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_table_name
		<<" set order_status="<<ORDER_STATUS_WAITING_FETCH
		<<", pay_status="<<ORDER_PAY_STATUS_PAYED
		<<", pay_ts=unix_timestamp()"
		<<" where orderid="<<m_order_info.order_id();
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}


int DaoOrderInfo::FetchTry(uint64_t qwOrderId
							, const std::string& strFentchOpenid
							, lce::cgi::CMysql& mysql
							, std::string& strErrMsg)
{
	m_order_info.set_order_id(qwOrderId);

	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(qwOrderId, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "order FetchTry failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	int iWaitingUserNum = m_order_info.order_waiting_user_list_size();
	if(iWaitingUserNum >= DB_ORDER_INFO_OPENID_WAITING_MAX_NUM)
	{
		strErrMsg = "order FetchTry failed, msg=iWaitingUserNum=" + int_2_str(iWaitingUserNum);
		strErrMsg += "too many waiting user for max_num=" + int_2_str(DB_ORDER_INFO_OPENID_WAITING_MAX_NUM);
		return DAO_RET_LOGIC_TOO_MANY;
	}

	if(order_waiting_user_exists(m_order_info, strFentchOpenid))
	{
		strErrMsg = "order FetchTry failed, msg=already in fentching, strFentchOpenid=" + strFentchOpenid;
		return DAO_RET_LOGIC_REPEATED;
	}

	hoosho::msg::z::OrderWaitingUser* pOrderWaitingUser = m_order_info.add_order_waiting_user_list();
	pOrderWaitingUser->set_openid(strFentchOpenid);
	pOrderWaitingUser->set_create_ts(time(0));
	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_table_name
		<<" set openid_waiting='"<<sql_escape(order_waiting_user_pb_2_str(m_order_info))<<"'"
		<<" where orderid="<<m_order_info.order_id();
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "order FetchTry failed, mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoOrderInfo::FetchCanceled(uint64_t qwOrderId
							, const std::string& strFentchOpenid
							, lce::cgi::CMysql& mysql
							, std::string& strErrMsg)
{
	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(qwOrderId, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "order FetchCanceled failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}


	if(strFentchOpenid == m_order_info.openid_slave())
	{
		strErrMsg = "order FetchCanceled failed, msg=m_order_info.slave_openid=" +  m_order_info.openid_slave();
		strErrMsg = ", has been choson as slave_openid, can not cancel fetch waiting!!!!";
		return DAO_RET_INVALID;
	}

	order_waiting_user_del(m_order_info, strFentchOpenid);

	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_table_name
		<<" set openid_waiting='"<<sql_escape(order_waiting_user_pb_2_str(m_order_info))<<"'"
		<<" where orderid="<<m_order_info.order_id();
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoOrderInfo::FetchAccepted(uint64_t qwOrderId
							, const std::string& strMasterOpenid
							, const std::string& strSlaveOpenid
							, lce::cgi::CMysql& mysql
							, std::string& strErrMsg)
{
	m_order_info.set_order_id(qwOrderId);

	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(qwOrderId, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "order FetchAccepted failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	if(m_order_info.order_status() != ORDER_STATUS_WAITING_FETCH)
	{
		strErrMsg = "FetchAccepted failed, msg=order_status=";
		strErrMsg += m_order_info.order_status() + ",  not ORDER_STATUS_WAITING_FETCH";
		return DAO_RET_INVALID;
	}

	if(m_order_info.openid_master() != strMasterOpenid)
	{
		strErrMsg = "FetchAccepted failed, msg=order_openid_master=" + m_order_info.openid_master();
		strErrMsg += ", req strMasterOpenid=" + strMasterOpenid + ",  not equal, refused!!! sb";
		return DAO_RET_FAIL;
	}

	if(!order_waiting_user_exists(m_order_info, strSlaveOpenid))
	{
		strErrMsg = "FetchAccepted failed, msg=strSlaveOpenid=" + strSlaveOpenid;
		strErrMsg += ",  not in waiting_user_list={" + order_waiting_user_pb_2_str(m_order_info);
		strErrMsg += "}";
		return DAO_RET_INVALID;
	}

	m_order_info.set_openid_slave(strSlaveOpenid);
	m_order_info.set_fetch_ts(time(0));

	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_table_name
		<<" set order_status="<<ORDER_STATUS_FETCHED
		<<", openid_slave='"<<sql_escape(strSlaveOpenid)<<"'"
		<<", fentch_ts=unix_timestamp()"
		<<" where orderid="<<m_order_info.order_id();
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoOrderInfo::Finished(uint64_t qwOrderId, const std::string& strMasterOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_order_info.set_order_id(qwOrderId);

	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(qwOrderId, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "order Finished failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	if(m_order_info.order_status() != ORDER_STATUS_FETCHED)
	{
		strErrMsg = "order Finished failed, msg=order_status=";
		strErrMsg += m_order_info.order_status() + ",  not ORDER_STATUS_FETCHED";
		return DAO_RET_FAIL;
	}

	if(m_order_info.openid_master() != strMasterOpenid)
	{
		strErrMsg = "order Finished failed, msg=order_openid_master=" + m_order_info.openid_master();
		strErrMsg += ", order_openid_master=" + strMasterOpenid + ",  not equal, refused!!! sb";
		return DAO_RET_FAIL;
	}
	m_order_info.set_order_status(ORDER_STATUS_FINISHED);
	m_order_info.set_finish_ts(time(0));
	
	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_table_name
		<<" set order_status="<<ORDER_STATUS_FINISHED
		<<", finish_ts=unix_timestamp()"
		<<" where orderid="<<m_order_info.order_id();
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoOrderInfo::Canceled(uint64_t qwOrderId, const std::string& strMasterOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_order_info.set_order_id(qwOrderId);

	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(qwOrderId, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "order Canceled failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	if(ORDER_STATUS_CANCELED == m_order_info.order_status())
	{	
		//repeated operations
		return DAO_RET_OK;
	}

	if(m_order_info.order_status() != ORDER_STATUS_INIT && m_order_info.order_status() != ORDER_STATUS_WAITING_FETCH)
	{
		strErrMsg = "order Canceled failed, msg=order_status=";
		strErrMsg += m_order_info.order_status() + ",  cancel op is not permitted!!!";
		return DAO_RET_FAIL;
	}

	if(m_order_info.order_status() == ORDER_STATUS_WAITING_FETCH && m_order_info.order_waiting_user_list_size() > 0)
	{
		strErrMsg = "order Canceled failed, msg=order_status=ORDER_STATUS_WAITING_FETCH, but already has waiting users={";
		strErrMsg += order_waiting_user_pb_2_str(m_order_info) + ", cancel op is not permitted!!!";
		return DAO_RET_FAIL;
	}

	if(m_order_info.openid_master() != strMasterOpenid)
	{
		strErrMsg = "order Canceled failed, msg=order_openid_master=" + m_order_info.openid_master();
		strErrMsg += ", order_openid_master=" + strMasterOpenid + ",  not equal, refused!!! sb";
		return DAO_RET_FAIL;
	}

	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_table_name
		<<" set order_status="<<ORDER_STATUS_CANCELED
		<<", cancel_ts=unix_timestamp()"
		<<" where orderid="<<m_order_info.order_id();
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoOrderInfo::Commented(uint64_t qwOrderId, const std::string& strCommentOpenid, std::string& strAnoOpenid, uint32_t dwStar, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_order_info.set_order_id(qwOrderId);

	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(qwOrderId, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "order Canceled failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	if(m_order_info.order_status() != ORDER_STATUS_FINISHED )
	{
		strErrMsg = "order Commented failed, msg=order_status=";
		strErrMsg += m_order_info.order_status() + ",  Commented op is not permitted!!!";
		return DAO_RET_FAIL;
	}



	std::ostringstream oss;
	oss.str("");

	if(m_order_info.openid_master() == strCommentOpenid)
	{
		strAnoOpenid = m_order_info.openid_slave();
		if(m_order_info.star_from_master() != 0)
		{
			strErrMsg = "order Commented failed, msg=order has  alreaady been commented to star_from_master = " + int_2_str(m_order_info.star_from_master());
			return DAO_RET_INVALID;
		}

		oss<<"update "<<m_table_name
			<<" set star_from_master="<<dwStar
			<<" where orderid="<<m_order_info.order_id();
		if(!mysql.Query(oss.str()))
		{
			strErrMsg = "mysql query failed, msg=";
			strErrMsg += mysql.GetErrMsg();
			return DAO_RET_FAIL;
		}
	}
	else if(m_order_info.openid_slave() == strCommentOpenid)
	{
		strAnoOpenid = m_order_info.openid_master();
		if(m_order_info.star_from_slave() != 0)
		{
			strErrMsg = "order Commented failed, msg=order has  alreaady been commented to star_from_slave = " + int_2_str(m_order_info.star_from_slave());
			return DAO_RET_INVALID;
		}

		oss<<"update "<<m_table_name
			<<" set star_from_slave="<<dwStar
			<<" where orderid="<<m_order_info.order_id();
		if(!mysql.Query(oss.str()))
		{
			strErrMsg = "mysql query failed, msg=";
			strErrMsg += mysql.GetErrMsg();
			return DAO_RET_FAIL;
		}
	}
	else
	{
		strErrMsg = "order Commented failed, msg=order_openid_master=" + m_order_info.openid_master();
		strErrMsg += ", order_openid_slave=" + m_order_info.openid_slave();
		strErrMsg += ", req_openid=" + strCommentOpenid + ",  not match, refused!!! sb";
		return DAO_RET_INVALID;
	}

	return DAO_RET_OK;
}


int DaoOrderInfo::GetByPrimaryKey(uint64_t qwOrderId, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_order_info.set_order_id(qwOrderId);

	std::ostringstream oss;
	oss.str("");
	oss << "select * from "<<m_table_name<<" where orderid="<<m_order_info.order_id();

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		order_info_db_2_pb(mysql, m_order_info);
		return DAO_RET_OK;
	}

	strErrMsg = "orderid = " + int_2_str(m_order_info.order_id()) + " not exists";
	return DAO_RET_NOT_EXIST;
}

int DaoOrderInfo::DeleteFromDB(lce::cgi::CMysql &mysql, std::string &strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "DELETE FROM " << m_table_name << " WHERE orderid = " << m_order_info.order_id();

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	return DAO_RET_OK;
}

uint64_t DaoOrderInfo::CreateRandomRewardFeeForNewUser(const std::string& strOpenid, const std::string& strTableUserInfoName, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{	
	uint64_t qwRewardFee = 0;
	DaoUserInfo stDaoUserInfo(strTableUserInfoName);
	int iRet = stDaoUserInfo.GetByPrimaryKey(strOpenid, mysql, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		return qwRewardFee;
	}
	
	if((stDaoUserInfo.m_user_info.user_flag() & USER_FLAG_NEWUSER) && (m_order_info.price() > 0))
	{
		Json::Value jValue;
		Json::Reader jReader;
		if(!jReader.parse(m_order_info.extra_data(), jValue, false))
		{			
			strErrMsg = "parse json error, data =";
			strErrMsg += m_order_info.extra_data();
			return qwRewardFee;
		}

		uint64_t qwMinFee = m_order_info.price() > 50 ? 50 : m_order_info.price();
		qwRewardFee = common::util::GetRandNum(1, qwMinFee);				//unit: RMB fen 
		jValue["reward_fee"] = qwRewardFee;				
		m_order_info.set_extra_data(jValue.toJsonString());			
	}	

	return qwRewardFee;
}

int DaoOrderInfo::FinishRewardFeeForNewUser(const std::string& strOpenid, 
											const std::string& strTableUserInfoName, 
											const std::string& m_db_table_user_cash_name, 
											const std::string& m_db_table_cash_flow_name, 
											lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	DaoUserInfo stDaoUserInfo(strTableUserInfoName);
	int iRet = stDaoUserInfo.GetByPrimaryKey(strOpenid, mysql, strErrMsg);
	if(iRet)
	{
		return iRet;
	}

	uint64_t qwUserFlag = stDaoUserInfo.m_user_info.user_flag();
	if(qwUserFlag & USER_FLAG_NEWUSER)
	{
		stDaoUserInfo.m_user_info.set_user_flag(qwUserFlag & (~ USER_FLAG_NEWUSER));
		int iRet = stDaoUserInfo.UpdateUserInfo(mysql, strErrMsg);
		if(iRet != DAO_RET_OK)
		{
			return iRet;
		}

		Json::Value jValue;
		Json::Reader jReader;
		if(!jReader.parse(m_order_info.extra_data(), jValue, false))
		{			
			strErrMsg = "parse json error, data =";
			strErrMsg += m_order_info.extra_data();
			return DAO_RET_FAIL;
		}

		if(!jValue.hasKey("reward_fee"))
		{
			strErrMsg = "json key [reward_fee] not found";
			return DAO_RET_FAIL;
		}
		uint64_t qwRewardFee = jValue["reward_fee"].asUInt();

		DAOUserCash stDAOUserCash(m_db_table_user_cash_name, m_db_table_cash_flow_name);
		iRet = stDAOUserCash.AddBalance(strOpenid, qwRewardFee, CASH_FLOW_TYPE_INCOME_ORDER_REWARD_BY_PA, mysql, strErrMsg, "");
		if(iRet != DAO_RET_OK)
		{
			return iRet;
		}		
	} 

	return DAO_RET_OK;
}

int DaoOrderMediaInfo::AddOrUpdate(uint64_t qwOrderId
						, uint64_t qwMediaType
						, const std::string& strMediaContent
						, lce::cgi::CMysql& mysql
						, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "insert into "<<m_table_name 
		<< " set order_id="<<qwOrderId
		<<", media_type="<<qwMediaType
		<<", media_content='"<<sql_escape(strMediaContent)<<"'"
		<<" on duplicate key update media_content='"<<sql_escape(strMediaContent)<<"'";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	return DAO_RET_OK;
}

int DaoOrderFetchState::GetByPrimaryKey(uint64_t qwOrderId, const std::string& strFetchOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	/*
			mysql> desc t_order_fetch_state;
	+--------------+---------------------+------+-----+---------+-------+
	| Field        | Type                | Null | Key | Default | Extra |
	+--------------+---------------------+------+-----+---------+-------+
	| order_id     | bigint(20) unsigned | NO   | PRI | 0       |       |
	| fetch_openid | char(128)           | NO   | PRI |         |       |
	| fetch_state  | tinyint(3) unsigned | NO   |     | 1       |       |
	| fentch_ts    | bigint(20) unsigned | NO   |     | 0       |       |
	| accpet_ts    | bigint(20) unsigned | NO   |     | 0       |       |
	+--------------+---------------------+------+-----+---------+-------+
	*/

	std::ostringstream oss;
	oss.str("");
	oss << "select * from "<<m_table_name<<" where order_id="<<qwOrderId<<" and fetch_openid='"<<sql_escape(strFetchOpenid)<<"'";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		order_openid_fetch_state_db_2_pb(mysql, m_order_openid_fetch_state);
		return DAO_RET_OK;
	}

	strErrMsg = "orderid = " + int_2_str(qwOrderId) + ", fetch_openid=" + strFetchOpenid + " not exists";
	return DAO_RET_NOT_EXIST;

}

int DaoOrderFetchState::DeleteByPrimaryKey(uint64_t qwOrderId, const std::string& strFetchOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "delete from "<<m_table_name<<" where order_id="<<qwOrderId<<" and fetch_openid='"<<sql_escape(strFetchOpenid)<<"'";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	
	return DAO_RET_OK;
}


int DaoOrderFetchState::AddNewWaiting(uint64_t qwOrderId, const std::string& strFetchOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "insert into "<<m_table_name 
		<< " set order_id="<<qwOrderId
		<<", fetch_openid='"<<sql_escape(strFetchOpenid)<<"'"
		<<", fetch_state="<<ORDER_OPENID_FETCH_STATE_WAITING
		<<", fentch_ts=unix_timestamp()"
		<<", accpet_ts=0";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	
	return DAO_RET_OK;
}

int DaoOrderFetchState::UpdateToAccepted(uint64_t qwOrderId, const std::string& strFetchOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "update "<<m_table_name
		<<" set fetch_state="<<ORDER_OPENID_FETCH_STATE_ACCEPTED
		<<", accpet_ts=unix_timestamp()"
		<<" where order_id="<<qwOrderId<<" and fetch_openid='"<<sql_escape(strFetchOpenid)<<"'";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	
	return DAO_RET_OK;
}

int DaoOrderFetchStateList::GetFentchedOrderList(const std::string& strOpenid
										, uint64_t qwBeforeOrderId
										, uint32_t dwNeedNum
										, lce::cgi::CMysql& mysql
										, std::string& strErrMsg)
{
	if(0 == qwBeforeOrderId)
	{
		qwBeforeOrderId = 0xFFFFFFFFFFFFFFFF;
	}

	if(0 == dwNeedNum || dwNeedNum > 20)
	{
		dwNeedNum = 10;
	}

	std::ostringstream oss;
	oss.str("");
	oss << "select * from " << m_table_name
		<< " where fetch_openid = '" << sql_escape(strOpenid) << "'"
		<< " and order_id < "<<qwBeforeOrderId
		<<" order by order_id desc limit "<<dwNeedNum;
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(0 == mysql.GetRowCount())
	{
		return DAO_RET_NOT_EXIST;	
	}

	m_vec_order_fetch_state_list.clear();
	while(mysql.Next())
	{
		hoosho::msg::z::OrderOpenidFetchState orderOpenidFetchState;
		order_openid_fetch_state_db_2_pb(mysql, orderOpenidFetchState);
		m_vec_order_fetch_state_list.push_back(orderOpenidFetchState);
	}

	return DAO_RET_OK;
}


int DaoOrderInfoList::GetCreatedOrderList(const std::string& strOpenid
										, uint64_t qwBeforeOrderId
										, uint32_t dwNeedNum
										, lce::cgi::CMysql& mysql
										, std::string& strErrMsg)
{
	if(0 == qwBeforeOrderId)
	{
		qwBeforeOrderId = 0xFFFFFFFFFFFFFFFF;
	}

	if(0 == dwNeedNum || dwNeedNum > 20)
	{
		dwNeedNum = 10;
	}

	std::ostringstream oss;
	oss.str("");
	oss << "select * from " << m_table_name
		<< " where openid_master = '" << sql_escape(strOpenid) << "'"
		<< " and orderid < "<<qwBeforeOrderId
		<< " and order_status != " << ORDER_STATUS_INIT
		<<" order by orderid desc limit "<<dwNeedNum;
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(0 == mysql.GetRowCount())
	{
		strErrMsg = "not exists";
		return DAO_RET_NOT_EXIST;	
	}

	m_order_info_list.clear();
	while(mysql.Next())
	{
		hoosho::msg::z::OrderInfo orderInfo;
		order_info_db_2_pb(mysql, orderInfo);
		m_order_info_list.push_back(orderInfo);
	}

	return DAO_RET_OK;
}

int DaoOrderReward::AddNew(lce::cgi::CMysql &mysql, std::string &strErrMsg)
{
	std::ostringstream oss;
	oss.str("");

	oss << "insert into " << m_table_name
			<< " set out_trade_no = '" << sql_escape(m_out_trade_no) << "'"
			<< ", orderid = " << m_orderid
			<< ", openid_from = '" << sql_escape(m_openid_from) << "'"
			<< ", openid_to = '" << sql_escape(m_openid_to) << "'"
			<< ", reward_ts = " << m_reward_ts
			<< ", amount = " << m_amount
			<< ", pay_status = " << m_pay_status
			<< ", pay_ts = " << m_pay_ts;

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoOrderReward::Payed(const std::string &strOutTradeNo, lce::cgi::CMysql &mysql, std::string &strErrMsg)
{
	int iRet = GetByPrimaryKey(strOutTradeNo, mysql, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		strErrMsg = "payed failed, msg = " + strErrMsg;
		return iRet;
	}

	std::ostringstream oss;
	oss.str("");
	oss << "UPDATE " << m_table_name
		<< " SET pay_status = " << ORDER_PAY_STATUS_PAYED
		<< ", pay_ts=unix_timestamp()"
		<< " WHERE out_trade_no = '" << sql_escape(strOutTradeNo) << "'";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "sql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	return DAO_RET_OK;
}

int DaoOrderReward::GetByPrimaryKey(const std::string &strOutTradeNo, lce::cgi::CMysql &mysql, std::string &strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "SELECT * FROM " << m_table_name << " WHERE out_trade_no = '" << sql_escape(m_out_trade_no) << "'";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "sql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	if(mysql.GetRowCount() && mysql.Next())
	{
		m_out_trade_no = mysql.GetRow(0);
		m_orderid = strtoul(mysql.GetRow(1), NULL, 10);
		m_openid_from = mysql.GetRow(2);
		m_openid_to = mysql.GetRow(3);
		m_reward_ts = strtoul(mysql.GetRow(4), NULL, 10);
		m_amount = strtoul(mysql.GetRow(5), NULL, 10);
		m_pay_status = strtoul(mysql.GetRow(6), NULL, 10);
		m_pay_ts = strtoul(mysql.GetRow(7), NULL, 10);
		return DAO_RET_OK;
	}

	strErrMsg = "OutTradeNo = " + strOutTradeNo + "not exist";
	return DAO_RET_NOT_EXIST;
}


int DaoUserInfo::GetByPrimaryKey(const std::string& strOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_user_info.set_openid(strOpenid);

	std::ostringstream oss;
	oss.str("");
	oss<<"select * from "<<m_table_name
		<<" where openid='"<<sql_escape(strOpenid)<<"'";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	
	if(mysql.GetRowCount() && mysql.Next())
	{
		user_info_db_2_pb(mysql, m_user_info);
		return DAO_RET_OK;
	}

	strErrMsg = "openid=" + strOpenid + " not exists" ;
	return DAO_RET_NOT_EXIST;
}

int DaoUserInfo::UpdateUserInfo(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");	
	oss<<"insert into "<<m_table_name
		<<" set openid='"<<sql_escape(m_user_info.openid())<<"'"		
		<<", nickname='"<<sql_escape(m_user_info.nickname())<<"'"
		<<", sex="<<m_user_info.sex()
		<<", headimgurl='"<<sql_escape(m_user_info.headimgurl())<<"'"
		<<", user_type="<<m_user_info.user_type()
		<<", user_flag="<<m_user_info.user_flag()
		<<" on duplicate key update"		
		<<", nickname='"<<sql_escape(m_user_info.nickname())<<"'"
		<<", sex="<<m_user_info.sex()
		<<", headimgurl='"<<sql_escape(m_user_info.headimgurl())<<"'"
		<<", user_type="<<m_user_info.user_type()
		<<", user_flag="<<m_user_info.user_flag();

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int	DaoUserInfo::IncreaseUserScore(const std::string& strOpenid, int iScore, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_table_name
	   <<" set user_score=user_score+"<<iScore
	   <<" where openid='"<<sql_escape(strOpenid)<<"'";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoUserInfo::GetRankingList(uint32_t iLen, uint32_t iPage, std::vector<hoosho::msg::z::UserInfo>& vecPBUserInfoList, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from "<<m_table_name
	   <<" order by user_score desc limit "<< iPage*iLen << "," <<iLen;

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::z::UserInfo stUserInfo;
			user_info_db_2_pb(mysql, stUserInfo);		
			vecPBUserInfoList.push_back(stUserInfo);	
		}
	}	 	    	

	return DAO_RET_OK;
}


int DaoUserTrust::SelectFromDB(lce::cgi::CMysql &mysql, std::string &strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "SELECT * FROM " << m_table_name
		<< " WHERE openid_from = '" << sql_escape(m_openid_from) << "'"
		<< " AND openid_to = '" << sql_escape(m_openid_to) << "'";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	if(mysql.GetRowCount() && mysql.Next())
	{
		return DAO_RET_OK;
	}
	strErrMsg = "openid:" + m_openid_from + " don't trust openid: " + m_openid_to;
	return DAO_RET_NOT_EXIST;
}


int DaoUserExtraInfo::AddNewUserStar(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	/*
	mysql> desc t_user_star;
	+-----------+---------------------+------+-----+---------+-------+
	| Field     | Type                | Null | Key | Default | Extra |
	+-----------+---------------------+------+-----+---------+-------+
	| openid    | char(128)           | NO   | PRI |         |       |
	| orderid   | bigint(20) unsigned | NO   | PRI | 0       |       |
	| star      | tinyint(3) unsigned | NO   |     | 0       |       |
	| create_ts | bigint(20) unsigned | NO   |     | 0       |       |
	+-----------+---------------------+------+-----+---------+-------+
	*/

	std::ostringstream oss;
	oss.str("");
	oss<<"insert into "<<m_table_user_star_name
		<<" set openid='"<<sql_escape(m_openid)<<"'"
		<<", orderid="<<m_order_id
		<<", star="<<m_star
		<<", create_ts=unix_timestamp()";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoUserExtraInfo::CountAverageStar(const std::string& strOpenid, float& fAvgStar, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_openid = strOpenid;
	
	std::ostringstream oss;
	oss.str("");
	oss<<"select avg(star) from "<<m_table_user_star_name
		<<" where openid='"<<sql_escape(m_openid)<<"'";	
	
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	const char* pcResult = NULL;
	if(mysql.GetRowCount() && mysql.Next())
	{
		pcResult = mysql.GetRow(0);
	}

	if(!pcResult)
	{
		fAvgStar= 0.0f;		
	}
	else
	{
		fAvgStar = strtof(pcResult, NULL);				
	}

	return DAO_RET_OK;
}

int DaoUserExtraInfo::CountUserLevel(const std::string& strOpenid, uint32_t& iLevel, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_openid = strOpenid;
	
	std::ostringstream oss;
	oss.str("");
	oss<<"select user_score from t_user_info"
		<<" where openid='"<<sql_escape(m_openid)<<"'";	
	
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	const char* pcResult = NULL;
	if(mysql.GetRowCount() && mysql.Next())
	{
		pcResult = mysql.GetRow(0);
	}

	if(!pcResult)
	{
		iLevel= USER_LEVEL_1;		
	}
	else
	{
		uint64_t qwScore = strtoul(pcResult, NULL, 10);	
		if(qwScore < USER_LEVEL_SCORE_2)
		{
			iLevel= USER_LEVEL_1;
		}
		else if(qwScore < USER_LEVEL_SCORE_3)
		{
			iLevel= USER_LEVEL_2;
		}
		else if(qwScore < USER_LEVEL_SCORE_4)
		{
			iLevel= USER_LEVEL_3;
		}
		else if(qwScore < USER_LEVEL_SCORE_5)
		{
			iLevel= USER_LEVEL_4;
		}
		else if(qwScore < USER_LEVEL_SCORE_6)
		{
			iLevel= USER_LEVEL_5;
		}
		else
		{
			iLevel= USER_LEVEL_6;
		}		
	}

	return DAO_RET_OK;
}

int DaoUserExtraInfo::GetUserExtraInfo(const std::string& strOpenid, ::hoosho::msg::z::UserExtraInfo& stUserExtraInfo, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	float fAvgStar;
	uint32_t iLevel;
	if(CountAverageStar(strOpenid, fAvgStar, mysql, strErrMsg) != DAO_RET_OK)
	{
		return DAO_RET_FAIL;
	}

	if(CountUserLevel(strOpenid, iLevel, mysql, strErrMsg) != DAO_RET_OK)
	{
		return DAO_RET_FAIL;
	}

	stUserExtraInfo.set_openid(strOpenid);
	stUserExtraInfo.set_star(fAvgStar);
	stUserExtraInfo.set_level(iLevel);
	
	return DAO_RET_OK;
}

	int DaoUserActivityInfo::GetByPrimaryKey(const std::string& strInfoId, const uint32_t dwActivityId, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "SELECT * FROM " << m_table_name
		<< " WHERE info_id = '" << sql_escape(strInfoId) << "'"
		<< " AND activity_type = " << dwActivityId;
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	if(mysql.GetRowCount() && mysql.Next())
	{
		user_activity_info_db_2_pb(mysql, m_user_activity_info);
		return DAO_RET_OK;
	}

	strErrMsg = "info_id=" + strInfoId + ", activity_type=" + int_2_str(dwActivityId) + " not exist";
	return DAO_RET_NOT_EXIST;
}

int DaoUserActivityInfo::AddNew(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "INSERT INTO " << m_table_name
		<< " SET openid = '" << sql_escape(m_user_activity_info.openid()) << "'"
		<< ", info_id = '" << sql_escape(m_user_activity_info.info_id()) << "'"
		<< ", activity_type = " << m_user_activity_info.activity_type()
		<< ", activity_info = '" << m_user_activity_info.activity_info() << "'";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "msyql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	return DAO_RET_OK;
}

int DaoUserActivityInfo::Update(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "UPDATE " << m_table_name
		<< " SET activity_info = '" << m_user_activity_info.activity_info() << "'"
		<< " WHERE info_id = '" << m_user_activity_info.info_id() << "'"
		<< " AND activity_type = " << m_user_activity_info.activity_type();
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(!mysql.GetAffectedRows())
	{
		strErrMsg = "not found openid=" + m_user_activity_info.openid();
		strErrMsg += ", info_id=" + m_user_activity_info.info_id();
		strErrMsg += ", activity_type=" + int_2_str(m_user_activity_info.activity_type());
		return DAO_RET_NOT_EXIST;
	}
	return DAO_RET_OK;
}

int DAOWXOuttrade::AddNew(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"insert into "<<m_table_name
		<<" set openid='"<<sql_escape(m_openid)<<"'"
		<<", out_trade_no='"<<sql_escape(m_outtrade_no)<<"'"
		<<", prepay_id='"<<sql_escape(m_prepay_id)<<"'"
		<<", body='"<<sql_escape(m_body)<<"'"
		<<", detail='"<<sql_escape(m_detail)<<"'"
		<<", total_fee="<<m_total_fee
		<<", attach='"<<sql_escape(m_attach)<<"'"
		<<", fee_type='"<<sql_escape(m_fee_type)<<"'"
		<<", spbill_create_ip='"<<sql_escape(m_spbill_create_ip)<<"'"
		<<", time_start='"<<sql_escape(m_time_start)<<"'"
		<<", time_expire='"<<sql_escape(m_time_expire)<<"'"
		<<", goods_tag='"<<sql_escape(m_goods_tag)<<"'"
		<<", trade_type='"<<sql_escape(m_trade_type)<<"'"
		<<", product_id='"<<sql_escape(m_product_id)<<"'"
		<<", limit_pay='"<<sql_escape(m_limit_pay)<<"'"
		<<", appid='"<<sql_escape(m_appid)<<"'"
		<<", mch_id='"<<sql_escape(m_mch_id)<<"'"
		<<", device_info='"<<sql_escape(m_device_info)<<"'"
		<<", pay_state="<<m_pay_state
		<<", pay_reason="<<m_pay_reason;

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DAOWXOuttrade::UpdatePayState(const std::string& strOpenid, const std::string& strOuttradeNO, uint16_t wPayState, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_openid = strOpenid;
	m_outtrade_no = strOuttradeNO;
	m_pay_state = wPayState;
	
	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(strOpenid, strOuttradeNO, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "update pay state failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_table_name
		<<" set pay_state="<<m_pay_state
		<<" where openid='"<<sql_escape(m_openid)<<"'"
		<<" and out_trade_no='"<<sql_escape(m_outtrade_no)<<"'";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DAOWXOuttrade::GetByPrimaryKey(const std::string& strOpenid, const std::string& strOuttradeNO, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_openid = strOpenid;
	m_outtrade_no = strOuttradeNO;

	std::ostringstream oss;
	oss.str("");
	oss<<"select * from "<<m_table_name
		<<" where openid='"<<sql_escape(m_openid)<<"'"
		<<" and out_trade_no='"<<sql_escape(m_outtrade_no)<<"'";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	
	if(mysql.GetRowCount() && mysql.Next())
	{
		m_prepay_id = mysql.GetRow(2);
		m_body = mysql.GetRow(3);
		m_detail = mysql.GetRow(4);
		m_total_fee = strtoul(mysql.GetRow(5), NULL, 10);
		m_attach = mysql.GetRow(6);
		m_fee_type = mysql.GetRow(7);
		m_spbill_create_ip = mysql.GetRow(8);
		m_time_start = mysql.GetRow(9);
		m_time_expire = mysql.GetRow(10);
		m_goods_tag = mysql.GetRow(11);
		m_trade_type = mysql.GetRow(12);
		m_product_id = mysql.GetRow(13);
		m_limit_pay = mysql.GetRow(14);
		m_appid = mysql.GetRow(15);
		m_mch_id= mysql.GetRow(16);
		m_device_info = mysql.GetRow(17);
		m_pay_state = atoi(mysql.GetRow(18));
		m_pay_reason = atoi(mysql.GetRow(19));

		return DAO_RET_OK;
	}

	strErrMsg = "not exists";
	return DAO_RET_NOT_EXIST;
}


int DAOCashFlow::AddNew(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_table_id = common::util::generate_unique_id();
	
	std::ostringstream oss;
	oss.str("");

	oss << "insert into " << m_table_name
			<< " set openid = '" << sql_escape(m_openid)<<"'"
			<< ", table_id = " << m_table_id
			<< ", out_trade_no = '" <<sql_escape(m_outrade_no)<<"'"
			<< ", add_or_reduce = " << m_add_or_reduce
			<< ", amount = " << m_amount
			<< ", type = " << m_flow_type
			<< ", balance = " << m_balance
			<< ", create_ts = unix_timestamp()";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoCashFlowList::GetByTsOrder(const std::string& strOpenid, uint64_t qwBeforeTS, uint32_t dwNeedNum
						, lce::cgi::CMysql& mysql, std::string& strErrMsg, std::vector<hoosho::msg::z::CashFlowInfo>& vecPBList)
{
	int iRet = GetByTsOrder(strOpenid, qwBeforeTS, dwNeedNum, mysql, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		return iRet;
	}

	vecPBList.clear();
	for(size_t i=0; i!=m_cash_flow_list.size(); ++i)
	{
		hoosho::msg::z::CashFlowInfo pbCashFlowInfo;
		cash_flow_info_mem_2_pb(m_cash_flow_list[i], pbCashFlowInfo);
		vecPBList.push_back(pbCashFlowInfo);
	}

	return DAO_RET_OK;
}


int DaoCashFlowList::GetByTsOrder(const std::string& strOpenid, uint64_t qwBeforeTS, uint32_t dwNeedNum, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	if(0 == qwBeforeTS)
	{
		qwBeforeTS = time(0);
	}

	if(0 == dwNeedNum || dwNeedNum > 20)
	{
		dwNeedNum = 10;
	}

	std::ostringstream oss;
	oss.str("");
	oss << "select * from " << m_table_name
		<< " where openid = '" << sql_escape(strOpenid) << "'"
		<< " and create_ts < "<<qwBeforeTS
		<<" order by create_ts desc limit "<<dwNeedNum;
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(0 == mysql.GetRowCount())
	{
		strErrMsg = "not exists";
		return DAO_RET_NOT_EXIST;	
	}

	m_cash_flow_list.clear();
	while(mysql.Next())
	{
		/*
			mysql> desc t_cash_flow;
			+---------------+---------------------+------+-----+---------+-------+
			| Field         | Type                | Null | Key | Default | Extra |
			+---------------+---------------------+------+-----+---------+-------+
			| table_id      | bigint(20) unsigned | NO   | PRI | 0       |       |
			| openid        | char(128)           | NO   | MUL |         |       |
			| out_trade_no  | char(64)            | NO   |     |         |       |
			| amount        | bigint(20) unsigned | NO   |     | 0       |       |
			| add_or_reduce | tinyint(3) unsigned | NO   |     | 0       |       |
			| type          | tinyint(3) unsigned | NO   |     | 0       |       |
			| balance       | bigint(20) unsigned | NO   |     | 0       |       |
			| create_ts     | bigint(20) unsigned | NO   |     | 0       |       |
			+---------------+---------------------+------+-----+---------+-------+
		*/
		DAOCashFlow stDaoCashFlow(m_table_name);
		stDaoCashFlow.m_table_id = strtoul(mysql.GetRow(0), NULL, 10);
		stDaoCashFlow.m_openid = mysql.GetRow(1);
		stDaoCashFlow.m_outrade_no = mysql.GetRow(2);
		stDaoCashFlow.m_amount = strtoul(mysql.GetRow(3), NULL, 10);
		stDaoCashFlow.m_add_or_reduce = strtoul(mysql.GetRow(4), NULL, 10);
		stDaoCashFlow.m_flow_type = strtoul(mysql.GetRow(5), NULL, 10);
		stDaoCashFlow.m_balance = strtoul(mysql.GetRow(6), NULL, 10);
		stDaoCashFlow.m_create_ts = strtoul(mysql.GetRow(7), NULL, 10);

		m_cash_flow_list.push_back(stDaoCashFlow);
	}

	return DAO_RET_OK;
}


int DAOUserCash::GetByPrimaryKey(const std::string& strOpenid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_openid = strOpenid;
	
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from "<<m_table_name_user_cash
		<<" where openid='"<<sql_escape(m_openid)<<"'";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(0 == mysql.GetRowCount())
	{
		//new comer at the first time
		m_balance = 0;
		m_pending = 0;
	}
  	else if(mysql.Next())
	{
		m_balance = strtoul(mysql.GetRow(1), NULL, 10);
		m_pending = strtoul(mysql.GetRow(2), NULL, 10);
	}

	return DAO_RET_OK;
}

int DAOUserCash::AddBalance(const std::string& strOpenid, uint64_t qwAmount, int iReason, lce::cgi::CMysql& mysql, std::string& strErrMsg, const std::string& strOuttradeNO)
{	
	m_openid = strOpenid;
	
	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(strOpenid, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "add cash failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	m_balance += qwAmount;

	std::ostringstream oss;
	oss.str("");
	oss<<"insert into "<<m_table_name_user_cash
		<<" set openid='"<<sql_escape(m_openid)<<"'"
		<<", balance="<<m_balance
		<<", pending="<<m_pending
		<<" on duplicate key "
		<<" update balance="<<m_balance
		<<", pending="<<m_pending;
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "add cash failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	//new cash flow
	DAOCashFlow stDAOCashFlow(m_table_name_cash_flow);
	stDAOCashFlow.m_table_id = common::util::generate_unique_id();
	stDAOCashFlow.m_openid = m_openid;
	stDAOCashFlow.m_add_or_reduce = 1;
	stDAOCashFlow.m_amount = qwAmount;
	stDAOCashFlow.m_balance = m_balance;
	stDAOCashFlow.m_create_ts = time(0);
	stDAOCashFlow.m_flow_type = iReason;
	if(CASH_FLOW_TYPE_RECHARGE == iReason)
	{
		stDAOCashFlow.m_outrade_no = strOuttradeNO;
	}
	stDAOCashFlow.AddNew(mysql, strInnerErrMsg);

	return DAO_RET_OK;
}

int DAOUserCash::ReduceBalance(const std::string& strOpenid, uint64_t qwAmount, int iReason, lce::cgi::CMysql& mysql, std::string& strErrMsg, const std::string& strOutTradeNo)
{
	m_openid = strOpenid;
	
	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(strOpenid, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "reduce cash failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	if(m_balance < qwAmount)
	{
		strErrMsg = "reduce cash failed, msg=balance not enough";
		return DAO_RET_FAIL;
	}

	m_balance -= qwAmount;
	if(CASH_FLOW_TYPE_OUTCOME == iReason)
	{
		m_pending += qwAmount;
	}

	std::ostringstream oss;
	oss.str("");
	oss<<"insert into "<<m_table_name_user_cash
		<<" set openid='"<<sql_escape(m_openid)<<"'"
		<<", balance="<<m_balance
		<<", pending="<<m_pending
		<<" on duplicate key "
		<<" update balance="<<m_balance
		<<", pending="<<m_pending;
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "reduce cash failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	//new cash flow
	DAOCashFlow stDAOCashFlow(m_table_name_cash_flow);
	stDAOCashFlow.m_table_id = common::util::generate_unique_id();
	stDAOCashFlow.m_openid = m_openid;
	if(CASH_FLOW_TYPE_CONSUME_ORDER_CREATE == iReason && !strOutTradeNo.empty())
	{
		stDAOCashFlow.m_outrade_no = strOutTradeNo;
	}
	stDAOCashFlow.m_add_or_reduce = 2;
	stDAOCashFlow.m_amount = qwAmount;
	stDAOCashFlow.m_balance = m_balance;
	stDAOCashFlow.m_create_ts = time(0);
	stDAOCashFlow.m_flow_type = iReason;
	stDAOCashFlow.AddNew(mysql, strInnerErrMsg);
	
	return DAO_RET_OK;
}

int DAOUserCash::ReducePending(const std::string& strOpenid, uint64_t qwAmount, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_openid = strOpenid;
	
	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(strOpenid, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "reduce pending failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	if(m_pending < qwAmount)
	{
		strErrMsg = "reduce pending failed, msg=balance not enough";
		return DAO_RET_FAIL;
	}

	m_pending -= qwAmount;

	std::ostringstream oss;
	oss.str("");
	oss<<"insert into "<<m_table_name_user_cash
		<<" set openid='"<<sql_escape(m_openid)<<"'"
		<<", balance="<<m_balance
		<<", pending="<<m_pending
		<<" on duplicate key "
		<<" update balance="<<m_balance
		<<", pending="<<m_pending;
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "reduce pending failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}
	
	return DAO_RET_OK;
}

int DaoOutcome::GetByPrimaryKey(uint64_t qwtableId, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_table_id = qwtableId;
	
	std::ostringstream oss;
	oss.str("");
	oss<<"select * from "<<m_table_name_outcome
		<<" where table_id = " << m_table_id;

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg=";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() &&mysql.Next())
	{
		m_openid = mysql.GetRow(1);
		m_amount = strtoul(mysql.GetRow(2), NULL, 10);
		m_state = strtoul(mysql.GetRow(3), NULL, 10);
		m_create_ts = strtoul(mysql.GetRow(4), NULL, 10);
		return DAO_RET_OK;
	}

	strErrMsg = "not exists";
	return DAO_RET_NOT_EXIST;
}

int DaoOutcome::Commit(const std::string& strOpenid, uint64_t qwAmount, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_openid = strOpenid;
	m_amount = qwAmount;
	
	std::string strInnerErrMsg = "";
	DAOUserCash stDAOUserCash(m_table_name_usercash, m_table_name_cashflow);
	if(stDAOUserCash.ReduceBalance(strOpenid, qwAmount, CASH_FLOW_TYPE_OUTCOME, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "Commit failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	m_table_id = common::util::generate_unique_id();
	m_state = OUTCOME_STATE_WAITING;
	std::ostringstream oss;
	oss.str("");
	oss << "insert into " << m_table_name_outcome
			<< " set openid = '" << sql_escape(m_openid)<<"'"
			<< ", table_id = " << m_table_id
			<< ", amount = " << m_amount
			<< ", state = " << m_state
			<< ", create_ts = unix_timestamp()";
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoOutcome::Finish(uint64_t qwTableId, bool bSucc, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::string strInnerErrMsg = "";
	if(GetByPrimaryKey(qwTableId, mysql, strInnerErrMsg) != DAO_RET_OK)
	{
		strErrMsg = "finish failed, msg=";
		strErrMsg += strInnerErrMsg;
		return DAO_RET_FAIL;
	}

	if(OUTCOME_STATE_SUCC == m_state)
	{
		return DAO_RET_OK;
	}

	if(bSucc)
	{
		//user cash
		DAOUserCash stDAOUserCash(m_table_name_usercash, m_table_name_cashflow);
		if(stDAOUserCash.ReducePending(m_openid, m_amount, mysql, strInnerErrMsg) != DAO_RET_OK)
		{
			strErrMsg = "finish failed, msg=";
			strErrMsg += strInnerErrMsg;
			return DAO_RET_FAIL;
		}
	}

	m_state = bSucc? OUTCOME_STATE_SUCC : OUTCOME_STATE_FAIL;
	std::ostringstream oss;
	oss.str("");
	oss<<"update "<<m_table_name_outcome
		<<" set state="<<m_state
		<<" where table_id="<<m_table_id;
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoOutcomeList::GetByStateWithTsOrder(const std::string& strOpenid
										, uint32_t dwState
										, uint64_t qwBeforeTS
										, uint32_t dwNeedNum
										, lce::cgi::CMysql& mysql
										, std::string& strErrMsg
										, std::vector<hoosho::msg::z::OutcomeReqInfo>& vecPBList)
{
	int iRet = GetByStateWithTsOrder(strOpenid, dwState, qwBeforeTS, dwNeedNum, mysql, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		return iRet;
	}

	vecPBList.clear();
	for(size_t i=0; i!=m_outcome_list.size(); ++i)
	{
		hoosho::msg::z::OutcomeReqInfo pbOutcomeReqInfo;
		out_come_req_info_mem_2_pb(m_outcome_list[i], pbOutcomeReqInfo);
		vecPBList.push_back(pbOutcomeReqInfo);
	}

	return DAO_RET_OK;
}


int DaoOutcomeList::GetByStateWithTsOrder(const std::string& strOpenid
										, uint32_t dwState
										, uint64_t qwBeforeTS
										, uint32_t dwNeedNum
										, lce::cgi::CMysql& mysql
										, std::string& strErrMsg)
{
	if(0 == qwBeforeTS)
	{
		qwBeforeTS = time(0);
	}

	if(0 == dwNeedNum || dwNeedNum > 20)
	{
		dwNeedNum = 10;
	}

	string strStateCond = (0==dwState)?(" state!=0"):(" state="+int_2_str(dwState));

	std::ostringstream oss;
	oss.str("");
	oss << "select * from " << m_table_name_outcome
		<< " where openid = '" << sql_escape(strOpenid) << "'"
		<<" and "<<strStateCond
		<< " and create_ts < "<<qwBeforeTS
		<<" order by create_ts desc limit "<<dwNeedNum;
	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query failed, msg = ";
		strErrMsg += mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(0 == mysql.GetRowCount())
	{
		strErrMsg = "nont exists";
		return DAO_RET_NOT_EXIST;	
	}

	m_outcome_list.clear();
	while(mysql.Next())
	{
		/*
			mysql> desc t_outcome_req;
			+-----------+---------------------+------+-----+---------+-------+
			| Field     | Type                | Null | Key | Default | Extra |
			+-----------+---------------------+------+-----+---------+-------+
			| table_id  | bigint(20) unsigned | NO   | PRI | 0       |       |
			| openid    | char(128)           | NO   | MUL |         |       |
			| amount    | bigint(20) unsigned | NO   |     | 0       |       |
			| state     | bigint(20) unsigned | NO   |     | 0       |       |
			| create_ts | bigint(20) unsigned | NO   |     | 0       |       |
			+-----------+---------------------+------+-----+---------+-------+
		*/
		DaoOutcome stDaoOutcome(m_table_name_outcome, m_table_name_usercash, m_table_name_cashflow);
		stDaoOutcome.m_table_id = strtoul(mysql.GetRow(0), NULL, 10);
		stDaoOutcome.m_openid = mysql.GetRow(1);
		stDaoOutcome.m_amount = strtoul(mysql.GetRow(2), NULL, 10);
		stDaoOutcome.m_state = strtoul(mysql.GetRow(3), NULL, 10);
		stDaoOutcome.m_create_ts = strtoul(mysql.GetRow(4), NULL, 10);

		m_outcome_list.push_back(stDaoOutcome);
	}

	return DAO_RET_OK;
}

int DaoOrderFavorInfo::AddFavor(const std::string& strOpenid, uint64_t qwOrderid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "INSERT INTO " << m_table_name 
		<< " SET openid = '" << sql_escape(strOpenid) << "'"
		<< ", orderid = " << qwOrderid
		<< ", create_ts = unix_timestamp() "
		<< " ON DUPLICATE KEY "
		<< " UPDATE create_ts = unix_timestamp()";
	SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
	return DAO_RET_OK;
}


int DaoOrderFavorInfo::DelFavor(const std::string& strOpenid, uint64_t qwOrderid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "DELETE FROM " << m_table_name
		<< " WHERE openid = '" << sql_escape(strOpenid) << "'"
		<< " AND orderid = " << qwOrderid;
	SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
	return DAO_RET_OK;
}

int DaoOrderFavorInfo::GetByPrimaryKey(const std::string &strOpenid
										, uint64_t qwOrderid
									    , hoosho::msg::z::OrderFavorInfo &stFavorInfo
										, lce::cgi::CMysql &mysql
										, std::string &strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "SELECT * FROM " << m_table_name
		<< " WHERE openid = '" << sql_escape(strOpenid) << "'"
		<< " AND orderid = " << qwOrderid;
	SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
	if(mysql.GetRowCount() && mysql.Next())
	{
		order_favor_info_db_2_pb(mysql, stFavorInfo);
		return DAO_RET_OK;
	}
	strErrMsg = "openid = " + strOpenid + " don't favor orderid = " + int_2_str(qwOrderid);
	return DAO_RET_FAIL;
}

int DaoOrderFavorInfoList::GetFavorList(uint64_t qwOrderid, uint64_t qwBeginTs, uint32_t dwNeedNum, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	if(qwBeginTs == 0)
	{
		qwBeginTs = 0xFFFFFFFFFFFFFFFF;
	}
	if(dwNeedNum == 0 || dwNeedNum > 20)
	{
		dwNeedNum = 20;
	}
	std::ostringstream oss;
	oss.str("");
	oss << "SELECT * FROM " << m_table_name
		<< " WHERE orderid = " << qwOrderid
		<< " AND create_ts < " << qwBeginTs
		<< " ORDER BY create_ts DESC"
		<< " LIMIT " << dwNeedNum;
	
	SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
	if(mysql.GetRowCount() == 0)
	{
		strErrMsg = "not exist";
		return DAO_RET_NOT_EXIST;
	}
	hoosho::msg::z::OrderFavorInfo stOrderFavorInfo;
	m_favor_list.clear();
	while(mysql.Next())
	{
		order_favor_info_db_2_pb(mysql, stOrderFavorInfo);
		m_favor_list.push_back(stOrderFavorInfo);
	}
	return DAO_RET_OK;
}


int DaoOrderFollowInfo::AddComment(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	int iRet = AddFollow(mysql, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		return iRet;
	}

	ostringstream oss;
	oss.str("");
	oss << "INSERT INTO " << m_table_name_comment_index
		<< " SET orderid = " << m_follow_info.orderid()
		<< ", comment_id = " << m_follow_info.follow_id();
	
	SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
	return DAO_RET_OK;
}

int DaoOrderFollowInfo::AddReply(lce::cgi::CMysql& mysql, std::string& strErrMsg, std::string& strOriginCommentOpenid)
{
	//save
	::hoosho::msg::z::OrderFollowInfo stFollowInfo;
	stFollowInfo.CopyFrom(m_follow_info);

	int iRet;
	iRet = GetByPrimaryKey(m_follow_info.origin_comment_id(), mysql, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		if(iRet == DAO_RET_NOT_EXIST)
		{
			strErrMsg += ", origin_comment_id not found";
		}
		return iRet;
	}
	strOriginCommentOpenid = m_follow_info.openid_from();
	//
	m_follow_info.CopyFrom(stFollowInfo);

	iRet = AddFollow(mysql, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		return iRet;
	}

	ostringstream oss;
	oss.str("");
	oss << "INSERT INTO " << m_table_name_reply_index
		<< " SET origin_comment_id = " << m_follow_info.origin_comment_id()
		<< ", reply_id = " << m_follow_info.follow_id();
	
	SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
	return DAO_RET_OK;
}

int DaoOrderFollowInfo::AddFollow(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "INSERT INTO " << m_table_name_follow_info
		<< " SET follow_id = " << m_follow_info.follow_id()
		<< ", content = '" << sql_escape(m_follow_info.content()) << "'"
		<< ", orderid = " << m_follow_info.orderid()
		<< ", type = " << m_follow_info.type()
		<< ", origin_comment_id = " << m_follow_info.origin_comment_id()
		<< ", openid_from = '" << sql_escape(m_follow_info.openid_from()) << "'"
		<< ", openid_to = '" << sql_escape(m_follow_info.openid_to()) << "'"
		<< ", create_ts = " << m_follow_info.create_ts()
		<< ", del_ts = " << m_follow_info.del_ts();

	SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
	return DAO_RET_OK;
}

int DaoOrderFollowInfo::DelFollow(const std::string& strOpenid, uint64_t qwFollowId, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	int iRet = GetByPrimaryKey(qwFollowId, mysql, strErrMsg);
	if(iRet != DAO_RET_OK)
	{
		return iRet;
	}

	if(m_follow_info.openid_from() != strOpenid)
	{
		strErrMsg = "follow_id = ";
		strErrMsg += int_2_str(m_follow_info.follow_id());
		strErrMsg += " own by openid = ";
		strErrMsg += m_follow_info.openid_from();
		strErrMsg += ", not openid = ";
		strErrMsg += strOpenid;
		return DAO_RET_INVALID;
	}

	std::ostringstream oss;
	oss.str("");
	oss << "UPDATE " << m_table_name_follow_info
		<< " SET del_ts = unix_timestamp() "
		<< " WHERE follow_id = " << qwFollowId;

	SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);

	if(m_follow_info.type() == ORDER_FOLLOW_TYPE_COMMMENT)
	{
		oss.str("");
		oss << "DELETE FROM " << m_table_name_comment_index
			<< " WHERE comment_id = " << m_follow_info.follow_id();
		SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
	}
	else if(m_follow_info.type() == ORDER_FOLLOW_TYPE_REPLY)
	{
		oss.str("");
		oss << "DELETE FROM " << m_table_name_reply_index
			<< " WHERE reply_id = " << m_follow_info.follow_id();
		SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
	}
	else 
	{
		strErrMsg = "invalid data in db, type can't be ";
		strErrMsg += int_2_str(m_follow_info.type());
		return DAO_RET_FAIL;
	}
	
	return DAO_RET_OK;
}


int DaoOrderFollowInfo::GetByPrimaryKey(uint64_t qwFollowId, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "SELECT * FROM " << m_table_name_follow_info
		<< " WHERE follow_id = " << qwFollowId;

	SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);	
	if(mysql.GetRowCount() && mysql.Next())
	{
		order_follow_info_db_2_pb(mysql, m_follow_info);
		return DAO_RET_OK;
	}
	strErrMsg = "not exist follow_id = " + int_2_str(qwFollowId);
	return DAO_RET_NOT_EXIST;
}

int DaoOrderCommentInfoList::GetCommentList(uint64_t qwOrderid, uint64_t qwBeginCommentid, uint32_t dwNeedNum, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	if(qwBeginCommentid == 0)
	{
		qwBeginCommentid = 0xFFFFFFFFFFFFFFFF;
	}
	if(dwNeedNum == 0 || dwNeedNum > 20)
	{
		dwNeedNum = 20;
	}
	std::ostringstream oss;
	oss.str("");
	oss << "SELECT * FROM " << m_table_name_comment_index
		<< " WHERE orderid = " << qwOrderid
		<< " AND comment_id < " << qwBeginCommentid
		<< " ORDER BY comment_id DESC"
		<< " LIMIT " << dwNeedNum;
	SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
	if(mysql.GetRowCount() == 0)
	{
		strErrMsg = "not exist";
		return DAO_RET_NOT_EXIST;
	}

	std::vector<uint64_t> vecCommentid;
	uint64_t qwCommentid;
	while(mysql.Next())
	{
		qwCommentid = strtoul(mysql.GetRow(1), NULL, 10);
		vecCommentid.push_back(qwCommentid);
	}

	return GetCommentDetail(vecCommentid, mysql, strErrMsg);
}

int DaoOrderCommentInfoList::GetCommentDetail(std::vector<uint64_t> vecCommentid, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_comment_list.clear();
	std::ostringstream oss;
	int iRet;

	for(size_t i = 0; i < vecCommentid.size(); i++)
	{
		hoosho::msg::z::OrderCommentInfo stOrderCommentInfo;

		iRet = GetByPrimaryKey(vecCommentid[i], mysql, strErrMsg);
		if(iRet == DAO_RET_NOT_EXIST)
		{
			continue;		
		}
		if(iRet != DAO_RET_OK)
		{
			return iRet;
		}

		hoosho::msg::z::OrderFollowInfo *pOrderFollowInfo = stOrderCommentInfo.mutable_comment();
		pOrderFollowInfo->CopyFrom(m_follow_info);

		oss.str("");
		oss << "SELECT * FROM " << m_table_name_reply_index
			<< " WHERE origin_comment_id = " << vecCommentid[i];

		SQL_QUERY_FAIL_RETURN(mysql, oss, strErrMsg);
		std::vector<uint64_t> vecReplyid;
		uint64_t qwReplyid;
		while(mysql.GetRowCount() && mysql.Next())
		{
			qwReplyid = strtoul(mysql.GetRow(1), NULL, 10);
			vecReplyid.push_back(qwReplyid);
		}

		for(size_t i = 0; i < vecReplyid.size(); i++)
		{
			iRet = GetByPrimaryKey(vecReplyid[i], mysql, strErrMsg);
			if(iRet == DAO_RET_NOT_EXIST)
			{
				continue;
			}
			if(iRet != DAO_RET_OK)
			{
				return iRet;
			}
			stOrderCommentInfo.add_reply_list()->CopyFrom(m_follow_info);
		}
		m_comment_list.push_back(stOrderCommentInfo);
	}
	return DAO_RET_OK;
}


int DaoMsg::AddNewMsg(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	// 1.insert into t_msg_content
	std::ostringstream oss;
	oss.str("");
	oss<<"INSERT INTO t_msg_content"
	   <<" SET msg_id="<<m_msg_content.msg_id()
	   <<", content='"<<sql_escape(m_msg_content.content())<<"'"
	   <<", openid_from='"<<sql_escape(m_msg_content.openid_from())<<"'"
	   <<", openid_to='"<<sql_escape(m_msg_content.openid_to())<<"'"
	   <<", create_ts="<<m_msg_content.create_ts();	

	if(!mysql.Query(oss.str()))
    {
        strErrMsg = "mysql query error, msg=";
        strErrMsg += mysql.GetErrMsg();
        return DAO_RET_FAIL;
    }

    // 2.insert into t_session_msg
    uint64_t session_id = common::util::generate_session_id_bystr(m_msg_content.openid_from(), 
    														m_msg_content.openid_to());

    oss.str("");
    oss<<"INSERT INTO t_session_msg"
       <<" SET session_id="<<session_id
       <<", msg_id="<<m_msg_content.msg_id()
       <<", del_status="<<MSG_DEL_STATUS_NODEL
       <<", create_ts="<<m_msg_content.create_ts();    

    if(!mysql.Query(oss.str()))
    {
        strErrMsg = "mysql query error, msg=";
        strErrMsg += mysql.GetErrMsg();
        return DAO_RET_FAIL;
    }

    // 3.insert into t_user_session     
    std::string strOpenidA = MIN_OPENID(m_msg_content.openid_from(), m_msg_content.openid_to());
    std::string strOpenidB = MAX_OPENID(m_msg_content.openid_from(), m_msg_content.openid_to());
    int iNewMsgStatus = 0x00;
    if(strOpenidA == m_msg_content.openid_from())
    {
		iNewMsgStatus = MSG_NEW_STATUS_MAX_TRUE;
    }
    else
    {
		iNewMsgStatus = MSG_NEW_STATUS_MIN_TRUE;
    }
    
    oss.str("");
    oss<<"INSERT INTO t_user_session"
       <<" SET openid_a='"<<sql_escape(strOpenidA)<<"'"
       <<", openid_b='"<<sql_escape(strOpenidB)<<"'"
       <<", session_id="<<session_id
       <<", content='"<<sql_escape(m_msg_content.content())<<"'"
       <<", newmsg_status=newmsg_status | "<<iNewMsgStatus
       <<", del_status="<<MSG_DEL_STATUS_NODEL
       <<", create_ts="<<m_msg_content.create_ts()
       <<" ON DUPLICATE KEY UPDATE"
       <<" content='"<<sql_escape(m_msg_content.content())<<"'"
       <<", newmsg_status=newmsg_status | "<<iNewMsgStatus
       <<", del_status="<<MSG_DEL_STATUS_NODEL
       <<", create_ts="<<m_msg_content.create_ts();    

    if(!mysql.Query(oss.str()))
    {
        strErrMsg = "mysql query error, msg=";
        strErrMsg += mysql.GetErrMsg();
        return DAO_RET_FAIL;
    }   

	// 4.set redpoint
    DaoRedPoint stDaoRedPoint;   
    
	return stDaoRedPoint.Add(m_msg_content.openid_to(), RED_POINT_TYPE_MSG, mysql, strErrMsg);
}

int DaoMsg::GetMsgList(uint64_t qwSessionId, uint64_t qwBeginMsgId, uint32_t iLen, int iDelStatus, std::vector<hoosho::msg::z::MsgContent>& vecMsgContent, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	if(qwBeginMsgId == 0)
	{
		qwBeginMsgId = 0xFFFFFFFFFFFFFFFF;
	}
	if(iLen == 0 || iLen > 20)
	{
		iLen = 20;
	}
	
	std::ostringstream oss;
	oss.str("");
	oss<<"SELECT msg_id FROM t_session_msg"
	   <<" WHERE session_id = "<<qwSessionId
	   <<" AND msg_id < "<<qwBeginMsgId
	   <<" AND del_status != "<<iDelStatus
	   <<" ORDER BY create_ts DESC LIMIT "<<iLen;	

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	std::vector<uint64_t> vecMsgId;
	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			vecMsgId.push_back(common::util::charToUint64_t(mysql.GetRow(0)));
		}
	}
	else
	{
		strErrMsg = "SELECT E_NOT_EXIST: " + oss.str();
		return DAO_RET_NOT_EXIST;
	}
	
	return GetMsgDetail(vecMsgId, vecMsgContent, mysql, strErrMsg);
}

int DaoMsg::GetMsgDetail(std::vector<uint64_t>& vecMsgId, std::vector<hoosho::msg::z::MsgContent>& vecMsgContent, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;

	for(size_t i=0; i<vecMsgId.size(); i++)
	{
		oss.str("");
		oss<<"SELECT * FROM t_msg_content WHERE msg_id = "<<vecMsgId[i];

		if(!mysql.Query(oss.str()))
		{
			strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
			return DAO_RET_FAIL;
		}

		if(mysql.GetRowCount())
		{
			while(mysql.Next())
			{
				::hoosho::msg::z::MsgContent stMsgContent;
				stMsgContent.set_msg_id(common::util::charToUint64_t(mysql.GetRow(0)));
				stMsgContent.set_content(mysql.GetRow(1));
				stMsgContent.set_openid_from(mysql.GetRow(2));
				stMsgContent.set_openid_to(mysql.GetRow(3));
				stMsgContent.set_create_ts(common::util::charToUint64_t(mysql.GetRow(4)));

				vecMsgContent.push_back(stMsgContent);
			}
		}
		else
		{
			strErrMsg = "SELECT E_NOT_EXIST: " + oss.str();
			//return DAO_RET_NOT_EXIST;
		}
		
	}

	return DAO_RET_OK;
}

int DaoMsg::ClearUserSessionNewMsgStatus(const std::string& strOpenidFrom, const std::string& strOpenidTo, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::string strOpenidA = MIN_OPENID(strOpenidFrom, strOpenidTo);
    std::string strOpenidB = MAX_OPENID(strOpenidFrom, strOpenidTo);
    
	std::ostringstream oss;
	oss.str("");
	oss<<"UPDATE t_user_session SET newmsg_status = 0"
	   <<" WHERE openid_a='"<<sql_escape(strOpenidA)<<"'"
	   <<" AND openid_b='"<<sql_escape(strOpenidB)<<"'";

	if(!mysql.Query(oss.str()))
    {
        strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
        return DAO_RET_FAIL;
    }

	//check if all session newmsg_status clear, if true, clear redpoint
	uint32_t iRetNewMsgStatus = 0;
	if(GetNewMsgStatus(strOpenidFrom, iRetNewMsgStatus, mysql, strErrMsg) != DAO_RET_OK)
	{
		return DAO_RET_FAIL;
	}
	else
	{
		if(iRetNewMsgStatus == 0)
		{
			DaoRedPoint stDaoRedPoint;
			return stDaoRedPoint.ClearValue(strOpenidFrom, RED_POINT_TYPE_MSG, mysql, strErrMsg);
		}
	}
	
    return DAO_RET_OK;
}

int DaoMsg::GetSessionList(const std::string& strOpenid, uint64_t qwBeginTs, uint32_t iLen, 
						std::set<std::string>& strOpenidSet,
						std::vector<hoosho::msg::z::Session>& vecSession, 
						lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	if(qwBeginTs == 0)
	{
		qwBeginTs = 0xFFFFFFFFFFFFFFFF;
	}
	if(iLen == 0 || iLen > 20)
	{
		iLen = 20;
	}
	std::ostringstream oss;
	oss.str("");
	oss<<"SELECT * FROM t_user_session "
	   <<"WHERE create_ts < "<<qwBeginTs
	   <<" AND ((openid_a = '"<<sql_escape(strOpenid)<<"'"
	   <<" AND del_status & "<<MSG_DEL_STATUS_BY_MIN<<" = 0)"
	   <<" OR (openid_b = '"<<sql_escape(strOpenid)<<"'"
	   <<" AND del_status & "<<MSG_DEL_STATUS_BY_MAX<<" = 0))"
	   <<" ORDER BY create_ts DESC LIMIT "<<iLen;

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::z::Session stSession;			
			stSession.set_session_id(common::util::charToUint64_t(mysql.GetRow(0)));
			stSession.set_openid_from(strOpenid);
			if(strcmp(mysql.GetRow(1), strOpenid.c_str()) == 0)
			{
				stSession.set_openid_to(mysql.GetRow(2));
				strOpenidSet.insert(mysql.GetRow(2));
			}
			else
			{
				stSession.set_openid_to(mysql.GetRow(1));
				strOpenidSet.insert(mysql.GetRow(1));
			}
			stSession.set_content(mysql.GetRow(3));
			stSession.set_newmsg_status(common::util::charToUint64_t(mysql.GetRow(4)));
			stSession.set_del_status(common::util::charToUint64_t(mysql.GetRow(5)));
			stSession.set_create_ts(common::util::charToUint64_t(mysql.GetRow(6)));

			vecSession.push_back(stSession);			
		}
	}
	else
	{
		strErrMsg = "SELECT E_NOT_EXIST: " + oss.str();
		return DAO_RET_NOT_EXIST;
	}

	return DAO_RET_OK;	   
}


int DaoMsg::DelSession(const std::string& strOpenidFrom, const std::string& strOpenidTo, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::string strOpenidA = MIN_OPENID(strOpenidFrom, strOpenidTo);
    std::string strOpenidB = MAX_OPENID(strOpenidFrom, strOpenidTo);
    uint64_t qwSessionId = common::util::generate_session_id_bystr(strOpenidFrom, strOpenidTo);

	int iDelStatus = MSG_DEL_STATUS_NODEL;
    if(strOpenidA == strOpenidFrom)
    {
		iDelStatus = MSG_DEL_STATUS_BY_MIN;
    }
    else
    {
		iDelStatus = MSG_DEL_STATUS_BY_MAX;
    }
    
	std::ostringstream oss;
	oss.str("");
	oss<<"UPDATE t_user_session"
	   <<" SET del_status = del_status | "<<iDelStatus
	   <<" WHERE session_id = "<<qwSessionId
	   <<" AND openid_a = '"<<sql_escape(strOpenidA)<<"'"
	   <<" AND openid_b = '"<<sql_escape(strOpenidB)<<"'";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DelMsg(qwSessionId, iDelStatus, mysql, strErrMsg);
}

int DaoMsg::DelMsg(uint64_t qwSessionId, int iDelStatus, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss<<"UPDATE t_session_msg"
	   <<" SET del_status = del_status | "<<iDelStatus
	   <<" WHERE session_id = "<<qwSessionId;

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoMsg::GetNewMsgStatus(const std::string& strOpenid, uint32_t& iNewMsgStatus, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"SELECT * FROM t_user_session "
	   <<" WHERE ((openid_a = '"<<sql_escape(strOpenid)<<"'"
	   <<" AND del_status & "<<MSG_DEL_STATUS_BY_MIN<<" = 0"
	   <<" AND newmsg_status & "<<MSG_NEW_STATUS_MIN_TRUE<<" = 1)"
	   <<" OR (openid_b = '"<<sql_escape(strOpenid)<<"'"
	   <<" AND del_status & "<<MSG_DEL_STATUS_BY_MAX<<" = 0"
	   <<" AND newmsg_status &"<<MSG_NEW_STATUS_MAX_TRUE<<" = 1))";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount() && mysql.Next())
	{
		iNewMsgStatus = 1;
	}
	else
	{
		iNewMsgStatus = 0;
	}

	return DAO_RET_OK;
}

int DaoRedPoint::Add(const std::string& strOpenid, uint32_t type, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"INSERT INTO t_red_point"
	   <<" SET openid='"<<sql_escape(strOpenid)<<"'"
	   <<", type="<<type
	   <<", value=1"
	   <<" ON DUPLICATE KEY UPDATE"
	   <<" value=value+1";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoRedPoint::Get(const std::string& strOpenid, std::vector<hoosho::msg::z::RedPointInfo>& vecRedPointInfo, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"SELECT type,value FROM t_red_point"
	   <<" WHERE openid='"<<sql_escape(strOpenid)<<"'";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::z::RedPointInfo stRedPointInfo;
			stRedPointInfo.set_type(common::util::charToUint64_t(mysql.GetRow(0)));
			stRedPointInfo.set_value(common::util::charToUint64_t(mysql.GetRow(1)));

			vecRedPointInfo.push_back(stRedPointInfo);
		}
	}

	return DAO_RET_OK;
}

int DaoRedPoint::ClearValue(const std::string& strOpenid, uint32_t type, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"INSERT INTO t_red_point"
	   <<" SET openid='"<<sql_escape(strOpenid)<<"'"
	   <<", type="<<type
	   <<", value=0"
	   <<" ON DUPLICATE KEY UPDATE"
	   <<" value=0";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoRedPoint::Reduce(const std::string& strOpenid, uint32_t type, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"UPDATE t_red_point"
	   <<" SET value=value-1"
	   <<" WHERE openid='"<<sql_escape(strOpenid)<<"'"
	   <<" AND type="<<type
	   <<" AND value>0";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;
}

int DaoNoticeInfo::Add(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"INSERT INTO t_user_notice"
	   <<" SET table_id="<<m_notice_info.table_id()
	   <<", openid='"<<sql_escape(m_notice_info.openid())<<"'"
	   <<", type="<<m_notice_info.type()
	   <<", status="<<m_notice_info.status()
	   <<", extra_data_0='"<<sql_escape(m_notice_info.extra_data_0())<<"'"
	   <<", extra_data_1='"<<sql_escape(m_notice_info.extra_data_1())<<"'"
	   <<", extra_data_2='"<<sql_escape(m_notice_info.extra_data_2())<<"'"
	   <<", create_ts="<<m_notice_info.create_ts();

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	DaoRedPoint stDaoRedPoint;
	return stDaoRedPoint.Add(m_notice_info.openid(), m_notice_info.type(), mysql, strErrMsg);	   
}

int DaoNoticeInfo::Del(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"DELETE FROM t_user_notice"
	   <<" WHERE table_id="<<m_notice_info.table_id();	   

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	DaoRedPoint stDaoRedPoint;
	return stDaoRedPoint.Reduce(m_notice_info.openid(), m_notice_info.type(), mysql, strErrMsg);
}

int DaoNoticeInfo::DelNoticeFavorite(uint64_t qwOrderId, std::string& strOpenidFavor, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"SELECT * FROM t_user_notice"
	   <<" WHERE type="<<RED_POINT_TYPE_FAVORITE
	   <<" AND extra_data_0='"<<sql_escape(int_2_str(qwOrderId))<<"'"
	   <<" AND extra_data_1='"<<sql_escape(strOpenidFavor)<<"'";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{	
			notice_info_db_2_pb(mysql, m_notice_info);			
		}
	}
	else
	{
		strErrMsg = "delete notice favorite info not exist";
		return DAO_RET_FAIL;
	}

	return Del(mysql, strErrMsg);
}

int DaoNoticeInfo::DelNoticeFollow(uint64_t qwOrderId, uint64_t qwFollowId, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"SELECT * FROM t_user_notice"
	   <<" WHERE type="<<RED_POINT_TYPE_COMMENT
	   <<" AND extra_data_0='"<<sql_escape(int_2_str(qwOrderId))<<"'"
	   <<" AND extra_data_2='"<<sql_escape(int_2_str(qwFollowId))<<"'";

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{			
			notice_info_db_2_pb(mysql, m_notice_info);				
		}
	}
	else
	{
		strErrMsg = "delete notice comment info not exist";
		return DAO_RET_FAIL;
	}
	
	return Del(mysql, strErrMsg);
}

int DaoNoticeInfo::ClearStatus(const std::string& strOpenid, uint32_t type, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss<<"UPDATE t_user_notice SET status=1"
	   <<" WHERE openid='"<<sql_escape(strOpenid)<<"'"
	   <<" AND type="<<type;

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	return DAO_RET_OK;	  
}

int DaoNoticeInfo::GetByType(const std::string& strOpenid, uint32_t type, uint64_t qwBeginTs, uint32_t iLen, std::vector<hoosho::msg::z::NoticeInfo>& vecNoticeInfo, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	if(qwBeginTs == 0)
	{
		qwBeginTs = 0xFFFFFFFFFFFFFFFF;
	}
	if(iLen == 0 || iLen > 20)
	{
		iLen = 20;
	}
	std::ostringstream oss;
	oss.str("");
	oss<<"SELECT * FROM t_user_notice"
	   <<" WHERE openid='"<<sql_escape(strOpenid)<<"'"
	   <<" AND type="<<type
	   <<" AND create_ts<"<<qwBeginTs
	   <<" ORDER BY create_ts LIMIT "<<iLen;

	if(!mysql.Query(oss.str()))
	{
		strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
	}

	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{
			::hoosho::msg::z::NoticeInfo stNoticeInfo;
			stNoticeInfo.set_table_id(common::util::charToUint64_t(mysql.GetRow(0)));
			stNoticeInfo.set_openid(mysql.GetRow(1));
			stNoticeInfo.set_type(common::util::charToUint64_t(mysql.GetRow(2)));
			stNoticeInfo.set_status(common::util::charToUint64_t(mysql.GetRow(3)));
			stNoticeInfo.set_extra_data_0(mysql.GetRow(4));
			stNoticeInfo.set_extra_data_1(mysql.GetRow(5));
			stNoticeInfo.set_extra_data_2(mysql.GetRow(6));
			stNoticeInfo.set_create_ts(common::util::charToUint64_t(mysql.GetRow(7)));			
			
			vecNoticeInfo.push_back(stNoticeInfo);
		}
	}

	return ClearStatus(strOpenid, type, mysql, strErrMsg);
}

int DaoLotteryInfo::AddNewLotteryInfo(const std::string& strOpenid, uint32_t iPrize, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	uint64_t qwTableId = common::util::generate_unique_id();
	
	std::ostringstream oss;	
	oss.str("");
	oss<<"insert into t_user_lottery"
	   <<" set table_id="<<qwTableId
	   <<", openid='"<<sql_escape(strOpenid)<<"'"
	   <<", prize="<<iPrize
	   <<", create_ts=unix_timestamp()";

	if(!mysql.Query(oss.str()))
    {
        strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
    }

    oss.str("");
    oss<<"update t_user_extra_info set available_lottery_time=available_lottery_time - 1"
       <<" where openid='"<<sql_escape(strOpenid)<<"'"
       <<" and available_lottery_time>0";

    if(!mysql.Query(oss.str()))
    {
        strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
    }

    return DAO_RET_OK;
}

int DaoLotteryInfo::GetUserAvailableLotteryTime(const std::string& strOpenid, uint32_t& times, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream oss;	
	oss.str("");
	oss<<"select available_lottery_time from t_user_extra_info where openid='"<<sql_escape(strOpenid)<<"'";

	if(!mysql.Query(oss.str()))
    {
        strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
    }

    if(mysql.GetRowCount() && mysql.Next())
    {
    	times = strtoul(mysql.GetRow(0), NULL, 10);
    }		
    else
    {
		times = 0;
    }

    return DAO_RET_OK;
}

int DaoLotteryInfo::GetUserLotteryRecordList(const std::string& strOpenid, uint64_t qwBeginTs, uint32_t iLen, std::vector<hoosho::msg::z::LotteryInfo>& vecLotteryInfoList, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	if(qwBeginTs == 0)
		qwBeginTs = 0xFFFFFFFFFFFFFFFF;

	std::ostringstream oss;	
	oss.str("");
	oss<<"select prize,prize_desc,create_ts from t_user_lottery"
	   <<" where openid='"<<sql_escape(strOpenid)<<"'"
	   <<" and create_ts<"<<qwBeginTs
	   <<" order by create_ts limit "<<iLen;

	if(!mysql.Query(oss.str()))
    {
        strErrMsg = "mysql query error, sql=" + oss.str() + ", msg=" + mysql.GetErrMsg();
		return DAO_RET_FAIL;
    }

    if(mysql.GetRowCount())
    {
		while(mysql.Next())
		{
			::hoosho::msg::z::LotteryInfo stLotteryInfo;
			lottery_info_db_2_pb(mysql, stLotteryInfo);
			vecLotteryInfoList.push_back(stLotteryInfo);
		}
    }

    return DAO_RET_OK;
}
