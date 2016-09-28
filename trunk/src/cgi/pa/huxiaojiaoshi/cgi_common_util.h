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

};

//CGI 登陆态check配置
enum CGI_NEED_LOGIN
{
	CGI_NEED_LOGIN_NO = 0,
	CGI_NEED_LOGIN_YES = 1,
};


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
	any["subscribe"] = int_2_str(pb.subscribe()); \
	any["subscribe_time"] = int_2_str(pb.subscribe_time()); \
	any["nickname"] = pb.nickname(); \
	any["sex"] = int_2_str(pb.sex()); \
	any["country"] = pb.country(); \
	any["province"] = pb.province(); \
	any["city"] = pb.city(); \
	any["headimgurl"] = pb.headimgurl(); \
	any["self_desc"] = pb.self_desc(); \
}

#define FeedInfoPB2Any(pb, any) \
{ \
	any["feed_id"] = int_2_str(pb.feed_id()); \
	any["openid"] = pb.openid(); \
	any["content"] = pb.content(); \
	any["ts"] = pb.ts(); \
	any["num_follow"] = pb.num_follow(); \
	any["num_listen"] = pb.num_listen(); \
	any["need_sex"] = pb.need_sex(); \
}

#define FollowInfoPB2Any(pb, any) \
{ \
	any["follow_id"] = int_2_str(pb.follow_id()); \
	any["feed_id"] = int_2_str(pb.feed_id()); \
	any["openid"] = pb.openid(); \
	any["content_type"] = pb.content_type(); \
	any["content_id"] = pb.content_id(); \
	any["content_duration"] = pb.content_duration(); \
	any["ts"] = pb.ts(); \
	any["num_listen"] = pb.num_listen(); \
	any["num_comment_good"] = pb.num_comment_good(); \
	any["num_comment_default"] = pb.num_comment_default(); \
	any["num_comment_bad"] = pb.num_comment_bad(); \
	any["out_trade_no"] = pb.out_trade_no(); \
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
	any["out_trade_openid"] = pb.out_trade_openid(); \
	any["amount"] = int_2_str(pb.amount()); \
	any["type"] = pb.type(); \
	any["reason"] = pb.reason(); \
	any["balance"] = int_2_str(pb.balance()); \
	any["create_ts"] = int_2_str(pb.create_ts()); \
}


inline bool CheckLogin(const lce::cgi::CCgiInput& stCgiInput
					, std::string& strCookieKeyName
					, std::string& strCookieValueName
					, std::string& strUserServerIP
					, int iUserServerPort
					, std::string& strErrMsg)
{
	if(strCookieKeyName.empty() || strCookieValueName.empty())
	{
		strCookieKeyName = "c_huxiaojiaoshi_key";
		strCookieValueName = "c_huxiaojiaoshi_value";
	}

	if(strUserServerIP.empty() || 0 == iUserServerPort)
	{
		strUserServerIP = "127.0.0.1";
		iUserServerPort = 50000;
	}

	std::string strCookieKey =  (std::string)stCgiInput.GetCookie(strCookieKeyName);
	std::string strCookieValue =  (std::string)stCgiInput.GetCookie(strCookieValueName);

	//request
	::hoosho::msg::Msg stRequest;
	::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
	pHead->set_cmd(::hoosho::msg::J_CHECK_LOGIN_REQ);
	pHead->set_seq(time(0));
	::hoosho::j::user::CheckLoginReq* pCheckLoginReq = stRequest.mutable_check_login_req();
	pCheckLoginReq->set_cookie_j_key(strCookieKey);
	pCheckLoginReq->set_cookie_j_value(strCookieValue);
	
	//io
	std::string strInnerErrMsg = "";
	::hoosho::msg::Msg stResponse;
	::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(strUserServerIP, iUserServerPort);
	int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strInnerErrMsg);
	if(iRet < 0)
	{
		strErrMsg = "CheckLogin failed, stProtoIOTcpClient.io, strErrMsg=" + strInnerErrMsg;
		return false;
	}

	//parse response
	const ::hoosho::msg::MsgHead& stHead = stResponse.head();
	if(stHead.cmd() != ::hoosho::msg::J_CHECK_LOGIN_RES)
	{
		strErrMsg = "response.cmd=" + int_2_str(stHead.cmd()) + ", unknown, fuck!!!";
		return false;	
	}


	if(stHead.result() != ::hoosho::msg::E_OK)
	{
		strErrMsg = "response.result=" + int_2_str(stHead.result());
		return false;
	}
	return true;
}

