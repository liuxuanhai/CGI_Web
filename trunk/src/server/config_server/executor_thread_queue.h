#ifndef _TEST_SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _TEST_SERVER_EXECUTOR_THREAD_QUEUE_H_

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
        T_SAVE_HOOSHO_NO_ACCOUNT,
    };

    enum Error
    {
        E_OK = 0,
        E_FAIL = 1,
    };

    static const std::string type_string(int type)
    {
        switch(type)
        {
            case T_SAVE_HOOSHO_NO_ACCOUNT:
                return "ExecutorThreadRequestType::T_SAVE_HOOSHO_NO_ACCOUNT";
				
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
    	: m_request_type(request_type)
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
          <<", c_key: "<<m_c_key
          <<", c_value: "<<m_c_value
          <<"}";

        return ss.str();
    }

public:
	//necessary
	uint32_t m_request_type;
	bool m_need_reply;

	//req param
    uint64_t m_c_key;
    uint64_t m_c_value;
};

class ExecutorThreadResponseElement
{
public:
    ExecutorThreadResponseElement()
    {
    }
    
    ExecutorThreadResponseElement(int request_type)
        : m_request_type(request_type)
        , m_result_code(ExecutorThreadRequestType::E_OK)
    {
    }

    const std::string ToString() const
    {
        std::ostringstream ss;
        ss<<"{"
		   <<" req_type: "<<ExecutorThreadRequestType::type_string(m_request_type)
           <<", result_code: "<<ExecutorThreadRequestType::err_string(m_result_code)
           <<"}";

        return ss.str();
    }

public:
	//necessary
    uint32_t m_request_type;
    uint32_t m_result_code;
};

typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif

