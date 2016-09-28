#ifndef _HOOSHO_TOKEN_EXECUTOR_THREAD_QUEUE_H_
#define _HOOSHO_TOKEN_EXECUTOR_THREAD_QUEUE_H_

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
        T_PA_TOKEN_UPDATE, // PA->accessToken, xxxTickets
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
			case T_PA_TOKEN_UPDATE:
				return "ExecutorThreadRequestType::T_PA_TOKEN_UPDATE";
			
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
          <<", need_reply: "<<m_need_reply;
          
		if(ExecutorThreadRequestType::T_PA_TOKEN_UPDATE == m_request_type)
		{
			ss<<", pa_appid: "<<m_pa_appid
				<<", begin_create_ts: "<<m_pa_appsecret;
		}		
		
        ss<<"}";

        return ss.str();
    }

public:
	//necessary
	uint32_t m_request_type;
	bool m_need_reply;

	std::string m_pa_appid;
	std::string m_pa_appsecret;
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
          <<", result_code: "<<ExecutorThreadRequestType::err_string(m_result_code);
        if(ExecutorThreadRequestType::T_PA_TOKEN_UPDATE == m_request_type)
        {
        	ss<<", m_pa_appid:"<<m_pa_appid;
        	ss<<", m_pa_appsecret:"<<m_pa_appsecret;
			ss<<", m_pa_accesstoken:"<<m_pa_accesstoken;
			ss<<", m_pa_ticket_jsapi:"<<m_pa_ticket_jsapi;
        }
       
        ss<<"}";

        return ss.str();
    }

public:
	//necessary
    uint32_t m_request_type;
    uint32_t m_result_code;

	std::string m_pa_appid;
	std::string m_pa_appsecret;
	std::string m_pa_accesstoken;//pa accesstoken
	std::string m_pa_ticket_jsapi;//ticket jsapi
};

typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


