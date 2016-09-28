#ifndef _TEST_SERVER_GLOBAL_VAR_H_
#define _TEST_SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "fsm_container.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "user_info_fsm.h"
#include "user_info_cache_pool.h"
#include "client_processor.h"
#include <sys/resource.h>
#include "app/timer.h"

extern TestServer* 						  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

extern UserInfoCachePool*                 g_user_info_cache_pool;

extern lce::app::TimerContainer*          g_timer_container;

extern FsmContainer<UserInfoFsm>*         g_user_info_fsm_container;


#endif 

