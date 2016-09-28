#ifndef _HOOSHO_J_USER_SERVER_BUSINESS_FSM_H_
#define _HOOSHO_J_USER_SERVER_BUSINESS_FSM_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"
#include "comm_struct.pb.h"

class FsmBusinessState;
class FsmBusiness : public lce::app::TimerHandler
{
public:
    FsmBusiness();
    virtual ~FsmBusiness();

public:
    void set_state(FsmBusinessState& state);

    void client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	void thread_reply_event(ExecutorThreadResponseElement& element);
	
    void timeout_event(void* param);

public:
    void cancel_timer();
    void reset_timer(int wait_time);
    void handle_timeout(void* param);

public:
	int reply_fail(int iErrCode);

public:
    uint32_t _id;
    uint32_t _conn_id;
    FsmBusinessState* _state;
    long _timer_id;
    
   	::hoosho::msg::Msg _msg;
    
private:
    DECL_LOGGER(logger);
};

#endif

