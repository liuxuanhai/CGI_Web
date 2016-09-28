#ifndef _FEEDS_SERVER_FSM_FEED_H_
#define _FEEDS_SERVER_FSM_FEED_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class FsmFeedState;
class FsmFeed : public lce::app::TimerHandler
{
public:
    FsmFeed();
    virtual ~FsmFeed();

public:
    void set_state(FsmFeedState& state);

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
    FsmFeedState* _state;
    long _timer_id;

public:
	FeedInfo _feed_info;
//	vector<FeedInfo> _feed_list;
	/*
	uint64_t _begin_feed_id;
	uint64_t _limit;
	uint64_t _total;
	uint64_t _count;
	*/

private:
    DECL_LOGGER(logger);
};

#endif

