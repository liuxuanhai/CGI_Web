#ifndef _TEST_SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _TEST_SERVER_EXECUTOR_THREAD_QUEUE_H_

#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include "memory/duplex_queue.h"
#include "msg.pb.h"
#include "fsm_container.h"

using namespace std;

class ExecutorThreadRequestType
{
public:
    enum Type
    {
        T_USER_LOGIN,
        T_USERINFO_UPDATE,
        T_USERINFO_QUERY,
    };

    enum Error
    {
        E_OK = 0,
        E_WX_ERR = 1,
        E_DB_ERR = 2,
        E_PARAM_ERR = 3,
        E_INNER_ERR = 4,
        E_NOT_PA_FANS = 5,
    };

    static const std::string type_string(int type)
    {
        switch(type)
        {
            case T_USERINFO_QUERY:
                return "ExecutorThreadRequestType::T_USERINFO_QUERY";

			case T_USERINFO_UPDATE:
				return "ExecutorThreadRequestType::T_USERINFO_UPDATE";

			case T_USER_LOGIN:
				return "ExecutorThreadRequestType::T_USER_LOGIN";

			case E_INNER_ERR:
				return "ExecutorThreadRequestType::E_INNER_ERR";
				
            default:
                return "ExecutorThreadRequestType::T_UNKNOWN";
        }
    }

    static const std::string err_string(int err)
    {
        switch(err)
        {
            case E_OK:
                return "ExecutorThreadRequestType::E_OK";

            case E_WX_ERR:
                return "ExecutorThreadRequestType::E_WX_ERR";

            case E_DB_ERR:
            	return "ExecutorThreadRequestType::E_DB_ERR";

			case E_PARAM_ERR:
            	return "ExecutorThreadRequestType::E_PARAM_ERR";
				
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
	
    ExecutorThreadRequestElement(int request_type, int fsm_type, int fsm_id)
    	: m_request_type(request_type)
        , m_fsm_type(fsm_type)
        , m_fsm_id(fsm_id)
        , m_need_reply(false)
    {
    }

    void need_reply()
    {
        m_need_reply = true;
    }

    const std::string ToString() const
    {
        std::ostringstream ss;
        ss<<"{"
          <<"request_type: "<<ExecutorThreadRequestType::type_string(m_request_type)
          <<", need_reply: "<<m_need_reply
          <<", fsm_type: "<<FsmContainer<int>::fsm_type_string(m_fsm_type)
          <<", fsm_id: "<<m_fsm_id;
        if(ExecutorThreadRequestType::T_USERINFO_QUERY == m_request_type)
        {
			ss<<", openid_md5_list: {";
			for(size_t i=0; i<m_openid_list.size(); ++i)
			{
				if(i != 0)
				{
					ss<<", ";
				}

				ss<<m_openid_list[i];
			}
			ss<<"}";
        }
        else if(ExecutorThreadRequestType::T_USERINFO_UPDATE == m_request_type)
        {
			ss<<", user_detail_info:"<<m_user_info.Utf8DebugString();
        }
        else if(ExecutorThreadRequestType::T_USER_LOGIN == m_request_type)
        {
        	ss<<", snsapi_base_pre_auth_code: "<<m_snsapi_base_pre_auth_code;
        }
      
        return ss.str();
    }

public:
	//necessary
	uint32_t m_request_type;
	int m_fsm_type;
    uint32_t m_fsm_id;
	bool m_need_reply;

	//req param
    ::hoosho::j::commstruct::UserInfo m_user_info;
    std::vector<std::string> m_openid_list;
	std::string m_snsapi_base_pre_auth_code;
};

class ExecutorThreadResponseElement
{
public:
    ExecutorThreadResponseElement()
    {
    }
    
    ExecutorThreadResponseElement(int request_type, int fsm_type, int fsm_id)
        : m_request_type(request_type)
        , m_fsm_type(fsm_type)
        , m_fsm_id(fsm_id)
        , m_result_code(ExecutorThreadRequestType::E_OK)
    {
    }

    const std::string ToString() const
    {
        std::ostringstream ss;
        ss<<"{"
		   <<" req_type: "<<ExecutorThreadRequestType::type_string(m_request_type)
		   <<", fsm_type: "<<FsmContainer<int>::fsm_type_string(m_fsm_type)
          <<", fsm_id: "<<m_fsm_id
          <<", result_code: "<<ExecutorThreadRequestType::err_string(m_result_code);
        if(ExecutorThreadRequestType::T_USER_LOGIN == m_request_type)
        {
			ss<<", user_info:"<<m_user_info.Utf8DebugString();
        }
		else if(ExecutorThreadRequestType::T_USERINFO_UPDATE == m_request_type)
        {
			ss<<", user_info.self_desc:"<<m_user_info.self_desc();
        }
        else if(ExecutorThreadRequestType::T_USERINFO_QUERY == m_request_type)
        {
        	ss<<", user_info_list: {";
			for(size_t i=0; i<m_user_info_list.size(); ++i)
			{
				if(i != 0)
				{
					ss<<", ";
				}

				ss<<m_user_info_list[i].Utf8DebugString();
			}
        }

        ss<<"}";

        return ss.str();
    }

public:
	//necessary
    uint32_t m_request_type;
    uint32_t m_fsm_type;
    uint32_t m_fsm_id;
    uint32_t m_result_code;


	//reply result
	::hoosho::j::commstruct::UserInfo m_user_info;
	std::vector<hoosho::j::commstruct::UserInfo> m_user_info_list;
};

typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


