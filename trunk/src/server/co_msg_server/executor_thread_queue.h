#ifndef _SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _SERVER_EXECUTOR_THREAD_QUEUE_H_

#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include "memory/duplex_queue.h"
#include "msg.pb.h"

using namespace std;

class ExecutorThreadRequestElement
{
public:
	ExecutorThreadRequestElement()
	{
	}
	
    ExecutorThreadRequestElement(int seq,int request_type, uint64_t coroutine_id):
    	m_seq(seq),
		m_request_type(request_type),
		m_coroutine_id(coroutine_id)
    {
    }

	const std::string type_string() const
	{
		switch(m_request_type)
		{
			case ::hoosho::msg::QUERY_MSG_REQ:
				return "QUERY_MSG_REQ";
			case ::hoosho::msg::ADD_MSG_REQ:
				return "ADD_MSG_REQ";
			case ::hoosho::msg::DELETE_MSG_REQ:
				return "DELETE_MSG_REQ";
			case ::hoosho::msg::QUERY_MSG_NEWMSG_STATUS_REQ:
				return "QUERY_MSG_NEWMSG_STATUS_REQ";
			case ::hoosho::msg::QUERY_MSG_SESSION_LIST_REQ:
				return "QUERY_MSG_SESSION_LIST_REQ";
			case ::hoosho::msg::DELETE_SESSION_REQ:
				return "DELETE_SESSION_REQ";
			case ::hoosho::msg::QUERY_SYSTEM_RED_POINT_REQ:
				return "QUERY_SYSTEM_RED_POINT_REQ";
			case ::hoosho::msg::UPDATE_SYSTEM_RED_POINT_REQ:
				return "UPDATE_SYSTEM_RED_POINT_REQ";
			case ::hoosho::msg::ADD_NOTICE_RECORD_REQ:
				return "ADD_NOTICE_RECORD_REQ";
			case ::hoosho::msg::QUERY_NOTICE_RECORD_REQ:
				return "QUERY_NOTICE_RECORD_REQ";
			case ::hoosho::msg::DELETE_NOTICE_RECORD_REQ:
				return "DELETE_NOTICE_RECORD_REQ";
			case ::hoosho::msg::QUERY_SYSTEM_MSG_REQ:
				return "QUERY_SYSTEM_MSG_REQ";

			default:
				return "UNKOWN MSG CMD TYPE";
		}
	}

    const std::string ToString() const
    {
        std::ostringstream ss;
        ss<<"{"
          <<"\n  coroutine_id: "<<m_coroutine_id
          <<"\n  request_type: "<<type_string();

        if(::hoosho::msg::QUERY_MSG_REQ == m_request_type){
			ss<<"\n  msg_id: "<<m_msg_id
			  <<"\n  amount: "<<m_amount
			  <<"\n  openid_md5_from: "<<m_openid_md5_from
			  <<"\n  openid_md5_to: "<<m_openid_md5_to;
        }else if(::hoosho::msg::ADD_MSG_REQ == m_request_type){
        	ss<<"\n  content: "<<m_content
        	  <<"\n  openid_md5_from: "<<m_openid_md5_from
        	  <<"\n  openid_md5_to: "<<m_openid_md5_to;			
        }else if(::hoosho::msg::DELETE_MSG_REQ == m_request_type){
			ss<<"\n  content: "<<m_content
        	  <<"\n  openid_md5_from: "<<m_openid_md5_from
        	  <<"\n  openid_md5_to: "<<m_openid_md5_to;
        }else if(::hoosho::msg::QUERY_MSG_NEWMSG_STATUS_REQ == m_request_type){
        	ss<<"\n  openid_md5: "<<m_openid_md5;
        }else if(::hoosho::msg::QUERY_MSG_SESSION_LIST_REQ == m_request_type){
			ss<<"\n  openid_md5: "<<m_openid_md5;
        }else if(::hoosho::msg::DELETE_SESSION_REQ == m_request_type){
        	ss<<"\n  openid_md5_from: "<<m_openid_md5_from
        	  <<"\n  openid_md5_to: "<<m_openid_md5_to;			
        }else if(::hoosho::msg::QUERY_SYSTEM_RED_POINT_REQ == m_request_type){
        	ss<<"\n  openid_md5: "<<m_openid_md5
        	  <<"\n  pa_appid_md5: "<<m_pa_appid_md5;			
        }else if(::hoosho::msg::UPDATE_SYSTEM_RED_POINT_REQ == m_request_type){
        	ss<<"\n  openid_md5: "<<m_openid_md5
        	  <<"\n  pa_appid_md5: "<<m_pa_appid_md5
        	  <<"\n  redpoint_type: "<<m_redpoint_type
        	  <<"\n  redpoint_opt: "<<m_redpoint_opt;			
        }else if(::hoosho::msg::ADD_NOTICE_RECORD_REQ == m_request_type){
        	ss<<"\n  notice_record: "<<m_notice_record.Utf8DebugString();
        }else if(::hoosho::msg::QUERY_NOTICE_RECORD_REQ == m_request_type){
			ss<<"\n  pa_appid_md5: "<<m_pa_appid_md5
			  <<"\n  openid_md5: "<<m_openid_md5
			  <<"\n  notice_record_type: "<<m_notice_record_type
			  <<"\n  limit_ts: "<<m_limit_ts
			  <<"\n  pagesize: "<<m_pagesize;        
        }else if(::hoosho::msg::DELETE_NOTICE_RECORD_REQ == m_request_type){
        	ss<<"\n  pa_appid_md5: "<<m_pa_appid_md5
        	  <<"\n  openid_md5: "<<m_openid_md5
        	  <<"\n  create_ts: "<<m_create_ts
        	  <<"\n  notice_record_type: "<<m_notice_record_type;			
        }else if(::hoosho::msg::QUERY_SYSTEM_MSG_REQ == m_request_type){
        	ss<<"\n  openid_md5: "<<m_openid_md5
        	  <<"\n  pa_appid_md5: "<<m_pa_appid_md5
        	  <<"\n  limit_ts: "<<m_limit_ts
        	  <<"\n  pagesize: "<<m_pagesize;			
        }
          			
		
        ss<<"\n}";

        return ss.str();
    }

public:		
	uint64_t m_seq;
	uint32_t m_request_type;		
	uint64_t m_coroutine_id;
	uint64_t m_openid_md5;
	uint64_t m_pa_appid_md5;

	uint64_t m_msg_id;
	uint64_t m_create_ts;
	std::string   m_content;
	uint64_t m_openid_md5_from;
	uint64_t m_openid_md5_to;

	uint32_t m_amount;

	uint32_t m_redpoint_type;
	uint32_t m_redpoint_opt;

	uint64_t m_notice_record_type;
	uint64_t m_limit_ts;
	uint64_t m_pagesize;

	::hoosho::commstruct::NoticeRecord m_notice_record;

	
	//::hoosho::msg::Msg m_msg;	
    
};    

typedef lce::memory::ArrayCircularQueue<ExecutorThreadRequestElement> ExecutorThreadQueue;

#endif


