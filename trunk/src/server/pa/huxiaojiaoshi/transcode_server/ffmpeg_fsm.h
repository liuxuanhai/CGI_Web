#ifndef _SERVER_FSM_H_
#define _SERVER_FSM_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class FfmpegState;
class FfmpegFsm : public lce::app::TimerHandler
{
public:
    FfmpegFsm();
    virtual ~FfmpegFsm();

public:
    void set_state(FfmpegState& state);

    void client_req_event(lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	void db_reply_event(ExecutorThreadResponseElement& element);
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
    FfmpegState* _state;
    long _timer_id;

	::hoosho::msg::Msg _msg;
	
private:
    DECL_LOGGER(logger);
};

#endif

