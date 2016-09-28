#include "cgi_pay_server.h"
#include "../cgi_common_util.h"

class CgiPayCheckOutTradeNo: public CgiPayServer
{
public:
	CgiPayCheckOutTradeNo():
		CgiPayServer(0, "config.ini", "logger.properties", 0)
	{		
	}
	
	bool InnerProcess()
	{
		//1.get HTTP params
		string openid = (string)GetInput().GetValue("openid");
		string strOutTradeNoList = (string)GetInput().GetValue("out_trade_no_list");		

		if(openid.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "OPENID IS EMPTY");
			return true;
		}
		
		if(strOutTradeNoList.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "OUT_TRADE_NO IS EMPTY");
			return true;
		}

		std::vector<std::string> vecOutTradeNo;
		lce::cgi::Split(strOutTradeNoList, "|", vecOutTradeNo);		

		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::J_PAY_UNIFIEDORDER_QUERY_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::j::pay::UnifiedOrderQueryReq* stUnifiedOrderQueryReq = stRequestMsg.mutable_query_unifiedorder_req();
		stUnifiedOrderQueryReq->set_openid(openid);

		for(size_t i=0; i != vecOutTradeNo.size(); i++)
		{
			stUnifiedOrderQueryReq->add_out_trade_no_list(vecOutTradeNo[i]);		
		}
		

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
		if(stHead.cmd() != ::hoosho::msg::J_PAY_UNIFIEDORDER_QUERY_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			if(stHead.result() == ::hoosho::msg::E_PAY_FAIL)
			{
				//4.Build strResponse Json from responseMsg protobuf	
				const ::hoosho::j::pay::UnifiedOrderQueryRes& stUnifiedOrderQueryRes = stResponseMsg.query_unifiedorder_res();
				
				for(int i=0; i<stUnifiedOrderQueryRes.out_trade_no_list_size(); i++)
				{
					GetAnyValue()["fault_out_trade_no_list"].push_back(stUnifiedOrderQueryRes.out_trade_no_list(i));
				}
				DoReply(CGI_RET_CODE_NO_PAY, "out_trade_no_list check fault!", false);
			}
			else
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
			}
			
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;	
		}		
		
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
	
};

int main(int argc, char** argv)
{
	CgiPayCheckOutTradeNo cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

