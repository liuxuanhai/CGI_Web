#ifndef _HOOSHO_USER_SERVER_PREAUTH_FSM_H_
#define _HOOSHO_USER_SERVER_PREAUTH_FSM_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"
#include "comm_struct.pb.h"

class FsmPreAuthState;
class FsmPreAuth : public lce::app::TimerHandler
{
public:
    FsmPreAuth();
    virtual ~FsmPreAuth();

public:
    void set_state(FsmPreAuthState& state);

    void req_preauth_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	void preauth_reply_event(const ::hoosho::msg::Msg& stMsg);
	void user_info_get_reply_event(ExecutorThreadResponseElement& element);
	void apply_hoosho_no_reply_event(const ::hoosho::msg::Msg& stMsg);
	void user_info_update_reply_event(const ExecutorThreadResponseElement& element);
	
    void timeout_event(void* param);

public:
    void cancel_timer();
    void reset_timer(int wait_time);
    void handle_timeout(void* param);

public:
	int reply_fail(int iErrCode);
	int reply_ok();

public:
    uint32_t _id;
    uint32_t _conn_id;
    FsmPreAuthState* _state;
    long _timer_id;
    
    uint32_t _req_seq_id;
	std::string _req_preauth_code;

	//inner data
	::hoosho::commstruct::UserDetailInfo _user_detail_info;
    
private:
    DECL_LOGGER(logger);
};

#endif

