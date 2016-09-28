#include "client_processor.h"
#include "global_var.h"
#include "util/lce_util.h"
#include "common_util.h"
#include "executor_thread_queue.h"
#include "fsm_business.h"
#include <openssl/sha.h>

IMPL_LOGGER(ClientProcessor, logger);

uint64_t max_user_id = 0;
uint64_t max_show_id = 0;

ClientProcessor::ClientProcessor()
{
}

ClientProcessor::~ClientProcessor()
{
	
}

int ClientProcessor::init()
{
	const lce::app::Config& stConfig = g_server->config();
	m_db_ip = stConfig.get_string_param("DB", "ip"); 
	m_db_user = stConfig.get_string_param("DB", "user"); 
	m_db_passwd = stConfig.get_string_param("DB", "passwd"); 
	m_db_name = stConfig.get_string_param("DB", "db_name"); 
	m_db_table_user_info_name = stConfig.get_string_param("DB", "table_user_info_name"); 	

	assert(m_mysql_helper.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd));

	return load_global_var();
}

int ClientProcessor::load_global_var()
{
	//1.max_user_id
	std::ostringstream oss;
	oss.str("");
	oss<<"select max(user_id) from t_user_info";

	if(!m_mysql_helper.Query(oss.str()))
	{
		LOG4CPLUS_FATAL(logger, "ClientProcessor::load_global_var failed, error: "<<m_mysql_helper.GetErrMsg());
		return -1;
	}

	if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
	{
		max_user_id = strtoul(m_mysql_helper.GetRow(0), NULL, 10);		
	}
	else
	{
		max_user_id = HOOSHO_USER_BASE_ID;
	}

	//2.max_show_id
	oss.str("");
	oss<<"select max(show_id) from t_show_info";

	if(!m_mysql_helper.Query(oss.str()))
	{
		LOG4CPLUS_FATAL(logger, "ClientProcessor::load_global_var failed, error: "<<m_mysql_helper.GetErrMsg());
		return -1;
	}

	if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
	{
		max_show_id = strtoul(m_mysql_helper.GetRow(0), NULL, 10);		
	}
	else
	{
		max_show_id = HOOSHO_SHOW_BASE_ID;
	}

	return 0;	
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

	if(stMsg.head().cmd() != ::hoosho::msg::S_PROJECT_REQ)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_input fail, cmd: "<<stMsg.head().cmd()<<" != ::hoosho::msg::S_PROJECT_REQ");
		conn.close();
		return;
	}	
		
	::hoosho::msg::s::MsgReq stSMsgReq = stMsg.s_msg_req();
	uint32_t dwSubCmd = stSMsgReq.sub_cmd();
	switch(dwSubCmd)
	{
		case ::hoosho::msg::s::GET_PA_JSAPI_SIGNATURE_REQ:
			return process_get_jsapi_ticket(conn, stMsg);

		case ::hoosho::msg::s::GET_PA_ACCESS_TOKEN_REQ:
			return process_get_access_token(conn, stMsg);

		case ::hoosho::msg::s::USER_LOGIN_REQ:
			return process_user_login(conn, stMsg);

		//Do authentication first unless USER_LOGIN_REQ && GET_PA_JSAPI_SIGNATURE_REQ && WX_PAY_CALLBACK_NTF
		default:
			return process_input_authenticate(conn, stMsg);
	}


	return;
}