inline int FetchUserInfo(const std::set<std::string>& setOpenid
						, std::map<std::string, lce::cgi::CAnyValue>& mapResult
						, std::string& strUserServerIP
						, int iUserServerPort
						, std::string strErrMsg
						)
{
	if(strUserServerIP.empty() || 0 == iUserServerPort)
	{
		strUserServerIP = "127.0.0.1";
		iUserServerPort = 50000;
	}
	
	//request		
	::hoosho::msg::Msg stRequest;
	::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
	pHead->set_cmd(::hoosho::msg::J_GET_USER_INFO_REQ);
	pHead->set_seq(time(0));
	::hoosho::j::user::GetUserInfoReq* pGetUserInfoReq = stRequest.mutable_get_user_info_req();
	for(std::set<std::string>::iterator iter=setOpenid.begin(); iter!=setOpenid.end(); ++iter)		
	{
		pGetUserInfoReq->add_openid_list(*iter);
	}
	
	//io
	std::string strInnerErrMsg = "";
	::hoosho::msg::Msg stResponse;
	::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(strUserServerIP, iUserServerPort);
	int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
	if(iRet < 0)
	{
		strErrMsg = "FentchUserInfo failed, stProtoIOTcpClient.io, strErrMsg=" + strInnerErrMsg;		
		return -1;
	}

	//parse response
	const ::hoosho::msg::MsgHead& stHead = stResponse.head();
	if(stHead.cmd() != ::hoosho::msg::J_GET_USER_INFO_RES)
	{
		strErrMsg = "response.cmd=" + int_2_str(stHead.cmd()) + ", unknown, fuck!!!";		
		return -1;	
	}

	if(stHead.result() != ::hoosho::msg::E_OK)
	{
		strErrMsg = "response.result=" + int_2_str(stHead.result());
		return -1;	
	}

	mapResult.clear();
	for(int i=0; i<stResponse.get_user_info_res().userinfo_list_size(); ++i)
	{
		lce::cgi::CAnyValue stAnyValue;
		const ::hoosho::j::commstruct::UserInfo& stUserInfo = stResponse.get_user_info_res().userinfo_list(i);
		UserInfoPB2Any(stUserInfo, stAnyValue);	
		mapResult[int_2_str(stUserInfo.openid())] = stAnyValue;	
	}

	return 0;
}

// return -1, proto io failed
// return 0, proto io succ, 仍需检查qwErrCode
inline int FetchFeedList(const std::string& strFeedsServerIp, const int& dwFeedsServerPort,
						const uint64_t& qwBeginFeedId, const uint64_t& qwLimit,
						const uint64_t& qwSceneType, const uint64_t& qwNeedSex,
						const std::string& strOpenid, uint64_t& qwErrcode,
						 std::string& strErrMsg,
						 std::vector<hoosho::j::commstruct::FeedInfo>& vecFeedInfo)
{
	//Build requestMsg protobuf
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(hoosho::msg::J_GET_FEED_LIST_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::j::feeds::GetFeedListReq* stGetFeedListReq = stRequestMsg.mutable_j_feeds_get_feed_list_req();

	::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);

	stGetFeedListReq->set_begin_feed_id(qwBeginFeedId);
	stGetFeedListReq->set_openid(strOpenid);
	stGetFeedListReq->set_limit(qwLimit);
	stGetFeedListReq->set_scene_type(qwSceneType);
	stGetFeedListReq->set_need_sex(qwNeedSex);

	//send to server, and recv responseMsg protobuf
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet != 0)
	{
		strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
		qwErrcode = ::hoosho::msg::E_SERVER_INNER_ERROR;
		return -1;
	}
	qwErrcode = stResponseMsg.head().result();

	if(qwErrcode != ::hoosho::msg::E_OK)
	{
		return 0;
	}

	vecFeedInfo.clear();
	const ::hoosho::j::feeds::GetFeedListRes& stGetFeedListRes = stResponseMsg.j_feeds_get_feed_list_res();
	for(int i = 0; i < stGetFeedListRes.feed_list_size(); i++)
	{
		vecFeedInfo.push_back(stGetFeedListRes.feed_list(i));
	}
	return 0;
}

