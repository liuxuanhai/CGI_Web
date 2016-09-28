#include "cgi_feeds_base.h"
#include <time.h>
#include "common_util.h"

class CgiAddFeed: public CgiFeedsBase
{
public:
	CgiAddFeed():
	CgiFeedsBase(0, "config.ini", "logger.properties", true)

	{

	}

	int AddFeed(const string& strFeedsServerIp, const int& dwFeedsServerPort,
				  const ::hoosho::j::commstruct::FeedInfo& stFeedInfo, uint64_t& qwErrcode, string& strErrMsg)
	{
		//Build requestMsg protobuf
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::J_ADD_FEED_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::j::feeds::AddFeedReq* stAddFeedReq = stRequestMsg.mutable_j_feeds_add_feed_req();

		::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);
		::hoosho::j::commstruct::FeedInfo* pFeedInfo = stAddFeedReq->mutable_feed_info();
		pFeedInfo->CopyFrom(stFeedInfo);

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
		string strContent = GetInput().GetValue("content");
		uint64_t qwNeedSex = GetInput().GetValue("need_sex");

		EMPTY_STR_RETURN(strContent);
		ZERO_INT_RETURN(qwNeedSex);

		if(strContent.size() > m_feed_length_limit)
		{
			LOG4CPLUS_ERROR(logger, "content.size = " << strContent.size() << ", too long");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		::hoosho::j::commstruct::FeedInfo stFeedInfo;
		uint64_t qwErrcode = 0;
		string strErrMsg = "";

		stFeedInfo.set_openid(strOpenid);
		stFeedInfo.set_feed_id(::common::util::generate_unique_id());
		stFeedInfo.set_content(strContent);
		stFeedInfo.set_ts(time(NULL));
		stFeedInfo.set_num_follow(0);
		stFeedInfo.set_num_listen(0);
		stFeedInfo.set_need_sex(qwNeedSex);

		if(AddFeed(m_feeds_server_ip, m_feeds_server_port, stFeedInfo, qwErrcode, strErrMsg) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		SERVER_NOT_OK_RETURN(qwErrcode);

		//Build strResponse Json from responseMsg protobuf
		GetAnyValue()["feed_id"] = int_2_str(stFeedInfo.feed_id());
		DoReply(CGI_RET_CODE_OK);

		return true;
	}

};

int main()
{
	CgiAddFeed cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