void ClientProcessor::process_get_jsapi_ticket(lce::net::ConnectionInfo& conn,  const ::hoosho::msg::Msg& msg)
{
	::hoosho::msg::Msg stRespMsg;
	::hoosho::msg::MsgHead* pRespMsgHead = stRespMsg.mutable_head();
	pRespMsgHead->set_cmd(::hoosho::msg::S_PROJECT_RES);
	pRespMsgHead->set_seq(msg.head().seq());
	pRespMsgHead->set_result(::hoosho::msg::E_OK);

	const ::hoosho::msg::s::MsgReq& stSMsgReq = msg.s_msg_req();

	//req invalid
	std::string strAppid = stSMsgReq.get_pa_jsapi_sign_req().pa_appid();
	std::string strReqUrl = stSMsgReq.get_pa_jsapi_sign_req().req_url();
	strAppid = lce::util::StringOP::Trim(strAppid);
	strReqUrl = lce::util::StringOP::Trim(strReqUrl);
	if(strAppid.empty() || strReqUrl.empty())
	{
		LOG4CPLUS_ERROR(logger, "process_get_jsapi_ticket failed, for invalid strReqUrl="<<strReqUrl
								<<" or invalid strAppid="<<strAppid);
		pRespMsgHead->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
		send_datagram(conn.get_id(), stRespMsg);
		return;
	}

	std::string strBaseAccessToken = "";
	std::string strJSAPITiket = "";
	if(g_token_server_processor->get_pa_info(strAppid, strBaseAccessToken, strJSAPITiket) < 0)
	{
		LOG4CPLUS_ERROR(logger, "process_get_jsapi_ticket failed, PAToken not found for strAppid="<<strAppid);
		pRespMsgHead->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
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

		::hoosho::msg::s::MsgRes* pSMsgRes = stRespMsg.mutable_s_msg_res();
		pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_PA_JSAPI_SIGNATURE_RES);
		
		::hoosho::msg::s::GetPAJSAPISignRes* pRespBody = pSMsgRes->mutable_get_pa_jsapi_sign_res();
		pRespBody->set_pa_appid(strAppid);
		pRespBody->set_req_url(strReqUrl);
		pRespBody->set_sign(strJSAPISignature);
		pRespBody->set_nonce(strNonce);
		pRespBody->set_timestamp(now);
	}	
	
	send_datagram(conn.get_id(), stRespMsg);
	return;
}

void ClientProcessor::process_get_access_token(lce::net::ConnectionInfo& conn,  const ::hoosho::msg::Msg& msg)
{
	::hoosho::msg::Msg stRespMsg;
	::hoosho::msg::MsgHead* pRespMsgHead = stRespMsg.mutable_head();
	pRespMsgHead->set_cmd(::hoosho::msg::S_PROJECT_RES);
	pRespMsgHead->set_seq(msg.head().seq());
	pRespMsgHead->set_result(::hoosho::msg::E_OK);

	const ::hoosho::msg::s::MsgReq& stSMsgReq = msg.s_msg_req();

	//req invalid
	std::string strAppid = stSMsgReq.get_pa_access_token_req().pa_appid();
	strAppid = lce::util::StringOP::Trim(strAppid);
	if(strAppid.empty())
	{
		LOG4CPLUS_ERROR(logger, "process_get_access_token failed, for invalid strAppid="<<strAppid);
		pRespMsgHead->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
		send_datagram(conn.get_id(), stRespMsg);
		return;
	}

	std::string strBaseAccessToken = "";
	std::string strJSAPITiket = "";
	if(g_token_server_processor->get_pa_info(strAppid, strBaseAccessToken, strJSAPITiket) < 0)
	{
		LOG4CPLUS_ERROR(logger, "process_get_access_token failed, PAToken not found for strAppid="<<strAppid);
		pRespMsgHead->set_result(::hoosho::msg::E_INVALID_REQ_PARAM);
		send_datagram(conn.get_id(), stRespMsg);
		return;
	}

	::hoosho::msg::s::MsgRes* pSMsgRes = stRespMsg.mutable_s_msg_res();
	pSMsgRes->set_sub_cmd(::hoosho::msg::s::GET_PA_ACCESS_TOKEN_RES);
	::hoosho::msg::s::GetPAAccessTokenRes* pRespBody = pSMsgRes->mutable_get_pa_access_token_res();
	pRespBody->set_pa_access_token(strBaseAccessToken);

	send_datagram(conn.get_id(), stRespMsg);
	return;
}

