#include "cgi_order_base.h"
#include "msg.pb.h"

class CgiOrderReward:public CgiOrderBase
{
public:
	CgiOrderReward():CgiOrderBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");

		uint64_t qwOrderid = strtoul(((string)GetInput().GetValue("orderid")).c_str(), NULL, 10);
		uint64_t qwAmount = strtol(((string)GetInput().GetValue("amount")).c_str(), NULL, 10);

		string strUserIP = (string)GetInput().GetClientIPStr();

		EMPTY_STR_RETURN(strCode);
		ZERO_INT_RETURN(qwOrderid);
		NOT_POSITIVE_INT_RETURN(qwAmount);
	
		::hoosho::msg::Msg stReqMsg;
		::hoosho::msg::Msg stResMsg;

		::hoosho::msg::MsgHead* pReqMsgHead = stReqMsg.mutable_head();
		pReqMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pReqMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stReqMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::REWARD_ORDER_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::RewardOrderReq* pRewardOrderReq = pZMsgReq->mutable_reward_order_req();
		pRewardOrderReq->set_orderid(qwOrderid);
		pRewardOrderReq->set_amount(qwAmount);
		pRewardOrderReq->set_user_ip(strUserIP);

		string strErrMsg;
		int iRet;

		::common::protoio::ProtoIOTcpClient ioclient(m_z_server_ip, m_z_server_port);
		iRet = ioclient.io(stReqMsg, stResMsg, strErrMsg);

		if(iRet != 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			return true;
		}

		const ::hoosho::msg::MsgHead& msgHead = stResMsg.head();
		if(msgHead.cmd() != ::hoosho::msg::Z_PROJECT_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd = " << msgHead.cmd() << " unknow");
			return true;
		}
		SERVER_NOT_OK_RETURN(msgHead.result());
		LOG4CPLUS_DEBUG(logger, stResMsg.Utf8DebugString());

		const ::hoosho::msg::z::MsgRes stZMsgRes = stResMsg.z_msg_res();
		const ::hoosho::msg::z::RewardOrderRes stRewardOrderRes = stZMsgRes.reward_order_res();

		GetAnyValue()["cost_type"] = stRewardOrderRes.cost_type();

		if(stRewardOrderRes.cost_type() == 2)
		{
			const ::hoosho::msg::z::WXUnifiedOrderParam stWXUnifiedOrderParam = stRewardOrderRes.wx_unified_order_param();
			GetAnyValue()["wx_unified_order_param"]["appid"] = stWXUnifiedOrderParam.appid();
			GetAnyValue()["wx_unified_order_param"]["timestamp"] = stWXUnifiedOrderParam.timestamp();
			GetAnyValue()["wx_unified_order_param"]["nonceStr"] = stWXUnifiedOrderParam.noncestr();
			GetAnyValue()["wx_unified_order_param"]["package"] = stWXUnifiedOrderParam.package();
			GetAnyValue()["wx_unified_order_param"]["signType"] = stWXUnifiedOrderParam.signtype();
			GetAnyValue()["wx_unified_order_param"]["paySign"] = stWXUnifiedOrderParam.paysign();
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiOrderReward cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
