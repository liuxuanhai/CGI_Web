#include "cgi_order_base.h"
#include "msg.pb.h"

class CgiOrderGetUserFetchOrderList:public CgiOrderBase
{
public:
	CgiOrderGetUserFetchOrderList():CgiOrderBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");
		string strOpenid = GetInput().GetValue("openid");
		uint64_t qwBeginOrderid = strtoul(((string)GetInput().GetValue("begin_orderid")).c_str(), NULL, 10);
		uint32_t dwLimit = strtoul(((string)GetInput().GetValue("limit")).c_str(), NULL, 10);

		EMPTY_STR_RETURN(strCode);
		ZERO_INT_RETURN(dwLimit);

		::hoosho::msg::Msg stReqMsg;
		::hoosho::msg::Msg stResMsg;

		::hoosho::msg::MsgHead* pReqMsgHead = stReqMsg.mutable_head();
		pReqMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pReqMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stReqMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_USER_FETCH_ORDER_LIST_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetUserFetchOrderListReq* pGetUserFetchOrderListReq = pZMsgReq->mutable_get_user_fetch_order_list_req();
		pGetUserFetchOrderListReq->set_openid(strOpenid);
		pGetUserFetchOrderListReq->set_begin_orderid(qwBeginOrderid);
		pGetUserFetchOrderListReq->set_limit(dwLimit);

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

		const ::hoosho::msg::z::MsgRes stZMsgRes = stResMsg.z_msg_res();
		
		for(int i = 0; i < stZMsgRes.get_user_fetch_order_list_res().order_openid_fetch_state_list_size(); ++i)
		{
			lce::cgi::CAnyValue any;
			OrderFetchStatePB2Any(stZMsgRes.get_user_fetch_order_list_res().order_openid_fetch_state_list(i), any);
			GetAnyValue()["fetch_list"].push_back(any);
		}
		for(int i = 0; i < stZMsgRes.order_info_list_size(); i++)
		{
			lce::cgi::CAnyValue any;
			OrderInfoPB2Any(stZMsgRes.order_info_list(i), any);
			GetAnyValue()["order_list"].push_back(any);
		}
		for(int i = 0; i < stZMsgRes.user_info_list_size(); i++)
		{
			lce::cgi::CAnyValue any;
			LOG4CPLUS_DEBUG(logger, stZMsgRes.user_info_list(i).Utf8DebugString());
			UserInfoPB2Any(stZMsgRes.user_info_list(i), any);
			GetAnyValue()["user_list"].push_back(any);
		}

		const ::hoosho::msg::z::GetUserFetchOrderListRes stGetUserFetchOrderListRes = stZMsgRes.get_user_fetch_order_list_res();
		for(int i = 0; i < stGetUserFetchOrderListRes.user_extra_info_list_size(); i++)
		{
			lce::cgi::CAnyValue item, stAnyValue;
			const ::hoosho::msg::z::UserExtraInfo& stUserExtraInfo = stGetUserFetchOrderListRes.user_extra_info_list(i);
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
	CgiOrderGetUserFetchOrderList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
