#ifndef _FEEDS_SERVER_FSM_FAVORITE_H_
#define _FEEDS_SERVER_FSM_FAVORITE_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class FsmFavoriteState;
class FsmFavorite : public lce::app::TimerHandler
{
public:
    FsmFavorite();
    virtual ~FsmFavorite();

public:
    void set_state(FsmFavoriteState& state);

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
    FsmFavoriteState* _state;
    long _timer_id;

public:
	FavoriteInfo _favorite_info;
//	vector<uint64_t> _openid_md5_list;
	/*
	uint64_t _feed_id;
	uint64_t _pa_appid_md5;
	uint64_t _openid_md5;
	uint64_t _begin_openid_md5;
	uint64_t _limit;
	uint64_t _total;
	uint64_t _count;
	*/
	
private:
    DECL_LOGGER(logger);
};

#endif

