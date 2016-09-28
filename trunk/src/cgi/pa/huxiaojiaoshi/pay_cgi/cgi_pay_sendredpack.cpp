#include "cgi_pay_server.h"
#include "../cgi_common_util.h"

class CgiPaySendRedPakc: public CgiPayServer
{
public:
	CgiPaySendRedPakc():
		CgiPayServer(0, "config.ini", "logger.properties", 0)
	{		
	}
	
	bool InnerProcess()
	{
		//1.get HTTP params
		string openid = (string)GetInput().GetValue("openid");
		uint64_t amount = strtoul(((string)GetInput().GetValue("amount")).c_str(), NULL, 10);		
		string client_ip = (string)GetInput().GetClientIPStr();
		uint64_t table_id = strtoul(((string)GetInput().GetValue("table_id")).c_str(), NULL, 10);	

		if(openid.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "NO OPENID IN COOKIE");
			return true;
		}

		// 最小提现为 1元 = 100分 (单位: 分)
		if((amount < 100) || (amount%100 != 0))	
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID PARAM, amount: " << amount << " < 100");
			return true;
		}

		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::J_PAY_SENDREDPACK_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::j::pay::SendRedPackReq* stSendRedPackReq = stRequestMsg.mutable_send_redpack_req();
		stSendRedPackReq->set_openid(openid);
		stSendRedPackReq->set_amount(amount);
		stSendRedPackReq->set_client_ip(client_ip);
		stSendRedPackReq->set_table_id(table_id);

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
		if(stHead.cmd() != ::hoosho::msg::J_PAY_SENDREDPACK_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			if(stHead.result() == ::hoosho::msg::E_PAY_FAIL)
			{
				DoReply(CGI_RET_CODE_OUTCOME_FAIL);
			}
			else
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
			}
			
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;	
		}

		//4.Build strResponse Json from responseMsg protobuf
		const ::hoosho::j::pay::SendRedPackRes& sendRedPackRes = stResponseMsg.send_redpack_res();
		GetAnyValue()["total_income"] = int_2_str(sendRedPackRes.total_income());
		GetAnyValue()["total_outcome"] = int_2_str(sendRedPackRes.total_outcome());	
		GetAnyValue()["total_outcome_pending"] = int_2_str(sendRedPackRes.total_outcome_pending());	
		
		
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
	
};

int main(int argc, char** argv)
{
	CgiPaySendRedPakc cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

