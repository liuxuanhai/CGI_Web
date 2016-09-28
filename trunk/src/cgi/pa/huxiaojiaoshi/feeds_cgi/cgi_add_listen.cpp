#include "cgi_feeds_base.h"
#include <time.h>
#include "common_util.h"

class CgiAddListen: public CgiFeedsBase
{
public:
	CgiAddListen():
		CgiFeedsBase(0, "config.ini", "logger.properties", true)
	{

	}

	int AddListen(const string& strFeedsServerIp, const int& dwFeedsServerPort,
				  const ::hoosho::j::commstruct::ListenInfo& stListenInfo, uint64_t& qwErrcode, string& strErrMsg)
	{
		//Build requestMsg protobuf
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::J_ADD_LISTEN_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::j::feeds::AddListenReq* stAddListenReq = stRequestMsg.mutable_j_feeds_add_listen_req();

		::common::protoio::ProtoIOTcpClient ioclient(strFeedsServerIp, dwFeedsServerPort);
		::hoosho::j::commstruct::ListenInfo* pListenInfo = stAddListenReq->mutable_listen_info();
		pListenInfo->CopyFrom(stListenInfo);

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
		uint64_t qwFeedId = strtoul(((string)GetInput().GetValue("feed_id")).c_str(), NULL, 10);
		string strOutTradeNo = "";

		ZERO_INT_RETURN(qwFollowId);
		ZERO_INT_RETURN(qwFeedId);

		::hoosho::j::commstruct::ListenInfo stListenInfo;
		uint64_t qwErrcode = 0;
		string strErrMsg = "";

		stListenInfo.set_listen_id(::common::util::generate_unique_id());
		stListenInfo.set_openid(strOpenid);
		stListenInfo.set_feed_id(qwFeedId);
		stListenInfo.set_follow_id(qwFollowId);
		stListenInfo.set_ts(time(NULL));
		stListenInfo.set_comment_type(::hoosho::j::commenum::COMMENT_TYPE_NULL);
		stListenInfo.set_out_trade_no(strOutTradeNo);

		if(AddListen(m_feeds_server_ip, m_feeds_server_port, stListenInfo, qwErrcode, strErrMsg) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		SERVER_NOT_OK_RETURN(qwErrcode);

		//Build strResponse Json from responseMsg protobuf
		GetAnyValue()["listen_id"] = int_2_str(stListenInfo.listen_id());
		DoReply(CGI_RET_CODE_OK);

		return true;
	}

};

int main()
{
	CgiAddListen cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
