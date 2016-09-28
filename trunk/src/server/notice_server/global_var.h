#ifndef _NOTICE_SERVER_GLOBAL_VAR_H_
#define _NOTICE_SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "fsm_container.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "sys_notice_fsm.h"
#include "sys_notice_cache_pool.h"
#include "client_processor.h"
#include <sys/resource.h>
#include "app/timer.h"

extern NoticeServer* g_server;

extern lce::memory::FixedSizeAllocator* g_lbb_allcator;

extern ClientProcessor* g_client_processor;

extern ExecutorThreadProcessor* g_executor_thread_processor;

extern SysNoticeCachePool* g_sys_notice_cache_pool;

extern lce::app::TimerContainer* g_timer_container;

extern FsmContainer<SysNoticeFsm>* g_sys_notice_fsm_container;

enum NOTICE_TYPE
{
	SYS_NOTICE_LT = 1,
	NOTICE_USER_LT = 2,
	NOTICE_USER_LIKED_LT = 3, //1:被点赞
	NOTICE_USER_COMMENTED_LT = 4, //2：被评论
	NOTICE_USER_FOLLOWED_LT = 5, //3：被关注
//	NOTICE_USER_MESSAGED_LT = 6, //4：收到私信
};
enum SYS_NOTICE_QUERY_FLAG
{
	NOTICE_NOTIFY_SYS_NOTICE = 1,
	SYS_NOTICE_RECORD = 2,
};
enum NOTICE_USER_QUERY_FLAG
{
	NOTICE_NOTIFY = 1,
	NOTICE_NOTIFY_TYPE_LIST = 2,
	NOTICE_NOTIFY_TYPE=3,
	NOTICE_RECORD = 4
};
enum NOTICE_USER_TYPE_LIST_TYPE
{
	NOTICE_USER_LIKED = 1, //1:被点赞
	NOTICE_USER_COMMENTED = 2, //2：被评论
	NOTICE_USER_FOLLOWED = 3, //3：被关注
//	NOTICE_USER_MESSAGED = 4, //4：收到私信
};
#define DB_FAIL -1
#define DB_SUCCESS 1
#define DB_NONEXIST 0

#endif 

