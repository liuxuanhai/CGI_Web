#include "cgi_pay_server.h"
#include "../cgi_common_util.h"

class CgiPayUnifiedOrderCallback: public CgiPayServer
{
public:
	CgiPayUnifiedOrderCallback():
		CgiPayServer(0, "config.ini", "logger.properties", 0)
	{		
	}

	void ReturnCode(std::string return_code="SUCCESS", std::string return_msg="OK")
	{
		std::string xml_code="<xml> \n";
		xml_code += "<return_code><![CDATA[" + return_code + "]]></return_code> \n";
		xml_code += "<return_msg><![CDATA[" + return_msg + "]]></return_msg> \n";
		xml_code += "</xml>";
		
		std::ostringstream contentLength;
		contentLength.str("");
		contentLength<<"Content-Length: "<<xml_code.size();
		
		GetHeader().SetContentType("text/xml");
		GetHeader().AddHeader(contentLength.str());
		GetHeader().Output();

		if(!std::cout.good())
		{
			std::cout.clear();
			std::cout.flush();
		}

		std::cout<<xml_code;		
		return;
	}
	
	bool InnerProcess()
	{
		//1.get HTTP params
		string callbackXML = (string)GetInput().GetPostData();		
		string client_ip = (string)GetInput().GetClientIPStr();

		if(callbackXML.empty())
		{
			ReturnCode("FAIL", "HTTP BODY IS EMPTY");
			LOG4CPLUS_ERROR(logger, "UnifiedOrder Callback HTTP body is empty!");
			return true;
		}		
		LOG4CPLUS_DEBUG(logger, "client_ip: \n" << client_ip);	
		LOG4CPLUS_DEBUG(logger, "callbackXML: \n" << callbackXML);	
		
		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::J_PAY_UNIFIEDORDER_CALLBACK_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::j::pay::UnifiedOrderCallbackReq* stUnifiedOrderCallbackReq = stRequestMsg.mutable_unifiedorder_callback_req();
		stUnifiedOrderCallbackReq->set_callback_xml(callbackXML);		
		stUnifiedOrderCallbackReq->set_client_ip(client_ip);

		//3.send to server, and recv responseMsg protobuf
		common::protoio::ProtoIOTcpClient ioclient(PayServerIP, PayServerPort);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

		if(iRet != 0)
		{			
			ReturnCode("SUCCESS", "OK");
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "response Msg: \n" << stResponseMsg.Utf8DebugString());	
		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponseMsg.head();
		if(stHead.cmd() != ::hoosho::msg::J_PAY_UNIFIEDORDER_CALLBACK_RES)
		{
			ReturnCode("SUCCESS", "OK");
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			ReturnCode("SUCCESS", "OK");
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;	
		}

		//4.Build strResponse Json from responseMsg protobuf
		const ::hoosho::j::pay::UnifiedOrderCallbackRes& unifiedOrderCallbackRes = stResponseMsg.unifiedorder_callback_res();

		ReturnCode(unifiedOrderCallbackRes.return_code(), unifiedOrderCallbackRes.return_msg());
		return true;
	}
	
};

int main(int argc, char** argv)
{
	CgiPayUnifiedOrderCallback cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

