#ifndef _HOOSHO_TOKEN_SERVER_GLOBAL_VAR_H_
#define _HOOSHO_TOKEN_SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "client_processor.h"
#include "pa_info_manager.h"
#include <sys/resource.h>
#include "app/timer.h"


extern TokenServer* 					  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern PAInfoManager*					  g_pa_info_manager;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

extern lce::app::TimerContainer*          g_timer_container;



#endif 

