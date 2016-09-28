#ifndef _FEEDS_SERVER_FSM_FAVORITE_STATE_H_
#define _FEEDS_SERVER_FSM_FAVORITE_STATE_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"

class FsmFavorite;
class FsmFavoriteStateInit;
class FsmFavoriteStateWaitQueryDB;
class FsmFavoriteStateWaitAddDB;
class FsmFavoriteStateWaitUpdateDB;
class FsmFavoriteStateWaitDeleteDB;
class FsmFavoriteStateEnd;

class FsmFavoriteState
{
public:
	FsmFavoriteState(const std::string& name): _name(name)
	{

	}

	virtual ~FsmFavoriteState()
	{

	}

	virtual std::string name() const
	{
		return _name;
	}

public:
	virtual void enter(FsmFavorite& fsm);
	virtual void exit(FsmFavorite& fsm);

public:
	virtual void client_req_event(FsmFavorite& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
	virtual void db_reply_event(FsmFavorite& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFavorite& fsm, void* param);


public:
	static FsmFavoriteStateInit             state_init;
	static FsmFavoriteStateWaitQueryDB      state_wait_query_db;
	static FsmFavoriteStateWaitAddDB		state_wait_add_db;
	static FsmFavoriteStateWaitUpdateDB     state_wait_update_db;
	static FsmFavoriteStateWaitDeleteDB		state_wait_delete_db;
	static FsmFavoriteStateEnd				state_end;
public:
	DECL_LOGGER(logger);

private:
	std::string _name;	
};

class FsmFavoriteStateInit : public FsmFavoriteState
{
public:
	FsmFavoriteStateInit(const std::string name):FsmFavoriteState(name)
	{

	}

	virtual void client_req_event(FsmFavorite& fsm, lce::net::ConnectionInfo& conn, const hoosho::msg::Msg& stMsg);
};

class FsmFavoriteStateWaitQueryDB : public FsmFavoriteState
{
public:
	FsmFavoriteStateWaitQueryDB(const std::string name):FsmFavoriteState(name)
	{

	}

	virtual void enter(FsmFavorite& fsm);
	virtual void exit(FsmFavorite& fsm);
   	virtual void db_reply_event(FsmFavorite& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFavorite& fsm, void* param);
};

class FsmFavoriteStateWaitAddDB : public FsmFavoriteState
{
public:
	FsmFavoriteStateWaitAddDB(const std::string name):FsmFavoriteState(name)
	{

	}

	virtual void enter(FsmFavorite& fsm);
	virtual void exit(FsmFavorite& fsm);
   	virtual void db_reply_event(FsmFavorite& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFavorite& fsm, void* param);
};

class FsmFavoriteStateWaitUpdateDB : public FsmFavoriteState
{
public:
	FsmFavoriteStateWaitUpdateDB(const std::string name):FsmFavoriteState(name)
	{

	}

	virtual void enter(FsmFavorite& fsm);
	virtual void exit(FsmFavorite& fsm);
   	virtual void db_reply_event(FsmFavorite& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFavorite& fsm, void* param);
};

class FsmFavoriteStateWaitDeleteDB : public FsmFavoriteState
{
public:
	FsmFavoriteStateWaitDeleteDB(const std::string name):FsmFavoriteState(name)
	{

	}

	virtual void enter(FsmFavorite& fsm);
	virtual void exit(FsmFavorite& fsm);
   	virtual void db_reply_event(FsmFavorite& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmFavorite& fsm, void* param);
};


class FsmFavoriteStateEnd : public FsmFavoriteState
{
public:
	FsmFavoriteStateEnd(const std::string name):FsmFavoriteState(name)
	{

	}

	virtual void enter(FsmFavorite& fsm);
};

#endif

