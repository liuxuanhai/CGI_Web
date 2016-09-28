#ifndef _NOTICE_SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _NOTICE_SERVER_EXECUTOR_THREAD_QUEUE_H_

#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include "memory/duplex_queue.h"
#include "dao.h"

using namespace std;

class ExecutorThreadRequestType
{
public:
	enum Type
	{
		T_SYS_NOTICE_ADD,
		T_SYS_NOTICE_NEW_QUERY,
		T_SYS_NOTICE_ALL_QUERY,
		T_SYS_NOTICE_DELETE,
		T_SYS_NOTICE_SELECT_UPDATE,
		T_NOTICE_NOTIFY_SYS_NOTICE_QUERY,
		T_NOTICE_NOTIFY_QUERY,
		T_NOTICE_NOTIFY_TYPE_LIST_QUERY,
		T_NOTICE_RECORD_QUERY,
		T_NOTICE_RECORD_ADD,
		T_NOTICE_RECORD_DELETE,
		T_NOTICE_RECORD_SELECT_UPDATE,

	};

	enum Error
	{
		E_OK = 0, E_NOT_EXIST, E_DB_FAIL, E_DB_ALREADY_EXIST,
	};

	static const std::string type_string(int type)
	{
		switch (type)
		{
// ---------------------------------sys_notice----------------------------------------------------//
		case T_SYS_NOTICE_ALL_QUERY:
			return "ExecutorThreadRequestType::T_SYS_NOTICE_ALL_QUERY";

		case T_SYS_NOTICE_NEW_QUERY:
			return "ExecutorThreadRequestType::T_SYS_NOTICE_NEW_QUERY";

		case T_SYS_NOTICE_SELECT_UPDATE:
			return "ExecutorThreadRequestType::T_SYS_NOTICE_SELECT_UPDATE";
		case T_SYS_NOTICE_ADD:
			return "ExecutorThreadRequestType::T_SYS_NOTICE_ADD";
		case T_SYS_NOTICE_DELETE:
			return "ExecutorThreadRequestType::T_SYS_NOTICE_DELETE";

//----------------------------------notice_notify---------------------------------------------//
		case T_NOTICE_NOTIFY_SYS_NOTICE_QUERY:
			return "ExecutorThreadRequestType::T_NOTICE_NOTIFY_SYS_NOTICE_QUERY";
		case T_NOTICE_NOTIFY_QUERY:
			return "ExecutorThreadRequestType::T_NOTICE_NOTIFY_QUERY";
		case T_NOTICE_NOTIFY_TYPE_LIST_QUERY:
			return "ExecutorThreadRequestType::T_NOTICE_NOTIFY_TYPE_LIST_QUERY";

//-------------------------------------notice_record-----------------------------------------//
		case T_NOTICE_RECORD_QUERY:
			return "ExecutorThreadRequestType::T_NOTICE_RECORD_QUERY";
		case T_NOTICE_RECORD_ADD:
			return "ExecutorThreadRequestType::T_NOTICE_RECORD_UPDATE";
		case T_NOTICE_RECORD_DELETE:
			return "ExecutorThreadRequestType::T_NOTICE_RECORD_DELETE";
		case T_NOTICE_RECORD_SELECT_UPDATE:
			return "ExecutorThreadRequestType::T_NOTICE_RECORD_SELECT_UPDATE";
		default:
			return "ExecutorThreadRequestType::T_UNKNOWN";
		}
	}

	static const std::string err_string(int err)
	{
		switch (err)
		{
		case E_OK:
			return "ExecutorThreadRequestType::E_OK";

		case E_NOT_EXIST:
			return "ExecutorThreadRequestType::E_NOT_EXIST";

		case E_DB_FAIL:
			return "ExecutorThreadRequestType::E_DB_FAIL";

		default:
			return "ExecutorThreadRequestType::E_UNKNOWN";
		}
	}
};

class ExecutorThreadRequestElement
{
public:
	ExecutorThreadRequestElement()
	{
	}

	ExecutorThreadRequestElement(int request_type)
	{
		m_request_type = request_type;
		m_need_reply = false;

	}

	void need_reply()
	{
		m_need_reply = true;
	}

	const std::string ToString() const
	{
		std::ostringstream ss;
		ss << "{" << "\nrequest_type: " << ExecutorThreadRequestType::type_string(m_request_type) << " , fsm_id: " << m_fsm_id << ", need_reply: "
				<< m_need_reply << ", sys_notice: " << m_sys_notice.ToString() << ", pagesize: " << m_pagesize << ", openid_md5: " << m_openid_md5
				<< ",notice_user: "<<m_notice_user.ToString()
				<< "}";

		return ss.str();
	}

public:
	uint32_t m_request_type;
	uint32_t m_fsm_id;
	DaoSysNotice m_sys_notice;
	DaoNoticeUser m_notice_user;
	uint32_t m_pagesize;
	uint64_t m_openid_md5;

	//    uint64_t m_pa_appid_md5;
	//    uint64_t m_limit_ts;
	bool m_need_reply;
};

class ExecutorThreadResponseElement
{
public:
	ExecutorThreadResponseElement()
	{
	}

	ExecutorThreadResponseElement(uint32_t dwReqType, uint32_t dwFsmId) :
			m_request_type(dwReqType), m_fsm_id(dwFsmId), m_result_code(ExecutorThreadRequestType::E_OK)
	{
	}

	const std::string ToString() const
	{
		std::ostringstream ss;
		ss << "{" << " req_type: " << ExecutorThreadRequestType::type_string(m_request_type) << ", fsm_id: " << m_fsm_id << ", result_code: "
				<< ExecutorThreadRequestType::err_string(m_result_code) << ", sys_notice_list:" << m_sys_notice_list.ToString() << ", notice_state:"
				<< m_dao_notice_state.ToString() << "}";

		return ss.str();
	}

public:
	uint32_t m_request_type;
	uint32_t m_fsm_id;
	uint32_t m_result_code;
	DaoNoticeState m_dao_notice_state;
	DaoSysNoticeByAppidAndLimitTs m_sys_notice_list;
	DaoNoticeUserByAll m_notice_user_list;
};

typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif

