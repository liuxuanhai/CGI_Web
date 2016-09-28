#include "cgi_feeds_base.h"
#include "msg.pb.h"

class CgiFeedsAddReply:public CgiFeedsBase
{
public:
	CgiFeedsAddReply():CgiFeedsBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");
		uint64_t qwOrderid = strtoul(((string)GetInput().GetValue("orderid")).c_str(), NULL, 10);
		string strContent = GetInput().GetValue("content");
		uint64_t qwOriginCommentid = strtoul(((string)GetInput().GetValue("origin_comment_id")).c_str(), NULL, 10);
		string strOpenidTo = GetInput().GetValue("openid_to");

		ZERO_INT_RETURN(qwOrderid);
		EMPTY_STR_RETURN(strContent);
		if(strContent.size() > 200)
		{
			LOG4CPLUS_ERROR(logger, "invalid content size = " << strContent.size());
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		ZERO_INT_RETURN(qwOriginCommentid);
		EMPTY_STR_RETURN(strOpenidTo);

		::hoosho::msg::Msg stReqMsg;
		::hoosho::msg::Msg stResMsg;

		::hoosho::msg::MsgHead* pReqMsgHead = stReqMsg.mutable_head();
		pReqMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pReqMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stReqMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::ADD_ORDER_FOLLOW_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::AddOrderFollowReq *pAddOrderFollowReq = pZMsgReq->mutable_add_order_follow_req();
		::hoosho::msg::z::OrderFollowInfo *pOrderFollowInfo = pAddOrderFollowReq->mutable_follow_info();

		pOrderFollowInfo->set_content(strContent);
		pOrderFollowInfo->set_orderid(qwOrderid);
		pOrderFollowInfo->set_type(2);
		pOrderFollowInfo->set_origin_comment_id(qwOriginCommentid);
		pOrderFollowInfo->set_openid_to(strOpenidTo);

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

		const ::hoosho::msg::z::MsgRes& stZMsgRes = stResMsg.z_msg_res();
		const ::hoosho::msg::z::AddOrderFollowRes& stAddOrderFollowRes = stZMsgRes.add_order_follow_res();
		GetAnyValue()["follow_id"] = int_2_str(stAddOrderFollowRes.follow_id());

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiFeedsAddReply cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
