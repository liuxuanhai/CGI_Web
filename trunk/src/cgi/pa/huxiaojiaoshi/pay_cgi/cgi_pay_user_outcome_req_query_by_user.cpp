#include "cgi_pay_server.h"
#include "../cgi_common_util.h"

class CgiPayOutcomeReqQueryByUser: public CgiPayServer
{
public:
	CgiPayOutcomeReqQueryByUser():
		CgiPayServer(0, "config.ini", "logger.properties", 1)
	{		
	}
	
	bool InnerProcess()
	{
		//1.get HTTP params
		string openid = m_cookie_value_key;		
		uint64_t limit = strtoul(((string)GetInput().GetValue("limit")).c_str(), NULL, 10);
		uint64_t begin_ts = strtoul(((string)GetInput().GetValue("begin_ts")).c_str(), NULL, 10);
		
		if(limit < 0 || limit > 30)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID PARAM limit=" << limit);
			return true;
		}

		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::J_PAY_OUTCOME_QUERY_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::j::pay::QueryOutcomeReq* stQueryOutcomeReq = stRequestMsg.mutable_query_outcome_req();
		stQueryOutcomeReq->set_openid(openid);
		stQueryOutcomeReq->set_state(0);
		stQueryOutcomeReq->set_limit(limit);
		stQueryOutcomeReq->set_begin_ts(begin_ts);		

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
		if(stHead.cmd() != ::hoosho::msg::J_PAY_OUTCOME_QUERY_RES)
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
		const ::hoosho::j::pay::QueryOutcomeRes& queryOutcomeRes = stResponseMsg.query_outcome_res();

		lce::cgi::CAnyValue stAnyValue;
		for(int i=0; i<queryOutcomeRes.outcome_req_info_list_size(); i++)
		{
			lce::cgi::CAnyValue item;
			const ::hoosho::j::commstruct::OutcomeReqInfo& outcome_req_info = queryOutcomeRes.outcome_req_info_list(i);
			item["table_id"] = int_2_str(outcome_req_info.table_id());
			item["openid"] = int_2_str(outcome_req_info.openid());
			item["amount"] = int_2_str(outcome_req_info.amount());
			item["state"] = int_2_str(outcome_req_info.state());
			item["create_ts"] = int_2_str(outcome_req_info.create_ts());

			stAnyValue.push_back(item);
		}

		GetAnyValue()["outcome_req_info_list"] = stAnyValue;
		
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
	
};

int main(int argc, char** argv)
{
	CgiPayOutcomeReqQueryByUser cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

