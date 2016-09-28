#ifndef _SERVER_FSM_STATE_H_
#define _SERVER_FSM_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class FfmpegFsm;
class FfmpegFsmStateInit;
class FfmpegStateWaitQueryDB;
class FfmpegStateEnd;

class FfmpegState
{
public:
	FfmpegState(const std::string& name): _name(name)
	{

	}

	virtual ~FfmpegState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(FfmpegFsm& fsm);
	virtual void exit(FfmpegFsm& fsm);

public:
	virtual void client_req_event(FfmpegFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void db_reply_event(FfmpegFsm& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(FfmpegFsm& fsm, void* param);


public:
	static FfmpegFsmStateInit             state_init;
	static FfmpegStateWaitQueryDB      state_wait_query_db;
	static FfmpegStateEnd				state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class FfmpegFsmStateInit : public FfmpegState
{
public:
	FfmpegFsmStateInit(const std::string name):FfmpegState(name)
	{

	}

	virtual void client_req_event(FfmpegFsm& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class FfmpegStateWaitQueryDB : public FfmpegState
{
public:
	FfmpegStateWaitQueryDB(const std::string name):FfmpegState(name)
	{

	}

	virtual void enter(FfmpegFsm& fsm);
	virtual void exit(FfmpegFsm& fsm);
   	virtual void db_reply_event(FfmpegFsm& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(FfmpegFsm& fsm, void* param);
};

class FfmpegStateEnd : public FfmpegState
{
public:
	FfmpegStateEnd(const std::string name):FfmpegState(name)
	{

	}

	virtual void enter(FfmpegFsm& fsm);
};

#endif

