#include "client_processor.h"
#include "global_var.h"

IMPL_LOGGER(ClientProcessor, logger);

ClientProcessor::ClientProcessor()
{

}

ClientProcessor::ClientProcessor(int sec):
	_timeout(sec)
{

}


ClientProcessor::~ClientProcessor()
{

}

void ClientProcessor::accept(lce::net::ConnectionInfo & conn)
{	
	LOG4CPLUS_DEBUG(logger, "###### ONE CLIENT CONNECTED");
    //cerr<<"one client connected";
    /*
    LOG4CPLUS_TRACE(logger, "trace");
    LOG4CPLUS_DEBUG(logger, "debug");
    LOG4CPLUS_INFO(logger, "info");
    LOG4CPLUS_WARN(logger, "warn");
    LOG4CPLUS_ERROR(logger, "error");
    LOG4CPLUS_FATAL(logger, "fatal");
    */
    ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter == _conn_map.end())
	{
		_conn_map.insert(std::make_pair(conn.get_id(), &conn));
	}
}

void ClientProcessor::remove(lce::net::ConnectionInfo & conn)
{
    //cerr<<"one client disconnected";
    LOG4CPLUS_DEBUG(logger, "###### ONE CLIENT DISCONNECTED\n");

	ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter != _conn_map.end())
	{
		_conn_map.erase(iter);
	}
}


void ClientProcessor::process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram )
{ 
	//Do Coroutine Create
	
	CoroutineModuleBase<TimerModule>& co_module = Singleton<CoroutineModuleBase<TimerModule>>::Instance();
	
	int64_t coroutine_id = co_module.CreateCoroutine(&ClientProcessor::CoCallBack, _timeout, &ogram, &conn);

	if(coroutine_id <= 0)
	{
		LOG4CPLUS_ERROR(logger, "CreateCoroutine failed, coroutine_id=" << coroutine_id);
		conn.close();
		return;
	}

	co_module.Resume(coroutine_id);   
	
    return;
}

int ClientProcessor::send_datagram(uint32_t conn_id, const hoosho::msg::Msg& stMsg)
{
	ConnMapIterator iter = _conn_map.find(conn_id);
	if(iter == _conn_map.end())
	{
		LOG4CPLUS_DEBUG(logger, "ClientProcessor::send_datagram failed, connection not found, conn_id="<<conn_id);
		return -1;
	}
	lce::net::ConnectionInfo* conn = iter->second;

	lce::net::DatagramStringBuffer datagram;
	if(!stMsg.SerializeToString(&datagram._strbuffer))
	{
		LOG4CPLUS_DEBUG(logger, "ClientProcessor::send_datagram failed, msg  SerializeToString failed");
		return -1;
	}
	
	LOG4CPLUS_DEBUG(logger, "send_datagram, one msg: \n"<<stMsg.Utf8DebugString());
	conn->write(datagram);
	
	return 0;
}


void ClientProcessor::process_output()
{

}

