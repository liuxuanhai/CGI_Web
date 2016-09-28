#include "cgi_wx_base.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


IMPL_LOGGER(CgiWXBase, logger);

bool CgiWXBase::DerivedInit()
{

	GetConfig().GetValue("token_server", "ip", m_token_server_ip, "");
	GetConfig().GetValue("token_server", "port", m_token_server_port, 0);

	return true;
}

bool CgiWXBase::Process()
{
	LOG4CPLUS_DEBUG(logger, "begin cgi ----------------- "<<GetCgiName()<<"----------------- ");

	LOG4CPLUS_DEBUG(logger, "query_string="<<(GetInput().GetQueryString()));
	LOG4CPLUS_DEBUG(logger, "post_data="<<(GetInput().GetPostData()));

	

	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();

	LOG4CPLUS_DEBUG(logger, "end cgi ----------------- "<<GetCgiName()<<"----------------- \n\n\n");

	return true;
}

int CgiWXBase::PAGetAccessToken(const std::string &strAppid, std::string &strPAAccessToken)
{
	/*

	std::string strErrMsg = "";
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;
	::common::protoio::ProtoIOTcpClient ioclient(m_token_server_ip, m_token_server_port);

	::hoosho::msg::MsgHead *stRequestMsgHead = stRequestMsg.mutable_head();
	stRequestMsgHead->set_cmd(::hoosho::msg::TOKEN_GET_PA_BASE_ACCESS_TOKEN_REQ);
	stRequestMsgHead->set_seq(time(NULL));

	::hoosho::token::GetPABaseAccessTokenReq* stTokenGetPaBaseAccessTokenReq =
			stRequestMsg.mutable_token_get_pa_base_access_token_req();
	stTokenGetPaBaseAccessTokenReq->set_pa_appid(strAppid);

	int iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
	if(iRet < 0)
	{
		LOG4CPLUS_ERROR(logger, "PAGetAccessToken failed, protoio failed, msg="<<strErrMsg);
		return -1;
	}

	LOG4CPLUS_DEBUG(logger, "Response msg="<<stResponseMsg.Utf8DebugString());
	const ::hoosho::msg::MsgHead stResponseMsgHead = stResponseMsg.head();
	if(stResponseMsgHead.cmd() != ::hoosho::msg::TOKEN_GET_PA_BASE_ACCESS_TOKEN_RES)
	{
		LOG4CPLUS_ERROR(logger, "PAGetAccessToken failed, proto io response cmd="<<stResponseMsgHead.cmd());
		return -1;
	}
	if(stRequestMsgHead->result() != ::hoosho::msg::E_OK)
	{
		LOG4CPLUS_ERROR(logger, "PAGetAccessToken failed, proto io response result="<<stRequestMsgHead->result());
		return -1;
	}
	strPAAccessToken = stResponseMsg.token_get_pa_base_access_token_res().pa_access_token();

	if(strPAAccessToken.empty())
	{
		LOG4CPLUS_ERROR(logger, "PAGetAccessToken succ, but PAAccessToken is empty, maybe invalid appid="<<strAppid);
		return -1;
	}
	LOG4CPLUS_DEBUG(logger, "PAGetAccessToken succ, PAAccessToken="<<strPAAccessToken);
	 */
	return 0;
}