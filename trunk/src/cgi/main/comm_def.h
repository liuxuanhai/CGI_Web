#ifndef _WEB_HX_COMM_DEF_H_
#define _WEB_HX_COMM_DEF_H_

//图片是否私有
enum PICTURE_PRIVATE_TYPE 
{
	PICTURE_PRIVATE_TYPE_NO = 0, 
	PICTURE_PRIVATE_TYPE_YES = 1,
};

//平台来源
enum PLATFORM_SOURCE
{
	PLATFORM_SOURCE_MAIN_WEBSITE = 1,  //官网
	PLATFORM_SOURCE_WX_CARD = 2,  // 卡券平台
};

//文档类型
enum DOCUMENT_TYPE
{
	DOCUMENT_TYPE_TXT = 1,   //普通文本
	DOCUMENT_TYPE_WORD = 2,  // word文档
	DOCUMENT_TYPE_PPT = 3,   // ppt文档
	DOCUMENT_TYPE_EXCEL = 4,   // excel文档
};

//用户类型(需求方/服务方)
enum USER_TYPE
{
	USER_TYPE_DEMAND_SIDE = 1,  //需求方(用户/广告主)
	USER_TYPE_SERVICE_SIDE = 2, // 服务方(开发者/自媒体)
};

//开发者类型
enum DEVELOPER_TYPE
{
	DEVELOPER_TYPE_PERSON = 1,  //个人开发者
	DEVELOPER_TYPE_COMPANY = 2, // 公司开发者
};

//开发者状态
enum DEVELOPER_STATUS
{
	DEVELOPER_STATUS_VERIFY_PENDING = 0,  //待审核
	DEVELOPER_STATUS_VERIFY_OK = 1,       //审核通过
	DEVELOPER_STATUS_VERIFY_FAIL = 400,     //审核失败
};

//开发领域
enum DEVELOP_FIELD
{
	DEVELOP_FIELD_PAGE_RECONSTRUCTION = 1,  //页面重构
	DEVELOP_FIELD_FRONT_END = 2,           //前端
	DEVELOP_FIELD_BACK_END = 4,            //后台
	DEVELOP_FIELD_APP = 8,                 // 移动开发
	DEVELOP_FIELD_OPERATION = 16,         //运维管理
};

//开发语言
enum DEVELOP_LANGUAGE
{
	DEVELOP_LANGUAGE_C_OR_CPLUS = 1,      
	DEVELOP_LANGUAGE_JAVA = 2 , 
	DEVELOP_LANGUAGE_JAVASCRIPT = 4, 
	DEVELOP_LANGUAGE_H5 = 8, 
	DEVELOP_LANGUAGE_PHP = 16, 
	DEVELOP_LANGUAGE_OC = 32, 
	DEVELOP_LANGUAGE_PYTHON = 64, 
	DEVELOP_LANGUAGE_SHELL = 128, 
};

//运营者状态
enum OPERATOR_STATUS
{
	OPERATOR_STATUS_VERIFY_PENDING = 0,  //待审核
	OPERATOR_STATUS_VERIFY_OK = 1,       //审核通过
	OPERATOR_STATUS_VERIFY_FAIL = 400,     //审核失败
};

//开发订单状态
enum DEV_ORDER_STATUS
{
	DEV_ORDER_STATUS_VERIFY_PENDING = 0,  //审核中
	DEV_ORDER_STATUS_COMMUNICATION = 1,   //审核通过， 沟通中
	DEV_ORDER_STATUS_DEVELOPING = 2,      // 开发中
	DEV_ORDER_STATUS_FINISH = 3,           //完成
	DEV_ORDER_STATUS_CANCEL = 400,         //取消
};

//运营订单状态
enum OPER_ORDER_STATUS
{
	OPER_ORDER_STATUS_VERIFY_PENDING = 0,  //审核中
	OPER_ORDER_STATUS_COMMUNICATION = 1,   //审核完成， 沟通中
	OPER_ORDER_STATUS_OPERATING = 2,      // 推广中
	OPER_ORDER_STATUS_FINISH = 3,           //完成
	OPER_ORDER_STATUS_CANCEL = 400,         //取消
};

//开通订单类型
enum OPEN_ORDER_TYPE
{
	OPEN_ORDER_TYPE_ACCOUNT = 1,    //公众号
	OPEN_ORDER_TYPE_ACCOUNT_VERIFY = 2, //公众号+ 认证
	OPEN_ORDER_TYPE_ACCOUNT_VERIFY_PAY = 3, //公众号 + 认证+开通支付
	OPEN_ORDER_TYPE_FANS_ZONE = 4, //开通"粉丝空间"后台管理端
	OPEN_ORDER_TYPE_GAME = 5, //接入游戏
};

