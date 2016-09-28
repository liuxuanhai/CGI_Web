#ifndef _HOOSHO_MSG_SERVER_GLOBAL_VAR_H_
#define _HOOSHO_MSG_SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "fsm_container.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "msg_fsm.h"
#include "msg_cache_pool.h"
#include "client_processor.h"
#include "server_processor_notice.h"
#include <sys/resource.h>
#include "app/timer.h"

extern MsgServer* 						  	g_server;

extern lce::memory::FixedSizeAllocator*   	g_lbb_allcator;

extern ClientProcessor*                   	g_client_processor;

extern ServerProcessorNotice*			  	g_server_processor_notice;

extern ExecutorThreadProcessor*           	g_executor_thread_processor;

extern MsgContentCachePool*                 g_msg_content_cache_pool;

extern lce::app::TimerContainer*          	g_timer_container;

extern FsmContainer<MsgFsm>*         		g_msg_content_fsm_container;

#define sql_escape(str) (lce::cgi::CMysql::MysqlEscape(str))


enum RED_POINT_OPT_TYPE_ENUM
{
	RED_POINT_OPT_TYPE_REDUCE = 0,
	RED_POINT_OPT_TYPE_INCREASE = 1,
	RED_POINT_OPT_TYPE_CLEAR = 2,
};


#endif 

