#ifndef _TEST_SERVER_FSM_STATE_H_
#define _TEST_SERVER_FSM_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class UserInfoFsm;
class UserInfoFsmStateInit;
class UserInfoFsmStateWaitQueryDB;
class UserInfoFsmStateWaitUpdateDB;
class UserInfoFsmStateEnd;

class UserInfoFsmState
{
public:
	UserInfoFsmState(const std::string& name): _name(name)
	{

	}

	virtual ~UserInfoFsmState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(UserInfoFsm& fsm);
	virtual void exit(UserInfoFsm& fsm);

public:
	virtual void client_req_event(UserInfoFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void db_reply_event(UserInfoFsm& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(UserInfoFsm& fsm, void* param);


public:
	static UserInfoFsmStateInit             state_init;
	static UserInfoFsmStateWaitQueryDB      state_wait_query_db;
	static UserInfoFsmStateWaitUpdateDB     state_wait_update_db;
	static UserInfoFsmStateEnd				state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class UserInfoFsmStateInit : public UserInfoFsmState
{
public:
	UserInfoFsmStateInit(const std::string name):UserInfoFsmState(name)
	{

	}

	virtual void client_req_event(UserInfoFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class UserInfoFsmStateWaitQueryDB : public UserInfoFsmState
{
public:
	UserInfoFsmStateWaitQueryDB(const std::string name):UserInfoFsmState(name)
	{

	}

	virtual void enter(UserInfoFsm& fsm);
	virtual void exit(UserInfoFsm& fsm);
   	virtual void db_reply_event(UserInfoFsm& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(UserInfoFsm& fsm, void* param);
};

class UserInfoFsmStateWaitUpdateDB : public UserInfoFsmState
{
public:
	UserInfoFsmStateWaitUpdateDB(const std::string name):UserInfoFsmState(name)
	{

	}

	virtual void enter(UserInfoFsm& fsm);
	virtual void exit(UserInfoFsm& fsm);
   	virtual void db_reply_event(UserInfoFsm& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(UserInfoFsm& fsm, void* param);
};


class UserInfoFsmStateEnd : public UserInfoFsmState
{
public:
	UserInfoFsmStateEnd(const std::string name):UserInfoFsmState(name)
	{

	}

	virtual void enter(UserInfoFsm& fsm);
};

#endif

