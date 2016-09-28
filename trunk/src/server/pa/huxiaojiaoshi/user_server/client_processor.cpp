#include "client_processor.h"
#include "global_var.h"
#include "util/lce_util.h"
#include "common_util.h"
#include "executor_thread_queue.h"
#include "fsm_business.h"
#include <openssl/sha.h>

IMPL_LOGGER(ClientProcessor, logger);

ClientProcessor::ClientProcessor()
{
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
	switch(dwCmd)
	{
#define CLIENT_PROCESSOR_CMD_HANDLER(cmd, handler) \
			case cmd: \
				handler(conn, stMsg); \
				break;

		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::J_USER_LOGIN_REQ, process_user_login);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::J_CHECK_LOGIN_REQ, process_user_check_login);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::J_GET_USER_INFO_REQ, process_user_detail_info_get);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::J_UPDATE_USER_INFO_REQ, process_user_detail_info_update);
		CLIENT_PROCESSOR_CMD_HANDLER(::hoosho::msg::J_GET_PA_JSAPI_SIGNATURE_REQ, process_get_jsapi_ticket);
		
#undef CLIENT_PROCESSOR_CMD_HANDLER

		default:
			LOG4CPLUS_ERROR(logger, "ClientProcessor::process_input fail, unknown cmd="<<dwCmd);
			conn.close();
			break;
	}


	return;
}

void ClientProcessor::process_user_login(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_login fail, new FsmPreAuth failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_user_check_login(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	std::string strCookieJKey = msg.check_login_req().cookie_j_key();
	std::string strCookieJValue = msg.check_login_req().cookie_j_value();
	bool bSucc = check_login_session(strCookieJKey, strCookieJValue);
	
	hoosho::msg::Msg stRespMsg;
    hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
    header->set_result(bSucc? hoosho::msg::E_OK : hoosho::msg::E_LOGIN_INVALID);
    header->set_seq(msg.head().seq());
    header->set_cmd(::hoosho::msg::J_CHECK_LOGIN_RES);
    
	g_client_processor->send_datagram(conn.get_id(), stRespMsg);
    return;
}


void ClientProcessor::process_user_detail_info_get(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_detail_info_get fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_user_detail_info_update(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_detail_info_update fail, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->client_req_event(conn, msg);
    return;
}

void ClientProcessor::process_get_jsapi_ticket(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	::hoosho::msg::Msg stRespMsg;
	::hoosho::msg::MsgHead* pRespMsg = stRespMsg.mutable_head();
	pRespMsg->set_cmd(::hoosho::msg::J_GET_PA_JSAPI_SIGNATURE_RES);
	pRespMsg->set_seq(msg.head().seq());
	pRespMsg->set_result(::hoosho::msg::E_OK);

	//req invalid
	std::string strAppid = msg.get_pa_jsapi_sign_req().pa_appid();
	std::string strReqUrl = msg.get_pa_jsapi_sign_req().req_url();
	strAppid = lce::util::StringOP::Trim(strAppid);
	strReqUrl = lce::util::StringOP::Trim(strReqUrl);
	if(strAppid.empty() || strReqUrl.empty())
	{
		LOG4CPLUS_ERROR(logger, "process_get_jsapi_ticket failed, for invalid strReqUrl="<<strReqUrl
								<<" or invalid strAppid="<<strAppid);
		pRespMsg->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
		send_datagram(conn.get_id(), stRespMsg);
		return;
	}

	std::string strBaseAccessToken = "";
	std::string strJSAPITiket = "";
	if(g_token_server_processor->get_pa_info(strAppid, strBaseAccessToken, strJSAPITiket) < 0)
	{
		LOG4CPLUS_ERROR(logger, "process_get_jsapi_ticket failed, PAToken not found for strAppid="<<strAppid);
		pRespMsg->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
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
		oss<<"jsapi_ticket="<<strJSAPITiket;
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

		::hoosho::j::user::GetPAJSAPISignRes* pRespBody = stRespMsg.mutable_get_pa_jsapi_sign_res();
		pRespBody->set_pa_appid(strAppid);
		pRespBody->set_req_url(strReqUrl);
		pRespBody->set_sign(strJSAPISignature);
		pRespBody->set_nonce(strNonce);
		pRespBody->set_timestamp(now);
	}	
	
	send_datagram(conn.get_id(), stRespMsg);
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
	
	LOG4CPLUS_DEBUG(logger, "send_datagram, one msg:"<<stMsg.Utf8DebugString());
	conn->write(datagram);
	
	return 0;
}

void ClientProcessor::add_login_session(const std::string& strOpenid, LoginSessionInfo& stLoginSessionInfo)
{
	time_t now = time(0);
	uint64_t qwSessionExpireTS = now + g_server->config().get_int_param("SESSION", "life", 0);

	LoginSessionInfoMapIterator iter = _login_session_map.find(strOpenid);
	if(iter == _login_session_map.end())
	{
		//build session
		std::ostringstream oss;
		oss.str("");
		oss<<strOpenid<<"_"<<now<<"_"<<(common::util::generate_unique_id());
		std::string strSessionSeed = oss.str();
		md5 stMD5;
		std::string strSession = lce::util::StringOP::TypeToStr(stMD5.hash64(strSessionSeed.c_str(), strSessionSeed.size()));
		stLoginSessionInfo._openid = strOpenid;
		stLoginSessionInfo._session = strSession;
		stLoginSessionInfo._expire_ts = qwSessionExpireTS;
		_login_session_map.insert(std::make_pair(strOpenid, stLoginSessionInfo));
	}
	else
	{
		iter->second._expire_ts = qwSessionExpireTS;
		stLoginSessionInfo = iter->second;
	}

	return;
}

bool ClientProcessor::check_login_session(const std::string& strOpenid, const std::string strSession)
{
	LoginSessionInfoMapIterator iter = _login_session_map.find(strOpenid);
	if(iter == _login_session_map.end())
	{
		LOG4CPLUS_DEBUG(logger, "session not exists");
		return false;
	}

	uint64_t now = time(0);
	LoginSessionInfo& stLoginSessionInfo = iter->second;
	if(now >= stLoginSessionInfo._expire_ts)
	{
		LOG4CPLUS_DEBUG(logger, "session expired");
		return false;
	}

	if(strSession != stLoginSessionInfo._session)
	{
		LOG4CPLUS_DEBUG(logger, "session not equal");
		return false;
	}
	
	stLoginSessionInfo._expire_ts = now + g_server->config().get_int_param("SESSION", "life", 0);
	return true;
}

void ClientProcessor::process_output()
{

}


