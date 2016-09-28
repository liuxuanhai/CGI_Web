#ifndef _MSG_SERVER_FSM_STATE_H_
#define _MSG_SERVER_FSM_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class MsgFsm;
class MsgFsmStateInit;
class MsgFsmStateWaitQueryDB;
class MsgFsmStateWaitAddDB;
class MsgFsmStateWaitDelDB;
class MsgFsmStateEnd;

class MsgFsmState
{
public:
	MsgFsmState(const std::string& name): _name(name)
	{

	}

	virtual ~MsgFsmState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(MsgFsm& fsm);
	virtual void exit(MsgFsm& fsm);

public:
	virtual void client_req_event(MsgFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void db_reply_event(MsgFsm& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(MsgFsm& fsm, void* param);


public:
	static MsgFsmStateInit             	state_init;
	static MsgFsmStateWaitQueryDB      	state_wait_query_db;
	static MsgFsmStateWaitAddDB			state_wait_add_db;
	static MsgFsmStateWaitDelDB			state_wait_del_db;
	static MsgFsmStateEnd				state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class MsgFsmStateInit : public MsgFsmState
{
public:
	MsgFsmStateInit(const std::string name):MsgFsmState(name)
	{

	}

	virtual void client_req_event(MsgFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class MsgFsmStateWaitQueryDB : public MsgFsmState
{
public:
	MsgFsmStateWaitQueryDB(const std::string name):MsgFsmState(name)
	{

	}

	virtual void enter(MsgFsm& fsm);
	virtual void exit(MsgFsm& fsm);
   	virtual void db_reply_event(MsgFsm& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(MsgFsm& fsm, void* param);
};

class MsgFsmStateWaitAddDB : public MsgFsmState
{
public:
	MsgFsmStateWaitAddDB(const std::string name):MsgFsmState(name)
	{

	}

	virtual void enter(MsgFsm& fsm);
	virtual void exit(MsgFsm& fsm);
	virtual void db_reply_event(MsgFsm& fsm, ExecutorThreadResponseElement& element);
	virtual void timeout_event(MsgFsm& fsm, void* param);
};

class MsgFsmStateWaitDelDB : public MsgFsmState
{
public:
	MsgFsmStateWaitDelDB(const std::string name):MsgFsmState(name)
	{

	}

	virtual void enter(MsgFsm& fsm);
	virtual void exit(MsgFsm& fsm);
	virtual void db_reply_event(MsgFsm& fsm, ExecutorThreadResponseElement& element);
	virtual void timeout_event(MsgFsm& fsm, void* param);
};



class MsgFsmStateEnd : public MsgFsmState
{
public:
	MsgFsmStateEnd(const std::string name):MsgFsmState(name)
	{

	}

	virtual void enter(MsgFsm& fsm);
};

#endif

