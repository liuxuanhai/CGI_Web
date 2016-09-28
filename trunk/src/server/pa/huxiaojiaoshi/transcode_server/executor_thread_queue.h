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
			T_FFMPEG_TRANSCODE_REQ,
        };

        enum Error
        {
            E_OK = 0,
            E_FAIL = 1,
            E_DB_FAIL = 2,
        };

        static const std::string type_string(int type)
        {
            switch(type)
            {
				case T_FFMPEG_TRANSCODE_REQ:
					return "ExecutorThreadRequestType::T_FFMPEG_TRANSCODE_REQ";				

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

                case E_FAIL:
                    return "ExecutorThreadRequestType::E_FAIL";

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

			if(ExecutorThreadRequestType::T_FFMPEG_TRANSCODE_REQ == m_request_type )
			{
				ss<<", wx_media_id: "<<m_wx_media_id;
			}			
			
            ss<<"}";

            return ss.str();
        }

    public:    	
		uint32_t m_request_type;
		uint32_t m_fsm_id;
    
        bool m_need_reply;
        uint32_t m_media_type;
        std::string m_wx_media_id;
		
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
              <<", local_media_id: "<<m_local_media_id
              <<"}";

            return ss.str();
        }

    public:
        uint32_t m_request_type;
        uint32_t m_fsm_id;
        uint32_t m_result_code;
        
		std::string m_local_media_id;
    };

    typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


