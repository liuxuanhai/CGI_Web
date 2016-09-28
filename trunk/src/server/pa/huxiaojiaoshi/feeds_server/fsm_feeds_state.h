#ifndef _FEEDS_SERVER_FSM_FEEDS_STATE_H_
#define _FEEDS_SERVER_FSM_FEEDS_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class FsmFeeds;
class FsmFeedsStateInit;
class FsmFeedsStateWaitQueryDB;
class FsmFeedsStateWaitAddDB;
class FsmFeedsStateWaitUpdateDB;
class FsmFeedsStateWaitDeleteDB;
class FsmFeedsStateEnd;

class FsmFeedsState
{
public:
    FsmFeedsState(const std::string& name): _name(name)
    {

    }

    virtual ~FsmFeedsState()
    {

    }

    virtual std::string name() const
    {
        return _name;
    }

public:
    virtual void enter(FsmFeeds& fsm);
    virtual void exit(FsmFeeds& fsm);

public:
    virtual void client_req_event(FsmFeeds& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
    virtual void db_reply_event(FsmFeeds& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFeeds& fsm, void* param);


public:
    static FsmFeedsStateInit             state_init;
    static FsmFeedsStateWaitQueryDB      state_wait_query_db;
    static FsmFeedsStateWaitAddDB		state_wait_add_db;
    static FsmFeedsStateWaitUpdateDB     state_wait_update_db;
    static FsmFeedsStateWaitDeleteDB		state_wait_delete_db;
    static FsmFeedsStateEnd				state_end;
public:
    DECL_LOGGER(logger);

private:
    std::string _name;
};

class FsmFeedsStateInit : public FsmFeedsState
{
public:
    FsmFeedsStateInit(const std::string name):FsmFeedsState(name)
    {

    }

    virtual void client_req_event(FsmFeeds& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class FsmFeedsStateWaitQueryDB : public FsmFeedsState
{
public:
    FsmFeedsStateWaitQueryDB(const std::string name):FsmFeedsState(name)
    {

    }

    virtual void enter(FsmFeeds& fsm);
    virtual void exit(FsmFeeds& fsm);
    virtual void db_reply_event(FsmFeeds& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFeeds& fsm, void* param);
};

class FsmFeedsStateWaitAddDB : public FsmFeedsState
{
public:
    FsmFeedsStateWaitAddDB(const std::string name):FsmFeedsState(name)
    {

    }

    virtual void enter(FsmFeeds& fsm);
    virtual void exit(FsmFeeds& fsm);
    virtual void db_reply_event(FsmFeeds& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFeeds& fsm, void* param);
};

class FsmFeedsStateWaitUpdateDB : public FsmFeedsState
{
public:
    FsmFeedsStateWaitUpdateDB(const std::string name):FsmFeedsState(name)
    {

    }

    virtual void enter(FsmFeeds& fsm);
    virtual void exit(FsmFeeds& fsm);
    virtual void db_reply_event(FsmFeeds& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFeeds& fsm, void* param);
};

class FsmFeedsStateWaitDeleteDB : public FsmFeedsState
{
public:
    FsmFeedsStateWaitDeleteDB(const std::string name):FsmFeedsState(name)
    {

    }

    virtual void enter(FsmFeeds& fsm);
    virtual void exit(FsmFeeds& fsm);
    virtual void db_reply_event(FsmFeeds& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFeeds& fsm, void* param);
};


class FsmFeedsStateEnd : public FsmFeedsState
{
public:
    FsmFeedsStateEnd(const std::string name):FsmFeedsState(name)
    {

    }

    virtual void enter(FsmFeeds& fsm);
};

#endif

