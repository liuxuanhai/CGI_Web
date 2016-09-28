#ifndef _HOOSHO_PA_SERVER_GLOBAL_VAR_H_
#define _HOOSHO_PA_SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "fsm_container.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "fsm_business.h"
#include "client_processor.h"
#include "token_server_processor.h"
#include <sys/resource.h>
#include "app/timer.h"
#include "util/lce_util.h"
#include "cgi/cgi_mysql.h"
#include <stdio.h>

extern ZServer* 						  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern TokenServerProcessor*              g_token_server_processor;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

extern lce::app::TimerContainer*          g_timer_container;

extern FsmContainer<FsmBusiness>*         g_fsm_business_container;


#define sql_escape(str) (lce::cgi::CMysql::MysqlEscape(str))
#define int_2_str(i) lce::util::StringOP::TypeToStr(i)

#define DB_ORDER_INFO_OPENID_WAITING_COMPONENT_LEN 2
#define DB_ORDER_INFO_OPENID_WAITING_MAX_NUM 10

#define DATE21160101 4607251200

#define UserInfoBD2PB(mysql, pb) \
{ \
	pb.set_openid(mysql.GetRow(0));	\
	pb.set_nickname(mysql.GetRow(1)); \
	pb.set_sex(atoi(mysql.GetRow(2))); \
	pb.set_headimgurl(mysql.GetRow(3)); \
	pb.set_self_desc(mysql.GetRow(4)); \
	pb.set_phone(mysql.GetRow(5)); \
	pb.set_user_type(atoi(mysql.GetRow(6))); \
	pb.set_user_flag(strtoul(mysql.GetRow(7), NULL, 10)); \
	pb.set_user_score(strtoul(mysql.GetRow(8), NULL, 10)); \
}

enum SEX
{
	SEX_UNKNOWN = 0,
	SEX_MALE = 1, 
	SEX_FEMALE = 2,
	SEX_BOTH = 4, //fuck
};

enum CASH_FLOW_TYPE
{
	CASH_FLOW_TYPE_RECHARGE = 1,  //充值

	CASH_FLOW_TYPE_CONSUME_ORDER_CREATE = 2, //下单 支出
	CASH_FLOW_TYPE_CONSUME_ORDER_REWARD = 3, //打赏 支出

	CASH_FLOW_TYPE_INCOME_FETCH = 4, //抢单收入
	CASH_FLOW_TYPE_INCOME_ORDER_CANCELED = 5, //取消订单退款
	CASH_FLOW_TYPE_INCOME_ORDER_REWARD_BY_USER = 6, //打赏收入

	CASH_FLOW_TYPE_OUTCOME = 7, // 提现

	CASH_FLOW_TYPE_INCOME_ORDER_REWARD_BY_PA = 8,
};

enum OUTCOME_STATE
{
	OUTCOME_STATE_WAITING = 1,
	OUTCOME_STATE_SUCC = 2,
	OUTCOME_STATE_FAIL = 3,
};

enum ORDER_VISIBLE
{
	ORDER_VISIBLE_ONLY_TRUST = 1,
	ORDER_VISIBLE_ALL = 2,
};

enum ORDER_STATUS
{
	ORDER_STATUS_INIT = 0,
	ORDER_STATUS_WAITING_FETCH = 1,
	ORDER_STATUS_FETCHED = 2,
	ORDER_STATUS_FINISHED = 3,
	ORDER_STATUS_CANCELED = 4,
};

enum ORDER_PAY_COST_TYPE
{
	ORDER_PAY_COST_TYPE_USER_CASH = 1,
	ORDER_PAY_COST_TYPE_WX_PAY = 2,
};

enum ORDER_PAY_STATUS
{ 
	ORDER_PAY_STATUS_NOT_PAYED_YET = 1,
	ORDER_PAY_STATUS_PAYED = 2,
};

enum WX_UNINFIED_ORDER_PAY_STATE
{
	WX_UNINFIED_ORDER_PAY_STATE_WAITING_PAY = 0,
	WX_UNINFIED_ORDER_PAY_STATE_SUCC_PAY = 1,
	WX_UNINFIED_ORDER_PAY_STATE_FAIL_PAY = 2,
};

enum WX_UNINFIED_ORDER_PAY_REASON
{
	WX_UNINFIED_ORDER_PAY_REASON_CREATE_ORDER = 1,
	WX_UNINFIED_ORDER_PAY_REASON_REWARD_ORDER = 2,
};

