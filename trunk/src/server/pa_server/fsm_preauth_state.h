#ifndef _HOOSHO_PA_SERVER_FSM_STATE_PREAUTH_H_
#define _HOOSHO_PA_SERVER_FSM_STATE_PREAUTH_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class FsmPreAuth;
class FsmPreAuthStateInit;
class FsmPreAuthStateWaitThreadPreAuth;
class FsmPreAuthStateEnd;

class FsmPreAuthState
{
public:
	FsmPreAuthState(const std::string& name): _name(name)
	{

	}

	virtual ~FsmPreAuthState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(FsmPreAuth& fsm);
	virtual void exit(FsmPreAuth& fsm);

public:
	virtual void req_preauth_event(FsmPreAuth& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void preauth_reply_event(FsmPreAuth& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmPreAuth& fsm, void* param);


public:
	static FsmPreAuthStateInit             state_init;
	static FsmPreAuthStateWaitThreadPreAuth state_wait_thread_pre_auth;
	static FsmPreAuthStateEnd				state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class FsmPreAuthStateInit : public FsmPreAuthState
{
public:
	FsmPreAuthStateInit(const std::string name):FsmPreAuthState(name)
	{

	}

	virtual void req_preauth_event(FsmPreAuth& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class FsmPreAuthStateWaitThreadPreAuth : public FsmPreAuthState
{
public:
	FsmPreAuthStateWaitThreadPreAuth(const std::string name):FsmPreAuthState(name)
	{

	}

	virtual void enter(FsmPreAuth& fsm);
	virtual void exit(FsmPreAuth& fsm);
   	virtual void preauth_reply_event(FsmPreAuth& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmPreAuth& fsm, void* param);
};

class FsmPreAuthStateEnd : public FsmPreAuthState
{
public:
	FsmPreAuthStateEnd(const std::string name):FsmPreAuthState(name)
	{

	}

	virtual void enter(FsmPreAuth& fsm);
};

#endif

