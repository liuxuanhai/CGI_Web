#ifndef _SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _SERVER_EXECUTOR_THREAD_QUEUE_H_

#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include "memory/duplex_queue.h"
#include "msg.pb.h"

using namespace std;

    class ExecutorThreadRequestType
    {
    public:
        enum Type
        {
			T_USER_MARK_QUERY,
			T_USER_MARK_ADD,
			T_USER_MARK_UPDATE,
			T_USER_MARK_DELETE,
        };

        enum Error
        {
            E_OK = 0,
            E_NOT_EXIST = 1,
            E_DB_FAIL = 2,
        };

        static const std::string type_string(int type)
        {
            switch(type)
            {
				case T_USER_MARK_QUERY:
					return "ExecutorThreadRequestType::T_USER_MARK_QUERY";

				case T_USER_MARK_ADD:
					return "ExecutorThreadRequestType::T_USER_MARK_ADD";

				case T_USER_MARK_UPDATE:
					return "ExecutorThreadRequestType::T_USER_MARK_UPDATE";

				case T_USER_MARK_DELETE:
					return "ExecutorThreadRequestType::T_USER_MARK_DELETE";

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
    	
        ExecutorThreadRequestElement(int request_type, int fsm_id):
			m_request_type(request_type),
			m_fsm_id(fsm_id),
			m_need_reply(false)
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
              <<", fsm_id: "<<m_fsm_id
              <<", need_reply: "<<m_need_reply;

			if(ExecutorThreadRequestType::T_USER_MARK_QUERY == m_request_type 
				|| ExecutorThreadRequestType::T_USER_MARK_DELETE == m_request_type)
			{
				ss<<", m_openid_md5: "<<m_openid_md5;
			}
			else if(ExecutorThreadRequestType::T_USER_MARK_ADD == m_request_type)
			{
				ss<<", m_usermark: "<<m_usermark.Utf8DebugString();
			}
			
            ss<<"}";

            return ss.str();
        }

    public:
    	uint32_t m_request_type;
        uint32_t m_fsm_id;
        bool m_need_reply;

		::hoosho::commstruct::UserMark m_usermark;
		uint64_t m_openid_md5;
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
              <<", usermark: " <<m_usermark.Utf8DebugString()
              <<"}";

            return ss.str();
        }

    public:
        uint32_t m_request_type;
        uint32_t m_fsm_id;
        uint32_t m_result_code;
        
		::hoosho::commstruct::UserMark m_usermark;
		std::vector<hoosho::commstruct::UserMark> vecUserMark;
    };

    typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


