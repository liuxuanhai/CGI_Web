#include "cgi_order_base.h"
#include "msg.pb.h"

class CgiOrderSearch:public CgiOrderBase
{
public:
	CgiOrderSearch():CgiOrderBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		std::string strCode = GetInput().GetValue("code");
		uint64_t qwLongitude = strtoul(((string)GetInput().GetValue("addr_longitude")).c_str(), NULL, 10);
		uint64_t qwLatitude = strtoul(((string)GetInput().GetValue("addr_latitude")).c_str(), NULL, 10);

		EMPTY_STR_RETURN(strCode);
		ZERO_INT_RETURN(qwLongitude);
		ZERO_INT_RETURN(qwLatitude);

		::hoosho::msg::Msg stReqMsg;
		::hoosho::msg::Msg stResMsg;

		::hoosho::msg::MsgHead* pReqMsgHead = stReqMsg.mutable_head();
		pReqMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pReqMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stReqMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::SEARCH_ORDER_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::SearchOrderReq* pSearchOrderReq = pZMsgReq->mutable_seach_order_req();
		pSearchOrderReq->set_addr_longitude(qwLongitude);
		pSearchOrderReq->set_addr_latitude(qwLatitude);

		std::string strErrMsg;
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
//		LOG4CPLUS_DEBUG(logger, stResMsg.Utf8DebugString());

		const ::hoosho::msg::z::MsgRes stZMsgRes = stResMsg.z_msg_res();

		for(int i = 0; i < stZMsgRes.order_info_list_size(); i++)
		{
			lce::cgi::CAnyValue stAnyValue;
			LOG4CPLUS_DEBUG(logger, stZMsgRes.order_info_list(i).Utf8DebugString());
			OrderInfoPB2Any(stZMsgRes.order_info_list(i), stAnyValue);
			GetAnyValue()["order_list"].push_back(stAnyValue);
		}

		for(int i = 0; i < stZMsgRes.user_info_list_size(); i++)
		{
			lce::cgi::CAnyValue stAnyValue;
			LOG4CPLUS_DEBUG(logger, stZMsgRes.user_info_list(i).Utf8DebugString());
			UserInfoPB2Any(stZMsgRes.user_info_list(i), stAnyValue);
			GetAnyValue()["user_info_list"][stZMsgRes.user_info_list(i).openid()] = stAnyValue;
		}

		const ::hoosho::msg::z::SearchOrderRes stSearchOrderRes = stZMsgRes.search_order_res();
		for(int i = 0; i < stSearchOrderRes.user_extra_info_list_size(); i++)
		{
			lce::cgi::CAnyValue item, stAnyValue;
			const ::hoosho::msg::z::UserExtraInfo& stUserExtraInfo = stSearchOrderRes.user_extra_info_list(i);
			UserExtraInfoPB2Any(stUserExtraInfo, stAnyValue);
			item[stUserExtraInfo.openid()] = stAnyValue;					
			GetAnyValue()["user_extra_info_list"].push_back(item);
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiOrderSearch cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
