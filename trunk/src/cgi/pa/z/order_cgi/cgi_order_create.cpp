#include "cgi_order_base.h"
#include "msg.pb.h"

class CgiOrderCreate:public CgiOrderBase
{
public:
	CgiOrderCreate():CgiOrderBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");

		uint32_t dwOrderVisible = strtoul(((string)GetInput().GetValue("order_visible")).c_str(), NULL, 10);
		uint64_t qwAddrToLongitude = strtoul(((string)GetInput().GetValue("addr_to_longitude")).c_str(), NULL, 10);
		uint64_t qwAddrToLatitude = strtoul(((string)GetInput().GetValue("addr_to_latitude")).c_str(), NULL, 10);
		string strAddrTo = GetInput().GetValue("addr_to");
		string strAddrToDetail = GetInput().GetValue("addr_to_detail");
		string strOrderDesc = GetInput().GetValue("order_desc");
		string strOrderReceiverName = GetInput().GetValue("order_receiver_name");
		string strOrderReceiverPhone = GetInput().GetValue("order_receiver_phone");
		int32_t dwPrice = strtol(((string)GetInput().GetValue("price")).c_str(), NULL, 10);
		uint32_t qwExpectFromTs = strtoul(((string)GetInput().GetValue("expect_from_ts")).c_str(), NULL, 10);
		uint32_t qwExpectToTs = strtoul(((string)GetInput().GetValue("expect_to_ts")).c_str(), NULL, 10);
		string strUserIP = (string)GetInput().GetClientIPStr();

		uint32_t dwOrderType = atoi(((string(GetInput().GetValue("order_type")).c_str())));
		uint32_t dwMediaType = atoi(((string(GetInput().GetValue("media_type")).c_str())));
		if(dwOrderType == 0)
		{
			dwOrderType = 1;
		}
		if(dwMediaType == 0)
		{
			dwMediaType = 1;
		}
		
		std::string strMediaServerId = "";
		if(dwMediaType == ORDER_MEDIA_TYPE_AUDIO || dwMediaType == ORDER_MEDIA_TYPE_PICTURE)
		{
			strMediaServerId = (string)GetInput().GetValue("media_server_id");
			EMPTY_STR_RETURN(strMediaServerId);
		}

		string strExtraData = GetInput().GetValue("extra_data");

		EMPTY_STR_RETURN(strCode);
		ZERO_INT_RETURN(dwOrderVisible);
		//ZERO_INT_RETURN(qwAddrToLongitude);
		//ZERO_INT_RETURN(qwAddrToLatitude);
		EMPTY_STR_RETURN(strAddrTo);
//		EMPTY_STR_RETURN(strAddrToDetail);
//		EMPTY_STR_RETURN(strOrderDesc);
		if(strOrderDesc.empty())
		{
			strOrderDesc = "ziga";
		}
//		EMPTY_STR_RETURN(strOrderReceiverName);
//		EMPTY_STR_RETURN(strOrderReceiverPhone);
		NEGATIVE_INT_RETURN(dwPrice);
		ZERO_INT_RETURN(qwExpectFromTs);
		ZERO_INT_RETURN(qwExpectToTs);

		::hoosho::msg::Msg stReqMsg;
		::hoosho::msg::Msg stResMsg;

		::hoosho::msg::MsgHead* pReqMsgHead = stReqMsg.mutable_head();
		pReqMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pReqMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stReqMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::CREATE_ORDER_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::CreateOrderReq* pCreateOrderReq = pZMsgReq->mutable_create_order_req();
		::hoosho::msg::z::OrderInfo* pOrderInfo = pCreateOrderReq->mutable_order_info();
		pOrderInfo->set_order_visible(dwOrderVisible);
		pOrderInfo->set_addr_to_longitude(qwAddrToLongitude);
		pOrderInfo->set_addr_to_latitude(qwAddrToLatitude);
		pOrderInfo->set_addr_to(strAddrTo);
		pOrderInfo->set_addr_to_detail(strAddrToDetail);
		pOrderInfo->set_order_desc(strOrderDesc);
        pOrderInfo->set_order_receiver_name(strOrderReceiverName);
		pOrderInfo->set_order_receiver_phone(strOrderReceiverPhone);
		pOrderInfo->set_price(dwPrice);
		pOrderInfo->set_expect_from_ts(qwExpectFromTs);
		pOrderInfo->set_expect_to_ts(qwExpectToTs);
		pOrderInfo->set_order_type(dwOrderType);
		pOrderInfo->set_media_type(dwMediaType);
		pOrderInfo->set_extra_data(strExtraData);

		pCreateOrderReq->set_user_ip(strUserIP);
		pCreateOrderReq->set_media_server_id(strMediaServerId);

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
		const ::hoosho::msg::z::CreateOrderRes stCreateOrderRes = stZMsgRes.create_order_res();

		GetAnyValue()["orderid"] = int_2_str(stCreateOrderRes.orderid());
		GetAnyValue()["cost_type"] = stCreateOrderRes.cost_type();
		GetAnyValue()["reward_fee"] = stCreateOrderRes.reward_fee();

		if(stCreateOrderRes.cost_type() == 2)
		{
			const ::hoosho::msg::z::WXUnifiedOrderParam stWXUnifiedOrderParam = stCreateOrderRes.wx_unified_order_param();
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
	CgiOrderCreate cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
