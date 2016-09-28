#ifndef _FEEDS_SERVER_FSM_FOLLOW_H_
#define _FEEDS_SERVER_FSM_FOLLOW_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class FsmFollowState;
class FsmFollow : public lce::app::TimerHandler
{
public:
    FsmFollow();
    virtual ~FsmFollow();

public:
    void set_state(FsmFollowState& state);

    void client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
    void query_db_reply_event(const ExecutorThreadResponseElement& element);
    void check_out_trade_no_valid_reply_event(const hoosho::msg::Msg& stMsg);
    void check_out_trade_no_unique_reply_event(const ExecutorThreadResponseElement& element);
    void transcode_reply_event(const hoosho::msg::Msg& stMsg);
    void insert_into_db_reply_event(const ExecutorThreadResponseElement& element);
    void timeout_event(void* param);

public:
    void cancel_timer();
    void reset_timer(int wait_time);
    void handle_timeout(void* param);

public:
    int reply_timeout();
    int reply_fail(uint32_t dwErrcode);
    int reply_ok();

public:
    uint32_t _id;
    uint32_t _conn_id;
    uint32_t _req_type;
    uint32_t _req_seq_id;
    FsmFollowState* _state;
    long _timer_id;

public:
    ::hoosho::j::commstruct::ListenInfo _listen_info;
    ::hoosho::j::commstruct::FollowInfo _follow_info;

private:
    DECL_LOGGER(logger);
};

#endif

