#include "cgi_order_base.h"
#include "msg.pb.h"

class CgiOrderGetUserCreateOrderList:public CgiOrderBase
{
public:
	CgiOrderGetUserCreateOrderList():CgiOrderBase(0, "config.ini", "logger.properties")
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
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_USER_CREATE_ORDER_LIST_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetUserCreateOrderListReq* pGetUserCreateOrderListReq = pZMsgReq->mutable_get_user_create_order_list_req();
		pGetUserCreateOrderListReq->set_openid(strOpenid);
		pGetUserCreateOrderListReq->set_begin_orderid(qwBeginOrderid);
		pGetUserCreateOrderListReq->set_limit(dwLimit);

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

		for(int i = 0; i < stZMsgRes.order_info_list_size(); i++)
		{
			LOG4CPLUS_DEBUG(logger, stZMsgRes.order_info_list(i).Utf8DebugString());
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

		const ::hoosho::msg::z::GetUserCreateOrderListRes stGetUserCreateOrderListRes = stZMsgRes.get_user_create_order_list_res();
		for(int i = 0; i < stGetUserCreateOrderListRes.user_extra_info_list_size(); i++)
		{
			lce::cgi::CAnyValue item, stAnyValue;
			const ::hoosho::msg::z::UserExtraInfo& stUserExtraInfo = stGetUserCreateOrderListRes.user_extra_info_list(i);
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
	CgiOrderGetUserCreateOrderList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
