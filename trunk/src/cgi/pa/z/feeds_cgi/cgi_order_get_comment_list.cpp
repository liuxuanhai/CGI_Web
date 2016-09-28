#include "cgi_feeds_base.h"
#include "msg.pb.h"

class CgiFeedsGetCommentList:public CgiFeedsBase
{
public:
	CgiFeedsGetCommentList():CgiFeedsBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");
		uint64_t qwOrderid = strtoul(((string)GetInput().GetValue("orderid")).c_str(), NULL, 10);
		uint64_t qwBeginCommentid = strtoul(((string)GetInput().GetValue("begin_comment_id")).c_str(), NULL, 10);
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
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_ORDER_COMMENT_LIST_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetOrderCommentListReq *pGetOrderCommentListReq = pZMsgReq->mutable_get_order_comment_list_req();
		pGetOrderCommentListReq->set_orderid(qwOrderid);
		pGetOrderCommentListReq->set_begin_comment_id(qwBeginCommentid);
		pGetOrderCommentListReq->set_limit(dwLimit);

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
		const ::hoosho::msg::z::GetOrderCommentListRes stGetOrderCommentListRes = stZMsgRes.get_order_comment_list_res();
		lce::cgi::CAnyValue anyFollow;
		for(int i = 0; i < stGetOrderCommentListRes.comment_list_size(); i++)
		{
			const ::hoosho::msg::z::OrderCommentInfo& stOrderCommentInfo = stGetOrderCommentListRes.comment_list(i);
			lce::cgi::CAnyValue anyComment;

			OrderFollowInfoPB2Any(stOrderCommentInfo.comment(), anyComment);

			for(int j = 0; j < stOrderCommentInfo.reply_list_size(); j++)
			{
				OrderFollowInfoPB2Any(stOrderCommentInfo.reply_list(j), anyFollow);
				anyComment["reply_list"].push_back(anyFollow);
			}

			GetAnyValue()["comment_list"].push_back(anyComment);
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
	CgiFeedsGetCommentList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
