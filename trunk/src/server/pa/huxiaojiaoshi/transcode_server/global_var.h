#ifndef _SERVER_GLOBAL_VAR_H_
#define _SERVER_GLOBAL_VAR_H_

#include "server.h"
#include "fsm_container.h"
#include "memory/fixed_size_allocator.h"
#include <ext/hash_map>
#include "executor_thread_processor.h"
#include "ffmpeg_fsm.h"
#include "client_processor.h"
#include "token_server_processor.h"
#include <sys/resource.h>
#include "app/timer.h"
#include "util/lce_util.h"


extern Server* 						  g_server;

extern lce::memory::FixedSizeAllocator*   g_lbb_allcator;

extern ClientProcessor*                   g_client_processor;

extern TokenServerProcessor*              g_token_server_processor;

extern ExecutorThreadProcessor*           g_executor_thread_processor;

extern lce::app::TimerContainer*          g_timer_container;

extern FsmContainer<FfmpegFsm>*         g_ffmpeg_fsm_container;


#define WXMEDIAID_TO_LOCALID(wd, ld) \
	{ \
		md5 stMD5; \
		uint64_t _md5 = stMD5.hash64(wd.c_str(), wd.size()); \
		std::ostringstream szs; \
		szs.str(""); \
		szs<<hex<<_md5; \
		ld = szs.str(); \
	}

#define int_2_str(i) lce::util::StringOP::TypeToStr(i) 


#endif 

