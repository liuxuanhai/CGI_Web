#ifndef _TEST_SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _TEST_SERVER_EXECUTOR_THREAD_QUEUE_H_

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
            T_USER_INFO_QUERY,
            T_USER_INFO_UPDATE,
            T_USER_INFO_DELETE,
        };

        enum Error
        {
            E_OK = 0,
            E_NOT_EXIST,
            E_DB_FAIL,
        };

        static const std::string type_string(int type)
        {
            switch(type)
            {
                case T_USER_INFO_QUERY:
                    return "ExecutorThreadRequestType::T_USER_INFO_QUERY";

				case T_USER_INFO_UPDATE:
					return "ExecutorThreadRequestType::T_USER_INFO_UPDATE";
					
            	case T_USER_INFO_DELETE:
					return "ExecutorThreadRequestType::T_USER_INFO_DELETE";

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
            ss<<"{"
              <<"request_type: "<<ExecutorThreadRequestType::type_string(m_request_type)
              <<"fsm_id: "<<m_fsm_id
              <<", need_reply: "<<m_need_reply
              <<", user_info: "<<m_user_info.ToString()
              <<"}";

            return ss.str();
        }

    public:
    	uint32_t m_request_type;
        uint32_t m_fsm_id;
        DaoUserInfo m_user_info;
        bool m_need_reply;
    };

    class ExecutorThreadResponseElement
    {
    public:
        ExecutorThreadResponseElement()
        {
        }
        
        ExecutorThreadResponseElement(uint32_t dwReqType, uint32_t dwFsmId)
            : m_request_type(dwReqType)
            , m_fsm_id(dwFsmId)
            , m_result_code(ExecutorThreadRequestType::E_OK)
        {
        }

        const std::string ToString() const
        {
            std::ostringstream ss;
            ss<<"{"
			   <<" req_type: "<<ExecutorThreadRequestType::type_string(m_request_type)
              <<", fsm_id: "<<m_fsm_id
              <<", result_code: "<<ExecutorThreadRequestType::err_string(m_result_code)
              <<", user_info:" <<m_user_info.ToString()
              <<"}";

            return ss.str();
        }

    public:
        uint32_t m_request_type;
        uint32_t m_fsm_id;
        uint32_t m_result_code;
        
        DaoUserInfo m_user_info;
    };

    typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


