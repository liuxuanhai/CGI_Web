#ifndef _MSG_SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _MSG_SERVER_EXECUTOR_THREAD_QUEUE_H_

#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include "memory/duplex_queue.h"
#include "msg_content.h"

using namespace std;

    class ExecutorThreadRequestType
    {
    public:
        enum Type
        {
			T_MSG_CONTENT_QUERY,
			T_MSG_CONTENT_ADD,
			T_MSG_CONTENT_DELETE,
			T_MSG_NEWMSG_STATUS_QUERY,
			T_MSG_SESSION_LIST_QUERY,
			T_MSG_SESSION_DELETE,
			T_MSG_RED_POINT_QUERY,
			T_MSG_RED_POINT_UPDATE,
			T_MSG_NOTICE_RECORD_ADD,
			T_MSG_NOTICE_RECORD_QUERY,
			T_MSG_NOTICE_RECORD_DELETE,
			T_MSG_SYSTEM_MSG_QUERY,
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
				case T_MSG_CONTENT_QUERY:
					return "ExecutorThreadRequestType::T_MSG_CONTENT_QUERY";

				case T_MSG_CONTENT_ADD:
					return "ExecutorThreadRequestType::T_MSG_CONTENT_ADD";

				case T_MSG_CONTENT_DELETE:
					return "ExecutorThreadRequestType::T_MSG_CONTENT_DELETE";

				case T_MSG_NEWMSG_STATUS_QUERY:
					return "ExecutorThreadRequestType::T_MSG_NEWMSG_STATUS_QUERY";

				case T_MSG_SESSION_LIST_QUERY:
					return "ExecutorThreadRequestType::T_MSG_SESSION_LIST_QUERY";

				case T_MSG_SESSION_DELETE:
					return "ExecutorThreadRequestType::T_MSG_SESSION_DELETE";

				case T_MSG_RED_POINT_QUERY:
					return "ExecutorThreadRequestType::T_MSG_RED_POINT_QUERY";

				case T_MSG_NOTICE_RECORD_ADD:
					return "ExecutorThreadRequestType::T_MSG_NOTICE_RECORD_ADD";

				case T_MSG_NOTICE_RECORD_QUERY:
					return "ExecutorThreadRequestType::T_MSG_NOTICE_RECORD_QUERY";

				case T_MSG_NOTICE_RECORD_DELETE:
					return "ExecutorThreadRequestType::T_MSG_NOTICE_RECORD_DELETE";

				case T_MSG_SYSTEM_MSG_QUERY:
					return "ExecutorThreadRequestType::T_MSG_SYSTEM_MSG_QUERY";

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
    	
        ExecutorThreadRequestElement(int request_type, int fsm_id)
			: m_request_type(request_type)
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
              <<"fsm_id: "<<m_fsm_id
              <<", need_reply: "<<m_need_reply
              <<", msg_content: "<<m_msg_content.ToString()
              <<"}";

            return ss.str();
        }

    public:
    	uint32_t m_request_type;
        uint32_t m_fsm_id;
        MsgContent m_msg_content;
        bool m_need_reply;

		uint64_t m_openid_md5;
		uint32_t m_redpoint_type;
		uint32_t m_redpoint_opt;

		::hoosho::commstruct::NoticeRecord m_notice_record;
		uint64_t m_pa_appid_md5;
		uint64_t m_notice_record_type;
		uint64_t m_limit_ts;
		uint64_t m_pagesize;
		
		uint64_t m_create_ts;		
		
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
              <<", msg_content:" <<m_msg_content.ToString()
              <<"}";

            return ss.str();
        }

    public:
        uint32_t m_request_type;
        uint32_t m_fsm_id;
        uint32_t m_result_code;
        
        MsgContent m_msg_content;
		std::vector<hoosho::commstruct::MsgContent> vecMsgContent;

		int newmsg_status;
		std::vector<hoosho::commstruct::Session> vecSession;
		std::vector<hoosho::commstruct::RedPointInfo> vecRedPointInfo;
		std::vector<hoosho::commstruct::NoticeRecord> vecNoticeRecord;
		std::vector<hoosho::commstruct::SystemMsg> 	vecSystemMsg;
    };

    typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


