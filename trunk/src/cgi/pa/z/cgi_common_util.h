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
};

enum ORDER_MEDIA_TYPE
{
	ORDER_MEDIA_TYPE_TEXT = 1,
	ORDER_MEDIA_TYPE_AUDIO = 2,
	ORDER_MEDIA_TYPE_PICTURE = 3,
};

enum ORDER_TYPE
{
	ORDER_TYPE_COMMON = 1,
	ORDER_TYPE_INVOICE = 2, //小票
};

enum ORDER_STATUS
{
	ORDER_STATUS_INIT = 0,
	ORDER_STATUS_WAITING_FETCH = 1,
	ORDER_STATUS_FETCHED = 2,
	ORDER_STATUS_FINISHED = 3,
	ORDER_STATUS_CANCELED = 4,

	//
	ORDER_STATUS_INVOICE_CHECK_FAIL = 0,
	ORDER_STATUS_INVOICE_WAITING_CHECK = 1,
	ORDER_STATUS_INVOICE_CHECK_SUCC = 3,
};

enum SCORE_TYPE
{
	SCORE_TYPE_ORDER_CREATE = 2,
	SCORE_TYPE_ORDER_FETCH_COMMIT = 1,
	SCORE_TYPE_ORDER_FETCH_ACCEPT = 1,
	SCORE_TYPE_ORDER_FETCH_SUCCESS = 1,
	SCORE_TYPE_ORDER_FINISH = 10,
	SCORE_TYPE_ORDER_COMMENT = 1,

	SCORE_TYPE_PHONE_COMPLETE = 5,
	SCORE_TYPE_TICKET_UPLOAD = 10,
};

#define SQL_QUERY_FAIL_REPLY(mysql, oss, logger)	\
	if(!mysql.Query(oss.str()))	\
	{	\
		LOG4CPLUS_ERROR(logger, "sql query failed, msg = " << mysql.GetErrMsg() << ", sql = " << oss.str());	\
		DoReply(CGI_RET_CODE_SERVER_BUSY);	\
		return true;	\
	}

#define sql_escape(str) (lce::cgi::CMysql::MysqlEscape(str))
#define int_2_str(i) lce::util::StringOP::TypeToStr(i) 

