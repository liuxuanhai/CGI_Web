#ifndef _FEEDS_SERVER_GLOBAL_VAR_H_
#define _FEEDS_SERVER_GLOBAL_VAR_H_

//#define LOCAL_TEST

#include "server.h"
#include "fsm_container.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "fsm_feeds.h"
#include "fsm_follow.h"
//#include "feeds_cache_pool.h"
#include "client_processor.h"
#include "server_processor_transcode.h"
#include "server_processor_pay.h"
#include "server_processor_token.h"
#include <sys/resource.h>
#include "app/timer.h"

extern FeedsServer* 						  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern ServerProcessorPay*              g_server_processor_pay;

extern ServerProcessorTranscode*             g_server_processor_transcode;

extern ServerProcessorToken*			g_server_processor_token;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

//extern FeedsCachePool*                 	  g_feeds_cache_pool;

extern lce::app::TimerContainer*          g_timer_container;

extern FsmContainer<FsmFeeds>*         g_fsm_feeds_container;

extern FsmContainer<FsmFollow>*         g_fsm_follow_container;


#define sql_escape(str) (lce::cgi::CMysql::MysqlEscape(str))

#endif