inline int FetchFeedInfo(const std::string& strFeedsServerIp, const int& dwFeedsServerPort,
						const std::vector<uint64_t>& vecFeedIdList, const std::string strOpenid, uint64_t& qwErrcode,
						 std::string& strErrMsg, std::vector<hoosho::j::commstruct::FeedInfo>& vecFeedInfo)
{
	//Build requestMsg protobuf
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(hoosho::msg::J_GET_FEED_DETAIL_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::j::feeds::GetFeedDetailReq* stGetFeedDetailReq = stRequestMsg.mutable_j_feeds_get_feed_detail_req();

	::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);

	for(size_t i = 0; i < vecFeedIdList.size(); i++)
	{
		stGetFeedDetailReq->add_feed_id_list(vecFeedIdList[i]);
	}
	stGetFeedDetailReq->set_openid(strOpenid);

	//send to server, and recv responseMsg protobuf
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet != 0)
	{
		strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
		qwErrcode = ::hoosho::msg::E_SERVER_INNER_ERROR;
		return -1;
	}
	qwErrcode = stResponseMsg.head().result();

	if(qwErrcode != ::hoosho::msg::E_OK)
	{
		return 0;
	}

	vecFeedInfo.clear();
	const ::hoosho::j::feeds::GetFeedDetailRes& stGetFeedDetailRes = stResponseMsg.j_feeds_get_feed_detail_res();
	for(int i = 0; i < stGetFeedDetailRes.feed_list_size(); i++)
	{
		vecFeedInfo.push_back(stGetFeedDetailRes.feed_list(i));
	}
	return 0;
}

inline int FetchFollowList(const std::string& strFeedsServerIp, const int& dwFeedsServerPort,
						const uint64_t& qwBeginFollowId, const uint64_t& qwLimit,
						const uint64_t& qwSceneType, const uint64_t& qwFeedId,
						const std::string& strOpenid, uint64_t& qwErrcode,
						 std::string& strErrMsg,
						 std::vector<hoosho::j::commstruct::FollowInfo>& vecFollowInfo)
{
	//Build requestMsg protobuf
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(hoosho::msg::J_GET_FOLLOW_LIST_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::j::feeds::GetFollowListReq* stGetFollowListReq = stRequestMsg.mutable_j_feeds_get_follow_list_req();

	::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);

	stGetFollowListReq->set_begin_follow_id(qwBeginFollowId);
	stGetFollowListReq->set_openid(strOpenid);
	stGetFollowListReq->set_limit(qwLimit);
	stGetFollowListReq->set_scene_type(qwSceneType);
	stGetFollowListReq->set_feed_id(qwFeedId);

	//send to server, and recv responseMsg protobuf
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet != 0)
	{
		strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
		qwErrcode = ::hoosho::msg::E_SERVER_INNER_ERROR;
		return -1;
	}
	qwErrcode = stResponseMsg.head().result();

	if(qwErrcode != ::hoosho::msg::E_OK)
	{
		return 0;
	}

	vecFollowInfo.clear();
	const ::hoosho::j::feeds::GetFollowListRes& stGetFollowListRes = stResponseMsg.j_feeds_get_follow_list_res();
	for(int i = 0; i < stGetFollowListRes.follow_list_size(); i++)
	{
		vecFollowInfo.push_back(stGetFollowListRes.follow_list(i));
	}
	return 0;
}

inline int FetchFollowInfo(const std::string& strFeedsServerIp, const int& dwFeedsServerPort,
						const std::vector<uint64_t>& vecFollowIdList, uint64_t& qwErrcode,
						 std::string& strErrMsg, std::vector<hoosho::j::commstruct::FollowInfo>& vecFollowInfo)
{
	//Build requestMsg protobuf
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(hoosho::msg::J_GET_FOLLOW_DETAIL_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::j::feeds::GetFollowDetailReq* stGetFollowDetailReq = stRequestMsg.mutable_j_feeds_get_follow_detail_req();

	::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);

	for(size_t i = 0; i < vecFollowIdList.size(); i++)
	{
		stGetFollowDetailReq->add_follow_id_list(vecFollowIdList[i]);
	}

	//send to server, and recv responseMsg protobuf
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet != 0)
	{
		strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
		qwErrcode = ::hoosho::msg::E_SERVER_INNER_ERROR;
		return -1;
	}
	qwErrcode = stResponseMsg.head().result();

	if(qwErrcode != ::hoosho::msg::E_OK)
	{
		return 0;
	}

	vecFollowInfo.clear();
	const ::hoosho::j::feeds::GetFollowDetailRes& stGetFollowDetailRes = stResponseMsg.j_feeds_get_follow_detail_res();
	for(int i = 0; i < stGetFollowDetailRes.follow_list_size(); i++)
	{
		vecFollowInfo.push_back(stGetFollowDetailRes.follow_list(i));
	}
	return 0;
}

