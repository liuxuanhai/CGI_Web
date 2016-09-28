#ifndef _CGI_PA_COMMON_UTIL_H_
#define _CGI_PA_COMMON_UTIL_H_

#include "util/lce_util.h"
#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"

enum CGI_RET_CODE
{
	CGI_RET_CODE_OK = 0,
	CGI_RET_CODE_NO_LOGIN = 100001, //未登录
	CGI_RET_CODE_SERVER_BUSY = 100002, //服务器
	CGI_RET_CODE_INVALID_PARAM = 100003, //参数无效
	CGI_RET_CODE_INVALID_PASSWORD = 100004, //密码错误
	CGI_RET_CODE_NOT_EXIST = 100005, //请求数据不存在
	CGI_RET_CODE_ALREADY_EXIST =100006, //请求的数据已经存在
	CGI_RET_CODE_INVALID_OP =100007, //不能进行的操作
	CGI_RET_CODE_NO_PAY = 100008, //未支付，不能偷听/回答
	CGI_RET_CODE_NOT_PA_FANS = 100009, //你他娘并不是公众号粉丝， 滚
	CGI_RET_CODE_BALANCE_NOT_ENOUGH = 100010, //余额不足
	CGI_RET_CODE_OUTCOME_FAIL = 100011, //提现失败
	CGI_RET_CODE_FOLLOW_HAS_COMMENT = 100012, //回答已经评价过	
	CGI_RET_CODE_OUTCOME_CHECK_FAIL = 100013, //提现申请资格审核失败， 提现失败
	CGI_RET_CODE_ORDER_CANNOT_FETCH = 100014, //订单已被抢或被取消

	CGI_RET_CODE_ORDER_CANNOT_FETCH_REPEATED = 100015, //已经抢过该单了，耐心等待下单者确认
	CGI_RET_CODE_ORDER_CANNOT_FETCH_TOOMANY = 100016, //该订单已经排满，不能继续排队了
	CGI_RET_CODE_ORDER_CANNOT_CANCEL_WHILE_WAITING_EXISTS = 100017, //该订单不能取消， 因为已经有人排队了
	CGI_RET_CODE_ORDER_CANNOT_FETCH_CANCEL_AFTER_CHOSEN = 100018, //抢单者不能退出订单排队，因为该sb抢单者已经被下单者确认选择了
	CGI_RET_CODE_AVAILABLE_LOTTERY_TIME_NOT_ENOUGH = 1000019, //可用抽奖次数不足

	CGI_RET_CODE_SHOW_REPEATED = 1000020, //重复晒单
	CGI_RET_CODE_UPLOAD_TICKET_LIMIT = 1000021, //同一活动上传小票次数达到限制
};

#define SQL_QUERY_FAIL_REPLY(mysql, oss, logger)	\
	if(!mysql.Query(oss.str()))	\
	{	\
		LOG4CPLUS_ERROR(logger, "sql query failed, msg = " << mysql.GetErrMsg() << ", sql = " << oss.str());	\
		DoReply(CGI_RET_CODE_SERVER_BUSY);	\
		return true;	\
	}

#define SQL_QUERY_FAIL_CONTINUE(mysql, oss, logger)	\
	if(!mysql.Query(oss.str()))	\
	{	\
		LOG4CPLUS_ERROR(logger, "sql query failed, msg = " << mysql.GetErrMsg() << ", sql = " << oss.str());	\
		DoReply(CGI_RET_CODE_SERVER_BUSY);	\
		continue;	\
	}


#define sql_escape(str) (lce::cgi::CMysql::MysqlEscape(str))
#define int_2_str(i) lce::util::StringOP::TypeToStr(i) 

