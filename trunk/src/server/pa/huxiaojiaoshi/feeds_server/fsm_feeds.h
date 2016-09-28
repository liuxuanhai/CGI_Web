#ifndef _FEEDS_SERVER_FSM_FEEDS_H_
#define _FEEDS_SERVER_FSM_FEEDS_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class FsmFeedsState;
class FsmFeeds : public lce::app::TimerHandler
{
public:
    FsmFeeds();
    virtual ~FsmFeeds();

public:
    void set_state(FsmFeedsState& state);

    void client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
    void db_reply_event(const ExecutorThreadResponseElement& element);
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
    uint32_t _req_type;
    uint32_t _req_seq_id;
    FsmFeedsState* _state;
    long _timer_id;

public:


private:
    DECL_LOGGER(logger);
};

#endif