#define EMPTY_STR_RETURN(p) if(p.empty()) \
	{ \
		DoReply(CGI_RET_CODE_INVALID_PARAM); \
		LOG4CPLUS_ERROR(logger, "invalid "<<#p<<" ="<<p); \
		return true; \
	}
#define EMPTY_STR_RETURN_ECHO(p,t) if(p.empty()) \
	{ \
		DoReply(CGI_RET_CODE_INVALID_PARAM); \
		LOG4CPLUS_ERROR(logger, "empty "<<#p<<" ="<<p<<", "<<#t<<"="<<t); \
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
	else \
	{  \
		DoReply(CGI_RET_CODE_SERVER_BUSY);  \
		LOG4CPLUS_ERROR(logger, "server return unknow error, ret = " << iRet);  \
	}  \
	return true;  \
}


#define UserInfoPB2Any(pb, any) \
{ \
	any["openid"] = pb.openid(); \
	any["nickname"] = pb.nickname(); \
	any["sex"] = int_2_str(pb.sex()); \
	any["headimgurl"] = pb.headimgurl(); \
	any["self_desc"] = pb.self_desc(); \
	any["phone"] = pb.phone(); \
	any["user_type"] = pb.user_type(); \
	any["user_flag"] = pb.user_flag(); \
	any["user_score"] = pb.user_score(); \
}

#define UserExtraInfoPB2Any(pb, any) \
{ \
	any["openid"] = pb.openid(); \
	any["star"] = int_2_str(pb.star()); \
	any["level"] = int_2_str(pb.level()); \
}

#define OutComeReqInfoPB2Any(pb, any) \
{ \
	any["table_id"] = int_2_str(pb.table_id()); \
	any["openid"] = pb.openid(); \
	any["amount"] = int_2_str(pb.amount()); \
	any["state"] = int_2_str(pb.state()); \
	any["create_ts"] = int_2_str(pb.create_ts()); \
}

#define CashFlowInfoPB2Any(pb, any) \
{ \
	any["openid"] = pb.openid(); \
	any["out_trade_no"] = pb.out_trade_no(); \
	any["amount"] = int_2_str(pb.amount()); \
	any["add_or_reduce"] = int_2_str(pb.add_or_reduce()); \
	any["type"] = pb.type(); \
	any["balance"] = int_2_str(pb.balance()); \
	any["create_ts"] = int_2_str(pb.create_ts()); \
}

#define OrderFetchStatePB2Any(pb, any) \
{ \
	any["orderid"] = int_2_str(pb.order_id()); \
	any["fetch_openid"] = pb.fetch_openid(); \
	any["fetch_state"] = pb.fetch_state(); \
	any["fetch_ts"] = int_2_str(pb.fetch_ts()); \
	any["accept_ts"] = int_2_str(pb.accept_ts()); \
}

#define OrderInfoPB2Any(pb, any) \
{ \
	any["orderid"] = int_2_str(pb.order_id()); \
	any["openid_master"] = pb.openid_master(); \
	any["openid_slave"] = pb.openid_slave(); \
	any["order_visible"] = pb.order_visible(); \
	any["order_status"] = pb.order_status(); \
	any["pay_status"] = pb.pay_status(); \
	any["addr_to_longitude"] = pb.addr_to_longitude(); \
	any["addr_to_latitude"] = pb.addr_to_latitude(); \
	any["addr_to"] = pb.addr_to(); \
	any["addr_to_detail"] = pb.addr_to_detail(); \
	any["order_desc"] = pb.order_desc(); \
	any["order_receiver_name"] = pb.order_receiver_name(); \
	any["order_receiver_phone"] = pb.order_receiver_phone(); \
	any["price"] = pb.price(); \
	any["expect_from_ts"] = pb.expect_from_ts(); \
	any["expect_to_ts"] = pb.expect_to_ts(); \
	any["pay_ts"] = pb.pay_ts(); \
	any["create_ts"] = pb.create_ts(); \
	any["finish_ts"] = pb.finish_ts(); \
	any["fetch_ts"] = pb.fetch_ts(); \
	any["cancel_ts"] = pb.cancel_ts(); \
	any["star_from_master"] = pb.star_from_master(); \
	any["star_from_slave"] = pb.star_from_slave(); \
	any["order_type"] = pb.order_type(); \
	any["media_type"] = pb.media_type(); \
	any["extra_data"] = pb.extra_data(); \
	for(int __i=0; __i!=pb.order_waiting_user_list_size(); ++__i) \
	{ \
		lce::cgi::CAnyValue tmp; \
		tmp["openid"] = pb.order_waiting_user_list(__i).openid(); \
		tmp["create_ts"] = pb.order_waiting_user_list(__i).create_ts(); \
		any["waiting_user_list"].push_back(tmp); \
	} \
}

#define OrderFavorInfoPB2Any(pb, any) \
{ \
	any["openid"] = int_2_str(pb.openid()); \
	any["orderid"] = int_2_str(pb.orderid()); \
	any["create_ts"] = pb.create_ts(); \
}

#define OrderFollowInfoPB2Any(pb, any)\
{ \
	any["follow_id"] = int_2_str(pb.follow_id()); \
	any["content"] = pb.content(); \
	any["orderid"] = int_2_str(pb.orderid()); \
	any["type"] = pb.type(); \
	any["origin_comment_id"] = int_2_str(pb.origin_comment_id()); \
	any["openid_from"] = pb.openid_from(); \
	any["openid_to"] = pb.openid_to(); \
	any["create_ts"] = pb.create_ts(); \
	any["del_ts"] = pb.del_ts(); \
}

#define SessionPB2Any(pb, any) \
{ \
	any["session_id"] = int_2_str(pb.session_id()); \
	any["openid_from"] = pb.openid_from(); \
	any["openid_to"] = pb.openid_to(); \
	any["content"] = pb.content(); \
	any["newmsg_status"] = int_2_str(pb.newmsg_status()); \
	any["create_ts"] = int_2_str(pb.create_ts()); \
}

#define NoticeInfoPB2Any(pb, any) \
{ \
	any["openid"] = pb.openid(); \
	any["type"] = int_2_str(pb.type()); \
	any["status"] = int_2_str(pb.status()); \
	any["extra_data_0"] = pb.extra_data_0(); \
	any["extra_data_1"] = pb.extra_data_1(); \
	any["extra_data_2"] = pb.extra_data_2(); \
	any["create_ts"] = pb.create_ts(); \
}

#define LotteryInfoPB2Any(pb, any) \
{ \
	any["prize"] = pb.prize(); \
	any["prize_desc"] = pb.prize_desc(); \
	any["create_ts"] = pb.create_ts(); \
}

inline int FetchUserInfo(const std::string& strCode
						, const std::set<std::string>& setOpenid
						, std::map<std::string, lce::cgi::CAnyValue>& mapResult
						, std::string& strZServerIP
						, int iZServerPort
						, std::string strErrMsg
						)
{
	if(strZServerIP.empty() || 0 == iZServerPort)
	{
		strZServerIP = "127.0.0.1";
		iZServerPort = 50000;
	}
	
	//request		
	::hoosho::msg::Msg stRequest;
	::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
	pHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
	pHead->set_seq(time(0));

	::hoosho::msg::z::MsgReq* pZMsgReq = stRequest.mutable_z_msg_req();
	pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_USER_INFO_REQ);
	pZMsgReq->set_code(strCode);

	::hoosho::msg::z::GetUserInfoReq* pGetUserInfoReq = pZMsgReq->mutable_get_user_info_req();	
	for(std::set<std::string>::iterator iter=setOpenid.begin(); iter!=setOpenid.end(); ++iter)		
	{
		pGetUserInfoReq->add_openid_list(*iter);
	}
	
	//io
	std::string strInnerErrMsg = "";
	::hoosho::msg::Msg stResponse;
	::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(strZServerIP, iZServerPort);
	int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strInnerErrMsg);
	if(iRet < 0)
	{
		strErrMsg = "FentchUserInfo failed, stProtoIOTcpClient.io, strErrMsg=" + strInnerErrMsg;		
		return -1;
	}

	//parse response
	const ::hoosho::msg::MsgHead& stHead = stResponse.head();
	if(stHead.cmd() != ::hoosho::msg::Z_PROJECT_RES)
	{
		strErrMsg = "response.cmd=" + int_2_str(stHead.cmd()) + ", unknown, fuck!!!";		
		return -1;	
	}

	if(stHead.result() != ::hoosho::msg::E_OK)
	{
		strErrMsg = "response.result=" + int_2_str(stHead.result());
		return -1;	
	}

	const ::hoosho::msg::z::MsgRes& stZMsgRes = stResponse.z_msg_res();
	if(stZMsgRes.sub_cmd() != ::hoosho::msg::z::GET_USER_INFO_RES)
	{
		strErrMsg = "response.sub_cmd=" + int_2_str(stZMsgRes.sub_cmd()) + ", unknown, fuck!!!";
		return -1;	
	}

	mapResult.clear();
	for(int i=0; i<stZMsgRes.user_info_list_size(); ++i)
	{
		lce::cgi::CAnyValue stAnyValue;
		const ::hoosho::msg::z::UserInfo& stUserInfo = stZMsgRes.user_info_list(i);
		UserInfoPB2Any(stUserInfo, stAnyValue);	
		mapResult[int_2_str(stUserInfo.openid())] = stAnyValue;	
	}

	return 0;
}

