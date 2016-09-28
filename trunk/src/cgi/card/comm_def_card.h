#ifndef _WEB_HX_COMM_CARD_DEF_H_
#define _WEB_HX_COMM_CARD_DEF_H_
enum LOGIN_TYPE
{
	NO_LOGIN = 0, USER_LOGIN = 1, USER_LOGIN_NOLOGIN = 2,
};
enum CARD_OWNED
{
	CARD_OWNED_OURS = 1, CARD_OWNED_OTHERS = 2
};

//user type
enum USER_TYPE
{
	USER_NO_LIMIT = 0,

	USER_NORMAL = 1,
	USER_FACILITATOR = 2,
	USER_FACILITATOR_DEVLOPER = 3,
	USER_FACILITATOR_OPERATOR = 4,
	USER_FACILITATOR_INVESTOR = 5,
	USER_FACILITATOR_OUT_LIMIT = 6,
	USER_OUT_LIMIT = 7
};
enum PICTURE_STATUS
{
	PICTURE_STATUS_PUBLIC = 1, PICTURE_STATUS_PRIVATE = 2
};

enum ENTITY_SHOP_STATUS
{
	ENTITY_SHOP_STATUS_NO_UPDATE = 0, ENTITY_SHOP_STATUS_UPDATING = 1
};

enum CGI_RET_CODE
{
	CGI_RET_CODE_OK = 0, CGI_RET_CODE_NO_LOGIN = 100001, //��¼̬У��ʧ��
	CGI_RET_CODE_SERVER_BUSY = 100002, //��������æ
	CGI_RET_CODE_INVALID_PARAM = 100003, //������Ч
	CGI_RET_CODE_INVALID_PASSWORD = 100004, //�������
	CGI_RET_CODE_NO_PREVILEDGES = 100005, //��Ȩ�鿴
	CGI_RET_CODE_ALREADY_EXISTS = 100006, //�Ѿ����ڣ������ظ��ύ
	CGI_RET_CODE_WX_INVALID_CODE = 40099,  // has consumed
	CGI_RET_CODE_WX_INVALID_TIME = 40079,  
	CGI_RET_CODE_WX_INVALID_SERIAL_CODE = 40056,  // has send
	CGI_RET_CODE_WX_INVALID_CATEGORIES = 65104, //
	CGI_RET_CODE_WX_INVALID_FILE_TYPE = 40005,
	CGI_RET_CODE_WX_INVALID_ARGS = 40097,
	CGI_RET_CODE_WX_INVALID_SIZE = 45001,
	CGI_RET_CODE_WX_REACH_MAX_MONTH_LIMIT = 45040,
};
enum OPTIONS_ID
{
	OPTIONS_ID_VERIFY_TICKET = 1, //΢�����͵�verify ticket
};
enum ID_TYPE
{
	ID_TYPE_PICTURE = 7, // ͼƬI
	ID_TYPE_ENTITY_SHOP = 16, // Entity_shop ID
};
// 卡券

enum CARD_TYPE
{
	CARD_MEMBER = 1, //会员卡
	CARD_GROUPON = 2, //团购券
	CARD_CASH = 3, // 代金券
	CARD_DISCOUNT = 4, //折扣券
	CARD_GIFT = 5, //礼品券
	CARD_GENERAL_COUPON = 6, //通用券
};
enum CARD_STATUS
{
	CARD_NOT_VERIFY = 1, //待审核
	CARD_PASS = 2, // 审核通过
	CARD_NOT_PASS = 3, // 审核不通过
	CARD_EXPIRED = 4, //过期
	CARD_DELETE = 5, // 用戶操作刪除
	CARD_DISPATCH = 2, // 在公众平台投放过的卡券
};
enum ENTITYSHOP_STATUS
{
	ENTITYSHOP_SYSTEM_ERROR = 1, //1 表示系统错误
	ENTITYSHOP_NOT_VERIFY = 2, //待审核
	ENTITYSHOP_PASS = 3, // 审核通过
	ENTITYSHOP_NOT_PASS = 4, // 审核不通过
	ENTITYSHOP_EXPIRED = 5, //过期
	ENTITYSHOP_DELETE = 6, // 用戶操作刪除
};
enum SUBMERCHANT_STATUS
{
	SUBMERCHANT_NOT_VERIFY = 1, //待审核
	SUBMERCHANT_PASS = 2, // 审核通过
	SUBMERCHANT_NOT_PASS = 3, // 审核不通过
	SUBMERCHANT_EXPIRED = 4, //过期
	SUBMERCHANT_DELETE = 5,
};

#define WX_THIRD_PLATFORM_DEV_APPID "wx2930c9a0f682439b"
#define WX_THIRD_PLATFORM_DEV_SECRET "0c79e1fa963cd80cc0be99b20a18faeb"
#define WX_THIRD_PLATFORM_DEV_MSG_CHECK_TOKEN "c78b29cc0e0c9d9952a3ed0575789a6b"
#define WX_THIRD_PLATFORM_DEV_MSG_CRYPT_KEY   "c78b29cc0e0c9d9952a3ed0575789a6b12345678912"
#define WX_HX_PLATFORM_DEV_MSG_CHECK_TOKEN "WORKFORSUPERHOOSHOLEAGUEWWSSADAD"
#define WX_HX_PLATFORM_DEV_MSG_CRYPT_KEY "VBHILTDpbf4jhxF9CCEqb3n68feRBvgGrqROS3ceN50"
#define WX_HX_PLATFORM_DEV_APPID "wxfd93b50f7122f71b"
#define WX_HX_PLATFORM_DEV_SECRET "26a8bb809b08e62b45be236e9ea0f36d"

#define sql_escape(str) (lce::cgi::CMysql::MysqlEscape(str))
#define int_2_str(i) (lce::cgi::ToStr(i))

//param check
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

#define CHECK_DB(p,t)  if(p.SelectFromDB(t) != TableBaseCard::TABLE_BASE_RET_OK) \
	{ \
		LOG4CPLUS_ERROR(logger, #p<<".SelectFromDB failed, errmsg="<<t);\
		DoReply(CGI_RET_CODE_SERVER_BUSY);\
		return true;\
	}

#define CHECK_DB_NOT_EXIST(p,t,id) if(p.SelectFromDB(t) != TableBaseCard::TABLE_BASE_RET_NOT_EXIST) \
	{ \
		DoReply(CGI_RET_CODE_ALREADY_EXISTS);\
		LOG4CPLUS_ERROR(logger, "db "<<p.m_table_name<<"already exists "<<#id<<"="<<id<<" , errmsg = " << t);\
		return true;\
	}
#define UPDATE_DB(p,t) if(p.UpdateToDB(t) != TableBaseCard::TABLE_BASE_RET_OK) \
    { \
        DoReply(CGI_RET_CODE_SERVER_BUSY); \
		LOG4CPLUS_ERROR(logger,#p<<".UpdateToDB failed, errmsg="<<t); \
		return true;\
    }
#define NOTEMPTY_STR_CHECK(p) (!p.empty())? true:false

#define ZERO_INT_RETURN(p) if(p == 0) \
		{ \
			DoReply(CGI_RET_CODE_INVALID_PARAM); \
			LOG4CPLUS_ERROR(logger, "invalid "<<#p<<" ="<<p); \
			return true; \
		}

#endif
