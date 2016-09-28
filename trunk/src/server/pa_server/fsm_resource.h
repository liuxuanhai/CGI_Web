#ifndef _HOOSHO_PA_SERVER_RESOURCE_FSM_H_
#define _HOOSHO_PA_SERVER_RESOURCE_FSM_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"
#include "comm_struct.pb.h"

class FsmResourceState;
class FsmResource : public lce::app::TimerHandler
{
public:
    FsmResource();
    virtual ~FsmResource();

public:
    void set_state(FsmResourceState& state);

    void download_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	void download_reply_event(const ExecutorThreadResponseElement& element);
    void timeout_event(void* param);

public:
    void cancel_timer();
    void reset_timer(int wait_time);
    void handle_timeout(void* param);

public:
	int reply_timeout();

public:
    uint32_t _id;
    uint32_t _conn_id;
    FsmResourceState* _state;
    long _timer_id;

    ::hoosho::msg::Msg _req_msg;
    
private:
    DECL_LOGGER(logger);
};

#endif

