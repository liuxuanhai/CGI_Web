#ifndef _HOOSHO_USER_SERVER_FSM_STATE_PREAUTH_H_
#define _HOOSHO_USER_SERVER_FSM_STATE_PREAUTH_H_

#include <string>
#include "net/connectioninfo.h"
#include "msg.pb.h"
#include "executor_thread_queue.h"
#include "util/logger.h"

class FsmPreAuth;
class FsmPreAuthStateInit;
class FsmPreAuthStateWaitPreauthReply;
class FsmPreAuthStateWaitUserInfoGetReply;
class FsmPreAuthStateWaitApplyHooshoNOReply;
class FsmPreAuthStateWaitUserInfoUpdateReply;
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
	virtual void preauth_reply_event(FsmPreAuth& fsm, const ::hoosho::msg::Msg& stMsg);
	virtual void user_info_get_reply_event(FsmPreAuth& fsm, ExecutorThreadResponseElement& element);
	virtual void apply_hoosho_no_reply_event(FsmPreAuth& fsm, const ::hoosho::msg::Msg& stMsg);
	virtual void user_info_update_reply_event(FsmPreAuth& fsm, const ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmPreAuth& fsm, void* param);


public:
	static FsmPreAuthStateInit             		  state_init;
	static FsmPreAuthStateWaitPreauthReply 		  state_wait_preauth_reply;
	static FsmPreAuthStateWaitUserInfoGetReply 	  state_wait_user_info_get_reply;
	static FsmPreAuthStateWaitApplyHooshoNOReply  state_wait_apply_hoosho_no_reply;
	static FsmPreAuthStateWaitUserInfoUpdateReply state_wait_user_info_update_reply;
	static FsmPreAuthStateEnd					  state_end;
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




class FsmPreAuthStateWaitPreauthReply : public FsmPreAuthState
{
public:
	FsmPreAuthStateWaitPreauthReply(const std::string name):FsmPreAuthState(name)
	{

	}

	virtual void enter(FsmPreAuth& fsm);
	virtual void exit(FsmPreAuth& fsm);
   	virtual void preauth_reply_event(FsmPreAuth& fsm, const ::hoosho::msg::Msg& stMsg);
    virtual void timeout_event(FsmPreAuth& fsm, void* param);
};

class FsmPreAuthStateWaitUserInfoGetReply : public FsmPreAuthState
{
public:
	FsmPreAuthStateWaitUserInfoGetReply(const std::string name):FsmPreAuthState(name)
	{

	}

	virtual void enter(FsmPreAuth& fsm);
	virtual void exit(FsmPreAuth& fsm);
   	virtual void user_info_get_reply_event(FsmPreAuth& fsm, ExecutorThreadResponseElement& element);
    virtual void timeout_event(FsmPreAuth& fsm, void* param);
};

class FsmPreAuthStateWaitApplyHooshoNOReply : public FsmPreAuthState
{
public:
	FsmPreAuthStateWaitApplyHooshoNOReply(const std::string name):FsmPreAuthState(name)
	{

	}

	virtual void enter(FsmPreAuth& fsm);
	virtual void exit(FsmPreAuth& fsm);
   	virtual void apply_hoosho_no_reply_event(FsmPreAuth& fsm, const ::hoosho::msg::Msg& stMsg);
    virtual void timeout_event(FsmPreAuth& fsm, void* param);
};

class FsmPreAuthStateWaitUserInfoUpdateReply : public FsmPreAuthState
{
public:
	FsmPreAuthStateWaitUserInfoUpdateReply(const std::string name):FsmPreAuthState(name)
	{

	}

	virtual void enter(FsmPreAuth& fsm);
	virtual void exit(FsmPreAuth& fsm);
   	virtual void user_info_update_reply_event(FsmPreAuth& fsm, const ExecutorThreadResponseElement& element);
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