//开通订单状态
enum OPEN_ORDER_STATUS
{
	OPEN_ORDER_STATUS_VERIFY_PENDING = 0,  //审核中
	OPEN_ORDER_STATUS_COMMUNICATION = 1,   //审核通过， 沟通中
	OPEN_ORDER_STATUS_OPENATING = 2,      // 开通中
	OPEN_ORDER_STATUS_FINISH = 3,           //完成
	OPEN_ORDER_STATUS_CANCEL = 400,         //取消
};

//推广类型
enum SPREAD_TYPE
{
	SPREAD_TYPE_SOFT = 0,   //软广
	SPREAD_TYPE_HARD = 1, 	//硬广
};

//推广位置
enum SPREAD_POS
{
	SPREAD_POS_MULTI_PIC_TEXT_FIRST = 0,   //多图文第一条
	SPREAD_POS_MULTI_PIC_TEXT_SECOND = 1,  //多图文第二条
	SPREAD_POS_MULTI_PIC_TEXT_THIRD = 2,   //多图文第三条
	SPREAD_POS_SINGLE_PIC_TEXT = 3, //单图文
};

//大号是否经过微信平台认证
enum WX_PUBLIC_ACCOUNT_CERTIFY
{
	WX_PUBLIC_ACCOUNT_CERTIFY_NO = 0,  
	WX_PUBLIC_ACCOUNT_CERTIFY_YES = 1,
};

//大号审核状态
enum WX_PUBLIC_ACCOUNT_VERIFY
{
	WX_PUBLIC_ACCOUNT_VERIFY_PEDING = 0,  //待审核
	WX_PUBLIC_ACCOUNT_VERIFY_OK = 1,   //审核ok
	WX_PUBLIC_ACCOUNT_VERIFY_FAIL = 400,  //审核失败
};

//大号投诉原因
enum WX_PUBLIC_ACCOUNT_FEEDBACK_REASON
{
	WX_PUBLIC_ACCOUNT_FEEDBACK_REASON_FAKE_FANS_NUM = 1,  //粉丝造假
	WX_PUBLIC_ACCOUNT_FEEDBACK_REASON_FAKE_READ_NUM = 2, //阅读数目造假
	WX_PUBLIC_ACCOUNT_FEEDBACK_REASON_OTHER = 100, //其他
};


//CGI 登陆态check配置
enum CGI_NEED_LOGIN
{
	CGI_NEED_LOGIN_NO = 0,
	CGI_NEED_LOGIN_YES = 1,
};

//CGI返回码
enum CGI_RET_CODE
{
	CGI_RET_CODE_OK = 0,
	CGI_RET_CODE_NO_LOGIN = 100001,    //登陆态校验失败
	CGI_RET_CODE_SERVER_BUSY = 100002, //系统繁忙
	CGI_RET_CODE_INVALID_PARAM = 100003, //参数非法
	CGI_RET_CODE_INVALID_PASSWORD = 100004, //密码有误
	CGI_RET_CODE_NO_PREVILEDGES = 100005, //无权限
	CGI_RET_CODE_ALREADY_EXISTS = 100006, //重复了
	CGI_RET_CODE_NOT_DEV = 100007,   //还不是开发者
	CGI_RET_CODE_LIMITED_DEV = 100008,   //等待审核的开发者
	CGI_RET_CODE_NOT_OPER = 100009,     //还不是运营者
	CGI_RET_CODE_LIMITED_OPER = 100010, //等待审核的运营者
	CGI_RET_CODE_WXPA_USING = 100011,  //大号正在接单， 不能做修改， 删除操作
	CGI_RET_CODE_LOGIN_ROLE_ERR = 100012, //登录身份有问题
	CGI_RET_CODE_NOT_REGISTER_YET = 100013, //未注册
	CGI_RET_CODE_EMPTY_CONTENT = 100014, //内容为空
};

//业务ID类型
enum ID_TYPE
{
	ID_TYPE_PIC = 1,
	ID_TYPE_DOC = 2,
	ID_TYPE_USER = 3,
	ID_TYPE_DEV = 4,
	ID_TYPE_OPER = 5,
	ID_TYPE_WX_PUBLIC_ACCNOUT_FEEDBACK = 6,
	ID_TYPE_ORDER_DEV = 7,
	ID_TYPE_ORDER_OPER = 8,
	ID_TYPE_ORDER_OPEN = 9,
};



// COMMON
#define sql_escape(str) (lce::cgi::CMysql::MysqlEscape(str))
#define int_2_str(i) (lce::cgi::ToStr(i))

//time 
#define ONE_YEAR_LATER (time(0) + 365 * 24 * 3600)


//param check
#define EMPTY_STR_RETURN(p) if(p.empty()) \
	{ \
		DoReply(CGI_RET_CODE_INVALID_PARAM); \
		LOG4CPLUS_ERROR(logger, "empty string, param: "#p); \
		return true; \
	}

#define ZERO_INT_RETURN(p) if(p == 0) \
		{ \
			DoReply(CGI_RET_CODE_INVALID_PARAM); \
			LOG4CPLUS_ERROR(logger, "zero value, param: "#p); \
			return true; \
		}



#endif




