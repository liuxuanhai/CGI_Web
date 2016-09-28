#ifndef _SERVER_FSM_STATE_H_
#define _SERVER_FSM_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class UserMarkFsm;
class UserMarkFsmStateInit;
class UserMarkStateWaitQueryDB;
class UserMarkStateEnd;

class UserMarkState
{
public:
	UserMarkState(const std::string& name): _name(name)
	{

	}

	virtual ~UserMarkState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(UserMarkFsm& fsm);
	virtual void exit(UserMarkFsm& fsm);

public:
	virtual void client_req_event(UserMarkFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void db_reply_event(UserMarkFsm& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(UserMarkFsm& fsm, void* param);


public:
	static UserMarkFsmStateInit             state_init;
	static UserMarkStateWaitQueryDB      state_wait_query_db;
	static UserMarkStateEnd				state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class UserMarkFsmStateInit : public UserMarkState
{
public:
	UserMarkFsmStateInit(const std::string name):UserMarkState(name)
	{

	}

	virtual void client_req_event(UserMarkFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class UserMarkStateWaitQueryDB : public UserMarkState
{
public:
	UserMarkStateWaitQueryDB(const std::string name):UserMarkState(name)
	{

	}

	virtual void enter(UserMarkFsm& fsm);
	virtual void exit(UserMarkFsm& fsm);
   	virtual void db_reply_event(UserMarkFsm& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(UserMarkFsm& fsm, void* param);
};

class UserMarkStateEnd : public UserMarkState
{
public:
	UserMarkStateEnd(const std::string name):UserMarkState(name)
	{

	}

	virtual void enter(UserMarkFsm& fsm);
};

#endif

