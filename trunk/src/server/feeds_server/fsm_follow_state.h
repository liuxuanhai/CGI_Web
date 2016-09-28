#ifndef _FEEDS_SERVER_FSM_FOLLOW_STATE_H_
#define _FEEDS_SERVER_FSM_FOLLOW_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class FsmFollow;
class FsmFollowStateInit;
class FsmFollowStateWaitQueryDB;
class FsmFollowStateWaitAddDB;
class FsmFollowStateWaitUpdateDB;
class FsmFollowStateWaitDeleteDB;
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
	virtual void db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFollow& fsm, void* param);


public:
	static FsmFollowStateInit             state_init;
	static FsmFollowStateWaitQueryDB      state_wait_query_db;
	static FsmFollowStateWaitAddDB		  state_wait_add_db;
	static FsmFollowStateWaitUpdateDB     state_wait_update_db;
	static FsmFollowStateWaitDeleteDB	  state_wait_delete_db;
	static FsmFollowStateEnd			  state_end;
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

class FsmFollowStateWaitQueryDB : public FsmFollowState
{
public:
	FsmFollowStateWaitQueryDB(const std::string name):FsmFollowState(name)
	{

	}

	virtual void enter(FsmFollow& fsm);
	virtual void exit(FsmFollow& fsm);
   	virtual void db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFollow& fsm, void* param);
};

class FsmFollowStateWaitAddDB : public FsmFollowState
{
public:
	FsmFollowStateWaitAddDB(const std::string name):FsmFollowState(name)
	{

	}

	virtual void enter(FsmFollow& fsm);
	virtual void exit(FsmFollow& fsm);
   	virtual void db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFollow& fsm, void* param);
};

class FsmFollowStateWaitUpdateDB : public FsmFollowState
{
public:
	FsmFollowStateWaitUpdateDB(const std::string name):FsmFollowState(name)
	{

	}

	virtual void enter(FsmFollow& fsm);
	virtual void exit(FsmFollow& fsm);
   	virtual void db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFollow& fsm, void* param);
};

class FsmFollowStateWaitDeleteDB : public FsmFollowState
{
public:
	FsmFollowStateWaitDeleteDB(const std::string name):FsmFollowState(name)
	{

	}

	virtual void enter(FsmFollow& fsm);
	virtual void exit(FsmFollow& fsm);
   	virtual void db_reply_event(FsmFollow& fsm, const ExecutorThreadResponseElement& element);
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


