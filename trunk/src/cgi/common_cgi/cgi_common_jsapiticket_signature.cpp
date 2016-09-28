#ifndef _HOOSHO_CGI_COMMON_JSAPI_SIGNATURE_H_
#define _HOOSHO_CGI_COMMON_JSAPI_SIGNATURE_H_

#include "cgi/cgi.h"
#include "../cgi_ret_code_def.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>

class CgiCommonJSAPISignature: public lce::cgi::Cgi
{
public:
	 CgiCommonJSAPISignature(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("pa_server", "ip", m_pa_server_ip, "");
		GetConfig().GetValue("pa_server", "port", m_pa_server_port, 0);
		return true;
	}
	
	virtual bool Process()
	{
		std::string strReqUrl = (std::string)GetInput().GetValue("url");
		if(strReqUrl.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strReqUrl is empty");
			return true;	
		}

		LOG4CPLUS_DEBUG(logger, "url="<<strReqUrl);
		size_t pos = strReqUrl.find("#");
		if(pos != std::string::npos)
		{
			strReqUrl = strReqUrl.substr(0, pos);
			LOG4CPLUS_DEBUG(logger, "# found, get split('#')[0] = "<<strReqUrl);	
		}
		

		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::GET_JSAPI_SGIN_REQ);
		pHead->set_seq(time(0));
		::hoosho::pa::GetJSAPISignReq* pGetJSAPISignReq = stRequest.mutable_get_jsapisign_req();
		pGetJSAPISignReq->set_req_url(strReqUrl);

		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_pa_server_ip, m_pa_server_port);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return true;	
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::GET_JSAPI_SGIN_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;	
		}

		if(stHead.seq() != pHead->seq())
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.seq="<<stHead.seq()<<", while request.seq="<<(pHead->seq()));
			return true;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;	
		}

		const ::hoosho::pa::GetJSAPISignRes& stGetJSAPISignRes = stResponse.get_jsapisign_res();
		lce::cgi::CAnyValue stAnyValue;
		stAnyValue["url"] = strReqUrl;
		stAnyValue["appid"] = stGetJSAPISignRes.pa_appid();
		stAnyValue["sign"] = stGetJSAPISignRes.sign();
		stAnyValue["nonce"] = stGetJSAPISignRes.nonce();
		stAnyValue["timestamp"] = stGetJSAPISignRes.timestamp();

		GetAnyValue()["data"] = stAnyValue;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	string m_pa_server_ip;
	int m_pa_server_port;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiCommonJSAPISignature, logger);

int main(int argc, char** argv)
{
	CgiCommonJSAPISignature cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

