#include "cgi_pay_server.h"
#include "../cgi_common_util.h"

class CgiPayUnifiedOrder: public CgiPayServer
{
public:
	CgiPayUnifiedOrder():
		CgiPayServer(0, "config.ini", "logger.properties", 1)
	{		
	}
	
	bool InnerProcess()
	{
		//1.get HTTP params
		string openid = m_cookie_value_key;
		uint64_t type = strtoul(((string)GetInput().GetValue("type")).c_str(), NULL, 10);	
		uint64_t business_id = strtoul(((string)GetInput().GetValue("business_id")).c_str(), NULL, 10);
		string client_ip = (string)GetInput().GetClientIPStr();

		if(openid.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "NO OPENID IN COOKIE");
			return true;
		}
		
		if((type != 1) && (type != 2))
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID PARAM");
			return true;
		}

		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::J_PAY_UNIFIEDORDER_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::j::pay::UnifiedOrderReq* stUnifiedOrderReq = stRequestMsg.mutable_unifiedorder_req();
		stUnifiedOrderReq->set_openid(openid);
		stUnifiedOrderReq->set_type(type);
		stUnifiedOrderReq->set_business_id(business_id);
		stUnifiedOrderReq->set_client_ip(client_ip);

		//3.send to server, and recv responseMsg protobuf
		common::protoio::ProtoIOTcpClient ioclient(PayServerIP, PayServerPort);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

		if(iRet != 0)
		{			
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "response Msg: \n" << stResponseMsg.Utf8DebugString());	
		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponseMsg.head();
		if(stHead.cmd() != ::hoosho::msg::J_PAY_UNIFIEDORDER_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;	
		}

		//4.Build strResponse Json from responseMsg protobuf
		const ::hoosho::j::pay::UnifiedOrderRes& unifiedOrderRes = stResponseMsg.unifiedorder_res();
		GetAnyValue()["appid"] = unifiedOrderRes.appid();
		GetAnyValue()["timestamp"] = unifiedOrderRes.timestamp();
		GetAnyValue()["nonceStr"] = unifiedOrderRes.nonce_str();
		GetAnyValue()["package"] = unifiedOrderRes.package();
		GetAnyValue()["signType"] = unifiedOrderRes.sign_type();
		GetAnyValue()["paySign"] = unifiedOrderRes.pay_sign();
		GetAnyValue()["out_trade_no"] = unifiedOrderRes.out_trade_no();		
		
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
	
};

int main(int argc, char** argv)
{
	CgiPayUnifiedOrder cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

