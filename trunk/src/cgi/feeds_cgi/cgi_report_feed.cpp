#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiReportUser: public CgiFeedsServer
{
public:
	CgiReportUser():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5From = strtoul( ((string) GetInput().GetValue("openid_md5_from")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5To = strtoul( ((string) GetInput().GetValue("openid_md5_to")).c_str(), NULL, 10);
		uint64_t qwFeedId = strtoul( ((string) GetInput().GetValue("feed_id")).c_str(), NULL, 10);
		string strReportReason = (string) GetInput().GetValue("report_reason");

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5From);
		ZERO_INT_RETURN(qwOpenidMd5To);
		ZERO_INT_RETURN(qwFeedId);
		EMPTY_STR_RETURN(strReportReason);

		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);
		int iRet;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::ADD_REPORT_FEED_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::AddReportFeedReq* stAddReportFeedReq = stRequestMsg.mutable_add_report_feed_req();
		::hoosho::commstruct::FeedReportInfo* stFeedReportInfo = stAddReportFeedReq->mutable_feed_report_info();
		stFeedReportInfo->set_pa_appid_md5(qwPaAppidMd5);
		stFeedReportInfo->set_openid_md5_from(qwOpenidMd5From);
		stFeedReportInfo->set_openid_md5_to(qwOpenidMd5To);
		stFeedReportInfo->set_feed_id(qwFeedId);
		stFeedReportInfo->set_report_reason(strReportReason);
		stFeedReportInfo->set_report_ts( time(0) );
		

		//3.send to server, and recv responseMsg protobuf

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
		DoReply(CGI_RET_CODE_OK);

		return true;
	}

};

int main()
{
	CgiReportUser cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