enum USER_LOGIN_STATUS
{
	USER_LOGIN_STATUS_SUCCESS = 0,
	USER_LOGIN_STATUS_NOT_EXIST = 1,
	USER_LOGIN_STATUS_EXPIRED = 2,
};

enum USER_TRUST_TYPE
{
	USER_TRUST_TYPE_ADD = 1,
	USER_TRUST_TYPE_CANCEL = 2,
};

enum USER_TRUST_SB_STATUS
{
	USER_TRUST_SB_STATUS_NO = 0,
	USER_TRUST_SB_STATUS_YES = 1,	
};

enum ORDER_FAVOR_OPER_TYPE
{
	ORDER_FAVOR_OPER_TYPE_ADD = 1,
	ORDER_FAVOR_OPER_TYPE_DEL = 2,
};

enum ORDER_FOLLOW_TYPE
{
	ORDER_FOLLOW_TYPE_COMMMENT = 1,
	ORDER_FOLLOW_TYPE_REPLY = 2,
};

enum USER_TYPE
{
	USER_TYPE_COMMON = 1, 
	USER_TYPE_ZOMBIE = 2,
};

enum USER_FLAG
{
	USER_FLAG_DEFAULT = 0x0000000000000000,
	USER_FLAG_NEWUSER = 0x0000000000000001,
};

enum ORDER_TYPE
{
	ORDER_TYPE_COMMON = 1,
	ORDER_TYPE_INVOICE = 2, //小票
};

enum ORDER_MEDIA_TYPE
{
	ORDER_MEDIA_TYPE_TEXT = 1,
	ORDER_MEDIA_TYPE_AUDIO = 2,
	ORDER_MEDIA_TYPE_PICTURE = 3,
};

enum ORDER_OPENID_FETCH_STATE
{
	ORDER_OPENID_FETCH_STATE_WAITING = 1,
	ORDER_OPENID_FETCH_STATE_ACCEPTED = 2,	
};

enum MSG_DEL_STATUS
{
	MSG_DEL_STATUS_NODEL = 0x00,
	MSG_DEL_STATUS_BY_MIN = 0x01,
	MSG_DEL_STATUS_BY_MAX = 0x02,	
	MSG_DEL_STATUS_BY_BOTH = 0x03,
};

enum MSG_NEW_STATUS
{
	MSG_NEW_STATUS_BOTH_FALSE = 0x00,
	MSG_NEW_STATUS_MIN_TRUE = 0x01,
	MSG_NEW_STATUS_MAX_TRUE = 0x02,
	MSG_NEW_STATUS_BOTH_TRUE = 0x03,
};

enum RED_POINT_TYPE
{
	RED_POINT_TYPE_FAVORITE = 1,
	RED_POINT_TYPE_COMMENT = 2,
	RED_POINT_TYPE_MSG = 3,
	RED_POINT_TYPE_TRUST = 4,
};

enum RED_POINT_OPT_TYPE
{
	RED_POINT_OPT_TYPE_REDUCE = 0,
	RED_POINT_OPT_TYPE_INCREASE = 1,
	RED_POINT_OPT_TYPE_CLEAR = 2,
};

enum NOTICE_TYPE
{
	NOTICE_TYPE_FAVORITE = 1,
	NOTICE_TYPE_COMMENT = 2,
};

enum NOTICE_STATUS
{
	NOTICE_STATUS_UNREAD = 0,
	NOTICE_STATUS_READ = 1,
};

enum USER_LEVEL_SCORE
{
	USER_LEVEL_SCORE_1 = 0,
	USER_LEVEL_SCORE_2 = 5,
	USER_LEVEL_SCORE_3 = 30,
	USER_LEVEL_SCORE_4 = 80,
	USER_LEVEL_SCORE_5 = 200,
	USER_LEVEL_SCORE_6 = 500,	
};

enum USER_LEVEL
{
	USER_LEVEL_1 = 1,	//default
	USER_LEVEL_2 = 2,	// >=5
	USER_LEVEL_3 = 3,	// >=30
	USER_LEVEL_4 = 4,	// >=80
	USER_LEVEL_5 = 5,	// >=200
	USER_LEVEL_6 = 6, 	// >=500
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

enum RANKING_TYPE
{
	RANKING_TYPE_ALL = 1,
	RANKING_TYPE_TRUST = 2,
};
#endif 

