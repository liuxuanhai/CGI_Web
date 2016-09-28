#ifndef _EXECUTOR_THREAD_QUEUE_H_
#define _EXECUTOR_THREAD_QUEUE_H_

#include <string>
#include <vector>
#include <set>
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
    	//user
        T_USER_LOGIN,        		
        T_GET_PHONE_VC,
        T_SET_PHONE,
        T_UPDATE_USER_INFO,
        T_GET_USER_INFO,
        T_UPDATE_USER_IDENTITY_INFO,
        T_GET_USER_IDENTITY_INFO,
        T_ADD_USER_RECV_ADDR_INFO,
        T_DEL_USER_RECV_ADDR_INFO,
        T_UPDATE_USER_RECV_ADDR_INFO,
        T_GET_USER_RECV_ADDR_INFO,
        

		//feed
		_________T_I_AM_SPITOR_FOR_FEED_________,

		T_GET_GOOD_TYPE,
		T_GET_GOOD_INFO,

		T_ADD_TICKET,
		T_GET_USER_FEED_LIST,
		T_GET_FEED_LIST,
		T_GET_USER_SHOW_LIST,
		T_GET_SHOW_LIST,
		T_GET_SHOW_DETAIL,
		T_ADD_SHOW,
		T_GET_USER_COLLECT_LIST,
		T_ADD_COLLECT,		
		T_GET_FEED_JOIN_USER_LIST,
		T_GET_USER_FEED_CONTEND_ID_LIST,
		T_GET_BANNER_LIST,
		T_DEL_COLLECT,
		T_CHECK_USER_COLLECT,
		T_GET_USER_LUCK_LIST,
		T_GET_FEED_DETATIL,
		T_GET_USER_TICKET_LIST,
    };

    enum Error
    {
        E_OK = 0,
        E_WX_ERR = 1,
        E_DB_ERR = 2,
        E_PARAM_ERR = 3,
        E_INNER_ERR = 4,
        E_NOT_PA_FANS = 5,

        E_SHOW_REPEATED = 6,
        E_UPLOAD_TICKET_LIMIT = 7,
    };

    static const std::string type_string(int type)
    {
        switch(type)
        {
        	//user
        	case T_USER_LOGIN:
				return "ExecutorThreadRequestType::T_USER_LOGIN";
			case T_GET_PHONE_VC:
				return "ExecutorThreadRequestType::T_GET_PHONE_VC";			
			case T_SET_PHONE:
				return "ExecutorThreadRequestType::T_SET_PHONE";
			case T_UPDATE_USER_INFO:
				return "ExecutorThreadRequestType::T_UPDATE_USER_INFO";
			case T_GET_USER_INFO:
				return "ExecutorThreadRequestType::T_GET_USER_INFO";
			case T_UPDATE_USER_IDENTITY_INFO:
				return "ExecutorThreadRequestType::T_UPDATE_USER_IDENTITY_INFO";
			case T_GET_USER_IDENTITY_INFO:
				return "ExecutorThreadRequestType::T_GET_USER_IDENTITY_INFO";
			case T_ADD_USER_RECV_ADDR_INFO:
				return "ExecutorThreadRequestType::T_ADD_USER_RECV_ADDR_INFO";
			case T_DEL_USER_RECV_ADDR_INFO:
				return "ExecutorThreadRequestType::T_DEL_USER_RECV_ADDR_INFO";
			case T_UPDATE_USER_RECV_ADDR_INFO:
				return "ExecutorThreadRequestType::T_UPDATE_USER_RECV_ADDR_INFO";
			case T_GET_USER_RECV_ADDR_INFO:
				return "ExecutorThreadRequestType::T_GET_USER_RECV_ADDR_INFO";			

			//ticket
			case T_GET_GOOD_TYPE:
				return "ExecutorThreadRequestType::T_GET_GOOD_TYPE";	
			case T_GET_GOOD_INFO:
				return "ExecutorThreadRequestType::T_GET_GOOD_INFO";	
			case T_ADD_TICKET:
				return "ExecutorThreadRequestType::T_ADD_TICKET";	
			case T_GET_USER_FEED_LIST:
				return "ExecutorThreadRequestType::T_GET_USER_FEED_LIST";	
			case T_GET_FEED_LIST:
				return "ExecutorThreadRequestType::T_GET_FEED_LIST";	
			case T_GET_USER_SHOW_LIST:
				return "ExecutorThreadRequestType::T_GET_USER_SHOW_LIST";	
			case T_GET_SHOW_LIST:
				return "ExecutorThreadRequestType::T_GET_SHOW_LIST";	
			case T_GET_SHOW_DETAIL:
				return "ExecutorThreadRequestType::T_GET_SHOW_DETAIL";	
			case T_ADD_SHOW:
				return "ExecutorThreadRequestType::T_ADD_SHOW";	
			case T_GET_USER_COLLECT_LIST:
				return "ExecutorThreadRequestType::T_GET_USER_COLLECT_LIST";	
			case T_ADD_COLLECT:
				return "ExecutorThreadRequestType::T_ADD_COLLECT";	
			case T_GET_FEED_JOIN_USER_LIST:
				return "ExecutorThreadRequestType::T_GET_FEED_JOIN_USER_LIST";		
			case T_GET_USER_FEED_CONTEND_ID_LIST:
				return "ExecutorThreadRequestType::T_GET_USER_FEED_CONTEND_ID_LIST";	
			case T_GET_BANNER_LIST:
				return "ExecutorThreadRequestType::T_GET_BANNER_LIST";	
			case T_DEL_COLLECT:
				return "ExecutorThreadRequestType::T_DEL_COLLECT";	
			case T_CHECK_USER_COLLECT:
				return "ExecutorThreadRequestType::T_CHECK_USER_COLLECT";	
			case T_GET_USER_LUCK_LIST:
				return "ExecutorThreadRequestType::T_GET_USER_LUCK_LIST";	
			case T_GET_FEED_DETATIL:
				return "ExecutorThreadRequestType::T_GET_FEED_DETATIL";
			case T_GET_USER_TICKET_LIST:
				return "ExecutorThreadRequestType::T_GET_USER_TICKET_LIST";
			

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
            case E_SHOW_REPEATED:
            	return "ExecutorThreadRequestType::E_SHOW_REPEATED";
            case E_UPLOAD_TICKET_LIMIT:
            	return "ExecutorThreadRequestType::E_UPLOAD_TICKET_LIMIT";
            	
            	
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
	
    ExecutorThreadRequestElement(int request_type, int fsm_type, int fsm_id, uint64_t user_id)
    	: m_request_type(request_type)
        , m_fsm_type(fsm_type)
        , m_fsm_id(fsm_id)
        , m_need_reply(false)
        , m_self_user_id(user_id)
    {
    }

	void init(int request_type, int fsm_type, int fsm_id, uint64_t user_id = 0)
    {
		m_request_type = request_type;
		m_fsm_type = fsm_type;
		m_fsm_id = fsm_id;
		m_need_reply = false;
		m_self_user_id = user_id;
    }

    void need_reply()
    {
        m_need_reply = true;
    }

    const std::string ToString() const
    {
        std::ostringstream ss;
        ss<<"{"
          <<"\n  request_type: "<<ExecutorThreadRequestType::type_string(m_request_type)
          <<"\n  need_reply: "<<m_need_reply
          <<"\n  fsm_type: "<<FsmContainer<int>::fsm_type_string(m_fsm_type)
          <<"\n  fsm_id: "<<m_fsm_id
          <<"\n  self_user_id: "<<m_self_user_id;

        if(ExecutorThreadRequestType::T_USER_LOGIN == m_request_type)
        {
        	ss<<"\n  snsapi_base_pre_auth_code: "<<m_snsapi_base_pre_auth_code;
        }          
		
		
		ss<<"\n }";
        return ss.str();
    }

public:
	//necessary
	uint32_t m_request_type;
	int m_fsm_type;
    uint32_t m_fsm_id;
	bool m_need_reply;
	uint64_t m_self_user_id;
	
	//user
	std::string m_snsapi_base_pre_auth_code;
	std::string m_phone;
	std::string m_phone_vc;
	std::string m_self_desc;

	std::set<uint64_t> m_user_id_set;
	::hoosho::msg::s::UserIdentityInfo m_user_identity_info;
	::hoosho::msg::s::UserRecvAddrInfo m_user_recv_addr_info; 

	uint32_t m_set_default_addr;
	uint32_t m_addr_num;

	//feed
	::hoosho::msg::s::TicketInfo m_ticket_info;
	uint64_t m_user_id;
	uint32_t m_offset;
	uint32_t m_limit;

	uint64_t m_good_id;
	uint32_t m_begin_feed_id;

	uint32_t m_begin_show_id;
	::hoosho::msg::s::ShowInfo m_show_info;

	uint64_t m_begin_ts;
	::hoosho::msg::s::CollectInfo m_collect_info;

	uint64_t m_feed_id;
	uint64_t m_begin_ticket_id;

	uint64_t m_show_id;
	std::set<uint64_t> m_feed_id_list;

	uint64_t m_begin_banner_id;
	
};

class ExecutorThreadResponseElement
{
public:
    ExecutorThreadResponseElement()
    {
    }
    
    ExecutorThreadResponseElement(int request_type, int fsm_type, int fsm_id, uint64_t user_id)
        : m_request_type(request_type)
        , m_fsm_type(fsm_type)
        , m_fsm_id(fsm_id)
        , m_self_user_id(user_id)
        , m_result_code(ExecutorThreadRequestType::E_OK)
    {
    }

    const std::string ToString() const
    {
        std::ostringstream ss;
        ss<<"{"
		  <<"\n  request_type: "<<ExecutorThreadRequestType::type_string(m_request_type)
		  <<"\n  fsm_type: "<<FsmContainer<int>::fsm_type_string(m_fsm_type)
          <<"\n  fsm_id: "<<m_fsm_id
          <<"\n  self_user_id:"<<m_self_user_id
          <<"\n  result_code: "<<ExecutorThreadRequestType::err_string(m_result_code)<<"\n";
          
        if(ExecutorThreadRequestType::T_USER_LOGIN == m_request_type)
        {
			ss<<"\n  user_info: "<<m_user_info.Utf8DebugString()
			  <<"\n  auth_code: "<<m_auth_code;
        }
		
        ss<<"}";
        return ss.str();
    }

public:
	//necessary
    uint32_t m_request_type;
    uint32_t m_fsm_type;
    uint32_t m_fsm_id;
	uint64_t m_self_user_id;
    uint32_t m_result_code;

	//reply result
	std::string m_auth_code;	
	::hoosho::msg::s::UserInfo m_user_info;

	std::string m_phone;
	std::string m_phone_vc;

	std::vector<hoosho::msg::s::UserInfo> m_user_info_list;

	::hoosho::msg::s::UserIdentityInfo m_user_identity_info;
	uint32_t m_addr_num;

	std::vector<hoosho::msg::s::UserRecvAddrInfo> m_user_recv_addr_info_list;
	std::vector<hoosho::msg::s::GoodType> m_good_type_list;
	std::vector<hoosho::msg::s::GoodInfo> m_good_info_list;

	uint64_t m_ticket_id;
	std::vector<hoosho::msg::s::FeedInfo> m_feed_info_list;

	std::vector<hoosho::msg::s::ShowInfo> m_show_info_list;
	uint64_t m_show_id;

	std::vector<hoosho::msg::s::CollectInfo> m_collect_info_list;
	std::vector<hoosho::msg::s::ContendInfo> m_contend_info_list;

	::hoosho::msg::s::ShowInfo m_show_info;
	::hoosho::msg::s::FeedInfo m_feed_info;

	std::vector<hoosho::msg::s::FeedContendInfo> m_feed_contend_info_list;
	std::vector<hoosho::msg::s::BannerInfo> m_banner_info_list;
	std::vector<hoosho::msg::s::UserCollectFlag> m_user_collect_flag_list;
	std::vector<hoosho::msg::s::TicketInfo> m_ticket_info_list;
	
};

typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


