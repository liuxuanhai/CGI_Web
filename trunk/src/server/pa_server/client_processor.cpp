#include "client_processor.h"
#include "global_var.h"
#include "util/lce_util.h"
#include "executor_thread_queue.h"
#include <openssl/sha.h>

IMPL_LOGGER(ClientProcessor, logger);

ClientProcessor::ClientProcessor()
{
	_hx_pa_accesstoken = "";
	_hx_ticket_jsapi = "";
	_hx_token_last_update_ts = 0;

	assert(g_timer_container->schedule(*this, 5, 60, NULL) >= 0);
}

ClientProcessor::~ClientProcessor()
{
	
}

void ClientProcessor::accept(lce::net::ConnectionInfo & conn)
{
    ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter == _conn_map.end())
	{
		_conn_map.insert(std::make_pair(conn.get_id(), &conn));
	}
}

void ClientProcessor::remove(lce::net::ConnectionInfo & conn)
{
	ConnMapIterator iter = _conn_map.find(conn.get_id());
	if(iter != _conn_map.end())
	{
		_conn_map.erase(iter);
	}
}

void ClientProcessor::handle_timeout(void * param)
{
	if(need_update_pa_token())
	{
		ExecutorThreadRequestElement element(ExecutorThreadRequestType::T_PA_TOKEN, -1, -1);
		element.need_reply();

		//secret info , hard code  as this!!!!!!
		element.m_pa_appid = HX_PA_APPID;
		element.m_pa_appsecret = HX_PA_APPSECRET;
		g_executor_thread_processor->send_request(element);
	}
}

bool ClientProcessor::need_update_pa_token()
{
	LOG4CPLUS_DEBUG(logger, "need_update_pa_token begin"
						<<", _hx_pa_accesstoken="<<_hx_pa_accesstoken
						<<", _hx_ticket_jsapi="<<_hx_ticket_jsapi
						<<", _hx_token_last_update_ts="<<_hx_token_last_update_ts);

	if(_hx_pa_accesstoken.empty() || _hx_ticket_jsapi.empty() || 0 == _hx_token_last_update_ts)
	{
		LOG4CPLUS_DEBUG(logger, "need_update_pa_token ??? YES!! reason: 'data is not full'");
		return true;
	}
	
	int iTokenExpiredSeconds = g_server->config().get_int_param("PA_TOKEN", "expired", 0);
	if(_hx_token_last_update_ts + iTokenExpiredSeconds <= (uint64_t)time(0))
	{
		LOG4CPLUS_DEBUG(logger, "need_update_pa_token ??? YES!! reason: 'data is not full'");
		return true;
	}

	LOG4CPLUS_DEBUG(logger, "need_update_pa_token ??? NO!!");
	return false;
}

void ClientProcessor::do_update_pa_token(const std::string& strAccessToekn, const std::string& strTicketJSAPI)
{
	_hx_pa_accesstoken = strAccessToekn;
	_hx_ticket_jsapi = strTicketJSAPI;
	_hx_token_last_update_ts = time(0);

	LOG4CPLUS_DEBUG(logger,"NOTICE:!!!!!!!!!! _hx_pa_accesstoken update to "<<_hx_pa_accesstoken
			<<", _hx_ticket_jsapi update to "<<_hx_ticket_jsapi
			<<", at ts: "<<_hx_token_last_update_ts);
}

void ClientProcessor::process_input(lce::net::ConnectionInfo& conn, lce::net::IDatagram& ogram )
{
	lce::net::DatagramStringBuffer& stDatagramStringBuffer = dynamic_cast<lce::net::DatagramStringBuffer&>(ogram);
	::hoosho::msg::Msg stMsg;
	if(!stMsg.ParseFromString(stDatagramStringBuffer._strbuffer))
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_input fail"
											<<", ParseFromString failed, data.size="<<stDatagramStringBuffer._strbuffer.size());
		conn.close();
		return;
	}

	LOG4CPLUS_DEBUG(logger, "process_input, one msg:"<<stMsg.Utf8DebugString());
	
	::hoosho::msg::MsgHead stMsgHead = stMsg.head();
	uint32_t dwCmd = stMsgHead.cmd();
	if(::hoosho::msg::PARSE_PRE_AUTH_CODE_REQ == dwCmd)
	{
		process_pre_auth_code(conn, stMsg);
	}
	else if(::hoosho::msg::GET_JSAPI_SGIN_REQ == dwCmd)
	{
		process_jsapi_ticket(conn, stMsg);
	}
	else if(::hoosho::msg::DOWNLOAD_PIC_RESOURCE_REQ == dwCmd)
	{
		process_download_resource(conn, stMsg);
	}
	else if(::hoosho::msg::QUERY_USER_PA_REQ == dwCmd)
	{
		process_pa_list_get(conn, stMsg);
	}
	else if(::hoosho::msg::QUERY_PA_TAB_REQ == dwCmd)
	{
		process_pa_tab_get(conn, stMsg);
	}	
	else if(::hoosho::msg::QUERY_PA_EXPERT_REQ == dwCmd)
	{
		process_pa_expert_query(conn, stMsg);
	}	
	else
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_input fail, unknown cmd="<<dwCmd);
		conn.close();
	}

	return;
}