#define EMPTY_STR_RETURN(p) if(p.empty()) \
	{ \
		DoReply(CGI_RET_CODE_INVALID_PARAM); \
		LOG4CPLUS_ERROR(logger, "invalid "<<#p<<" ="<<p); \
		return true; \
	}

#define ZERO_INT_RETURN(p) if(p == 0) \
	{ \
		DoReply(CGI_RET_CODE_INVALID_PARAM); \
		LOG4CPLUS_ERROR(logger, "invalid "<<#p<<" ="<<p); \
		return true; \
	}

#define NEGATIVE_INT_RETURN(p) if(p < 0) \
	{ \
		DoReply(CGI_RET_CODE_INVALID_PARAM); \
		LOG4CPLUS_ERROR(logger, "invalid "<<#p<<" ="<<p); \
		return true; \
	}

#define NOT_POSITIVE_INT_RETURN(p) if(p <= 0) \
	{ \
		DoReply(CGI_RET_CODE_INVALID_PARAM); \
		LOG4CPLUS_ERROR(logger, "invalid "<<#p<<" ="<<p); \
		return true; \
	}

#define SERVER_NOT_OK_RETURN(iRet) \
if(iRet != hoosho::msg::E_OK)  \
{  \
	if(iRet == hoosho::msg::E_SERVER_INNER_ERROR)  \
	{  \
		DoReply(CGI_RET_CODE_SERVER_BUSY);  \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: inner error");  \
	}  \
	else if(iRet == hoosho::msg::E_SERVER_TIMEOUT)  \
	{  \
		DoReply(CGI_RET_CODE_SERVER_BUSY);  \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: timeout");  \
	}  \
	else if(iRet == hoosho::msg::E_INVALID_REQ_PARAM)  \
	{  \
		DoReply(CGI_RET_CODE_INVALID_PARAM);  \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: invalid param");  \
	}  \
	else if(iRet == hoosho::msg::E_ALREADY_EXIST)  \
	{  \
		DoReply(CGI_RET_CODE_ALREADY_EXIST);  \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: already exist");  \
	}  \
	else if(iRet == hoosho::msg::E_REPLY_TO_SELF_INVALID)	\
	{  \
		DoReply(CGI_RET_CODE_INVALID_OP);  \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: invalid op");  \
	}  \
	else if(iRet == hoosho::msg::E_NO_PAY) \
	{ \
		DoReply(CGI_RET_CODE_NO_PAY); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: no pay"); \
	} \
	else if(iRet == hoosho::msg::E_LOGIN_INVALID) \
	{ \
		DoReply(CGI_RET_CODE_NO_LOGIN); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: no login"); \
	} \
	else if(iRet == hoosho::msg::E_BALANCE_NOT_ENOUGH) \
	{ \
		DoReply(CGI_RET_CODE_BALANCE_NOT_ENOUGH); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: balacnce not enough"); \
	} \
	else if(iRet == hoosho::msg::E_PAY_FAIL) \
	{ \
		DoReply(CGI_RET_CODE_SERVER_BUSY); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: pay fail"); \
	} \
	else if(iRet == hoosho::msg::E_NOT_PA_FANS) \
	{ \
		DoReply(CGI_RET_CODE_NOT_PA_FANS); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: no pa fans"); \
	} \
	else if(iRet == hoosho::msg::E_FOLLOW_HAS_COMMENT) \
	{ \
		DoReply(CGI_RET_CODE_FOLLOW_HAS_COMMENT); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: follow has comment"); \
	} \
	else if(iRet == hoosho::msg::E_VERIFYCODE_INVALID) \
	{ \
		DoReply(CGI_RET_CODE_INVALID_PARAM); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: verifycode invalid"); \
	} \
	else if(iRet == hoosho::msg::E_ORDER_CANNOT_FETCH) \
	{ \
		DoReply(CGI_RET_CODE_ORDER_CANNOT_FETCH); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: order cannot fetch"); \
	} \
	else if(iRet == hoosho::msg::E_ORDER_CANNOT_FETCH_REPEATED) \
	{ \
		DoReply(CGI_RET_CODE_ORDER_CANNOT_FETCH_REPEATED); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: order cannot fetch repeated"); \
	} \
	else if(iRet == hoosho::msg::E_ORDER_CANNOT_FETCH_TOOMANY) \
	{ \
		DoReply(CGI_RET_CODE_ORDER_CANNOT_FETCH_TOOMANY); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: order cannot fetch, too many user waiting this order"); \
	} \
	else if(iRet == hoosho::msg::E_ORDER_CANNOT_CANCEL_WHILE_WAITING_EXISTS) \
	{ \
		DoReply(CGI_RET_CODE_ORDER_CANNOT_CANCEL_WHILE_WAITING_EXISTS); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: order cannot cancel, users are waiting "); \
	} \
	else if(iRet == hoosho::msg::E_ORDER_CANNOT_FETCH_CANCEL_AFTER_CHOSEN) \
	{ \
		DoReply(CGI_RET_CODE_ORDER_CANNOT_FETCH_CANCEL_AFTER_CHOSEN); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: order cannot fetch cancel, has been chosen"); \
	} \
	else if(iRet == hoosho::msg::E_SHOW_REPEATED) \
	{ \
		DoReply(CGI_RET_CODE_ALREADY_EXIST); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: user has show yet"); \
	} \
	else if(iRet == hoosho::msg::E_SHOW_REPEATED) \
	{ \
		DoReply(CGI_RET_CODE_SHOW_REPEATED); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: user has show yet"); \
	} \
	else if(iRet == hoosho::msg::E_UPLOAD_TICKET_LIMIT) \
	{ \
		DoReply(CGI_RET_CODE_UPLOAD_TICKET_LIMIT); \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: user has show yet"); \
	} \
	else \
	{  \
		DoReply(CGI_RET_CODE_SERVER_BUSY);  \
		LOG4CPLUS_ERROR(logger, "server return unknow error, ret = " << iRet);  \
	}  \
	return true;  \
}