void ClientProcessor::process_user_login(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{
	std::string strAuthCode = msg.s_msg_req().code();
	hoosho::msg::s::UserInfo stUserInfo;
	int iStatus = check_login_session(strAuthCode, stUserInfo);
	if(USER_LOGIN_STATUS_SUCCESS == iStatus)
	{
		::hoosho::msg::Msg stRespMsg;
		::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
		header->set_seq(msg.head().seq());
		header->set_cmd(::hoosho::msg::S_PROJECT_RES);
		header->set_result(::hoosho::msg::E_OK);

		::hoosho::msg::s::MsgRes* pSMsgRes = stRespMsg.mutable_s_msg_res();
		pSMsgRes->set_sub_cmd(::hoosho::msg::s::USER_LOGIN_RES);
		::hoosho::msg::s::UserInfo* pUserInfo = pSMsgRes->add_user_info_list();
		pUserInfo->CopyFrom(stUserInfo);

		g_client_processor->send_datagram(conn.get_id(), stRespMsg);
		return;
	}
	else if(USER_LOGIN_STATUS_EXPIRED == iStatus)
	{
		//if login session expired, feedback direct, must input new pre_auth_code
		::hoosho::msg::Msg stRespMsg;
		::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
		header->set_seq(msg.head().seq());
		header->set_cmd(::hoosho::msg::S_PROJECT_RES);
		header->set_result(::hoosho::msg::E_LOGIN_INVALID);

		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_user_login failed, E_LOGIN_INVALID, please input new PRE_AUTH_CODE!");	
		g_client_processor->send_datagram(conn.get_id(), stRespMsg);
		return;
	}

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

void ClientProcessor::process_input_authenticate(lce::net::ConnectionInfo& conn, const ::hoosho::msg::Msg& msg)
{		
	::hoosho::msg::Msg stRespMsg;
	::hoosho::msg::MsgHead* header = stRespMsg.mutable_head();
	header->set_seq(msg.head().seq());
	header->set_cmd(::hoosho::msg::S_PROJECT_RES);
	header->set_result(::hoosho::msg::E_OK);

	//check login state
	const ::hoosho::msg::s::MsgReq& stSMsgReq = msg.s_msg_req();
	std::string strAuthCode = stSMsgReq.code();
	hoosho::msg::s::UserInfo stUserInfo;
	if((USER_LOGIN_STATUS_SUCCESS != check_login_session(strAuthCode, stUserInfo)) || stUserInfo.openid().empty())
	{
		LOG4CPLUS_DEBUG(logger, "user_info=" << stUserInfo.Utf8DebugString());
		header->set_result(::hoosho::msg::E_LOGIN_INVALID);
	    LOG4CPLUS_ERROR(logger, "ClientProcessor::process_input failed, E_LOGIN_INVALID, please relogin!");		
		g_client_processor->send_datagram(conn.get_id(), stRespMsg);
		return;
	}

	//check phone vc
	if(stSMsgReq.sub_cmd() == ::hoosho::msg::s::SET_PHONE_REQ)
	{
		std::string strPhone = stSMsgReq.set_phone_req().phone();	
		std::string strPhoneVC = stSMsgReq.set_phone_req().vc();	

		if(!check_phonevc_session(strPhone, strPhoneVC))
		{
			header->set_result(::hoosho::msg::E_VERIFYCODE_INVALID);
		    LOG4CPLUS_ERROR(logger, "ClientProcessor::process_input failed, E_VERIFYCODE_INVALID, please retry!");		
			g_client_processor->send_datagram(conn.get_id(), stRespMsg);
			return;
		}
	}
	

	FsmBusiness* fsm = g_fsm_business_container->new_fsm();
	if(!fsm)
	{
		LOG4CPLUS_ERROR(logger, "ClientProcessor::process_input failed, new FsmBusiness failed");
		conn.close();
		return;
	}

	fsm->_self_user_info.CopyFrom(stUserInfo);
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
	
	LOG4CPLUS_DEBUG(logger, "send_datagram, one msg:"<<stMsg.Utf8DebugString());
	conn->write(datagram);
	
	return 0;
}

void ClientProcessor::add_login_session(const std::string& strAuthCode, const hoosho::msg::s::UserInfo& stUserInfo, uint64_t qwExpireTs)
{
	LOG4CPLUS_DEBUG(logger, "authCode=" << strAuthCode << ", userInfo=" << stUserInfo.Utf8DebugString());
	//check in _openid_2_authcode_map, if exist then delete first
	Openid2AuthCodeMapIterator ocIter = _openid_2_authcode_map.find(stUserInfo.openid());
	if(ocIter != _openid_2_authcode_map.end())
	{
		//erase old code--->openid
		LoginSessionInfoMapIterator tmpIter = _login_session_map.find(ocIter->second);
		if(tmpIter != _login_session_map.end())
		{
			_login_session_map.erase(tmpIter);
		}

		//set new code
		ocIter->second = strAuthCode;

		//if new code has been used by other user, erase
		tmpIter = _login_session_map.find(ocIter->second);
		if(tmpIter != _login_session_map.end() && tmpIter->second._user_info.openid() != stUserInfo.openid())
		{
			ocIter = _openid_2_authcode_map.find(tmpIter->second._user_info.openid());
			if(ocIter != _openid_2_authcode_map.end())
			{
				_openid_2_authcode_map.erase(ocIter);
			}

			_login_session_map.erase(tmpIter);
		}
	}
	else
	{
		_openid_2_authcode_map.insert(std::make_pair(stUserInfo.openid(), strAuthCode));
	}

	//add new session
	time_t now = time(0);

	if(qwExpireTs == 0)
		qwExpireTs = now + g_server->config().get_int_param("LOGIN_SESSION", "life", 0);

	LoginSessionInfoMapIterator iter = _login_session_map.find(strAuthCode);
	if(iter == _login_session_map.end())
	{
		//build session		
		LoginSessionInfo stLoginSessionInfo(stUserInfo, qwExpireTs);
		_login_session_map.insert(std::make_pair(strAuthCode, stLoginSessionInfo));
	}
	else
	{
		iter->second._expire_ts = qwExpireTs;		
	}

	return;
}

int ClientProcessor::check_login_session(const std::string& strAuthCode, hoosho::msg::s::UserInfo& stUserInfo)
{
	LoginSessionInfoMapIterator iter = _login_session_map.find(strAuthCode);
	if(iter == _login_session_map.end())
	{
		LOG4CPLUS_ERROR(logger, "login session not exists");
		return USER_LOGIN_STATUS_NOT_EXIST;
	}

	uint64_t now = time(0);
	LoginSessionInfo& stLoginSessionInfo = iter->second;
	if(now >= stLoginSessionInfo._expire_ts)
	{
		//if expired, delete
		Openid2AuthCodeMapIterator ocIter = _openid_2_authcode_map.find(stLoginSessionInfo._user_info.openid());
		if(ocIter != _openid_2_authcode_map.end())
		{
			_openid_2_authcode_map.erase(ocIter);
		}
		_login_session_map.erase(iter);
		
		LOG4CPLUS_DEBUG(logger, "login session expired");
		return USER_LOGIN_STATUS_EXPIRED;
	}	
	
	stUserInfo.CopyFrom(stLoginSessionInfo._user_info);	
	stLoginSessionInfo._expire_ts = now + g_server->config().get_int_param("LOGIN_SESSION", "life", 0);
	return USER_LOGIN_STATUS_SUCCESS;
}

void ClientProcessor::add_phonevc_session(const std::string& strPhone, const std::string& strPhoneVC)
{
	time_t now = time(0);
	uint64_t qwSessionExpireTS = now + g_server->config().get_int_param("PHONEVC_SESSION", "life", 0);
	
	Phone2VerifyCodeMapIterator vcIter = _phone_2_verifycod_map.find(strPhone);
	if(vcIter != _phone_2_verifycod_map.end())
	{
		//update
		vcIter->second._vc = strPhoneVC;	
		vcIter->second._expire_ts = qwSessionExpireTS;
	}
	else
	{
		//build session		
		PhoneVCSessionInfo stPhoneVCSessionInfo;
		stPhoneVCSessionInfo._vc = strPhoneVC;		
		stPhoneVCSessionInfo._expire_ts = qwSessionExpireTS;
		_phone_2_verifycod_map.insert(std::make_pair(strPhone, stPhoneVCSessionInfo));
	}
	
}

void ClientProcessor::delete_phonevc_session(const std::string& strPhone)
{
	Phone2VerifyCodeMapIterator vcIter = _phone_2_verifycod_map.find(strPhone);
	if(vcIter != _phone_2_verifycod_map.end())
	{
		//delete
		_phone_2_verifycod_map.erase(vcIter);
	}
}

bool ClientProcessor::check_phonevc_session(const std::string& strPhone, const std::string& strPhoneVC)
{
	Phone2VerifyCodeMapIterator vcIter = _phone_2_verifycod_map.find(strPhone);
	if(vcIter == _phone_2_verifycod_map.end())
	{
		LOG4CPLUS_DEBUG(logger, "phonevc session not exists");
		return false;
	}

	uint64_t now = time(0);
	PhoneVCSessionInfo& stPhoneVCSessionInfo = vcIter->second;
	if(now >= stPhoneVCSessionInfo._expire_ts)
	{
		LOG4CPLUS_DEBUG(logger, "phonevc session expired");
		_phone_2_verifycod_map.erase(vcIter);
		return false;
	}	

	if(strcmp(strPhoneVC.c_str(), stPhoneVCSessionInfo._vc.c_str()))
	{
		LOG4CPLUS_DEBUG(logger, "phonevc session._vc: "<<stPhoneVCSessionInfo._vc<<" != req.phonevc: "<<strPhoneVC);
		return false;
	}
	
	stPhoneVCSessionInfo._expire_ts = now + g_server->config().get_int_param("PHONEVC_SESSION", "life", 0);
	return true;
}


void ClientProcessor::process_output()
{

}


