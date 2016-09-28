#include "cgi_order_base.h"
#include "msg.pb.h"

class CgiOrderFetch:public CgiOrderBase
{
public:
	CgiOrderFetch():CgiOrderBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");
		uint64_t qwOrderid = strtoul(((string)GetInput().GetValue("orderid")).c_str(), NULL, 10);
		string strExtradata = GetInput().GetValue("extra_data");

		EMPTY_STR_RETURN(strCode);
		ZERO_INT_RETURN(qwOrderid);

		::hoosho::msg::Msg stReqMsg;
		::hoosho::msg::Msg stResMsg;

		::hoosho::msg::MsgHead* pReqMsgHead = stReqMsg.mutable_head();
		pReqMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pReqMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stReqMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::FETCH_ORDER_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::FetchOrderReq* pFetchOrderReq = pZMsgReq->mutable_fetch_order_req();
		pFetchOrderReq->set_orderid(qwOrderid);
		pFetchOrderReq->set_extra_data(strExtradata);

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

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiOrderFetch cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