void* ClientProcessor::CoCallBack(int64_t coroutine_id, void* ptr_usr1, void* ptr_usr2)
{
	lce::net::IDatagram* stOgram = (lce::net::IDatagram*)ptr_usr1;
	lce::net::ConnectionInfo* ptConn = (lce::net::ConnectionInfo*) ptr_usr2;

	lce::net::DatagramStringBuffer& stDatagramStringBuffer = dynamic_cast<lce::net::DatagramStringBuffer&>(*stOgram);
	::hoosho::msg::Msg stReqMsg;
	
	if(!stReqMsg.ParseFromString(stDatagramStringBuffer._strbuffer))
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::CoCallBack fail"
											<<", ParseFromString failed, data.size="<<stDatagramStringBuffer._strbuffer.size());
		ptConn->close();
		return NULL;
	}	

	uint64_t originSeq = stReqMsg.head().seq();
	
	/*
	 * 1.send to other server
	 * 2.send to thread queue
	 */
	if(stReqMsg.head().cmd() == ::hoosho::msg::USER_PARSE_PRE_AUTH_CODE_REQ)
	{
		std::string _req_preauth_code = stReqMsg.user_parse_pre_auth_code_req().pre_auth_code();
		::hoosho::msg::Msg stPARequestMsg;
		::hoosho::msg::MsgHead* pPARequestHead = stPARequestMsg.mutable_head();
		
		pPARequestHead->set_cmd(::hoosho::msg::PARSE_PRE_AUTH_CODE_REQ);
		pPARequestHead->set_seq(stReqMsg.head().seq());
		::hoosho::pa::ParsePreAuthCodeReq* pPAParsePreAuthCodeReq = stPARequestMsg.mutable_parse_pre_auth_code_req();
		pPAParsePreAuthCodeReq->set_pre_auth_code(_req_preauth_code);
		g_server_processor_pa->send_datagram(stPARequestMsg);
	}	
	else
	{	

		/*
		 * should send to msg server, if push this request in thread queue, after thread hanld done, 
		 * swapcontent from thread -> main process, segment fault happed!!!!! 
		 * 
		 * so SEND TO MSG SERVER FOR SAFE
		 *
		 */

		stReqMsg.mutable_head()->set_seq(coroutine_id);
		g_server_processor_msg->send_datagram(stReqMsg);

		

		#if 0
        ExecutorThreadRequestElement req(stReqMsg.head().seq(), stReqMsg.head().cmd(), coroutine_id);

        if(::hoosho::msg::QUERY_MSG_REQ == stReqMsg.head().cmd()){
        
			req.m_msg_id = stReqMsg.msg_query_req().msg_id();
			req.m_amount = stReqMsg.msg_query_req().amount();
			req.m_openid_md5_from = stReqMsg.msg_query_req().openid_md5_from();
			req.m_openid_md5_to = stReqMsg.msg_query_req().openid_md5_to(); 
			
        }else if(::hoosho::msg::ADD_MSG_REQ == stReqMsg.head().cmd()){
        
        	const ::hoosho::commstruct::MsgContent& msg_content = stReqMsg.msg_add_req().msg_content();
			req.m_content = msg_content.content();
			req.m_openid_md5_from = msg_content.openid_md5_from();
			req.m_openid_md5_to = msg_content.openid_md5_to();			
			
        }else if(::hoosho::msg::DELETE_MSG_REQ == stReqMsg.head().cmd()){
        
			const ::hoosho::commstruct::MsgContent& msg_content = stReqMsg.msg_add_req().msg_content();
			req.m_content = msg_content.content();
			req.m_openid_md5_from = msg_content.openid_md5_from();
			req.m_openid_md5_to = msg_content.openid_md5_to();
			
        }else if(::hoosho::msg::QUERY_MSG_NEWMSG_STATUS_REQ == stReqMsg.head().cmd()){
        
			req.m_openid_md5 = stReqMsg.msg_query_newmsg_status_req().openid_md5();
			
        }else if(::hoosho::msg::QUERY_MSG_SESSION_LIST_REQ == stReqMsg.head().cmd()){
        
			req.m_openid_md5 = stReqMsg.msg_query_session_list_req().openid_md5();
			
        }else if(::hoosho::msg::DELETE_SESSION_REQ == stReqMsg.head().cmd()){
        
			req.m_openid_md5_from = stReqMsg.session_del_req().openid_md5_from();
			req.m_openid_md5_to = stReqMsg.session_del_req().openid_md5_to();
			
        }else if(::hoosho::msg::QUERY_SYSTEM_RED_POINT_REQ == stReqMsg.head().cmd()){
        
			req.m_openid_md5 = stReqMsg.query_system_red_point_req().openid_md5();
			req.m_pa_appid_md5 = stReqMsg.query_system_red_point_req().pa_appid_md5();
			
        }else if(::hoosho::msg::UPDATE_SYSTEM_RED_POINT_REQ == stReqMsg.head().cmd()){
        
			req.m_openid_md5 = stReqMsg.update_system_red_point_req().openid_md5();
			req.m_pa_appid_md5 = stReqMsg.update_system_red_point_req().pa_appid_md5();
			req.m_redpoint_type = stReqMsg.update_system_red_point_req().type();
			req.m_redpoint_opt = stReqMsg.update_system_red_point_req().opt();
			
        }else if(::hoosho::msg::ADD_NOTICE_RECORD_REQ == stReqMsg.head().cmd()){
        
			req.m_notice_record.CopyFrom(stReqMsg.notice_record_add_req().notice_record());
			
        }else if(::hoosho::msg::QUERY_NOTICE_RECORD_REQ == stReqMsg.head().cmd()){
        
			req.m_pa_appid_md5 = stReqMsg.notice_record_query_req().pa_appid_md5();
			req.m_openid_md5 = stReqMsg.notice_record_query_req().openid_md5();
			req.m_notice_record_type = stReqMsg.notice_record_query_req().type();
			req.m_limit_ts = stReqMsg.notice_record_query_req().limit_ts();
			req.m_pagesize = stReqMsg.notice_record_query_req().pagesize();
			
        }else if(::hoosho::msg::DELETE_NOTICE_RECORD_REQ == stReqMsg.head().cmd()){
        
			req.m_pa_appid_md5 = stReqMsg.notice_record_delete_req().pa_appid_md5();
			req.m_openid_md5 = stReqMsg.notice_record_delete_req().openid_md5();
			req.m_create_ts = stReqMsg.notice_record_delete_req().create_ts();
			req.m_notice_record_type = stReqMsg.notice_record_delete_req().type();
			
        }else if(::hoosho::msg::QUERY_SYSTEM_MSG_REQ == stReqMsg.head().cmd()){
        
			req.m_openid_md5 = stReqMsg.system_msg_query_req().openid_md5();
			req.m_pa_appid_md5 = stReqMsg.system_msg_query_req().pa_appid_md5();
			req.m_limit_ts = stReqMsg.system_msg_query_req().begin_ts();
			req.m_pagesize = stReqMsg.system_msg_query_req().limit();
			
        }

        //req.m_msg = *stReqMsg;
        g_executor_thread_processor->send_request(req);  

        #endif
	}



	

	//2.yield() -> main processs	
	void* ptr = NULL;
	ptr = Yield<CoroutineModuleBase<TimerModule>>();


	

	//3.swapcontext main processs -> coroutine, get response Msg
	::hoosho::msg::Msg* stRespMsg = (::hoosho::msg::Msg*)ptr;
	if(!stRespMsg) //coroutine time out
	{
		::hoosho::msg::Msg tmpMsg;
	    ::hoosho::msg::MsgHead* header = tmpMsg.mutable_head(); 

	    header->set_seq(stReqMsg.head().seq());    
		header->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);

	    switch(stReqMsg.head().cmd())
	    {
	    
#define CASE_CMD_REQ_RES(reqcmd, rescmd) \
			case reqcmd: \
				header->set_cmd(rescmd); \
				break; \

			CASE_CMD_REQ_RES(::hoosho::msg::QUERY_MSG_REQ, ::hoosho::msg::QUERY_MSG_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::ADD_MSG_REQ, ::hoosho::msg::ADD_MSG_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::DELETE_MSG_REQ, ::hoosho::msg::DELETE_MSG_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::QUERY_MSG_NEWMSG_STATUS_REQ, ::hoosho::msg::QUERY_MSG_NEWMSG_STATUS_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::QUERY_MSG_SESSION_LIST_REQ, ::hoosho::msg::QUERY_MSG_SESSION_LIST_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::DELETE_SESSION_REQ, ::hoosho::msg::DELETE_SESSION_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::QUERY_SYSTEM_RED_POINT_REQ, ::hoosho::msg::QUERY_SYSTEM_RED_POINT_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::UPDATE_SYSTEM_RED_POINT_REQ, ::hoosho::msg::UPDATE_SYSTEM_RED_POINT_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::ADD_NOTICE_RECORD_REQ, ::hoosho::msg::ADD_NOTICE_RECORD_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::QUERY_NOTICE_RECORD_REQ, ::hoosho::msg::QUERY_NOTICE_RECORD_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::DELETE_NOTICE_RECORD_REQ, ::hoosho::msg::DELETE_NOTICE_RECORD_RES);
			CASE_CMD_REQ_RES(::hoosho::msg::QUERY_SYSTEM_MSG_REQ, ::hoosho::msg::QUERY_SYSTEM_MSG_RES);		
				
#undef CASE_CMD_REQ_RES			

			default:
				LOG4CPLUS_DEBUG(logger, "UNKOWN REQ CMD TYPE!");
				break;				
	    }
	    

		g_client_processor->send_datagram(ptConn->get_id(), tmpMsg);
	}
	else
	{
		if(stRespMsg->head().cmd() == ::hoosho::msg::PARSE_PRE_AUTH_CODE_RES)
		{
			/*** add you code here ***/
			
		}	

		stRespMsg->mutable_head()->set_seq(originSeq);
		g_client_processor->send_datagram(ptConn->get_id(), *stRespMsg);
	}

	return NULL;
}



