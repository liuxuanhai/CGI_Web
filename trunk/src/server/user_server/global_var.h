#ifndef _HOOSHO_PA_SERVER_GLOBAL_VAR_H_
#define _HOOSHO_PA_SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "fsm_container.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "fsm_preauth.h"
#include "fsm_business.h"
#include "client_processor.h"
#include "server_processor_pa.h"
#include "server_processor_notice.h"
#include "server_processor_config.h"
#include <sys/resource.h>
#include "app/timer.h"
#include "util/lce_util.h"
#include "cgi/cgi_mysql.h"
#include <stdio.h>

extern UserServer* 						  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern ServerProcessorPA*                 g_server_processor_pa;

extern ServerProcessorMsg*             g_server_processor_msg;

extern ServerProcessorConfig*             g_server_processor_config;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

extern lce::app::TimerContainer*          g_timer_container;

extern FsmContainer<FsmPreAuth>*          g_fsm_preauth_container;

extern FsmContainer<FsmBusiness>*         g_fsm_business_container;



#define OPENID_TO_MD5(str, ui) \
	{ \
		md5 stMD5; \
		ui = stMD5.hash64(str.c_str(), str.size()); \
	}

#define UINT64_TO_TABLENAME(prefix, ui, tn) \
	{ \
		char aczbuf[64] = {0}; \
		::snprintf(aczbuf, 64, "%s%02x", prefix.c_str(), (uint8_t)(ui%256)); \
		tn = aczbuf; \
	}

#define sql_escape(str) (lce::cgi::CMysql::MysqlEscape(str))

#endif 

