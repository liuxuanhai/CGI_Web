#include "cgi_feeds_base.h"
#include <time.h>
#include "common_util.h"

class CgiAddFollow: public CgiFeedsBase
{
public:
	CgiAddFollow():
		CgiFeedsBase(0, "config.ini", "logger.properties", true)
	{

	}

	int AddFollow(const string& strFeedsServerIp, const int& dwFeedsServerPort,
				  const ::hoosho::j::commstruct::FollowInfo& stFollowInfo, uint64_t& qwErrcode, string& strErrMsg)
	{
		//Build requestMsg protobuf
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::J_ADD_FOLLOW_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::j::feeds::AddFollowReq* stAddFollowReq = stRequestMsg.mutable_j_feeds_add_follow_req();

		::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);
		::hoosho::j::commstruct::FollowInfo* pFollowInfo = stAddFollowReq->mutable_follow_info();
		pFollowInfo->CopyFrom(stFollowInfo);

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
		uint64_t qwFeedId = strtoul(((string)GetInput().GetValue("feed_id")).c_str(), NULL, 10);
		uint64_t qwContentType = strtoul(((string)GetInput().GetValue("content_type")).c_str(), NULL, 10);
		string strContentId = GetInput().GetValue("content_id");
		uint64_t qwContentDuration = strtoul(((string)GetInput().GetValue("content_duration")).c_str(), NULL, 10);
		string strOutTradeNo = "";

		ZERO_INT_RETURN(qwFeedId);
		if(qwContentType != ::hoosho::j::commenum::MEDIA_TYPE_VOICE)
		{
			LOG4CPLUS_ERROR(logger, "unknow content_type = " << qwContentType);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		EMPTY_STR_RETURN(strContentId);

		::hoosho::j::commstruct::FollowInfo stFollowInfo;
		uint64_t qwErrcode = 0;
		string strErrMsg = "";

		stFollowInfo.set_follow_id(::common::util::generate_unique_id());
		stFollowInfo.set_openid(strOpenid);
		stFollowInfo.set_feed_id(qwFeedId);
		stFollowInfo.set_content_type(qwContentType);
		stFollowInfo.set_content_id(strContentId);
		stFollowInfo.set_content_duration(qwContentDuration);
		stFollowInfo.set_ts(time(NULL));
		stFollowInfo.set_num_comment_good(0);
		stFollowInfo.set_num_comment_default(0);
		stFollowInfo.set_num_comment_bad(0);
		stFollowInfo.set_out_trade_no(strOutTradeNo);

		if(AddFollow(m_feeds_server_ip, m_feeds_server_port, stFollowInfo, qwErrcode, strErrMsg) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		SERVER_NOT_OK_RETURN(qwErrcode);

		//Build strResponse Json from responseMsg protobuf
		GetAnyValue()["follow_id"] = int_2_str(stFollowInfo.follow_id());
		DoReply(CGI_RET_CODE_OK);

		return true;
	}

};

int main()
{
	CgiAddFollow cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
