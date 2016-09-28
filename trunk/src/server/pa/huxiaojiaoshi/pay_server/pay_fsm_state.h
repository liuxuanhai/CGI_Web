#ifndef _SERVER_FSM_STATE_H_
#define _SERVER_FSM_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class PayFsm;
class PayFsmStateInit;
class PayStateWaitQueryDB;
class PayStateWaitThreadPayOrder;
class PayStateEnd;

class PayState
{
public:
	PayState(const std::string& name): _name(name)
	{

	}

	virtual ~PayState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(PayFsm& fsm);
	virtual void exit(PayFsm& fsm);

public:
	virtual void client_req_event(PayFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void db_reply_event(PayFsm& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(PayFsm& fsm, void* param);


public:
	static PayFsmStateInit             		state_init;
	static PayStateWaitQueryDB      		state_wait_query_db;
	static PayStateWaitThreadPayOrder   state_wait_thread_payorder;
	static PayStateEnd						state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class PayFsmStateInit : public PayState
{
public:
	PayFsmStateInit(const std::string name):PayState(name)
	{

	}

	virtual void client_req_event(PayFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class PayStateWaitQueryDB : public PayState
{
public:
	PayStateWaitQueryDB(const std::string name):PayState(name)
	{

	}

	virtual void enter(PayFsm& fsm);
	virtual void exit(PayFsm& fsm);
   	virtual void db_reply_event(PayFsm& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(PayFsm& fsm, void* param);
};

class PayStateWaitThreadPayOrder : public PayState
{
public:
	PayStateWaitThreadPayOrder(const std::string name):PayState(name)
	{

	}

	virtual void enter(PayFsm& fsm);
	virtual void exit(PayFsm& fsm);
   	virtual void db_reply_event(PayFsm& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(PayFsm& fsm, void* param);
};


class PayStateEnd : public PayState
{
public:
	PayStateEnd(const std::string name):PayState(name)
	{

	}

	virtual void enter(PayFsm& fsm);
};

#endif

