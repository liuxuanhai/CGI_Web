#include "cgi_feeds_base.h"
#include "common_util.h"

class CgiCommentFollow: public CgiFeedsBase
{
public:
	CgiCommentFollow():
		CgiFeedsBase(0, "config.ini", "logger.properties", true)
	{

	}

	int CommentFollow(const string& strFeedsServerIp, const int& dwFeedsServerPort,
				  const ::hoosho::j::commstruct::CommentFollowInfo& stCommentFollow, uint64_t& qwErrcode, string& strErrMsg)
	{
		//Build requestMsg protobuf
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::J_COMMENT_FOLLOW_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::j::feeds::CommentFollowReq* stCommentFollowReq = stRequestMsg.mutable_j_feeds_comment_follow_req();

		::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);
		::hoosho::j::commstruct::CommentFollowInfo* pCommentFollowInfo = stCommentFollowReq->mutable_comment_follow_info();
		pCommentFollowInfo->CopyFrom(stCommentFollow);

		//send to server, and recv responseMsg protobuf
		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
		if(iRet != 0)
		{
			strErrMsg = "ProtoIOTcpClient IO failed, errmsg = " + strErrMsg;
			return -1;
		}
		LOG4CPLUS_DEBUG(logger, "rep msg = " << stResponseMsg.Utf8DebugString());
		qwErrcode = stResponseMsg.head().result();

		return 0;
	}

	bool InnerProcess()
	{

		//get HTTP params
		string strOpenid = m_cookie_value_key;
		uint64_t qwFollowId = strtoul(((string)GetInput().GetValue("follow_id")).c_str(), NULL, 10);
		uint64_t qwCommentType = strtoul(((string)GetInput().GetValue("comment_type")).c_str(), NULL, 10);

		ZERO_INT_RETURN(qwFollowId);
		ZERO_INT_RETURN(qwCommentType);

		::hoosho::j::commstruct::CommentFollowInfo stCommentFollowInfo;
		uint64_t qwErrcode = 0;
		string strErrMsg = "";

		stCommentFollowInfo.set_openid(strOpenid);
		stCommentFollowInfo.set_follow_id(qwFollowId);
		stCommentFollowInfo.set_comment_type(qwCommentType);

		if(CommentFollow(m_feeds_server_ip, m_feeds_server_port, stCommentFollowInfo, qwErrcode, strErrMsg) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		if(qwErrcode == hoosho::msg::E_FOLLOW_HAS_COMMENT)
		{
			LOG4CPLUS_DEBUG(logger, "follow_id = " << qwFollowId << " has been commentd");
			DoReply(CGI_RET_CODE_FOLLOW_HAS_COMMENT);
			return true;
		}
		SERVER_NOT_OK_RETURN(qwErrcode);

		//Build strResponse Json from responseMsg protobuf
		DoReply(CGI_RET_CODE_OK);

		return true;
	}

};

int main()
{
	CgiCommentFollow cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
