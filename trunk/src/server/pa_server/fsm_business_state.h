#ifndef _HOOSHO_PA_SERVER_FSM_STATE_BUSINESS_H_
#define _HOOSHO_PA_SERVER_FSM_STATE_BUSINESS_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class FsmBusiness;
class FsmBusinessStateInit;
class FsmBusinessStateWaitThread;
class FsmBusinessStateEnd;

class FsmBusinessState
{
public:
	FsmBusinessState(const std::string& name): _name(name)
	{

	}

	virtual ~FsmBusinessState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(FsmBusiness& fsm);
	virtual void exit(FsmBusiness& fsm);

public:
	virtual void client_req_event(FsmBusiness& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void thread_reply_event(FsmBusiness& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmBusiness& fsm, void* param);


public:
	static FsmBusinessStateInit             		  state_init;
	static FsmBusinessStateWaitThread 				  state_wait_thread;
	static FsmBusinessStateEnd					  	  state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class FsmBusinessStateInit : public FsmBusinessState
{
public:
	FsmBusinessStateInit(const std::string name):FsmBusinessState(name)
	{

	}

	virtual void client_req_event(FsmBusiness& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class FsmBusinessStateWaitThread : public FsmBusinessState
{
public:
	FsmBusinessStateWaitThread(const std::string name):FsmBusinessState(name)
	{

	}

	virtual void enter(FsmBusiness& fsm);
	virtual void exit(FsmBusiness& fsm);
   	virtual void thread_reply_event(FsmBusiness& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmBusiness& fsm, void* param);
};

class FsmBusinessStateEnd : public FsmBusinessState
{
public:
	FsmBusinessStateEnd(const std::string name):FsmBusinessState(name)
	{

	}

	virtual void enter(FsmBusiness& fsm);
};

#endif

