#ifndef _MANAGER_CGI_UTIL_H_
#define _MANAGER_CGI_UTIL_H_

#include "msg.pb.h"


inline lce::cgi::CAnyValue ToAnyValue(const hoosho::commstruct::FeedInfo& feed)
{
	lce::cgi::CAnyValue stAnyValue;

	stAnyValue["pa_appid_md5"] = int_2_str(feed.pa_appid_md5());
	stAnyValue["openid_md5"] = int_2_str(feed.openid_md5());
	stAnyValue["feed_id"] = int_2_str(feed.feed_id());
	stAnyValue["cover_pic_id"] = int_2_str(feed.cover_pic_id());
	stAnyValue["pic_id_list"] = feed.pic_id_list();
	stAnyValue["content"] = feed.content();
	stAnyValue["num_favorite"] = feed.num_favorite();
	stAnyValue["num_share"] = feed.num_share();
	stAnyValue["num_comment"] = feed.num_comment();
	stAnyValue["num_read"] = feed.num_read();
	stAnyValue["create_ts"] = feed.create_ts();
	stAnyValue["del_ts"] = feed.del_ts();
	stAnyValue["feed_type"] = feed.feed_type();
	stAnyValue["extra_data_0"] = feed.extra_data_0();
	stAnyValue["extra_data_1"] = feed.extra_data_1();
	stAnyValue["extra_data_2"] = feed.extra_data_2();
	stAnyValue["num_feed"] = feed.num_feed();
	return stAnyValue;
}

inline lce::cgi::CAnyValue ToAnyValue(const hoosho::commstruct::FollowInfo& follow)
{
	lce::cgi::CAnyValue stAnyValue;

	stAnyValue["follow_id"] = int_2_str(follow.follow_id());
	stAnyValue["content"] = follow.content();
	stAnyValue["feed_id"] = int_2_str(follow.feed_id());
	stAnyValue["type"] = follow.type();
	stAnyValue["origin_comment_id"] = int_2_str(follow.origin_comment_id());
	stAnyValue["openid_md5_from"] = int_2_str(follow.openid_md5_from());
	stAnyValue["openid_md5_to"] = int_2_str(follow.openid_md5_to());
	stAnyValue["create_ts"] = follow.create_ts();
	stAnyValue["del_ts"] = follow.del_ts();

	return stAnyValue;
}


inline lce::cgi::CAnyValue ToAnyValue(const hoosho::commstruct::UserDetailInfo& pb)
{
	lce::cgi::CAnyValue stAnyValue;
	stAnyValue["openid_md5"] = int_2_str(pb.openid_md5()); \
	stAnyValue["portrait_pic_id"] = int_2_str(pb.portrait_pic_id()); \
	stAnyValue["wx_portrait_url"] = pb.portrait_pic_url_wx(); \
	stAnyValue["nick"] = pb.nick(); \
	stAnyValue["wx_nick"] = pb.nick_wx(); \
	stAnyValue["addr_country"] = pb.addr_country(); \
	stAnyValue["addr_city"] = pb.addr_city(); \
	stAnyValue["sex"] = pb.sex(); \
	stAnyValue["phone_no"] = int_2_str(pb.phone_no()); \
	stAnyValue["email"] = pb.email(); \
	stAnyValue["self_desc"] = pb.self_desc(); \
	stAnyValue["main_page_cover_pic_id"] = int_2_str(pb.main_page_cover_pic_id()); \
	stAnyValue["hoosho_no"] = int_2_str(pb.hoosho_no()); \
	stAnyValue["birthday"] = pb.birthday_ts(); \
	return stAnyValue;
}


#define UserInfoPB2Any(pb, any) \
{ \
	any["openid_md5"] = int_2_str(pb.openid_md5()); \
	any["portrait_pic_id"] = int_2_str(pb.portrait_pic_id()); \
	any["wx_portrait_url"] = pb.portrait_pic_url_wx(); \
	any["nick"] = pb.nick(); \
	any["wx_nick"] = pb.nick_wx(); \
	any["addr_country"] = pb.addr_country(); \
	any["addr_city"] = pb.addr_city(); \
	any["sex"] = pb.sex(); \
	any["phone_no"] = int_2_str(pb.phone_no()); \
	any["email"] = pb.email(); \
	any["self_desc"] = pb.self_desc(); \
	any["main_page_cover_pic_id"] = int_2_str(pb.main_page_cover_pic_id()); \
	any["hoosho_no"] = int_2_str(pb.hoosho_no()); \
	any["birthday"] = pb.birthday_ts(); \
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
	else if(iRet == hoosho::msg::E_FEED_DELETED)	\
	{  \
		DoReply(CGI_RET_CODE_FEED_DELETED);  \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: feed deleted");  \
	}  \
	else if(iRet == hoosho::msg::E_REPLY_TO_SELF_INVALID)	\
	{  \
		DoReply(CGI_RET_CODE_INVALID_OP);  \
		LOG4CPLUS_ERROR(logger, "SERVER RET ERROR: invalid op");  \
	}  \
	else \
	{  \
		DoReply(CGI_RET_CODE_SERVER_BUSY);  \
		LOG4CPLUS_ERROR(logger, "server return unknow error, ret = " << iRet);  \
	}  \
	return true;  \
}


#endif

