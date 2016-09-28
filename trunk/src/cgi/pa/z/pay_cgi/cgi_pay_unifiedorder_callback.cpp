#include "cgi/cgi.h"
#include "util/lce_util.h"
#include "../cgi_common_util.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>


class CgiPayUnifiedOrderCallback: public lce::cgi::Cgi
{
public:
	std::string m_z_server_ip;
	uint16_t m_z_server_port;

public:
	CgiPayUnifiedOrderCallback(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
		:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{		
	}

	virtual bool DerivedInit()
	{
		GetConfig().GetValue("z_server", "ip", m_z_server_ip, "");
		GetConfig().GetValue("z_server", "port", m_z_server_port, 0);
		return true;
	}
	
	void AlwaysReturnOK()
	{
		std::string xml_code="<xml> \n";
		xml_code += "<return_code><![CDATA[SUCCESS]]></return_code> \n";
		xml_code += "<return_msg><![CDATA[OK]]></return_msg> \n";
		xml_code += "</xml>";
		
		GetHeader().SetContentType("text/xml");
		GetHeader().Output();
		if(!std::cout.good())
		{
			std::cout.clear();
			std::cout.flush();
		}

		std::cout<<xml_code;		
		return;
	}
	
	virtual bool Process()
	{
		//0. AlwaysReturnOK
		AlwaysReturnOK();
		
		//1.get HTTP params
		string callbackXML = (string)GetInput().GetPostData();		
		LOG4CPLUS_DEBUG(logger, "callbackXML: \n" << callbackXML);	
		
		//2.Build requestMsg protobuf
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		stRequestMsgHead->set_seq(time(NULL));
		::hoosho::msg::z::MsgReq* pZMsgReq = stRequestMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::WX_PAY_CALLBACK_NTF);
		::hoosho::msg::z::WXPayCallBackNtf* pWXPayCallBackNtf = pZMsgReq->mutable_wx_pay_callback_ntf();
		pWXPayCallBackNtf->set_xml(callbackXML);


		//3.send to server, and recv responseMsg protobuf
		std::string strErrMsg = "";
		common::protoio::ProtoIOTcpClient ioclient(m_z_server_ip, m_z_server_port);
		if(ioclient.notify(stRequestMsg, strErrMsg) != 0)
		{			
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient Ntf failed, errmsg = " << strErrMsg);
		}

		return true;
	}

protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiPayUnifiedOrderCallback, logger);

int main(int argc, char** argv)
{
	CgiPayUnifiedOrderCallback cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

