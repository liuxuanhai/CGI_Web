#ifndef _TEST_SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _TEST_SERVER_EXECUTOR_THREAD_QUEUE_H_

#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include "memory/duplex_queue.h"
#include "msg.pb.h"
#include "wx_user_info.h"
#include "fsm_container.h"

using namespace std;

class ExecutorThreadRequestType
{
public:
    enum Type
    {
        T_PREAUTH,  //PreAuthCode->WxUserInfo
        T_PA_TOKEN,      // PA->accessToken, xxxTickets
        T_RESOURCE_DOWNLOAD, //download wx media 
        T_PA_LIST,		//get appid_md5 list
        T_PA_TAB,		//get pa tab        
        T_PA_EXPERT_QUERY,
    };

    enum Error
    {
        E_OK = 0,
        E_FAIL = 1,
        E_DB_ERR = 2,
    };

    static const std::string type_string(int type)
    {
        switch(type)
        {
            case T_PREAUTH:
                return "ExecutorThreadRequestType::T_PREAUTH";

			case T_PA_TOKEN:
				return "ExecutorThreadRequestType::T_PA_TOKEN";

			case T_RESOURCE_DOWNLOAD:
				return "ExecutorThreadRequestType::T_RESOURCE_DOWNLOAD";

			case T_PA_LIST:
				return "ExecutorThreadRequestType::T_PA_LIST";

			case T_PA_TAB:
				return "ExecutorThreadRequestType::T_PA_TAB";
			
			case T_PA_EXPERT_QUERY:
				return "ExecutorThreadRequestType::T_PA_EXPERT_QUERY";
			
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
        if(ExecutorThreadRequestType::T_PREAUTH == m_request_type)
        {
			ss<<", pre_auth_code:"<<m_pre_auth_code;
        }
        else if(ExecutorThreadRequestType::T_RESOURCE_DOWNLOAD == m_request_type)
        {
			ss<<", access_token: "<<m_pa_access_token
				<<", media_id: "<<m_media_id;
        }		
		else if(ExecutorThreadRequestType::T_PA_EXPERT_QUERY == m_request_type)
		{
			ss<<", pa_appid_md5: "<<m_pa_appid_md5
				<<", begin_create_ts: "<<m_begin_create_ts
				<<", limit: "<<m_limit;
		}		
		
        ss<<"}";

        return ss.str();
    }

public:
	//necessary
	uint32_t m_request_type;
	int m_fsm_type;
    uint32_t m_fsm_id;
	bool m_need_reply;

	//req param
    std::string m_pre_auth_code;
	std::string m_pa_appid;
	std::string m_pa_appsecret;
	std::string m_pa_access_token;
	std::string m_media_id;
	std::string m_uin;
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5;
	uint64_t m_begin_create_ts;
	uint64_t m_limit;
	
	std::vector<uint64_t> m_openid_md5_list;
	
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
        if(ExecutorThreadRequestType::T_PREAUTH == m_request_type)
        {
			ss<<", wx_user_info:"<<m_wx_user_info.to_string();
        }
        else if(ExecutorThreadRequestType::T_PA_TOKEN == m_request_type)
        {
			ss<<", m_pa_accesstoken:"<<m_pa_accesstoken;
			ss<<", m_pa_ticket_jsapi:"<<m_pa_ticket_jsapi;
        }
        else if(ExecutorThreadRequestType::T_RESOURCE_DOWNLOAD == m_request_type)
        {
			ss<<", resource_size: "<<m_resource_data.size();
        }		
		else if(ExecutorThreadRequestType::T_PA_LIST == m_request_type)
		{
			ss<<", appid_md5_list: {";
			for(size_t i=0; i<m_appid_md5_list.size(); ++i)
			{
				if(i != 0)
				{
					ss<<", ";
				}
		
				ss<<m_appid_md5_list[i];
			}
			ss<<"}";
		}
		else if(ExecutorThreadRequestType::T_PA_TAB == m_request_type)
		{
			ss<<", tab_info_list: {";
			for(size_t i=0; i<m_tab_info_list.size(); ++i)
			{
				if(i != 0)
				{
					ss<<", ";
				}
		
				ss<<m_tab_info_list[i].Utf8DebugString();
			}
			ss<<"}";
		}
		else if(ExecutorThreadRequestType::T_PA_EXPERT_QUERY== m_request_type)
		{
			ss<<", openid_md5_list: {";
			for(size_t i=0; i<m_openid_md5_list.size(); ++i)
			{
				if(i != 0)
				{
					ss<<", ";
				}
		
				ss<<m_openid_md5_list[i];
			}
			ss<<"}";
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
	::common::wxapi::WXUserInfo m_wx_user_info;//pre auth code
	
	std::string m_pa_accesstoken;//pa accesstoken
	std::string m_pa_ticket_jsapi;//ticket jsapi
	std::string m_resource_data; //resource data
	
	std::vector<uint64_t> m_appid_md5_list;
	std::vector<hoosho::commstruct::TabInfo> m_tab_info_list;
	std::vector<uint64_t> m_openid_md5_list;
};

typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


