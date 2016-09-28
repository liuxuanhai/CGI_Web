#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiFeedAdd: public CgiFeedsServer
{
public:
	CgiFeedAdd():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		uint64_t qwFeedId = strtoul( ((string) GetInput().GetValue("feed_id")).c_str(), NULL, 10);
		uint32_t dwType = (uint32_t) GetInput().GetValue("type");

		uint64_t qwOriginCommentId = strtoul( ((string) GetInput().GetValue("origin_comment_id")).c_str(), NULL, 10);
		string strContent = (string) GetInput().GetValue("content");

		uint64_t qwFeedOwnerOpenidMd5 = strtoul( ((string) GetInput().GetValue("feed_owner_openid_md5")).c_str(), NULL, 10);
		uint64_t qwOriginCommentOwnerOpenidMd5 = strtoul( ((string) GetInput().GetValue("origin_comment_owner_openid_md5")).c_str(), NULL, 10);
		uint64_t qwReplyOpenidMd5 = strtoul( ((string) GetInput().GetValue("reply_openid_md5")).c_str(), NULL, 10);

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		ZERO_INT_RETURN(qwFeedId);
		ZERO_INT_RETURN(dwType);
		EMPTY_STR_RETURN(strContent);
		ZERO_INT_RETURN(qwFeedOwnerOpenidMd5);
		ZERO_INT_RETURN(qwOriginCommentOwnerOpenidMd5);

		if(strContent.size() > m_follow_limit_length)
		{
			LOG4CPLUS_ERROR(logger, "content length too large, = " << strContent.size());
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		bool bAllSpace = true;
		for(size_t i = 0; i < strContent.size(); i++)
        {
            if(!isspace(strContent[i]))
            {
                bAllSpace = false;
                break;
            }
        }
        if(bAllSpace)
        {
            LOG4CPLUS_ERROR(logger, "content only contains space, = " << strContent.size());
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
        }
		//comment
		if(dwType == hoosho::commenum::FEEDS_FOLLOW_COMMENT)
		{
			qwReplyOpenidMd5 = qwOpenidMd5;
		}
		//reply
		else if(dwType == hoosho::commenum::FEEDS_FOLLOW_REPLY)
		{
			ZERO_INT_RETURN(qwOriginCommentId);
			ZERO_INT_RETURN(qwReplyOpenidMd5);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "unknown type = " << dwType);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}


		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::ADD_FOLLOW_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::AddFollowReq* stAddFollowReq = stRequestMsg.mutable_add_follow_req();

		stAddFollowReq->set_pa_appid_md5(qwPaAppidMd5);
		stAddFollowReq->set_openid_md5(qwOpenidMd5);
		stAddFollowReq->set_feed_id(qwFeedId);
		stAddFollowReq->set_type(dwType);

		stAddFollowReq->set_origin_comment_id(qwOriginCommentId);
		stAddFollowReq->set_content(strContent);
		stAddFollowReq->set_feed_owner_openid_md5(qwFeedOwnerOpenidMd5);
		stAddFollowReq->set_origin_comment_owner_openid_md5(qwOriginCommentOwnerOpenidMd5);
		stAddFollowReq->set_reply_openid_md5(qwReplyOpenidMd5);

		//3.send to server, and recv responseMsg protobuf
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
		if(iRet != 0)
		{
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		LOG4CPLUS_TRACE(logger, "resp = " << stResponseMsg.Utf8DebugString());
		iRet = stResponseMsg.head().result();
		SERVER_NOT_OK_RETURN(iRet);

		const hoosho::feeds::AddFollowRes& stAddFollowRes = stResponseMsg.add_follow_res();

		LOG4CPLUS_TRACE(logger, "follow_id = " << stAddFollowRes.follow_id());

		//4.Build strResponse Json from responseMsg protobuf
		GetAnyValue()["follow_id"] = int_2_str(stAddFollowRes.follow_id());
		DoReply(CGI_RET_CODE_OK);


		return true;
	}

};

int main()
{
	CgiFeedAdd cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
