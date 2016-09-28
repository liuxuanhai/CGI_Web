#ifndef _FEEDS_SERVER_FSM_COLLECT_STATE_H_
#define _FEEDS_SERVER_FSM_COLLECT_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class FsmCollect;
class FsmCollectStateInit;
class FsmCollectStateWaitQueryDB;
class FsmCollectStateWaitAddDB;
class FsmCollectStateWaitUpdateDB;
class FsmCollectStateWaitDeleteDB;
class FsmCollectStateEnd;

class FsmCollectState
{
public:
	FsmCollectState(const std::string& name): _name(name)
	{

	}

	virtual ~FsmCollectState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(FsmCollect& fsm);
	virtual void exit(FsmCollect& fsm);

public:
	virtual void client_req_event(FsmCollect& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void db_reply_event(FsmCollect& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmCollect& fsm, void* param);


public:
	static FsmCollectStateInit             	state_init;
	static FsmCollectStateWaitQueryDB      	state_wait_query_db;
	static FsmCollectStateWaitAddDB			state_wait_add_db;
	static FsmCollectStateWaitUpdateDB     	state_wait_update_db;
	static FsmCollectStateWaitDeleteDB		state_wait_delete_db;
	static FsmCollectStateEnd				state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class FsmCollectStateInit : public FsmCollectState
{
public:
	FsmCollectStateInit(const std::string name):FsmCollectState(name)
	{

	}

	virtual void client_req_event(FsmCollect& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class FsmCollectStateWaitQueryDB : public FsmCollectState
{
public:
	FsmCollectStateWaitQueryDB(const std::string name):FsmCollectState(name)
	{

	}

	virtual void enter(FsmCollect& fsm);
	virtual void exit(FsmCollect& fsm);
   	virtual void db_reply_event(FsmCollect& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmCollect& fsm, void* param);
};

class FsmCollectStateWaitAddDB : public FsmCollectState
{
public:
	FsmCollectStateWaitAddDB(const std::string name):FsmCollectState(name)
	{

	}

	virtual void enter(FsmCollect& fsm);
	virtual void exit(FsmCollect& fsm);
   	virtual void db_reply_event(FsmCollect& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmCollect& fsm, void* param);
};

class FsmCollectStateWaitUpdateDB : public FsmCollectState
{
public:
	FsmCollectStateWaitUpdateDB(const std::string name):FsmCollectState(name)
	{

	}

	virtual void enter(FsmCollect& fsm);
	virtual void exit(FsmCollect& fsm);
   	virtual void db_reply_event(FsmCollect& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmCollect& fsm, void* param);
};

class FsmCollectStateWaitDeleteDB : public FsmCollectState
{
public:
	FsmCollectStateWaitDeleteDB(const std::string name):FsmCollectState(name)
	{

	}

	virtual void enter(FsmCollect& fsm);
	virtual void exit(FsmCollect& fsm);
   	virtual void db_reply_event(FsmCollect& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmCollect& fsm, void* param);
};


class FsmCollectStateEnd : public FsmCollectState
{
public:
	FsmCollectStateEnd(const std::string name):FsmCollectState(name)
	{

	}

	virtual void enter(FsmCollect& fsm);
};

#endif

