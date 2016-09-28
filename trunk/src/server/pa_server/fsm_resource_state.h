#ifndef _HOOSHO_PA_SERVER_FSM_STATE_PREAUTH_H_
#define _HOOSHO_PA_SERVER_FSM_STATE_PREAUTH_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class FsmResource;
class FsmResourceStateInit;
class FsmResourceStateWaitThreadDealResource;
class FsmResourceStateEnd;

class FsmResourceState
{
public:
	FsmResourceState(const std::string& name): _name(name)
	{

	}

	virtual ~FsmResourceState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(FsmResource& fsm);
	virtual void exit(FsmResource& fsm);

public:
	virtual void download_req_event(FsmResource& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void download_reply_event(FsmResource& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmResource& fsm, void* param);


public:
	static FsmResourceStateInit             		state_init;
	static FsmResourceStateWaitThreadDealResource   state_wait_thread_deal_resource;
	static FsmResourceStateEnd						state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class FsmResourceStateInit : public FsmResourceState
{
public:
	FsmResourceStateInit(const std::string name):FsmResourceState(name)
	{

	}

	virtual void download_req_event(FsmResource& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class FsmResourceStateWaitThreadDealResource : public FsmResourceState
{
public:
	FsmResourceStateWaitThreadDealResource(const std::string name):FsmResourceState(name)
	{

	}

	virtual void enter(FsmResource& fsm);
	virtual void exit(FsmResource& fsm);
   	virtual void download_reply_event(FsmResource& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmResource& fsm, void* param);
};

class FsmResourceStateEnd : public FsmResourceState
{
public:
	FsmResourceStateEnd(const std::string name):FsmResourceState(name)
	{

	}

	virtual void enter(FsmResource& fsm);
};

#endif

