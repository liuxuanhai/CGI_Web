#ifndef _HOOSHO_MSG_SERVER_MSG_FSM_H_
#define _HOOSHO_MSG_SERVER_MSG_FSM_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class MsgFsmState;
class MsgFsm : public lce::app::TimerHandler
{
public:
    MsgFsm();
    virtual ~MsgFsm();

public:
    void set_state(MsgFsmState& state);

    void client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	void db_reply_event(ExecutorThreadResponseElement& element);
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
    MsgFsmState* _state;
    long _timer_id;

   	::hoosho::msg::Msg _msg;
	
private:
    DECL_LOGGER(logger);
};

#endif

