#include "cgi_feeds_base.h"
#include "msg.pb.h"

class CgiFeedsGetFavorList:public CgiFeedsBase
{
public:
	CgiFeedsGetFavorList():CgiFeedsBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");
		uint64_t qwOrderid = strtoul(((string)GetInput().GetValue("orderid")).c_str(), NULL, 10);
		uint64_t qwBeginTs = strtoul(((string)GetInput().GetValue("begin_ts")).c_str(), NULL, 10);
		uint32_t dwLimit = strtoul(((string)GetInput().GetValue("limit")).c_str(), NULL, 10);

		ZERO_INT_RETURN(qwOrderid);
		if(dwLimit <= 0 || dwLimit > 20)
		{
			LOG4CPLUS_ERROR(logger, "invalid limit = " << dwLimit);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		::hoosho::msg::Msg stReqMsg;
		::hoosho::msg::Msg stResMsg;

		::hoosho::msg::MsgHead* pReqMsgHead = stReqMsg.mutable_head();
		pReqMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pReqMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stReqMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_ORDER_FAVOR_LIST_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetOrderFavorListReq *pGetOrderFavorListReq = pZMsgReq->mutable_get_order_favor_list_req();
		pGetOrderFavorListReq->set_orderid(qwOrderid);
		pGetOrderFavorListReq->set_begin_ts(qwBeginTs);
		pGetOrderFavorListReq->set_limit(dwLimit);

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
		const ::hoosho::msg::z::GetOrderFavorListRes stGetOrderFavorListRes = stZMsgRes.get_order_favor_list_res();
		lce::cgi::CAnyValue any;
		for(int i = 0; i < stGetOrderFavorListRes.favor_list_size(); i++)
		{
			OrderFavorInfoPB2Any(stGetOrderFavorListRes.favor_list(i), any);
			GetAnyValue()["order_favor_info_list"].push_back(any);
		}

		lce::cgi::CAnyValue anyUser;
		for(int i = 0; i < stZMsgRes.user_info_list_size(); i++)
		{
			UserInfoPB2Any(stZMsgRes.user_info_list(i), anyUser);
			GetAnyValue()["user_info_list"][stZMsgRes.user_info_list(i).openid()] = anyUser;
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiFeedsGetFavorList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
