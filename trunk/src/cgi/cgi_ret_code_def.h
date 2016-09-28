#ifndef _CGI_COMMON_RET_CODE_H_
#define _CGI_COMMON_RET_CODE_H_

#include "util/lce_util.h"

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
	CGI_RET_CODE_FEED_DELETED = 100008, //feed已删除

	CGI_RET_CODE_MANAGER_NOT_EXIST = 100009, //不存在该管理者
	CGI_RET_CODE_USER_FORBID = 100010, //用户被禁止发言
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


#endif



