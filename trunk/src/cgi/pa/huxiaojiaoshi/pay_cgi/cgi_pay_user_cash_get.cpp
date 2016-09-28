#include "cgi_pay_server.h"
#include "../cgi_common_util.h"

class CgiPayUserCashGet: public CgiPayServer
{
public:
	CgiPayUserCashGet():
		CgiPayServer(0, "config.ini", "logger.properties", 1)
	{		
	}	
	
	bool InnerProcess()
	{
		//1.get HTTP params
		string openid = m_cookie_value_key;

		if(openid.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "NO OPENID IN COOKIE");
			return true;
		}		

		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::J_PAY_GET_USER_CASH_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::j::pay::GetUserCashReq* stGetUserCashReq = stRequestMsg.mutable_get_user_cash_req();
		stGetUserCashReq->set_openid(openid);

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
		if(stHead.cmd() != ::hoosho::msg::J_PAY_GET_USER_CASH_RES)
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
		const ::hoosho::j::pay::GetUserCashRes& getUserCashRes = stResponseMsg.get_user_cash_res();
		GetAnyValue()["total_income"] = int_2_str(getUserCashRes.total_income());
		GetAnyValue()["total_outcome"] = int_2_str(getUserCashRes.total_outcome());		
		GetAnyValue()["total_outcome_pending"] = int_2_str(getUserCashRes.total_outcome_pending());

		//5.Fetch UserInfo
		std::set<std::string> setOpenid;
		setOpenid.insert(openid);		
		std::map<std::string, lce::cgi::CAnyValue> mapResult;
		
		strErrMsg = "";
		if(FetchUserInfo(setOpenid, mapResult, UserServerIP, UserServerPort, strErrMsg) < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return true;
		}		

		GetAnyValue()["user_info"] = mapResult.begin()->second;		
		
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
	
};

int main(int argc, char** argv)
{
	CgiPayUserCashGet cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

