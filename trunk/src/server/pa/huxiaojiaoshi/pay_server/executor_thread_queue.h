#ifndef _SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _SERVER_EXECUTOR_THREAD_QUEUE_H_

#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include "memory/duplex_queue.h"
#include "msg.pb.h"
#include "pay_order.h"
#include "pay_common_util.h"

using namespace std;

    class ExecutorThreadRequestType
    {
    public:
        enum Type
        {			
			T_PAY_UNIFIEDORDER_REQ,
			T_PAY_TRANSFER_REQ,
			T_PAY_SENDREDPACK_REQ,
			T_PAY_GET_USER_CASH_REQ,
			T_PAY_QUERY_CASH_FLOW_REQ,
			T_PAY_UNIFIEDORDER_CALLBACK_REQ,
			T_PAY_UNIFIEDORDER_QUERY_REQ,
			T_PAY_LISTEN_INCOME_REQ,
			T_PAY_OUTCOME_QUERY_REQ,
			T_PAY_OUTCOME_COMMIT_REQ,
			T_PAY_OUTCOME_CHECK_REQ,
			T_PAY_OUTCOME_UPDATE_REQ,
        };

        enum Error
        {
            E_OK = 0,
            E_FAIL = 1,
            E_DB_ERR = 2,
            E_DB_NO_EXIST = 3,
            E_BALANCE_NOT_ENOUGH = 4,       
            E_PAY_FAIL = 5,
            E_REPEAT_LISTEN_INCOME = 6,
        };

        static const std::string type_string(int type)
        {
            switch(type)
            {
				case T_PAY_UNIFIEDORDER_REQ:
					return "ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_REQ";		
				case T_PAY_TRANSFER_REQ:
					return "ExecutorThreadRequestType::T_PAY_TRANSFER_REQ";
				case T_PAY_SENDREDPACK_REQ:
					return "ExecutorThreadRequestType::T_PAY_SENDREDPACK_REQ";
				case T_PAY_GET_USER_CASH_REQ:
					return "ExecutorThreadRequestType::T_PAY_GET_USER_CASH_REQ";
				case T_PAY_QUERY_CASH_FLOW_REQ:
					return "ExecutorThreadRequestType::T_PAY_QUERY_CASH_FLOW_REQ";
				case T_PAY_UNIFIEDORDER_CALLBACK_REQ:
					return "ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_CALLBACK_REQ";
				case T_PAY_UNIFIEDORDER_QUERY_REQ:
					return "ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_QUERY_REQ";
				case T_PAY_LISTEN_INCOME_REQ:
					return "ExecutorThreadRequestType::T_PAY_LISTEN_INCOME_REQ";
				case T_PAY_OUTCOME_QUERY_REQ:
					return "ExecutorThreadRequestType::T_PAY_OUTCOME_QUERY_REQ";
				case T_PAY_OUTCOME_COMMIT_REQ:
					return "ExecutorThreadRequestType::T_PAY_OUTCOME_COMMIT_REQ";
				case T_PAY_OUTCOME_CHECK_REQ:
					return "ExecutorThreadRequestType::T_PAY_OUTCOME_CHECK_REQ";
				case T_PAY_OUTCOME_UPDATE_REQ:
					return "ExecutorThreadRequestType::T_PAY_OUTCOME_UPDATE_REQ";

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
				case E_DB_ERR:
					return "ExecutorThreadRequestType::E_DB_ERR";
				case E_DB_NO_EXIST:
					return "ExecutorThreadRequestType::E_DB_NO_EXIST";
				case E_BALANCE_NOT_ENOUGH:
					return "ExecutorThreadRequestType::E_BALANCE_NOT_ENOUGH";
				case E_PAY_FAIL:
					return "ExecutorThreadRequestType::E_PAY_FAIL";
				case E_REPEAT_LISTEN_INCOME:
					return "ExecutorThreadRequestType::E_REPEAT_LISTEN_INCOME";

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
              <<"\n  request_type: "<<ExecutorThreadRequestType::type_string(m_request_type)
              <<"\n	 fsm_id: "<<m_fsm_id
              <<"\n  need_reply: "<<m_need_reply<<"\n";

			if(ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_REQ == m_request_type)
			{
				ss<<"\n  openid: "<<m_openid
				  <<"\n  business_type: "<<m_business_type
				  <<"\n  business_id: "<<m_business_id
				  <<"\n  client_ip: "<<m_client_ip;
			}		
			else if(ExecutorThreadRequestType::T_PAY_TRANSFER_REQ == m_request_type)
			{
				ss<<"\n  openid: "<<m_openid
				  <<"\n  amount: "<<m_amount
				  <<"\n  client_ip: "<<m_client_ip;
			}
			else if(ExecutorThreadRequestType::T_PAY_SENDREDPACK_REQ == m_request_type)
			{
				ss<<"\n  openid: "<<m_openid
				  <<"\n  amount: "<<m_amount
				  <<"\n  client_ip: "<<m_client_ip
				  <<"\n  table_id: "<<m_table_id;
			}
			else if(ExecutorThreadRequestType::T_PAY_GET_USER_CASH_REQ == m_request_type)
			{
				ss<<"\n  openid: "<<m_openid;
			}
			else if(ExecutorThreadRequestType::T_PAY_QUERY_CASH_FLOW_REQ == m_request_type)
			{
				ss<<"\n  openid: "<<m_openid
				  <<"\n  begin_ts: "<<m_begin_ts
				  <<"\n  limit: "<<m_limit;
			}
			else if(ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_CALLBACK_REQ == m_request_type)
			{
				ss<<"\n  client_ip: "<<m_client_ip
				  <<"\n  callback_xml: "<<m_callback_xml;
			}
			else if(ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_QUERY_REQ == m_request_type)
			{
				ss<<"\n  openid: "<<m_openid;
				for(size_t i=0; i<vecOutTradeNo.size(); i++)
				{
					ss<<"\n  out_trade_no: "<<vecOutTradeNo[i];
				}				  
			}
			else if(ExecutorThreadRequestType::T_PAY_LISTEN_INCOME_REQ == m_request_type)
			{
				ss<<"\n  openid: "<<m_openid
				  <<"\n  out_trade_no: "<<m_out_trade_no
				  <<"\n  share_type: "<<m_share_type
				  <<"\n  record_type: "<<m_record_type;
			}
			else if(ExecutorThreadRequestType::T_PAY_OUTCOME_QUERY_REQ == m_request_type)
			{
				ss<<"\n  openid: "<<m_openid
				  <<"\n  state: "<<m_state
				  <<"\n  limit: "<<m_limit
				  <<"\n  begin_ts: "<<m_begin_ts;
			}
			else if(ExecutorThreadRequestType::T_PAY_OUTCOME_COMMIT_REQ == m_request_type)
			{
				ss<<"\n  openid: "<<m_openid
				  <<"\n  amount: "<<m_amount;
			}
			else if(ExecutorThreadRequestType::T_PAY_OUTCOME_CHECK_REQ == m_request_type)
			{
				ss<<"\n  table_id: "<<m_table_id
				  <<"\n  openid: "<<m_openid
				  <<"\n  amount: "<<m_amount;
			}
			else if(ExecutorThreadRequestType::T_PAY_OUTCOME_UPDATE_REQ == m_request_type)
			{
				ss<<"\n  table_id: "<<m_table_id
				  <<"\n  state: "<<m_state
				  <<"\n  openid: "<<m_openid
				  <<"\n  amount: "<<m_amount;				  
			}
			
            ss<<"}";

            return ss.str();
        }

    public:    	
		uint32_t m_request_type;
		uint32_t m_fsm_id;    
        bool m_need_reply;
        
        uint32_t m_business_type;
        std::string m_openid;
        std::string m_appid;
        uint32_t m_state;
        uint64_t m_begin_ts;
        uint32_t m_limit;          
       

        std::string m_callback_xml;		

        std::string m_out_trade_no;
        std::string m_out_trade_openid;
        uint32_t m_share_type;
        uint32_t m_record_type;

		uint64_t m_business_id;
        std::string m_client_ip;
        uint64_t m_amount;
        uint64_t m_table_id;

        std::vector<std::string> vecOutTradeNo;
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
			  <<"\n  req_type: "<<ExecutorThreadRequestType::type_string(m_request_type)
              <<"\n  fsm_id: "<<m_fsm_id
              <<"\n  result_code: "<<ExecutorThreadRequestType::err_string(m_result_code)<<"\n";     
              
			if(ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_REQ == m_request_type)
			{				
				ss<<"\n  appid: "<<m_appid
				  <<"\n  timestamp: "<<m_time_stamp
				  <<"\n  nonce_str: "<<m_nonce_str
				  <<"\n  package: "<<m_package
				  <<"\n  sign_type: "<<m_sign_type
				  <<"\n  pay_sign: "<<m_pay_sign
				  <<"\n  out_trade_no: "<<m_out_trade_no;				  
			}
			else if(ExecutorThreadRequestType::T_PAY_TRANSFER_REQ == m_request_type 
					|| ExecutorThreadRequestType::T_PAY_SENDREDPACK_REQ == m_request_type
					|| ExecutorThreadRequestType::T_PAY_GET_USER_CASH_REQ == m_request_type
					|| ExecutorThreadRequestType::T_PAY_OUTCOME_COMMIT_REQ == m_request_type)
			{
				ss<<"\n  total_income: "<<m_userCash.total_income
				  <<"\n  total_outcome: "<<m_userCash.total_outcome
				  <<"\n  total_outcome_pending: "<<m_userCash.total_outcome_pending;
			}			
			else if(ExecutorThreadRequestType::T_PAY_QUERY_CASH_FLOW_REQ == m_request_type)
			{
				for(size_t i=0; i<vecCashFlowInfo.size(); i++)
				{
					ss<<"\n  CashFlowInfo: \n"<<vecCashFlowInfo[i].Utf8DebugString();
				}				
			}
			else if(ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_CALLBACK_REQ == m_request_type)
			{
				ss<<"\n  return_code: "<<m_return_code
				  <<"\n  return_msg: "<<m_return_msg;
			}
			else if(ExecutorThreadRequestType::T_PAY_UNIFIEDORDER_QUERY_REQ == m_request_type
					|| ExecutorThreadRequestType::T_PAY_OUTCOME_QUERY_REQ == m_request_type
					|| ExecutorThreadRequestType::T_PAY_LISTEN_INCOME_REQ == m_request_type
					|| ExecutorThreadRequestType::T_PAY_OUTCOME_CHECK_REQ == m_request_type
					|| ExecutorThreadRequestType::T_PAY_OUTCOME_UPDATE_REQ == m_request_type)
			{
				
			}			
					
              
            ss<<"}";

            return ss.str();
        }

    public:
        uint32_t m_request_type;
        uint32_t m_fsm_id;
        uint32_t m_result_code;        				

		std::string m_appid;
		std::string m_time_stamp;
		std::string m_nonce_str;
		std::string m_package;
		std::string m_sign_type;
		std::string m_pay_sign;
		std::string m_out_trade_no;

		UserCash m_userCash;		

		std::vector<hoosho::j::commstruct::CashFlowInfo> vecCashFlowInfo;
		std::vector<hoosho::j::commstruct::OutcomeReqInfo> vecOutcomeReqInfo;
		std::vector<std::string> vecFaultOutTradeNo;

		std::string m_return_code;
		std::string m_return_msg;
		uint32_t m_business_type;
		uint64_t m_business_id;
    };

    typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


