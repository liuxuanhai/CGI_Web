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
        T_USERINFO_QUERY,
        T_USERINFO_UPDATE,  //all field changed req data
        T_USERINFO_BATCH_QUERY, 
        T_USERFANS_LIST,
        T_USERFANS_NUM,
        T_USERFOLLOWS_LIST,
        T_FOLLOW,
        T_POWER_QUERY, 
        T_ZOMBIE_QUERY,
        T_ZOMANAGER_QUERY
    };

    enum Error
    {
        E_OK = 0,
        E_NOT_EXIST = 1,
        E_DB_ERR = 2,
        E_FUCK = 3,
    };

    static const std::string type_string(int type)
    {
        switch(type)
        {
            case T_USERINFO_QUERY:
                return "ExecutorThreadRequestType::T_USERINFO_QUERY";

			case T_USERINFO_UPDATE:
				return "ExecutorThreadRequestType::T_USERINFO_UPDATE";

			case T_USERINFO_BATCH_QUERY:
				return "ExecutorThreadRequestType::T_USERINFO_BATCH_QUERY";

			case T_USERFANS_LIST:
				return "ExecutorThreadRequestType::T_USERFANS_LIST";

			case T_USERFANS_NUM:
				return "ExecutorThreadRequestType::T_USERFANS_NUM";

			case T_USERFOLLOWS_LIST:
				return "ExecutorThreadRequestType::T_USERFOLLOWS_LIST";

			case T_FOLLOW:
				return "ExecutorThreadRequestType::T_FOLLOW";

			case T_POWER_QUERY:
				return "ExecutorThreadRequestType::T_POWER_QUERY";

			case T_ZOMBIE_QUERY:
				return "ExecutorThreadRequestType::T_ZOMBIE_QUERY";

			case T_ZOMANAGER_QUERY:
				return "ExecutorThreadRequestType::T_ZOMANAGER_QUERY";
				
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

            case E_DB_ERR:
            	return "ExecutorThreadRequestType::E_DB_ERR";

            case E_FUCK:
            	return "ExecutorThreadRequestType::E_FUCK";

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
			ss<<", user_detail_info:"<<m_user_detail_info_from.Utf8DebugString();
        }
        else if(ExecutorThreadRequestType::T_USERINFO_UPDATE == m_request_type)
        {
			ss<<", user_detail_info:"<<m_user_detail_info_from.Utf8DebugString();
        }
        else if(ExecutorThreadRequestType::T_USERINFO_BATCH_QUERY == m_request_type
        	|| ExecutorThreadRequestType::T_USERFANS_NUM == m_request_type)
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
        else if(ExecutorThreadRequestType::T_USERFANS_LIST == m_request_type)
        {
			ss<<", openid_md5:"<<m_user_detail_info_from.openid_md5();
        }
        else if(ExecutorThreadRequestType::T_POWER_QUERY == m_request_type)
        {
			ss<<", openid_md5:"<<m_openid_md5;
			ss<<", pa_appid_md5:"<<m_pa_appid_md5;
        }
		else if(ExecutorThreadRequestType::T_ZOMBIE_QUERY == m_request_type)
        {
			ss<<", uin:"<<m_uin;
        }
        else if(ExecutorThreadRequestType::T_USERFOLLOWS_LIST == m_request_type)
        {
			ss<<", openid_md5:"<<m_user_detail_info_from.openid_md5();
        }
        else if(ExecutorThreadRequestType::T_FOLLOW == m_request_type)
        {
			ss<<", openid_md5_from: "<<m_user_detail_info_from.openid_md5()
				<<", openid_md5_to: "<<m_user_detail_info_to.openid_md5()
				<<", relation: "<<m_relation_from_to;
			if(1 == m_relation_from_to)
			{
				ss<<"(follow)";
			}
			else if(2 == m_relation_from_to)
			{
				ss<<"(cancel_follow)";
			}
			else
			{
				ss<<"unkown relation";
			}
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
    ::hoosho::commstruct::UserDetailInfo m_user_detail_info_from;
    ::hoosho::commstruct::UserDetailInfo m_user_detail_info_to;
    uint32_t m_relation_from_to; //used in req of "A follow B"
    std::vector<uint64_t> m_openid_md5_list;
    uint64_t m_openid_md5;
    uint64_t m_pa_appid_md5;
	string m_uin;
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
        if(ExecutorThreadRequestType::T_USERINFO_UPDATE== m_request_type)
        {
			ss<<", user_detail_info:"<<m_user_detail_info.Utf8DebugString();
        }
        else if(ExecutorThreadRequestType::T_USERINFO_BATCH_QUERY == m_request_type)
        {
        	ss<<", user_detail_info_list: {";
			for(size_t i=0; i<m_user_detail_info_list.size(); ++i)
			{
				if(i != 0)
				{
					ss<<", ";
				}

				ss<<m_user_detail_info_list[i].Utf8DebugString();
			}
        }
        else if(ExecutorThreadRequestType::T_USERFANS_LIST == m_request_type
        		|| ExecutorThreadRequestType::T_USERFOLLOWS_LIST == m_request_type
        		|| ExecutorThreadRequestType::T_ZOMBIE_QUERY == m_request_type)
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
        else if(ExecutorThreadRequestType::T_POWER_QUERY == m_request_type)
        {
			ss<<", power_list: {";
			for(size_t i=0; i<m_power_list.size(); ++i)
			{
				if(i != 0)
				{
					ss<<", ";
				}

				ss<<m_power_list[i];
			}
			ss<<"}";
        }
        else if(ExecutorThreadRequestType::T_USERFANS_NUM == m_request_type)
        {
			ss<<", fans_num_map: {";
			for(std::map<uint64_t, uint32_t>::const_iterator iter = m_map_openid_md5_2_fans_num.begin()
				; iter != m_map_openid_md5_2_fans_num.end()
				; ++iter)
			{
				if(iter != m_map_openid_md5_2_fans_num.begin())
				{
					ss<<", ";
				}

				ss<<"{"<<(iter->first)<<": "<<(iter->second)<<"}";
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
	::hoosho::commstruct::UserDetailInfo m_user_detail_info;
	std::vector<hoosho::commstruct::UserDetailInfo> m_user_detail_info_list;
	std::vector<uint64_t> m_openid_md5_list;
	std::vector<uint64_t> m_power_list;
	std::map<uint64_t, uint32_t> m_map_openid_md5_2_fans_num;
	std::vector<std::string> m_uin_list;
};

typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