// DB2PB
inline void UserInfoDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::UserInfo& stUserInfo)
{
	stUserInfo.set_user_id(strtoul(mysql.GetRow(0), NULL, 10));
	stUserInfo.set_openid(mysql.GetRow(1));
	stUserInfo.set_nickname(mysql.GetRow(2));
	stUserInfo.set_sex(atoi(mysql.GetRow(3)));
	stUserInfo.set_headimgurl(mysql.GetRow(4));
	stUserInfo.set_self_desc(mysql.GetRow(5));
	stUserInfo.set_phone(mysql.GetRow(6));
	stUserInfo.set_identity_status(atoi(mysql.GetRow(7)));
	stUserInfo.set_user_type(atoi(mysql.GetRow(8)));
	stUserInfo.set_default_addr_num(atoi(mysql.GetRow(9)));
	stUserInfo.set_extra_data(mysql.GetRow(10));
}

inline void UserIdentityInfoDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::UserIdentityInfo& stUserIdentityInfo)
{
	stUserIdentityInfo.set_user_id(strtoul(mysql.GetRow(0), NULL, 10));
	stUserIdentityInfo.set_identity_status(atoi(mysql.GetRow(1)));
	stUserIdentityInfo.set_phone(mysql.GetRow(2));
	stUserIdentityInfo.set_identity_num(mysql.GetRow(3));
	stUserIdentityInfo.set_identity_pic1_id(mysql.GetRow(4));
	stUserIdentityInfo.set_identity_pic2_id(mysql.GetRow(5));
	stUserIdentityInfo.set_identity_pic3_id(mysql.GetRow(6));
	stUserIdentityInfo.set_create_ts(strtoul(mysql.GetRow(7), NULL, 10));
	stUserIdentityInfo.set_check_ts(strtoul(mysql.GetRow(8), NULL, 10));
	stUserIdentityInfo.set_check_mgr(mysql.GetRow(9));
}