inline int FetchListenList(const std::string& strFeedsServerIp, const int& dwFeedsServerPort,
						const uint64_t& qwBeginTs, const uint64_t& qwLimit,
						const uint64_t& qwSceneType, const uint64_t& qwFeedId,
						const std::string& strOpenid, uint64_t& qwErrcode,
						 std::string& strErrMsg,
						 std::vector<hoosho::j::commstruct::FollowInfo>& vecFollowInfo)
{
	//Build requestMsg protobuf
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(hoosho::msg::J_GET_LISTEN_LIST_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::j::feeds::GetListenListReq* stGetListenListReq = stRequestMsg.mutable_j_feeds_get_listen_list_req();

	::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);

	stGetListenListReq->set_begin_ts(qwBeginTs);
	stGetListenListReq->set_openid(strOpenid);
	stGetListenListReq->set_limit(qwLimit);
	stGetListenListReq->set_scene_type(qwSceneType);
	stGetListenListReq->set_feed_id(qwFeedId);

	//send to server, and recv responseMsg protobuf
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet != 0)
	{
		strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
		qwErrcode = ::hoosho::msg::E_SERVER_INNER_ERROR;
		return -1;
	}
	qwErrcode = stResponseMsg.head().result();

	if(qwErrcode != ::hoosho::msg::E_OK)
	{
		return 0;
	}

	vecFollowInfo.clear();
	const ::hoosho::j::feeds::GetListenListRes& stGetListenListRes = stResponseMsg.j_feeds_get_listen_list_res();
	for(int i = 0; i < stGetListenListRes.follow_list_size(); i++)
	{
		vecFollowInfo.push_back(stGetListenListRes.follow_list(i));
	}
	return 0;
}

inline int FetchListenDetail(const std::string& strFeedsServerIp, const int& dwFeedsServerPort,
						const uint64_t& qwFollowId, const std::string& strOpenid,
							 uint64_t& qwErrcode, std::string& strErrMsg,
							 hoosho::j::commstruct::ListenInfo& stListenInfo,
						 hoosho::j::commstruct::FollowInfo& stFollowInfo)
{
	//Build requestMsg protobuf
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(hoosho::msg::J_GET_LISTEN_DETAIL_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::j::feeds::GetListenDetailReq* stGetListenDetailReq = stRequestMsg.mutable_j_feeds_get_listen_detail_req();

	::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);

	stGetListenDetailReq->set_follow_id(qwFollowId);
	stGetListenDetailReq->set_openid(strOpenid);

	//send to server, and recv responseMsg protobuf
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet != 0)
	{
		strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
		qwErrcode = ::hoosho::msg::E_SERVER_INNER_ERROR;
		return -1;
	}
	qwErrcode = stResponseMsg.head().result();

	if(qwErrcode != ::hoosho::msg::E_OK)
	{
		return 0;
	}

	const ::hoosho::j::feeds::GetListenDetailRes& stGetListenDetailRes = stResponseMsg.j_feeds_get_listen_detail_res();

	stListenInfo.CopyFrom(stGetListenDetailRes.listen_info());
	stFollowInfo.CopyFrom(stGetListenDetailRes.follow_info());

	return 0;
}

