#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiFeedDelete: public CgiFeedsServer
{
public:
	CgiFeedDelete():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		uint64_t qwFeedId = strtoul( ((string) GetInput().GetValue("feed_id")).c_str(), NULL, 10);

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		ZERO_INT_RETURN(qwFeedId);
			
		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::DELETE_FEED_REQ);		
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::DeleteFeedReq* stDeleteFeedReq = stRequestMsg.mutable_delete_feed_req();
		stDeleteFeedReq->set_pa_appid_md5(qwPaAppidMd5);
		stDeleteFeedReq->set_openid_md5(qwOpenidMd5);
		stDeleteFeedReq->set_feed_id(qwFeedId);

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

		//4.Build strResponse Json from responseMsg protobuf
		LOG4CPLUS_TRACE(logger, "FEED_DELETE: openid = " << qwOpenidMd5 << ", feedid = " << qwFeedId << ", appid = " << qwPaAppidMd5);
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}
		
};

int main()
{
	CgiFeedDelete cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
