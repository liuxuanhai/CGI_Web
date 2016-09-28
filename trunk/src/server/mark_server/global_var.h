#ifndef _SERVER_GLOBAL_VAR_H_
#define _SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "fsm_container.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "usermark_fsm.h"
#include "client_processor.h"
#include <sys/resource.h>
#include "app/timer.h"

extern Server* 						  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

extern lce::app::TimerContainer*          g_timer_container;

extern FsmContainer<UserMarkFsm>*         g_user_mark_fsm_container;


#endif 

