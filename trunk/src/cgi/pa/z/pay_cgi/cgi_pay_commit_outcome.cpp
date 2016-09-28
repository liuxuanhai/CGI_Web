#include "cgi/cgi.h"
#include "util/lce_util.h"
#include "../cgi_common_util.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>


class CgiPayCommitOutcome: public lce::cgi::Cgi
{
public:
	std::string m_z_server_ip;
	uint16_t m_z_server_port;

public:
	CgiPayCommitOutcome(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
		:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	{		
	}

	virtual bool DerivedInit()
	{
		GetConfig().GetValue("z_server", "ip", m_z_server_ip, "");
		GetConfig().GetValue("z_server", "port", m_z_server_port, 0);
		return true;
	}
	
	virtual bool Process()
	{
		LOG4CPLUS_INFO(logger, "BEGIN CGI ----------------- "<<GetCgiName()<<"----------------- ");
		if(strcmp(GetInput().GetQueryString(), ""))
		{
			LOG4CPLUS_INFO(logger, "query_string: " << GetInput().GetQueryString());
		}
		if(strcmp(GetInput().GetPostData(), ""))
		{
			LOG4CPLUS_INFO(logger, "post_data: " << GetInput().GetPostData());
		}

		//1.get HTTP params
		string strCode = GetInput().GetValue("code");
		uint32_t dwAmount = strtoul(((string)GetInput().GetValue("amount")).c_str(), NULL, 10);
		EMPTY_STR_RETURN(strCode);
		ZERO_INT_RETURN(dwAmount);
		
		
		//2.Build requestMsg protobuf
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		stRequestMsgHead->set_seq(time(NULL));
		::hoosho::msg::z::MsgReq* pZMsgReq = stRequestMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::COMMIT_OUTCOME_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::CommitOutcomeReq* pCommitOutcomeReq = pZMsgReq->mutable_commit_outcome_req();
		pCommitOutcomeReq->set_amount(dwAmount);

		//3.send to server, and recv responseMsg protobuf
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponseMsg;
		common::protoio::ProtoIOTcpClient ioclient(m_z_server_ip, m_z_server_port);
		if(ioclient.io(stRequestMsg, stResponseMsg, strErrMsg) != 0)
		{			
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient.io  failed, errmsg = " << strErrMsg);
		}
		SERVER_NOT_OK_RETURN(stResponseMsg.head().result());

		const ::hoosho::msg::z::MsgRes zMsgRes = stResponseMsg.z_msg_res();
		const ::hoosho::msg::z::CommitOutcomeRes stCommitOutcomeRes = zMsgRes.commit_outcome_res();
		GetAnyValue()["balance"] = stCommitOutcomeRes.balance();
		GetAnyValue()["pending"] = stCommitOutcomeRes.pending();

		DoReply(CGI_RET_CODE_OK);

		LOG4CPLUS_INFO(logger, "END CGI \n");
		return true;
	}

protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiPayCommitOutcome, logger);

int main(int argc, char** argv)
{
	CgiPayCommitOutcome cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