void ClientProcessor::process_pre_auth_code(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmPreAuth* fsm = g_fsm_preauth_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_pre_auth_code fail, new FsmPreAuth failed");
		conn.close();
		return;
	}

	fsm->req_preauth_event(conn, msg);
    return;
}

void ClientProcessor::process_jsapi_ticket(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	::hoosho::msg::Msg stRespMsg;
	::hoosho::msg::MsgHead* pRespMsg = stRespMsg.mutable_head();
	pRespMsg->set_cmd(::hoosho::msg::GET_JSAPI_SGIN_RES);
	pRespMsg->set_seq(msg.head().seq());
	pRespMsg->set_result(::hoosho::msg::E_OK);

	//req invalid
	const std::string& strReqUrl = msg.get_jsapisign_req().req_url();
	if(strReqUrl.empty())
	{
		LOG4CPLUS_ERROR(logger, "process_jsapi_ticket failed, strReqUrl="<<strReqUrl);
		pRespMsg->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
		send_datagram(conn.get_id(), stRespMsg);
		return;
	}

	//not inited
	if(_hx_ticket_jsapi.empty())
	{
		LOG4CPLUS_ERROR(logger, "process_jsapi_ticket failed, _hx_ticket_jsapi="<<_hx_ticket_jsapi);
		pRespMsg->set_result(::hoosho::msg::E_SERVER_INNER_ERROR);
		send_datagram(conn.get_id(), stRespMsg);
		return;
	}

	//build jsapi signature
	{
		//nonce
		time_t now = time(0);
		struct timeval tv;
	    gettimeofday(&tv, NULL);
	    struct tm* t = ::localtime(&now);
	    char aczIDBuffer[32] = {0};
	    snprintf(aczIDBuffer, 256, "%04d%02d%02d%02d%02d%02d%06d"
	                        , t->tm_year+1900, t->tm_mon+1, t->tm_mday
	                        , t->tm_hour, t->tm_min, t->tm_sec
	                        , (int)tv.tv_usec);
		std::string strNonce = aczIDBuffer;

		//sha1 to jsapi signature
		//jsapi_ticket=xxxxx&noncestr=xxxxW&timestamp=1414587457&url=http://mp.weixin.qq.com?params=value
		std::ostringstream oss;
		oss.str("");
		oss<<"jsapi_ticket="<<_hx_ticket_jsapi;
		oss<<"&noncestr="<<strNonce;
		oss<<"&timestamp="<<now;
		oss<<"&url="<<strReqUrl;

		unsigned char aczDigest[SHA_DIGEST_LENGTH];
	    SHA1((unsigned char*)oss.str().c_str(), oss.str().size(), aczDigest);
	    char aczSHA1[SHA_DIGEST_LENGTH*2+1];
	    for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
	    {
	        sprintf(&aczSHA1[i*2], "%02x", (unsigned int)aczDigest[i]);
	    }

		std::string strJSAPISignature = aczSHA1;

		::hoosho::pa::GetJSAPISignRes* pGetJSAPISignRes = stRespMsg.mutable_get_jsapisign_res();
		pGetJSAPISignRes->set_pa_appid(HX_PA_APPID);
		pGetJSAPISignRes->set_sign(strJSAPISignature);
		pGetJSAPISignRes->set_nonce(strNonce);
		pGetJSAPISignRes->set_timestamp(now);
		pGetJSAPISignRes->set_req_url(strReqUrl);
	}	
	
	send_datagram(conn.get_id(), stRespMsg);
	return;
}

void ClientProcessor::process_download_resource(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmResource* fsm = g_fsm_resource_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_download_resource fail, new FsmResource failed");
		conn.close();
		return;
	}

	fsm->download_req_event(conn, msg);
    return;
}

void ClientProcessor::process_pa_list_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_download_resource fail, new FsmResource failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_pa_tab_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_pa_tab_get fail, new FsmResource failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_pa_expert_query(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_pa_tab_get fail, new FsmResource failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
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

	if(::hoosho::msg::DOWNLOAD_PIC_RESOURCE_RES != stMsg.head().cmd())
	{
		LOG4CPLUS_DEBUG(logger, "send_datagram, one msg:"<<stMsg.Utf8DebugString());
	}
	else
	{
		LOG4CPLUS_DEBUG(logger, "send_datagram, download_resource_resp.head:"<<stMsg.head().Utf8DebugString());
		const std::string& strMediaData = stMsg.download_pic_resource_res().media_data();
		LOG4CPLUS_DEBUG(logger, "send_datagram, download_resource_resp.resource.media_data.size:"<<strMediaData.size());
	}
	
	
	conn->write(datagram);
	
	return 0;
}


void ClientProcessor::process_output()
{

}



