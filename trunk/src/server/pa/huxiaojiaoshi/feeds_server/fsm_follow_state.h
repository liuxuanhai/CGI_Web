#ifndef _FEEDS_SERVER_FSM_FEEDS_STATE_H_
#define _FEEDS_SERVER_FSM_FEEDS_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class FsmFollow;
class FsmFollowStateInit;
class FsmFollowStateWaitQueryDb;
class FsmFollowStateWaitCheckOutTradeNoValid;
class FsmFollowStateWaitCheckOutTradeNoUnique;
class FsmFollowStateWaitTranscode;
class FsmFollowStateWaitInsertIntoDb;
class FsmFollowStateEnd;

class FsmFollowState
{
public:
    FsmFollowState(const std::string& name): _name(name)
    {

    }

    virtual ~FsmFollowState()
    {

    }

    virtual std::string name() const
    {
        return _name;
    }

public:
    virtual void enter(FsmFollow& fsm);
    virtual void exit(FsmFollow& fsm);

public:
    virtual void client_req_event(FsmFollow& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
    virtual void query_db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
    virtual void check_out_trade_no_valid_reply_event(FsmFollow& fsm, const hoosho::msg::Msg& stMsg);
    virtual void check_out_trade_no_unique_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
    virtual void transcode_reply_event(FsmFollow& fsm, const hoosho::msg::Msg& stMsg);
    virtual void insert_into_db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFollow& fsm, void* param);


public:
    static FsmFollowStateInit             state_init;
    static FsmFollowStateWaitQueryDb             state_wait_query_db;
    static FsmFollowStateWaitCheckOutTradeNoValid		state_wait_check_out_trade_no_valid;
    static FsmFollowStateWaitCheckOutTradeNoUnique      state_wait_check_out_trade_no_unique;
    static FsmFollowStateWaitTranscode     state_wait_transcode;
    static FsmFollowStateWaitInsertIntoDb		state_wait_insert_into_db;
    static FsmFollowStateEnd				state_end;
public:
    DECL_LOGGER(logger);

private:
    std::string _name;
};

class FsmFollowStateInit : public FsmFollowState
{
public:
    FsmFollowStateInit(const std::string name):FsmFollowState(name)
    {

    }

    virtual void client_req_event(FsmFollow& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class FsmFollowStateWaitQueryDb : public FsmFollowState
{
public:
    FsmFollowStateWaitQueryDb(const std::string name) :FsmFollowState(name)
    {

    }
    virtual void enter(FsmFollow& fsm);
    virtual void exit(FsmFollow& fsm);
    virtual void query_db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFollow& fsm, void* param);
};

class FsmFollowStateWaitCheckOutTradeNoValid : public FsmFollowState
{
public:
    FsmFollowStateWaitCheckOutTradeNoValid(const std::string name):FsmFollowState(name)
    {

    }

    virtual void enter(FsmFollow& fsm);
    virtual void exit(FsmFollow& fsm);
    virtual void check_out_trade_no_valid_reply_event(FsmFollow& fsm, const hoosho::msg::Msg& stMsg);
    virtual void timeout_event(FsmFollow& fsm, void* param);
};

class FsmFollowStateWaitCheckOutTradeNoUnique : public FsmFollowState
{
public:
    FsmFollowStateWaitCheckOutTradeNoUnique(const std::string name):FsmFollowState(name)
    {

    }

    virtual void enter(FsmFollow& fsm);
    virtual void exit(FsmFollow& fsm);
    virtual void check_out_trade_no_unique_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFollow& fsm, void* param);
};

class FsmFollowStateWaitTranscode : public FsmFollowState
{
public:
    FsmFollowStateWaitTranscode(const std::string name):FsmFollowState(name)
    {

    }

    virtual void enter(FsmFollow& fsm);
    virtual void exit(FsmFollow& fsm);
    virtual void transcode_reply_event(FsmFollow& fsm, const hoosho::msg::Msg& stMsg);
    virtual void timeout_event(FsmFollow& fsm, void* param);
};

class FsmFollowStateWaitInsertIntoDb : public FsmFollowState
{
public:
    FsmFollowStateWaitInsertIntoDb(const std::string name):FsmFollowState(name)
    {

    }

    virtual void enter(FsmFollow& fsm);
    virtual void exit(FsmFollow& fsm);
    virtual void insert_into_db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFollow& fsm, void* param);
};


class FsmFollowStateEnd : public FsmFollowState
{
public:
    FsmFollowStateEnd(const std::string name):FsmFollowState(name)
    {

    }

    virtual void enter(FsmFollow& fsm);
};

#endif

