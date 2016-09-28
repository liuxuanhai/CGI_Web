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
#include "common_util.h"
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
#define timems() ::common::util::GetCurrentMs()

#define HOOSHO_USER_BASE_ID 10158643
extern uint64_t max_user_id;

#define HOOSHO_SHOW_BASE_ID 6043239
extern uint64_t max_show_id;

#define UserInfoDB2PB(mysql, pb) \
{ \
	pb.set_user_id(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_openid(mysql.GetRow(1)); \
	pb.set_nickname(mysql.GetRow(2)); \
	pb.set_sex(atoi(mysql.GetRow(3))); \
	pb.set_headimgurl(mysql.GetRow(4)); \
	pb.set_self_desc(mysql.GetRow(5)); \
	pb.set_phone(mysql.GetRow(6)); \
	pb.set_identity_status(atoi(mysql.GetRow(7))); \
	pb.set_user_type(atoi(mysql.GetRow(8))); \
	pb.set_default_addr_num(atoi(mysql.GetRow(9))); \
	pb.set_extra_data(mysql.GetRow(10)); \
}

#define UserIdentityInfoDB2PB(mysql, pb) \
{ \
	pb.set_user_id(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_identity_status(atoi(mysql.GetRow(1))); \
	pb.set_phone(mysql.GetRow(2)); \
	pb.set_identity_num(mysql.GetRow(3)); \
	pb.set_identity_pic1_id(mysql.GetRow(4)); \
	pb.set_identity_pic2_id(mysql.GetRow(5)); \
	pb.set_identity_pic3_id(mysql.GetRow(6)); \
	pb.set_create_ts(strtoul(mysql.GetRow(7), NULL, 10)); \
	pb.set_check_ts(strtoul(mysql.GetRow(8), NULL, 10)); \
	pb.set_check_mgr(mysql.GetRow(9)); \
}

#define UserRecvInfoDB2PB(mysql, pb) \
{ \
	pb.set_user_id(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_addr_num(atoi(mysql.GetRow(1))); \
	pb.set_recv_name(mysql.GetRow(2)); \
	pb.set_identity_num(mysql.GetRow(3)); \
	pb.set_phone(mysql.GetRow(4)); \
	pb.set_post_num(mysql.GetRow(5)); \
	pb.set_addr_province(mysql.GetRow(6)); \
	pb.set_addr_city(mysql.GetRow(7)); \
	pb.set_addr_district(mysql.GetRow(8)); \
	pb.set_addr_detail(mysql.GetRow(9)); \
}

#define GoodTypeDB2PB(mysql, pb) \
{ \
	pb.set_good_type(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_good_type_text(mysql.GetRow(1)); \
	pb.set_good_type_icon(mysql.GetRow(2)); \
	pb.set_extra_data(mysql.GetRow(3)); \
	pb.set_create_ts(strtoul(mysql.GetRow(4), NULL, 10)); \
}

#define GoodInfoDB2PB(mysql, pb) \
{ \
	pb.set_good_id(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_good_id_text(mysql.GetRow(1)); \
	pb.set_good_type(strtoul(mysql.GetRow(2), NULL, 10)); \
	pb.set_title(mysql.GetRow(3)); \
	pb.set_good_desc(mysql.GetRow(4)); \
	pb.set_show_pics(mysql.GetRow(5)); \
	pb.set_detail_pic_id(strtoul(mysql.GetRow(6), NULL, 10)); \
	pb.set_total_join_num(strtoul(mysql.GetRow(7), NULL, 10)); \
	pb.set_create_ts(strtoul(mysql.GetRow(8), NULL, 10)); \
}

#define FeedInfoDB2PB(mysql, pb) \
{ \
	pb.set_feed_id(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_good_id(strtoul(mysql.GetRow(1), NULL, 10)); \
	pb.set_total_join_num(strtoul(mysql.GetRow(2), NULL, 10)); \
	pb.set_current_join_num(strtoul(mysql.GetRow(3), NULL, 10)); \
	pb.set_approved_num(strtoul(mysql.GetRow(4), NULL, 10)); \
	pb.set_pending_num(strtoul(mysql.GetRow(5), NULL, 10)); \
	pb.set_create_ts(strtoul(mysql.GetRow(6), NULL, 10)); \
	pb.set_feed_status(strtoul(mysql.GetRow(7), NULL, 10)); \
	pb.set_finish_ts(strtoul(mysql.GetRow(8), NULL, 10)); \
	pb.set_luck_contend_id(strtoul(mysql.GetRow(9), NULL, 10)); \
	pb.set_luck_user_id(strtoul(mysql.GetRow(10), NULL, 10)); \
	pb.set_luck_ts(strtoul(mysql.GetRow(11), NULL, 10)); \
	pb.set_show_status(atoi(mysql.GetRow(12))); \
	pb.set_recv_addr(mysql.GetRow(13)); \
	pb.set_express_name(mysql.GetRow(14)); \
	pb.set_express_num(mysql.GetRow(15)); \
	pb.set_extra_data(mysql.GetRow(16)); \
}

#define ShowInfoDB2PB(mysql, pb) \
{ \
	pb.set_show_id(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_user_id(strtoul(mysql.GetRow(1), NULL, 10)); \
	pb.set_feed_id(strtoul(mysql.GetRow(2), NULL, 10)); \
	pb.set_title(mysql.GetRow(3)); \
	pb.set_show_desc(mysql.GetRow(4)); \
	pb.set_show_pics(mysql.GetRow(5)); \
	pb.set_create_ts(strtoul(mysql.GetRow(6), NULL, 10)); \
}

#define CollectInfoDB2PB(mysql, pb) \
{ \
	pb.set_user_id(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_feed_id(strtoul(mysql.GetRow(1), NULL, 10)); \
	pb.set_create_ts(strtoul(mysql.GetRow(2), NULL, 10)); \
}

#define ContentdInfoDB2PB(mysql, pb) \
{ \
	pb.set_ticket_id(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_user_id(strtoul(mysql.GetRow(1), NULL, 10)); \
	pb.set_user_ip(mysql.GetRow(2)); \
	pb.set_feed_id(strtoul(mysql.GetRow(3), NULL, 10)); \
	pb.set_contend_id(strtoul(mysql.GetRow(4), NULL, 10)); \
	pb.set_create_ts(strtoul(mysql.GetRow(5), NULL, 10)); \
}

#define BannerInfoDB2PB(mysql, pb) \
{ \
	pb.set_banner_id(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_pic_id(strtoul(mysql.GetRow(1), NULL, 10)); \
	pb.set_content_type(atoi(mysql.GetRow(2))); \
	pb.set_content(mysql.GetRow(3)); \
	pb.set_del_status(atoi(mysql.GetRow(4))); \
	pb.set_create_ts(strtoul(mysql.GetRow(5), NULL, 10)); \
}

#define TicketInfoDB2PB(mysql, pb) \
{ \
	pb.set_ticket_id(strtoul(mysql.GetRow(0), NULL, 10)); \
	pb.set_ticket_pics(mysql.GetRow(1)); \
	pb.set_user_id(strtoul(mysql.GetRow(2), NULL, 10)); \
	pb.set_user_ip(mysql.GetRow(3)); \
	pb.set_feed_id(strtoul(mysql.GetRow(4), NULL, 10)); \
	pb.set_check_status(atoi(mysql.GetRow(5))); \
	pb.set_contend_id(strtoul(mysql.GetRow(6), NULL, 10)); \
	pb.set_ticket_price(strtoul(mysql.GetRow(7), NULL, 10)); \
	pb.set_consume_type(atoi(mysql.GetRow(8))); \
	pb.set_good_id(strtoul(mysql.GetRow(9), NULL, 10)); \
	pb.set_create_ts(strtoul(mysql.GetRow(10), NULL, 10)); \
	pb.set_check_ts(strtoul(mysql.GetRow(11), NULL, 10)); \
	pb.set_check_mgr(mysql.GetRow(12)); \
}

enum SEX
{
	SEX_UNKNOWN = 0,
	SEX_MALE = 1, 
	SEX_FEMALE = 2,
	SEX_BOTH = 4, //fuck
};

enum USER_LOGIN_STATUS
{
	USER_LOGIN_STATUS_SUCCESS = 0,
	USER_LOGIN_STATUS_NOT_EXIST = 1,
	USER_LOGIN_STATUS_EXPIRED = 2,
};

enum TICKET_CHECK_STATUS
{
	TICKET_CHECK_STATUS_PENDING = 1,
	TICKET_CHECK_STATUS_PASS = 2,
	TICKET_CHECK_STATUS_FAIL = 3,
};

enum SHOW_STATUS
{
	SHOW_STATUS_UNDO = 0,
	SHOW_STATUS_DONE = 1,
};

enum PIC_TYPE
{
	PIC_TYPE_PUBLIC = 0,
	PIC_TYPE_PRIVATE = 1,
};

enum USER_IDENTITY_STATUS
{
	USER_IDENTITY_STATUS_DEFAULT = 0,
	USER_IDENTITY_STATUS_PENDING = 1,
	USER_IDENTITY_STATUS_PASS = 2,
	USER_IDENTITY_STATUS_FAIL = 3,
};


#endif 

