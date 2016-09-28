#ifndef _SERVER_GLOBAL_VAR_H_
#define _SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "client_processor.h"
#include <sys/resource.h>
//#include "app/timer.h"
#include "coroutine_module_base.h"
#include "cgi/cgi_mysql.h"
#include "timer_module.h"
#include "singleton.h"
#include "server_processor_pa.h"
#include "server_processor_msg.h"


extern Server* 						  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

extern ServerProcessorPA*                 g_server_processor_pa;

extern ServerProcessorMsg*                g_server_processor_msg;

#define sql_escape(str) (lce::cgi::CMysql::MysqlEscape(str))

enum RED_POINT_OPT_TYPE_ENUM
{
	RED_POINT_OPT_TYPE_REDUCE = 0,
	RED_POINT_OPT_TYPE_INCREASE = 1,
	RED_POINT_OPT_TYPE_CLEAR = 2,
};


#endif 