inline int GetPAAccessToken(const std::string &strSvrIp, uint16_t wSvrPort, const std::string &strAppid, std::string &strPAAccessToken, std::string &strErrMsg)
{
	::hoosho::msg::Msg stRequest;
	::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
	pHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
	pHead->set_seq(time(0));
	
	::hoosho::msg::z::MsgReq* pZMsgReq = stRequest.mutable_z_msg_req();
	pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_PA_ACCESS_TOKEN_REQ);
	
	::hoosho::msg::z::GetPAAccessTokenReq* pGetAccessTokenReq = pZMsgReq->mutable_get_pa_access_token_req();
	pGetAccessTokenReq->set_pa_appid(strAppid);
	

	//io
	std::string strInnerErrMsg = "";
	::hoosho::msg::Msg stResponse;
	::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(strSvrIp, wSvrPort);
	int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strInnerErrMsg);
	if(iRet < 0)
	{
		strErrMsg = "Process failed, stProtoIOTcpClient.io, strErrMsg=" + strInnerErrMsg;
		return -1;	
	}

	//parse response
	const ::hoosho::msg::MsgHead& stHead = stResponse.head();
	if(stHead.result() != ::hoosho::msg::E_OK)
	{
		strErrMsg = "response.result=" + int_2_str(stHead.result());
		return -1;	
	}		
	
	if(stHead.cmd() != ::hoosho::msg::Z_PROJECT_RES)
	{
		strErrMsg = "response.cmd=" + int_2_str(stHead.cmd()) + ", unknown, fuck!!!";
		return -1;	
	}

	if(stHead.seq() != pHead->seq())
	{
		strErrMsg = "response.seq=" + int_2_str(stHead.seq()) + ", while request.seq=" + int_2_str(pHead->seq());
		return -1;	
	}
	
	const ::hoosho::msg::z::MsgRes& stZMsgRes = stResponse.z_msg_res();
	if(stZMsgRes.sub_cmd() != ::hoosho::msg::z::GET_PA_ACCESS_TOKEN_RES)
	{			
		strErrMsg = "response.sub_cmd=" + int_2_str(stHead.cmd()) + ", unknown, fuck!!!";
		return -1;
	}		

	const ::hoosho::msg::z::GetPAAccessTokenRes& stGetAccessTokenRes = stZMsgRes.get_pa_access_token_res();
	strPAAccessToken = stGetAccessTokenRes.pa_access_token();
		
	return 0;
}

#endif

