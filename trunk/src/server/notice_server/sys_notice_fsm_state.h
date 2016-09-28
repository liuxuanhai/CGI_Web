#ifndef _NOTICE_SERVER_FSM_STATE_H_
#define _NOTICE_SERVER_FSM_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class SysNoticeFsm;
class SysNoticeFsmStateInit;
class SysNoticeFsmStateWaitQueryDB;
class SysNoticeFsmStateWaitUpdateDB;
class SysNoticeFsmStateEnd;

class SysNoticeFsmState
{
public:
	SysNoticeFsmState(const std::string& name): _name(name)
	{

	}

	virtual ~SysNoticeFsmState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(SysNoticeFsm& fsm);
	virtual void exit(SysNoticeFsm& fsm);

public:
	virtual void client_req_event(SysNoticeFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void db_reply_event(SysNoticeFsm& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(SysNoticeFsm& fsm, void* param);


public:
	static SysNoticeFsmStateInit             state_init;
	static SysNoticeFsmStateWaitQueryDB      state_wait_query_db;
	static SysNoticeFsmStateWaitUpdateDB     state_wait_update_db;
	static SysNoticeFsmStateEnd				state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class SysNoticeFsmStateInit : public SysNoticeFsmState
{
public:
	SysNoticeFsmStateInit(const std::string name):SysNoticeFsmState(name)
	{

	}

	virtual void client_req_event(SysNoticeFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class SysNoticeFsmStateWaitQueryDB : public SysNoticeFsmState
{
public:
	SysNoticeFsmStateWaitQueryDB(const std::string name):SysNoticeFsmState(name)
	{

	}

	virtual void enter(SysNoticeFsm& fsm);
	virtual void exit(SysNoticeFsm& fsm);
   	virtual void db_reply_event(SysNoticeFsm& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(SysNoticeFsm& fsm, void* param);
};

class SysNoticeFsmStateWaitUpdateDB : public SysNoticeFsmState
{
public:
	SysNoticeFsmStateWaitUpdateDB(const std::string name):SysNoticeFsmState(name)
	{

	}

	virtual void enter(SysNoticeFsm& fsm);
	virtual void exit(SysNoticeFsm& fsm);
   	virtual void db_reply_event(SysNoticeFsm& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(SysNoticeFsm& fsm, void* param);
};


class SysNoticeFsmStateEnd : public SysNoticeFsmState
{
public:
	SysNoticeFsmStateEnd(const std::string name):SysNoticeFsmState(name)
	{

	}

	virtual void enter(SysNoticeFsm& fsm);
};

#endif