inline int FetchCommnetFollow(const std::string& strFeedsServerIp, const int& dwFeedsServerPort,
						 std::vector<hoosho::j::commstruct::CommentFollowInfo>& vecCommentFollow, uint64_t& qwErrcode,
						 std::string& strErrMsg)
{
	//Build requestMsg protobuf
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(hoosho::msg::J_GET_COMMENT_FOLLOW_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::j::feeds::GetCommentFollowReq* stGetCommentFollowReq = stRequestMsg.mutable_j_feeds_get_comment_follow_req();

	::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);

	for(size_t i = 0; i < vecCommentFollow.size(); i++)
	{
		::hoosho::j::commstruct::CommentFollowInfo* pCommentFollowInfo = stGetCommentFollowReq->add_comment_follow_list();
		pCommentFollowInfo->CopyFrom(vecCommentFollow[i]);
	}

	//send to server, and recv responseMsg protobuf
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet != 0)
	{
		strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
		qwErrcode = ::hoosho::msg::E_SERVER_INNER_ERROR;
		return -1;
	}
	qwErrcode = stResponseMsg.head().result();

	if(qwErrcode != ::hoosho::msg::E_OK)
	{
		return 0;
	}

	vecCommentFollow.clear();
	const ::hoosho::j::feeds::GetCommentFollowRes& stGetCommentFollowRes = stResponseMsg.j_feeds_get_comment_follow_res();
	for(int i = 0; i < stGetCommentFollowRes.comment_follow_list_size(); i++)
	{
		vecCommentFollow.push_back(stGetCommentFollowRes.comment_follow_list(i));
	}
	return 0;
}
inline int CheckListen(const std::string& strFeedsServerIp, const int& dwFeedsServerPort,
						 std::vector<hoosho::j::commstruct::CheckListenInfo>& vecCheckListen, uint64_t& qwErrcode,
						 std::string& strErrMsg)
{
	//Build requestMsg protobuf
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(hoosho::msg::J_CHECK_LISTEN_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::j::feeds::CheckListenReq* stCheckListenReq = stRequestMsg.mutable_j_feeds_check_listen_req();

	::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);

	for(size_t i = 0; i < vecCheckListen.size(); i++)
	{
		::hoosho::j::commstruct::CheckListenInfo* pCheckListenInfo = stCheckListenReq->add_check_listen_list();
		pCheckListenInfo->CopyFrom(vecCheckListen[i]);
	}

	//send to server, and recv responseMsg protobuf
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet != 0)
	{
		strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
		qwErrcode = ::hoosho::msg::E_SERVER_INNER_ERROR;
		return -1;
	}
	qwErrcode = stResponseMsg.head().result();

	if(qwErrcode != ::hoosho::msg::E_OK)
	{
		return 0;
	}

	vecCheckListen.clear();
	const ::hoosho::j::feeds::CheckListenRes& stCheckListenRes = stResponseMsg.j_feeds_check_listen_res();
	for(int i = 0; i < stCheckListenRes.check_listen_list_size(); i++)
	{
		vecCheckListen.push_back(stCheckListenRes.check_listen_list(i));
	}
	return 0;
}

inline int CheckBusinessIdValid(const std::string& strFeedsServerIp, const int& dwFeedsServerPort,
						 const uint64_t& qwBusinessId, const uint64_t& qwBusinessType, uint64_t& qwErrcode,
						 std::string& strErrMsg)
{
	//Build requestMsg protobuf
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(hoosho::msg::J_CHECK_BUSINESS_ID_VALID_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::j::feeds::CheckBusinessIdValidReq* stCheckBusinessIdValidReq = stRequestMsg.mutable_j_feeds_check_business_id_valid_req();

	::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);

	stCheckBusinessIdValidReq->set_business_id(qwBusinessId);
	stCheckBusinessIdValidReq->set_business_type(qwBusinessType);


	//send to server, and recv responseMsg protobuf
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet != 0)
	{
		strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
		qwErrcode = ::hoosho::msg::E_SERVER_INNER_ERROR;
		return -1;
	}
	qwErrcode = stResponseMsg.head().result();

	return 0;
}

inline int FetchHistoryList(const std::string& strFeedsServerIp, const int& dwFeedsServerPort,
						const uint64_t& qwBeginTs, const uint64_t& qwLimit,
						const std::string& strOpenid, uint64_t& qwErrcode,
						 std::string& strErrMsg,
						 std::vector<hoosho::j::commstruct::FeedInfo>& vecFeedInfo)
{
	//Build requestMsg protobuf
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(hoosho::msg::J_GET_HISTORY_LIST_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::j::feeds::GetHistoryListReq* stGetHistoryListReq = stRequestMsg.mutable_j_feeds_get_history_list_req();

	::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);

	stGetHistoryListReq->set_begin_ts(qwBeginTs);
	stGetHistoryListReq->set_openid(strOpenid);
	stGetHistoryListReq->set_limit(qwLimit);

	//send to server, and recv responseMsg protobuf
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet != 0)
	{
		strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
		qwErrcode = ::hoosho::msg::E_SERVER_INNER_ERROR;
		return -1;
	}
	qwErrcode = stResponseMsg.head().result();

	if(qwErrcode != ::hoosho::msg::E_OK)
	{
		return 0;
	}

	vecFeedInfo.clear();
	const ::hoosho::j::feeds::GetHistoryListRes& stGetHistoryListRes = stResponseMsg.j_feeds_get_history_list_res();
	for(int i = 0; i < stGetHistoryListRes.feed_list_size(); i++)
	{
		vecFeedInfo.push_back(stGetHistoryListRes.feed_list(i));
	}
	return 0;
}

#endif



