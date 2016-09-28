#include "cgi_feeds_base.h"
#include "msg.pb.h"

class CgiFeedsGetCommentDetail:public CgiFeedsBase
{
public:
	CgiFeedsGetCommentDetail():CgiFeedsBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");
		std::string strOriginCommentId = GetInput().GetValue("origin_comment_id_list");

		EMPTY_STR_RETURN(strOriginCommentId);

		std::vector<std::string> vecStrOriginCommentId;
		std::vector<uint64_t> vecOriginCommentId;
		lce::cgi::Split(strOriginCommentId, "|", vecStrOriginCommentId);
		ZERO_INT_RETURN(vecStrOriginCommentId.size());

		for(size_t i = 0; i < vecStrOriginCommentId.size(); i++)
		{
			vecOriginCommentId.push_back(strtoul(vecStrOriginCommentId[i].c_str(), NULL, 10));
		}

		::hoosho::msg::Msg stReqMsg;
		::hoosho::msg::Msg stResMsg;

		::hoosho::msg::MsgHead* pReqMsgHead = stReqMsg.mutable_head();
		pReqMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pReqMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stReqMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_ORDER_COMMENT_DETAIL_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetOrderCommentDetailReq *pGetOrderCommentDetailReq = pZMsgReq->mutable_get_order_comment_detail_req();
		for(size_t i = 0; i < vecOriginCommentId.size(); i++)
		{
			pGetOrderCommentDetailReq->add_origin_comment_id_list(vecOriginCommentId[i]);
		}

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
		const ::hoosho::msg::z::GetOrderCommentDetailRes stGetOrderCommentDetailRes = stZMsgRes.get_order_comment_detail_res();
		lce::cgi::CAnyValue anyFollow;
		for(int i = 0; i < stGetOrderCommentDetailRes.comment_list_size(); i++)
		{
			const ::hoosho::msg::z::OrderCommentInfo& stOrderCommentInfo = stGetOrderCommentDetailRes.comment_list(i);
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
	CgiFeedsGetCommentDetail cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
