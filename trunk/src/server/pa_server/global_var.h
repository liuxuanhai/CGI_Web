#ifndef _HOOSHO_PA_SERVER_GLOBAL_VAR_H_
#define _HOOSHO_PA_SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "fsm_container.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "fsm_preauth.h"
#include "fsm_resource.h"
#include "fsm_business.h"
#include "client_processor.h"
#include <sys/resource.h>
#include "app/timer.h"

//#define HX_PA_APPID "wxfd93b50f7122f71b"
//#define HX_PA_APPSECRET "26a8bb809b08e62b45be236e9ea0f36d"

extern const char* HX_PA_APPID;
extern const char* HX_PA_APPSECRET;

extern PAServer* 						  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

extern lce::app::TimerContainer*          g_timer_container;

extern FsmContainer<FsmPreAuth>*          g_fsm_preauth_container;

extern FsmContainer<FsmResource>*         g_fsm_resource_container;

extern FsmContainer<FsmBusiness>*         g_fsm_business_container;

#define UINT64_TO_TABLENAME(prefix, ui, tn) \
	{ \
		char aczbuf[64] = {0}; \
		::snprintf(aczbuf, 64, "%s%02x", prefix.c_str(), (uint8_t)(ui%256)); \
		tn = aczbuf; \
	}




#endif 

