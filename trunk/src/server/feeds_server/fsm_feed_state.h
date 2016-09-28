#ifndef _FEEDS_SERVER_FSM_FEED_STATE_H_
#define _FEEDS_SERVER_FSM_FEED_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class FsmFeed;
class FsmFeedStateInit;
class FsmFeedStateWaitQueryDB;
class FsmFeedStateWaitAddDB;
class FsmFeedStateWaitUpdateDB;
class FsmFeedStateWaitDeleteDB;
class FsmFeedStateEnd;

class FsmFeedState
{
public:
	FsmFeedState(const std::string& name): _name(name)
	{

	}

	virtual ~FsmFeedState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(FsmFeed& fsm);
	virtual void exit(FsmFeed& fsm);

public:
	virtual void client_req_event(FsmFeed& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void db_reply_event(FsmFeed& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFeed& fsm, void* param);


public:
	static FsmFeedStateInit             state_init;
	static FsmFeedStateWaitQueryDB      state_wait_query_db;
	static FsmFeedStateWaitAddDB		state_wait_add_db;
	static FsmFeedStateWaitUpdateDB     state_wait_update_db;
	static FsmFeedStateWaitDeleteDB		state_wait_delete_db;
	static FsmFeedStateEnd				state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class FsmFeedStateInit : public FsmFeedState
{
public:
	FsmFeedStateInit(const std::string name):FsmFeedState(name)
	{

	}

	virtual void client_req_event(FsmFeed& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class FsmFeedStateWaitQueryDB : public FsmFeedState
{
public:
	FsmFeedStateWaitQueryDB(const std::string name):FsmFeedState(name)
	{

	}

	virtual void enter(FsmFeed& fsm);
	virtual void exit(FsmFeed& fsm);
   	virtual void db_reply_event(FsmFeed& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFeed& fsm, void* param);
};

class FsmFeedStateWaitAddDB : public FsmFeedState
{
public:
	FsmFeedStateWaitAddDB(const std::string name):FsmFeedState(name)
	{

	}

	virtual void enter(FsmFeed& fsm);
	virtual void exit(FsmFeed& fsm);
   	virtual void db_reply_event(FsmFeed& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFeed& fsm, void* param);
};

class FsmFeedStateWaitUpdateDB : public FsmFeedState
{
public:
	FsmFeedStateWaitUpdateDB(const std::string name):FsmFeedState(name)
	{

	}

	virtual void enter(FsmFeed& fsm);
	virtual void exit(FsmFeed& fsm);
   	virtual void db_reply_event(FsmFeed& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFeed& fsm, void* param);
};

class FsmFeedStateWaitDeleteDB : public FsmFeedState
{
public:
	FsmFeedStateWaitDeleteDB(const std::string name):FsmFeedState(name)
	{

	}

	virtual void enter(FsmFeed& fsm);
	virtual void exit(FsmFeed& fsm);
   	virtual void db_reply_event(FsmFeed& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFeed& fsm, void* param);
};


class FsmFeedStateEnd : public FsmFeedState
{
public:
	FsmFeedStateEnd(const std::string name):FsmFeedState(name)
	{

	}

	virtual void enter(FsmFeed& fsm);
};

#endif