inline void UserRecvAddrInfoDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::UserRecvAddrInfo& stUserRecvAddrInfo)
{
	stUserRecvAddrInfo.set_user_id(strtoul(mysql.GetRow(0), NULL, 10));
	stUserRecvAddrInfo.set_addr_num(atoi(mysql.GetRow(1)));
	stUserRecvAddrInfo.set_recv_name(mysql.GetRow(2));
	stUserRecvAddrInfo.set_identity_num(mysql.GetRow(3));
	stUserRecvAddrInfo.set_phone(mysql.GetRow(4));
	stUserRecvAddrInfo.set_post_num(mysql.GetRow(5));
	stUserRecvAddrInfo.set_addr_province(mysql.GetRow(6));
	stUserRecvAddrInfo.set_addr_city(mysql.GetRow(7));
	stUserRecvAddrInfo.set_addr_district(mysql.GetRow(8));
	stUserRecvAddrInfo.set_addr_detail(mysql.GetRow(9));
}



inline void GoodInfoDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::GoodInfo& stGoodInfo)
{
	stGoodInfo.set_good_id(strtoul(mysql.GetRow(0), NULL, 10));
	stGoodInfo.set_good_id_text(mysql.GetRow(1));
	stGoodInfo.set_good_type(strtoul(mysql.GetRow(2), NULL, 10));
	stGoodInfo.set_title(mysql.GetRow(3));
	stGoodInfo.set_good_desc(mysql.GetRow(4));
	stGoodInfo.set_show_pics(mysql.GetRow(5));
	stGoodInfo.set_detail_pic_id(strtoul(mysql.GetRow(6), NULL, 10));
	stGoodInfo.set_total_join_num(strtoul(mysql.GetRow(7), NULL, 10));
	stGoodInfo.set_create_ts(strtoul(mysql.GetRow(8), NULL, 10));
}

inline void GoodTypeDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::GoodType& stGoodType)
{
	stGoodType.set_good_type(strtoul(mysql.GetRow(0), NULL, 10));
	stGoodType.set_good_type_text(mysql.GetRow(1));
	stGoodType.set_good_type_icon(mysql.GetRow(2));
	stGoodType.set_extra_data(mysql.GetRow(3));
	stGoodType.set_create_ts(strtoul(mysql.GetRow(4), NULL, 10));
}

inline void ConsumeTypeDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::ConsumeType& stConsumeType)
{
	stConsumeType.set_consume_type(strtoul(mysql.GetRow(0), NULL, 10));
	stConsumeType.set_consume_type_text(mysql.GetRow(1));
	stConsumeType.set_create_ts(strtoul(mysql.GetRow(2), NULL, 10));
}

inline void FeedInfoDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::FeedInfo& stFeedInfo)
{
	stFeedInfo.set_feed_id(strtoul(mysql.GetRow(0), NULL, 10));
	stFeedInfo.set_good_id(strtoul(mysql.GetRow(1), NULL, 10));
	stFeedInfo.set_total_join_num(strtoul(mysql.GetRow(2), NULL, 10));
	stFeedInfo.set_current_join_num(strtoul(mysql.GetRow(3), NULL, 10));
	stFeedInfo.set_approved_num(strtoul(mysql.GetRow(4), NULL, 10));
	stFeedInfo.set_pending_num(strtoul(mysql.GetRow(5), NULL, 10));
	stFeedInfo.set_create_ts(strtoul(mysql.GetRow(6), NULL, 10));
	stFeedInfo.set_feed_status(atoi(mysql.GetRow(7)));
	stFeedInfo.set_finish_ts(strtoul(mysql.GetRow(8), NULL, 10));
	stFeedInfo.set_luck_contend_id(strtoul(mysql.GetRow(9), NULL, 10));
	stFeedInfo.set_luck_user_id(strtoul(mysql.GetRow(10), NULL, 10));
	stFeedInfo.set_luck_ts(strtoul(mysql.GetRow(11), NULL, 10));
	stFeedInfo.set_show_status(atoi(mysql.GetRow(12)));
	stFeedInfo.set_recv_addr(mysql.GetRow(13));
	stFeedInfo.set_express_name(mysql.GetRow(14));
	stFeedInfo.set_express_num(mysql.GetRow(15));
	stFeedInfo.set_extra_data(mysql.GetRow(16));
}

