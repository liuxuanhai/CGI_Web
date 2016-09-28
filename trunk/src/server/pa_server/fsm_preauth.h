#ifndef _HOOSHO_PA_SERVER_PREAUTH_FSM_H_
#define _HOOSHO_PA_SERVER_PREAUTH_FSM_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"
#include "comm_struct.pb.h"
#include "wx_user_info.h"

class FsmPreAuthState;
class FsmPreAuth : public lce::app::TimerHandler
{
public:
    FsmPreAuth();
    virtual ~FsmPreAuth();

public:
    void set_state(FsmPreAuthState& state);

    void req_preauth_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	void preauth_reply_event(const ExecutorThreadResponseElement& element);
    void timeout_event(void* param);

public:
    void cancel_timer();
    void reset_timer(int wait_time);
    void handle_timeout(void* param);

public:
	int reply_timeout();
	
public:
	static void wxuserinfo_to_pbuserinfo(const ::common::wxapi::WXUserInfo& stWxUserInfo
										, ::hoosho::commstruct::UserDetailInfo* pUserDetailInfo);

public:
    uint32_t _id;
    uint32_t _conn_id;
    FsmPreAuthState* _state;
    long _timer_id;
    
    uint32_t _req_seq_id;
	std::string _req_preauth_code;
    
private:
    DECL_LOGGER(logger);
};

#endif

