#ifndef _TEST_SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _TEST_SERVER_EXECUTOR_THREAD_QUEUE_H_

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
    	//user some proto
        T_USER_LOGIN,
        T_USER_GET_PHONE_VC,
        T_USER_SET_PHONE,
        T_USERINFO_UPDATE,
        T_USERINFO_QUERY,
        T_USER_GET_CASH,
		T_USER_RECOMMEND_LIST, 
		T_USER_TRUST, 
		T_USER_TRUST_SB_CHECK,
		T_MY_TRUST_LIST_QUERY, 
		T_TRUST_ME_LIST_QUERY,
		T_USER_ACTIVITY_INFO_UPDATE,
		T_USER_ACTIVITY_INFO_QUERY,
		T_USER_GET_RANKING_LIST,
		T_USER_GET_AVAILABLE_LOTTERY_TIME,
		T_USER_DO_LOTTERY,
		T_USER_GET_LOTTERY_RECORD_LIST,

		//msg some proto
		T_MSG_ADD,
		T_MSG_GET_LIST,
		T_MSG_DEL,
		T_MSG_GET_SESSION_LIST,
		T_MSG_DEL_SESSION,
		T_MSG_GET_NEWMSG_STATUS,

		//red point
		T_REDPOINT_GET,
		T_REDPOINT_UPDATE,

		//notice
		T_NOTICE_ADD,
		T_NOTICE_GET,		

		_________T_I_AM_SPITOR_FOR_USER_ORDER_________,

		//cash some proto
		T_CASH_FLOW_QUERY,
		T_OUTCOME_COMMIT, 
		T_OUTCOME_LIST, 

		//order some proto
		T_ORDER_CREATE, 
		T_ORDER_FETCH,
		T_ORDER_SEARCH, 
		T_ORDER_CANCEL,
		T_ORDER_FINISH,
		T_ORDER_COMMENT,
		T_ORDER_INFO_QUERY,
		T_ORDER_LIST_USER_FETCHED,
		T_ORDER_LIST_USER_CREATED,
		T_ORDER_REWARD,

		
		T_ORDER_FETCH_CANCEL,
		T_ORDER_FETCH_ACCEPT,

		//feeds		
		T_ORDER_FAVOR_CHECK,
		T_ORDER_FAVOR_UPDATE,
		T_ORDER_FAVOR_LIST,
		T_ORDER_FOLLOW_ADD,
		T_ORDER_COMMENT_LIST,
		T_ORDER_COMMENT_DETAIL,
		T_ORDER_FOLLOW_DEL,


		//order payed succ
		T_ORDER_PAY_SUCC,
    };

    enum Error
    {
        E_OK = 0,
        E_WX_ERR = 1,
        E_DB_ERR = 2,
        E_PARAM_ERR = 3,
        E_INNER_ERR = 4,
        E_NOT_PA_FANS = 5,
        E_BAIDU_ERR = 6,
		E_ORDER_CANNOT_FETCH = 7,
		E_ORDER_CANNOT_FETCH_REPEATED = 8,
		E_ORDER_CANNOT_FETCH_CANCEL_AFTER_CHOSEN = 9,
		E_ORDER_CANNOT_FETCH_TOO_MANY_WAITING = 10,
		E_ORDER_CANNOT_CANCEL_WHILE_WAITING_EXISTS = 11,
		E_AVAILABLE_LOTTERY_TIME_NOT_ENOUGH = 12,
    };

    static const std::string type_string(int type)
    {
        switch(type)
        {
        	//user
        	case T_USER_LOGIN:
				return "ExecutorThreadRequestType::T_USER_LOGIN";

			case T_USER_GET_PHONE_VC:
				return "ExecutorThreadRequestType::T_USER_GET_PHONE_VC";

			case T_USER_SET_PHONE:
				return "ExecutorThreadRequestType::T_USER_SET_PHONE";
				
            case T_USERINFO_QUERY:
                return "ExecutorThreadRequestType::T_USERINFO_QUERY";

			case T_USERINFO_UPDATE:
				return "ExecutorThreadRequestType::T_USERINFO_UPDATE";

			case T_USER_GET_CASH:
				return "ExecutorThreadRequestType::T_USER_GET_CASH";

			case T_USER_RECOMMEND_LIST:
				return "ExecutorThreadRequestType::T_USER_RECOMMEND_LIST";
			
			case T_USER_TRUST:
				return "ExecutorThreadRequestType::T_USER_TRUST";

			case T_USER_TRUST_SB_CHECK:
				return "ExecutorThreadRequestType::T_USER_TRUST_SB_CHECK";
				
			case T_MY_TRUST_LIST_QUERY:
				return "ExecutorThreadRequestType::T_MY_TRUST_LIST_QUERY";
				
			case T_TRUST_ME_LIST_QUERY:
				return "ExecutorThreadRequestType::T_TRUST_ME_LIST_QUERY";
				
			case T_USER_ACTIVITY_INFO_UPDATE:
				return "ExecutorThreadRequestType::T_USER_ACTIVITY_INFO_UPDATE";
			
			case T_USER_ACTIVITY_INFO_QUERY:
				return "ExecutorThreadRequestType::T_USER_ACTIVITY_INFO_QUERY";

			case T_USER_GET_AVAILABLE_LOTTERY_TIME:
				return "ExecutorThreadRequestType::T_USER_GET_AVAILABLE_LOTTERY_TIME";

			case T_USER_DO_LOTTERY:
				return "ExecutorThreadRequestType::T_USER_DO_LOTTERY";

			case T_USER_GET_LOTTERY_RECORD_LIST:
				return "ExecutorThreadRequestType::T_USER_GET_LOTTERY_RECORD_LIST";

			case T_USER_GET_RANKING_LIST:
				return "ExecutorThreadRequestType::T_USER_GET_RANKING_LIST";

			//msg
			case T_MSG_ADD:
				return "ExecutorThreadRequestType::T_MSG_ADD";

			case T_MSG_GET_LIST:
                return "ExecutorThreadRequestType::T_MSG_GET_LIST";

			case T_MSG_DEL:
				return "ExecutorThreadRequestType::T_MSG_DEL";

			case T_MSG_GET_SESSION_LIST:
				return "ExecutorThreadRequestType::T_MSG_GET_SESSION_LIST";

			case T_MSG_DEL_SESSION:
				return "ExecutorThreadRequestType::T_MSG_DEL_SESSION";
			
			case T_MSG_GET_NEWMSG_STATUS:
				return "ExecutorThreadRequestType::T_MSG_GET_NEWMSG_STATUS";

			case T_REDPOINT_GET:
				return "ExecutorThreadRequestType::T_REDPOINT_GET";
				
			case T_REDPOINT_UPDATE:
				return "ExecutorThreadRequestType::T_REDPOINT_UPDATE";
				
			case T_NOTICE_ADD:
				return "ExecutorThreadRequestType::T_NOTICE_ADD";

			case T_NOTICE_GET:
				return "ExecutorThreadRequestType::T_NOTICE_GET";
				
			
			//cash
			case T_CASH_FLOW_QUERY:
				return "ExecutorThreadRequestType::T_CASH_FLOW_QUERY";

			case T_OUTCOME_COMMIT:
				return "ExecutorThreadRequestType::T_OUTCOME_COMMIT";

			case T_OUTCOME_LIST:
				return "ExecutorThreadRequestType::T_OUTCOME_LIST";


			//order
			case T_ORDER_CREATE:
				return "ExecutorThreadRequestType::T_ORDER_CREATE";

			case T_ORDER_FETCH:
				return "ExecutorThreadRequestType::T_ORDER_FETCH";

			case T_ORDER_SEARCH:
				return "ExecutorThreadRequestType::T_ORDER_SEARCH";

			case T_ORDER_CANCEL:
				return "ExecutorThreadRequestType::T_ORDER_CANCEL";

			case T_ORDER_FINISH:
				return "ExecutorThreadRequestType::T_ORDER_FINISH";

			case T_ORDER_COMMENT:
				return "ExecutorThreadRequestType::T_ORDER_COMMENT";

			case T_ORDER_INFO_QUERY:
				return "ExecutorThreadRequestType::T_ORDER_INFO_QUERY";

			case T_ORDER_LIST_USER_FETCHED:
				return "ExecutorThreadRequestType::T_ORDER_LIST_USER_FETCHED";

			case T_ORDER_LIST_USER_CREATED:
				return "ExecutorThreadRequestType::T_ORDER_LIST_USER_CREATED";

			case T_ORDER_PAY_SUCC:
				return "ExecutorThreadRequestType::T_ORDER_PAY_SUCC";
					
			case T_ORDER_REWARD:
				return "ExecutorThreadRequestType::T_ORDER_REWARD";

			case T_ORDER_FAVOR_CHECK:
				return "ExecutorThreadRequestType::T_ORDER_FAVOR_CHECK";

			case T_ORDER_FAVOR_UPDATE:
				return "ExecutorThreadRequestType::T_ORDER_FAVOR_UPDATE";

			case T_ORDER_FAVOR_LIST:
				return "ExecutorThreadRequestType::T_ORDER_FAVOR_LIST";

			case T_ORDER_FOLLOW_ADD:
				return "ExecutorThreadRequestType::T_ORDER_FOLLOW_ADD";

			case T_ORDER_COMMENT_LIST:
				return "ExecutorThreadRequestType::T_ORDER_COMMENT_LIST";

			case T_ORDER_COMMENT_DETAIL:
				return "ExecutorThreadRequestType::T_ORDER_COMMENT_DETAIL";

			case T_ORDER_FOLLOW_DEL:
				return "ExecutorThreadRequestType::T_ORDER_FOLLOW_DEL";

			case T_ORDER_FETCH_ACCEPT:
				return "ExecutorThreadRequestType::T_ORDER_FETCH_ACCEPT";

			case T_ORDER_FETCH_CANCEL:
				return "ExecutorThreadRequestType::T_ORDER_FETCH_CANCEL";	


			//msg

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

			case E_BAIDU_ERR:
				return "ExecutorThreadRequestType::E_BAIDU_ERR";

			case E_ORDER_CANNOT_FETCH:
				return "ExecutorThreadRequestType::E_ORDER_CANNOT_FETCH";

			case E_ORDER_CANNOT_FETCH_REPEATED:
				return "ExecutorThreadRequestType::E_ORDER_CANNOT_FETCH_REPEATED";

			case E_ORDER_CANNOT_FETCH_TOO_MANY_WAITING:
				return "ExecutorThreadRequestType::E_ORDER_CANNOT_FETCH_TOO_MANY_WAITING";

			case E_ORDER_CANNOT_CANCEL_WHILE_WAITING_EXISTS:
				return "ExecutorThreadRequestType::E_ORDER_CANNOT_CANCEL_WHILE_WAITING_EXISTS";

			case E_AVAILABLE_LOTTERY_TIME_NOT_ENOUGH:
				return "ExecutorThreadRequestType::E_AVAILABLE_LOTTERY_TIME_NOT_ENOUGH";

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
	
    ExecutorThreadRequestElement(int request_type, int fsm_type, int fsm_id, const std::string& self_openid)
    	: m_request_type(request_type)
        , m_fsm_type(fsm_type)
        , m_fsm_id(fsm_id)
        , m_need_reply(false)
        , m_self_openid(self_openid)
    {
    }

	void init(int request_type, int fsm_type, int fsm_id, const std::string& self_openid)
    {
		m_request_type = request_type;
		m_fsm_type = fsm_type;
		m_fsm_id = fsm_id;
		m_need_reply = false;
		m_self_openid = self_openid;
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
          <<"\n  self_openid: "<<m_self_openid;

        if(ExecutorThreadRequestType::T_USER_LOGIN == m_request_type)
        {
        	ss<<"\n  snsapi_base_pre_auth_code: "<<m_snsapi_base_pre_auth_code;
        }  
        else if(ExecutorThreadRequestType::T_USER_GET_PHONE_VC == m_request_type)
        {
			ss<<"\n  phone: "<<m_phone;
        }
        else if(ExecutorThreadRequestType::T_USER_SET_PHONE == m_request_type)
        {
			ss<<"\n  phone: "<<m_phone;			 
        }        
        else if(ExecutorThreadRequestType::T_USERINFO_QUERY == m_request_type
        		|| ExecutorThreadRequestType::T_USER_TRUST_SB_CHECK == m_request_type)
        {
			ss<<"\n  openid_md5_list: {";
			for(std::set<std::string>::iterator it = m_openid_set.begin(); it != m_openid_set.end(); ++it)
			{
				ss<<"\n    "<<*it;				
			}
			ss<<"\n  }";
        }
        else if(ExecutorThreadRequestType::T_USERINFO_UPDATE == m_request_type)
        {
			ss<<"\n  user_info:"<<m_user_info.Utf8DebugString();
        }
		else if(ExecutorThreadRequestType::T_USER_RECOMMEND_LIST == m_request_type)
		{
			ss<<"\n  offset_openid: "<<m_third_openid
			  <<"\n  len: "<<m_len;
		}
		else if(ExecutorThreadRequestType::T_USER_TRUST == m_request_type)
		{
			ss<<"\n  openid: "<<m_third_openid
			  <<"\n  opt: "<<m_trust_opt;
		}		
		else if(ExecutorThreadRequestType::T_MY_TRUST_LIST_QUERY == m_request_type
			|| ExecutorThreadRequestType::T_TRUST_ME_LIST_QUERY == m_request_type)
		{
			ss<<"\n  third_openid: "<<m_third_openid
			  <<"\n  offset_openid: "<<m_offset_openid
			  <<"\n  len: "<<m_len;
		}			
		else if(ExecutorThreadRequestType::T_USER_ACTIVITY_INFO_UPDATE == m_request_type
			|| ExecutorThreadRequestType::T_USER_ACTIVITY_INFO_QUERY == m_request_type)
		{
			ss<<"\n activity_info: "<<m_user_activity_info.Utf8DebugString();
		}
		else if(ExecutorThreadRequestType::T_CASH_FLOW_QUERY == m_request_type)
		{
			ss<<"\n  offset: "<<m_offset
			  <<"\n  len: "<<m_len;
		}
		else if(ExecutorThreadRequestType::T_OUTCOME_COMMIT == m_request_type)
		{
			ss<<"\n amount: "<<m_amount;
		}
		else if(ExecutorThreadRequestType::T_OUTCOME_LIST == m_request_type)
		{
			ss<<"\n outcome_state: "<<m_outcome_state
			  <<"\n  offset: "<<m_offset
			  <<"\n  len: "<<m_len;
		}
		else if(ExecutorThreadRequestType::T_ORDER_CREATE == m_request_type)
		{
			ss<<"\n order_info: "<<m_order_info.Utf8DebugString()
				<<"\n user_ip: "<<m_user_ip
				<<"\n media_server_id: "<<m_media_server_id;
		}
		else if(ExecutorThreadRequestType::T_ORDER_PAY_SUCC == m_request_type)
		{
			ss<<"\n wx_pay_callback_xml: "<<m_wx_pay_callback_xml;
		}
		else if(ExecutorThreadRequestType::T_ORDER_FETCH == m_request_type
			|| ExecutorThreadRequestType::T_ORDER_FINISH == m_request_type
			|| ExecutorThreadRequestType::T_ORDER_CANCEL == m_request_type)
		{
			ss<<"\n order_id: "<<m_order_id;
		}
		else if(ExecutorThreadRequestType::T_ORDER_COMMENT == m_request_type)
		{
			ss<<"\n order_id: "<<m_order_id
				<<"\n star: "<<m_order_star;
		}
		else if(ExecutorThreadRequestType::T_ORDER_SEARCH == m_request_type)
		{
			ss<<"\n longi: "<<m_longitude
				<<"\n lati: "<<m_latitude;
		}
		else if(ExecutorThreadRequestType::T_ORDER_INFO_QUERY == m_request_type)
		{
			ss<<"\n  order_id_list: {";
			for(size_t i=0; i<m_order_id_list.size(); ++i)
			{
				ss<<"\n    "<<m_order_id_list[i];				
			}
			ss<<"\n  }";
		}
		else if(ExecutorThreadRequestType::T_ORDER_LIST_USER_FETCHED == m_request_type
				|| ExecutorThreadRequestType::T_ORDER_LIST_USER_CREATED == m_request_type)
		{	
			ss<<"\n third_openid: "<<m_third_openid
			  <<"\n  offset: "<<m_offset
			  <<"\n  len: "<<m_len;
		}
		else if(ExecutorThreadRequestType::T_ORDER_REWARD == m_request_type)
		{
			ss<<"\n amount: "<< m_amount
			  <<"\n orderid: "<< m_order_id;
		}
		else if(ExecutorThreadRequestType::T_ORDER_FAVOR_CHECK == m_request_type)
		{
			ss<<"\n orderid_list: {";
			for(size_t i = 0; i < m_order_id_list.size(); i++)
			{
				ss<<"\n "<<m_order_id_list[i];
			}
			ss<<"\n }";
		}
		else if(ExecutorThreadRequestType::T_ORDER_FAVOR_UPDATE == m_request_type)
		{
			ss<<"\n orderid: "<<m_order_id
			  <<"\n oper: "<<m_oper;
		}
		else if(ExecutorThreadRequestType::T_ORDER_FAVOR_LIST == m_request_type)
		{
			ss<<"\n orderid: "<<m_order_id
			  <<"\n begin_ts: "<<m_begin_ts
			  <<"\n len: "<<m_len;
		}
		else if(ExecutorThreadRequestType::T_ORDER_FOLLOW_ADD == m_request_type)
		{
			ss<<"\n follow_info: " << m_order_follow_info.Utf8DebugString();
		}
		else if(ExecutorThreadRequestType::T_ORDER_COMMENT_LIST == m_request_type)
		{
			ss<<"\n orderid: "<<m_order_id
			  <<"\n begin_comment_id: "<<m_begin_comment_id
			  <<"\n len: "<<m_len;
		}
		else if(ExecutorThreadRequestType::T_ORDER_COMMENT_DETAIL == m_request_type)
		{
			ss<<"\n order_comment_id_list: ";
			for(size_t i = 0; i < m_origin_comment_id_list.size(); i++)
			{
				ss<<", "<<m_origin_comment_id_list[i];
			}
		}
		else if(ExecutorThreadRequestType::T_ORDER_FOLLOW_DEL == m_request_type)
		{
			ss<<"\n orderid: "<<m_order_id;
		}
		else if(ExecutorThreadRequestType::T_ORDER_FETCH_ACCEPT == m_request_type)
		{
			ss<<"\n orderid: "<<m_order_id
				<<"\n slaveid: "<<m_slave_openid;
		}
		else if(ExecutorThreadRequestType::T_ORDER_FETCH_CANCEL == m_request_type)
		{
			ss<<"\n orderid: "<<m_order_id;
		}
		else if(ExecutorThreadRequestType::T_MSG_ADD == m_request_type)
		{
			ss<<"\n  msgcontent: { \n"
			  <<m_msg_content.Utf8DebugString()
			  <<"\n  }";
		}
		else if(ExecutorThreadRequestType::T_MSG_GET_LIST == m_request_type)
		{
			ss<<"\n  begin_msg_id: "<<m_begin_msg_id
			  <<"\n  len: "<<m_len
			  <<"\n  openid_to: "<<m_msg_openid_to;
		}
		else if(ExecutorThreadRequestType::T_MSG_DEL == m_request_type)
		{
			
		}
		else if(ExecutorThreadRequestType::T_MSG_GET_SESSION_LIST == m_request_type)
		{
			ss<<"\n  begin_ts: "<<m_begin_ts
			  <<"\n  len: "<<m_len;
		}
		else if(ExecutorThreadRequestType::T_MSG_DEL_SESSION == m_request_type)
		{
			ss<<"\n  openid_to: "<<m_msg_openid_to;
		}		
		else if(ExecutorThreadRequestType::T_MSG_GET_NEWMSG_STATUS == m_request_type)
		{

		}
		else if(ExecutorThreadRequestType::T_REDPOINT_GET == m_request_type)
		{

		}
		else if(ExecutorThreadRequestType::T_REDPOINT_UPDATE == m_request_type)
		{
			ss<<"\n  redpoint_type: "<<m_redpoint_type
			  <<"\n  redpoint_opt: "<<m_redpoint_opt;
		}
		else if(ExecutorThreadRequestType::T_NOTICE_ADD == m_request_type)
		{
			ss<<"\n  notice_info: "<<m_notice_info.Utf8DebugString();
		}
		else if(ExecutorThreadRequestType::T_NOTICE_GET == m_request_type)
		{
			ss<<"\n  notice_type: "<<m_notice_type
			  <<"\n  begin_ts: "<<m_begin_ts
			  <<"\n  len: "<<m_len;
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
	std::string m_self_openid;
	
	//req param
	std::string m_phone;	
    ::hoosho::msg::z::UserInfo m_user_info;
	::hoosho::msg::z::OrderInfo m_order_info;
	::hoosho::msg::z::OrderFollowInfo m_order_follow_info;
	::hoosho::msg::z::UserActivityInfo m_user_activity_info;
    std::set<std::string> m_openid_set;
	std::vector<uint64_t> m_order_id_list;
	std::vector<uint64_t> m_origin_comment_id_list;

	std::string m_user_ip;
	std::string m_media_server_id;
	std::string m_snsapi_base_pre_auth_code;
	uint32_t m_amount;
	uint16_t m_outcome_state;
	uint64_t m_order_id;
	uint16_t m_order_star;
	uint64_t m_longitude;
	uint64_t m_latitude;
	uint64_t m_offset;
	uint32_t m_len;
	uint32_t m_oper;
	uint64_t m_begin_ts;
	uint64_t m_begin_comment_id;
	uint64_t m_follow_id;
	
	std::string m_wx_pay_callback_xml; 
	std::string m_third_openid; //use for query ThirdTrustlist
	std::string m_offset_openid;
	std::string m_extra_data;
	uint32_t m_trust_opt;
	std::string m_slave_openid;

	//msg
	::hoosho::msg::z::MsgContent m_msg_content;
	std::string m_msg_openid_to;
	uint64_t m_begin_msg_id;	

	//redpoint
	uint32_t m_redpoint_type;
	uint32_t m_redpoint_opt;

	//notice
	::hoosho::msg::z::NoticeInfo m_notice_info;
	uint32_t m_notice_type;
	uint32_t m_notice_table_id;

	//ranking
	uint32_t m_ranking_type;
	uint32_t m_ranking_page;
	
};

class ExecutorThreadResponseElement
{
public:
    ExecutorThreadResponseElement()
    {
    }
    
    ExecutorThreadResponseElement(int request_type, int fsm_type, int fsm_id, const std::string& self_openid)
        : m_request_type(request_type)
        , m_fsm_type(fsm_type)
        , m_fsm_id(fsm_id)
        , m_self_openid(self_openid)
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
          <<"\n  self_openid:"<<m_self_openid
          <<"\n  result_code: "<<ExecutorThreadRequestType::err_string(m_result_code)<<"\n";
          
        if(ExecutorThreadRequestType::T_USER_LOGIN == m_request_type)
        {
			ss<<"\n  user_info: "<<m_user_info.Utf8DebugString()
			  <<"\n  auth_code: "<<m_auth_code;
        }
		else if(ExecutorThreadRequestType::T_USERINFO_UPDATE == m_request_type)
        {
			ss<<"\n  user_info.self_desc: "<<m_user_info.self_desc();
        }
        else if(ExecutorThreadRequestType::T_USER_GET_PHONE_VC == m_request_type)
        {
			ss<<"\n  phone: "<<m_phone
			  <<"\n  phone_vc: "<<m_phone_vc;
        }
        else if(ExecutorThreadRequestType::T_USER_GET_CASH == m_request_type)
        {
			ss<<"\n  balance: "<<m_balance
			  <<"\n  pending: "<<m_pending;
        }
        else if(ExecutorThreadRequestType::T_USER_TRUST_SB_CHECK == m_request_type)
		{
			ss<<"\n  user_trust_sb_status_list: {";
			for(size_t i=0; i<m_user_trust_sb_status_list.size(); ++i)
			{
				ss<<"\n    "<<m_user_trust_sb_status_list[i].Utf8DebugString();				
			}
			
			ss<<"\n  }";			  
		}
		else if(ExecutorThreadRequestType::T_USER_ACTIVITY_INFO_UPDATE == m_request_type)
		{
		}
		else if(ExecutorThreadRequestType::T_USER_ACTIVITY_INFO_QUERY == m_request_type)
		{
			ss<<"\n activity_info: "<<m_user_activity_info.Utf8DebugString();
		}
        else if(ExecutorThreadRequestType::T_USERINFO_QUERY == m_request_type
			|| ExecutorThreadRequestType::T_USER_RECOMMEND_LIST == m_request_type
			|| ExecutorThreadRequestType::T_MY_TRUST_LIST_QUERY == m_request_type
			|| ExecutorThreadRequestType::T_TRUST_ME_LIST_QUERY == m_request_type)
        {
        	ss<<"\n  user_info_list: {";
			for(size_t i=0; i<m_user_info_list.size(); ++i)
			{
				ss<<"\n    "<<m_user_info_list[i].Utf8DebugString();				
			}
			
			ss<<"\n  }";
        }
		else if(ExecutorThreadRequestType::T_CASH_FLOW_QUERY == m_request_type)
        {
			ss<<"\n  cash_flow_info_list: {";
			for(size_t i=0; i<m_cash_flow_list.size(); ++i)
			{
				ss<<"\n    "<<m_cash_flow_list[i].Utf8DebugString();				
			}
			
			ss<<"\n  }";
        }
		else if(ExecutorThreadRequestType::T_OUTCOME_LIST == m_request_type)
        {
			ss<<"\n  outcome_req_info_list: {";
			for(size_t i=0; i<m_outcome_req_list.size(); ++i)
			{
				ss<<"\n    "<<m_outcome_req_list[i].Utf8DebugString();				
			}
			
			ss<<"\n  }";
        }
		else if(ExecutorThreadRequestType::T_ORDER_CREATE == m_request_type)
        {
			ss<<"\n  order_info: "<<m_order_info.Utf8DebugString()
			  <<"\n  const_type: "<<m_cost_type
			  <<"\n  wx_unified_order_param: "<<m_wx_unified_order_param.Utf8DebugString();
        }
		else if(ExecutorThreadRequestType::T_ORDER_SEARCH == m_request_type
			|| ExecutorThreadRequestType::T_ORDER_INFO_QUERY == m_request_type
			|| ExecutorThreadRequestType::T_ORDER_LIST_USER_CREATED == m_request_type)
        {
			ss<<"\n  order_info_list: {";
			for(size_t i=0; i<m_order_info_list.size(); ++i)
			{
				ss<<"\n    "<<m_order_info_list[i].Utf8DebugString();				
			}
			
			ss<<"\n  }";
        }
		else if(ExecutorThreadRequestType::T_ORDER_LIST_USER_FETCHED == m_request_type)
		{
			ss<<"\n order_fetch_state_list:{";
			for(size_t i=0; i<m_order_openid_fetch_state_list.size(); ++i)
			{
				ss<<"\n    "<<m_order_openid_fetch_state_list[i].Utf8DebugString();				
			}
			ss<<"\n }";
			
			ss<<"\n  order_info_list: {";
			for(size_t i=0; i<m_order_info_list.size(); ++i)
			{
				ss<<"\n    "<<m_order_info_list[i].Utf8DebugString();				
			}
			
			ss<<"\n  }";
		}
		else if(ExecutorThreadRequestType::T_ORDER_REWARD == m_request_type)
		{
			ss<<"\n  const_type: "<<m_cost_type
			  <<"\n  wx_unified_order_param: "<<m_wx_unified_order_param.Utf8DebugString();
		}
		else if(ExecutorThreadRequestType::T_ORDER_FAVOR_CHECK == m_request_type)
		{
			ss<<"\n favor_list: {";
			for(size_t i = 0; i < m_order_favor_info_list.size(); i++)
			{
				ss<<"\n "<<m_order_favor_info_list[i].Utf8DebugString();
			}
			ss<<"\n }";
		}
		else if(ExecutorThreadRequestType::T_ORDER_FAVOR_UPDATE == m_request_type)
		{

		}
		else if(ExecutorThreadRequestType::T_ORDER_FAVOR_LIST == m_request_type)
		{
			ss<<"\n order_favor_list{ ";
			for(size_t i = 0; i < m_order_favor_info_list.size(); i++)
			{
				ss<<"\n"<<m_order_favor_info_list[i].Utf8DebugString();
			}
			ss<<"\n }";
		}
		else if(ExecutorThreadRequestType::T_ORDER_FOLLOW_ADD == m_request_type)
		{
			ss<<"\n order_follow_info: "<<m_order_follow_info.Utf8DebugString();
		}
		else if(ExecutorThreadRequestType::T_ORDER_COMMENT_LIST == m_request_type)
		{
			ss<<"\n order_comment_list{ ";
			for(size_t i = 0; i < m_order_comment_info_list.size(); i++)
			{
				ss<<"\n "<<m_order_comment_info_list[i].Utf8DebugString();
			}
			ss<<"\n }";
		}
		else if(ExecutorThreadRequestType::T_ORDER_COMMENT_DETAIL == m_request_type)
		{
			ss<<"\n order_comment_list{ ";
			for(size_t i = 0; i < m_order_comment_info_list.size(); i++)
			{
				ss<<"\n "<<m_order_comment_info_list[i].Utf8DebugString();
			}
			ss<<"\n }";
		}
		else if(ExecutorThreadRequestType::T_ORDER_FOLLOW_DEL == m_request_type)
		{

		}		
		else if(ExecutorThreadRequestType::T_MSG_ADD == m_request_type)
		{
			ss<<"\n  msg_id: "<<m_msg_id
			  <<"\n  create_ts: "<<m_msg_create_ts;
		}
		else if(ExecutorThreadRequestType::T_MSG_GET_LIST == m_request_type)
		{
			ss<<"\n  msg_content_list{ ";
			for(size_t i = 0; i < m_msg_content_list.size(); i++)
			{
				ss<<"\n  "<<m_msg_content_list[i].Utf8DebugString();
			}
			ss<<"\n  }";
		}
		else if(ExecutorThreadRequestType::T_MSG_DEL == m_request_type)
		{

		}
		else if(ExecutorThreadRequestType::T_MSG_GET_SESSION_LIST == m_request_type)
		{
			ss<<"\n  session_list{ ";
			for(size_t i = 0; i < m_msg_session_list.size(); i++)
			{
				ss<<"\n  "<<m_msg_session_list[i].Utf8DebugString();
			}
			ss<<"\n  }";
		}
		else if(ExecutorThreadRequestType::T_MSG_DEL_SESSION == m_request_type)
		{

		}
		else if(ExecutorThreadRequestType::T_MSG_GET_NEWMSG_STATUS == m_request_type)
		{
			ss<<"\n  newmsg_status: "<<m_newmsg_status;
		}
		else if(ExecutorThreadRequestType::T_REDPOINT_GET == m_request_type)
		{
			ss<<"\n  redpoint_info_list{ ";
			for(size_t i = 0; i < m_red_point_info_list.size(); i++)
			{
				ss<<"\n  "<<m_red_point_info_list[i].Utf8DebugString();
			}
			ss<<"\n  }";
		}
		else if(ExecutorThreadRequestType::T_REDPOINT_UPDATE == m_request_type)
		{

		}
		else if(ExecutorThreadRequestType::T_NOTICE_ADD == m_request_type)
		{

		}
		else if(ExecutorThreadRequestType::T_NOTICE_GET == m_request_type)
		{
			ss<<"\n  notice_info_list{ ";
			for(size_t i = 0; i < m_notice_info_list.size(); i++)
			{
				ss<<"\n  "<<m_notice_info_list[i].Utf8DebugString();
			}
			ss<<"\n  }";
		}

        ss<<"}";
        return ss.str();
    }

public:
	//necessary
    uint32_t m_request_type;
    uint32_t m_fsm_type;
    uint32_t m_fsm_id;
	std::string m_self_openid;
    uint32_t m_result_code;

	//reply result
	std::string m_auth_code;
	::hoosho::msg::z::UserInfo m_user_info;
	std::vector<hoosho::msg::z::UserInfo> m_user_info_list;
	std::vector<hoosho::msg::z::CashFlowInfo> m_cash_flow_list;
	std::vector<hoosho::msg::z::OutcomeReqInfo> m_outcome_req_list;
	std::vector<hoosho::msg::z::OrderInfo> m_order_info_list;
	std::vector<hoosho::msg::z::OrderOpenidFetchState>  m_order_openid_fetch_state_list;
	std::vector<hoosho::msg::z::UserExtraInfo> m_user_extra_info_list;
	std::vector<hoosho::msg::z::UserTrustSBStatus> m_user_trust_sb_status_list;
	std::vector<hoosho::msg::z::OrderFavorInfo> m_order_favor_info_list;
	std::vector<hoosho::msg::z::OrderCommentInfo> m_order_comment_info_list;
	hoosho::msg::z::OrderInfo m_order_info;
	hoosho::msg::z::OrderFollowInfo m_order_follow_info;
	uint16_t m_cost_type;
	hoosho::msg::z::WXUnifiedOrderParam m_wx_unified_order_param;
	::hoosho::msg::z::UserActivityInfo m_user_activity_info;
	uint64_t m_reward_fee;

	std::string m_phone;
	std::string m_phone_vc;

	uint64_t m_follow_id;

	uint64_t m_balance;
	uint64_t m_pending;

	//msg
	uint64_t m_msg_id;
	uint64_t m_msg_create_ts;
	uint32_t m_newmsg_status;
	std::vector<hoosho::msg::z::MsgContent> m_msg_content_list;
	std::vector<hoosho::msg::z::Session> m_msg_session_list;

	//redpoint
	std::vector<hoosho::msg::z::RedPointInfo> m_red_point_info_list;

	//notice
	std::vector<hoosho::msg::z::NoticeInfo> m_notice_info_list;
	std::vector<hoosho::msg::z::OrderFollowInfo> m_order_follow_info_list;

	//lottery
	uint32_t m_available_lottery_times;
	uint32_t m_prize;
	std::string m_prize_desc;
	std::vector<hoosho::msg::z::LotteryInfo> m_lottery_info_list;
	
};

typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