inline void TicketInfoDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::TicketInfo& stTicketInfo)
{
	stTicketInfo.set_ticket_id(strtoul(mysql.GetRow(0), NULL, 10));
	stTicketInfo.set_ticket_pics(mysql.GetRow(1));
	stTicketInfo.set_user_id(strtoul(mysql.GetRow(2), NULL, 10));
	stTicketInfo.set_user_ip(mysql.GetRow(3));
	stTicketInfo.set_feed_id(strtoul(mysql.GetRow(4), NULL, 10));
	stTicketInfo.set_check_status(atoi(mysql.GetRow(5)));
	stTicketInfo.set_contend_id(strtoul(mysql.GetRow(6), NULL, 10));
	stTicketInfo.set_ticket_price(strtoul(mysql.GetRow(7), NULL, 10));
	stTicketInfo.set_consume_type(atoi(mysql.GetRow(8)));
	stTicketInfo.set_good_id(strtoul(mysql.GetRow(9), NULL, 10));
	stTicketInfo.set_create_ts(strtoul(mysql.GetRow(10), NULL, 10));
	stTicketInfo.set_check_ts(strtoul(mysql.GetRow(11), NULL, 10));
	stTicketInfo.set_check_mgr(mysql.GetRow(12));
}

inline void CollectInfoDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::CollectInfo& stCollectInfo)
{
	stCollectInfo.set_user_id(strtoul(mysql.GetRow(0), NULL, 10));
	stCollectInfo.set_feed_id(strtoul(mysql.GetRow(1), NULL, 10));
	stCollectInfo.set_create_ts(strtoul(mysql.GetRow(2), NULL, 10));
}

inline void ShowInfoDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::ShowInfo& stShowInfo)
{
	stShowInfo.set_show_id(strtoul(mysql.GetRow(0), NULL, 10));
	stShowInfo.set_user_id(strtoul(mysql.GetRow(1), NULL, 10));
	stShowInfo.set_feed_id(strtoul(mysql.GetRow(2), NULL, 10));
	stShowInfo.set_title(mysql.GetRow(3));
	stShowInfo.set_show_desc(mysql.GetRow(4));
	stShowInfo.set_show_pics(mysql.GetRow(5));
	stShowInfo.set_create_ts(strtoul(mysql.GetRow(6), NULL, 10));
}

inline void PicInfoDB2PB(lce::cgi::CMysql& mysql, hoosho::msg::s::PicInfo& stPicInfo)
{
	stPicInfo.set_pic_id(strtoul(mysql.GetRow(0), NULL, 10));
	stPicInfo.set_content(mysql.GetRow(1));
	stPicInfo.set_user_id(strtoul(mysql.GetRow(2), NULL, 10));
	stPicInfo.set_type(strtoul(mysql.GetRow(3), NULL, 10));
}

// DB2Any
inline void BannerInfoDB2Any(lce::cgi::CMysql& mysql, lce::cgi::CAnyValue& any)
{
	any["banner_id"] = strtoul(mysql.GetRow(0), NULL, 10);
	any["pic_id"] = strtoul(mysql.GetRow(1), NULL, 10);
	any["content_type"] = atoi(mysql.GetRow(2));
	any["content"] = mysql.GetRow(3);
	any["create_ts"] = strtoul(mysql.GetRow(5), NULL, 10);
}

