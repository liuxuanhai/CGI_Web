#ifndef _NOTICE_SERVER_SYS_NOTICE_FSM_H_
#define _NOTICE_SERVER_SYS_NOTICE_FSM_H_

#include <string>
#include "net/lce_net.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class SysNoticeFsmState;
class SysNoticeFsm : public lce::app::TimerHandler
{
public:
    SysNoticeFsm();
    virtual ~SysNoticeFsm();

public:
    void set_state(SysNoticeFsmState& state);

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
    SysNoticeFsmState* _state;
    long _timer_id;
	DaoSysNotice _sys_notice;

private:
    DECL_LOGGER(logger);
};

#endif

