#ifndef _FEEDS_SERVER_GLOBAL_VAR_H_
#define _FEEDS_SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "fsm_container.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "fsm_feed.h"
#include "fsm_follow.h"
#include "fsm_favorite.h"
#include "fsm_collect.h"
//#include "feeds_cache_pool.h"
#include "client_processor.h"
#include "server_processor_msg.h"
#include <sys/resource.h>
#include "app/timer.h"

extern FeedsServer* 						  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern ServerProcessorMsg*             g_server_processor_msg;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

//extern FeedsCachePool*                 	  g_feeds_cache_pool;

extern lce::app::TimerContainer*          g_timer_container;

extern FsmContainer<FsmFeed>*         g_fsm_feed_container;

extern FsmContainer<FsmFollow>*         g_fsm_follow_container;

extern FsmContainer<FsmFavorite>*         g_fsm_favorite_container;

extern FsmContainer<FsmCollect>*			g_fsm_collect_container;

#endif 