// PB2Any
inline lce::cgi::CAnyValue UserInfoPB2Any(hoosho::msg::s::UserInfo pb)
{
	lce::cgi::CAnyValue any;
	any["user_id"] = int_2_str(pb.user_id());
	any["openid"] = pb.openid();
	any["nickname"] = pb.nickname();
	any["sex"] = pb.sex();
	any["headimgurl"] = pb.headimgurl();
	any["self_desc"] = pb.self_desc();
	any["phone"] = pb.phone();
	any["identity_status"] = pb.identity_status();
	any["user_type"] = pb.user_type();
	any["default_addr_num"] = pb.default_addr_num();
	any["extra_data"] = pb.extra_data();
	return any;
}

inline lce::cgi::CAnyValue UserIdentityInfoPB2Any(hoosho::msg::s::UserIdentityInfo pb)
{
	lce::cgi::CAnyValue any;
	any["user_id"] = int_2_str(pb.user_id());
	any["identity_status"] = pb.identity_status();
	any["phone"] = pb.phone();
	any["identity_num"] = pb.identity_num();
	any["identity_pic1_id"] = int_2_str(pb.identity_pic1_id());
	any["identity_pic2_id"] = int_2_str(pb.identity_pic2_id());
	any["identity_pic3_id"] = int_2_str(pb.identity_pic3_id());
	any["create_ts"] = pb.create_ts();
	any["check_ts"] = pb.check_ts();
//	any["check_mgr"] = pb.check_mgr();
	return any;
}

inline lce::cgi::CAnyValue UserRecvAddrInfoPB2Any(hoosho::msg::s::UserRecvAddrInfo pb)
{
	lce::cgi::CAnyValue any;
	any["user_id"] = int_2_str(pb.user_id());
	any["addr_num"] = pb.addr_num();
	any["recv_name"] = pb.recv_name();
	any["identity_num"] = pb.identity_num();
	any["phone"] = pb.phone();
	any["post_num"] = pb.post_num();
	any["addr_province"] = pb.addr_province();
	any["addr_city"] = pb.addr_city();
	any["addr_district"] = pb.addr_district();
	any["addr_detail"] = pb.addr_detail();
	return any;
}



inline lce::cgi::CAnyValue GoodInfoPB2Any(hoosho::msg::s::GoodInfo pb)
{
	lce::cgi::CAnyValue any;
	any["good_id"] = int_2_str(pb.good_id());
	any["good_id_text"] = pb.good_id_text();
	any["good_type"] = int_2_str(pb.good_type());
	any["title"] = pb.title();
	any["good_desc"] = pb.good_desc();
	any["show_pics"] = pb.show_pics();
	any["detail_pic_id"] = int_2_str(pb.detail_pic_id());
	any["total_join_num"] = pb.total_join_num();
	any["create_ts"] = pb.create_ts();
	return any;
}

inline lce::cgi::CAnyValue GoodTypePB2Any(hoosho::msg::s::GoodType pb)
{
	lce::cgi::CAnyValue any;
	any["good_type"] = int_2_str(pb.good_type());
	any["good_type_text"] = pb.good_type_text();
	any["good_type_icon"] = pb.good_type_icon();
	any["extra_data"] = pb.extra_data();
	any["create_ts"] = pb.create_ts();
	return any;
}

inline lce::cgi::CAnyValue ConsumeTypePB2Any(hoosho::msg::s::ConsumeType pb)
{
	lce::cgi::CAnyValue any;
	any["consume_type"] = int_2_str(pb.consume_type());
	any["consume_type_text"] = pb.consume_type_text();
	any["create_ts"] = pb.create_ts();
	return any;
}

