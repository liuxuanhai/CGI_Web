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

extern UserServer* 						  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern TokenServerProcessor*              g_token_server_processor;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

extern lce::app::TimerContainer*          g_timer_container;

extern FsmContainer<FsmBusiness>*         g_fsm_business_container;


#define sql_escape(str) (lce::cgi::CMysql::MysqlEscape(str))

#endif 