inline lce::cgi::CAnyValue FeedInfoPB2Any(hoosho::msg::s::FeedInfo pb)
{
	lce::cgi::CAnyValue any;
	any["feed_id"] = int_2_str(pb.feed_id());
	any["good_id"] = int_2_str(pb.good_id());
	any["total_join_num"] = pb.total_join_num();
	any["current_join_num"] = pb.current_join_num();
	any["approved_num"] = pb.approved_num();
	any["pending_num"] = pb.pending_num();
	any["create_ts"] = pb.create_ts();
	any["feed_status"] = pb.feed_status();
	any["finish_ts"] = pb.finish_ts();
	any["luck_contend_id"] = int_2_str(pb.luck_contend_id());
	any["luck_user_id"] = int_2_str(pb.luck_user_id());
	any["luck_ts"] = pb.luck_ts();
	any["show_status"] = pb.show_status();
	any["recv_addr"] = pb.recv_addr();
	any["express_name"] = pb.express_name();
	any["express_num"] = pb.express_num();
	any["extra_data"] = pb.extra_data();
	return any;
}

inline lce::cgi::CAnyValue TicketInfoPB2Any(hoosho::msg::s::TicketInfo pb)
{
	lce::cgi::CAnyValue any;
	any["ticket_id"] = int_2_str(pb.ticket_id());
	any["ticket_pics"] = pb.ticket_pics();
	any["user_id"] = int_2_str(pb.user_id());
	any["user_ip"] = pb.user_ip();
	any["feed_id"] = int_2_str(pb.feed_id());
	any["check_status"] = pb.check_status();
	any["contend_id"] = int_2_str(pb.contend_id());
	any["ticket_price"] = int_2_str(pb.ticket_price());
	any["consume_type"] = pb.consume_type();
	any["good_id"] = int_2_str(pb.good_id());
	any["create_ts"] = pb.create_ts();
	any["check_ts"] = pb.check_ts();
//	any["check_mgr"] = pb.check_mgr();
	return any;
}

inline lce::cgi::CAnyValue ContendInfoPB2Any(hoosho::msg::s::ContendInfo pb)
{
	lce::cgi::CAnyValue any;
	any["ticket_id"] = int_2_str(pb.ticket_id());
	any["user_id"] = int_2_str(pb.user_id());
	any["user_ip"] = pb.user_ip();
	any["feed_id"] = int_2_str(pb.feed_id());
	any["contend_id"] = int_2_str(pb.contend_id());
	any["create_ts"] = pb.create_ts();
	return any;
}

inline lce::cgi::CAnyValue CollectInfoPB2Any(hoosho::msg::s::CollectInfo pb)
{
	lce::cgi::CAnyValue any;
	any["user_id"] = int_2_str(pb.user_id());
	any["feed_id"] = int_2_str(pb.feed_id());
	any["create_ts"] = pb.create_ts();
	return any;
}

inline lce::cgi::CAnyValue ShowInfoPB2Any(hoosho::msg::s::ShowInfo pb)
{
	lce::cgi::CAnyValue any;
	any["show_id"] = int_2_str(pb.show_id());
	any["user_id"] = int_2_str(pb.user_id());
	any["feed_id"] = int_2_str(pb.feed_id());
	any["title"] = pb.title();
	any["show_desc"] = pb.show_desc();
	any["show_pics"] = pb.show_pics();
	any["create_ts"] = pb.create_ts();
	return any;
}

inline lce::cgi::CAnyValue PicInfoPB2Any(hoosho::msg::s::PicInfo pb)
{
	lce::cgi::CAnyValue any;
	any["pic_id"] = int_2_str(pb.pic_id());
	any["content"] = pb.content();
	any["user_id"] = int_2_str(pb.user_id());
	any["type"] = int_2_str(pb.type());
	return any;
}

inline lce::cgi::CAnyValue BannerInfoPB2Any(hoosho::msg::s::BannerInfo pb)
{
	lce::cgi::CAnyValue any;
	any["banner_id"] = int_2_str(pb.banner_id());
	any["pic_id"] = int_2_str(pb.pic_id());
	any["content_type"] = int_2_str(pb.content_type());
	any["content"] = pb.content();
	any["del_status"] = int_2_str(pb.del_status());
	any["create_ts"] = int_2_str(pb.create_ts());
	return any;
}

#endif
