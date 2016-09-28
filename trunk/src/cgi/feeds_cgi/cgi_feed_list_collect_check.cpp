#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiFeedListCollectCheck: public CgiFeedsServer
{
public:
	CgiFeedListCollectCheck():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		string strFeedIdList = (string) GetInput().GetValue("feed_id_list");

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		EMPTY_STR_RETURN(strFeedIdList);

			
		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_LIST_COLLECT_REQ);		
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFeedListCollectReq* stQueryFeedListCollectReq = stRequestMsg.mutable_query_feed_list_collect_req();
		stQueryFeedListCollectReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFeedListCollectReq->set_openid_md5(qwOpenidMd5);

		vector<string> vecFeedId;
		lce::cgi::Split(strFeedIdList, "|", vecFeedId);
		for(size_t i=0; i != vecFeedId.size(); ++i)
		{
			stQueryFeedListCollectReq->add_feed_id_list( strtoul(vecFeedId[i].c_str(), NULL, 10) );
		}
		

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

		const hoosho::feeds::QueryFeedListCollectRes& stQueryFeedListCollectRes = stResponseMsg.query_feed_list_collect_res();

		ostringstream ossCollectList;
		ossCollectList.str("");
		ossCollectList << stQueryFeedListCollectRes.collect_list(0);
		for(int i = 1; i < stQueryFeedListCollectRes.collect_list_size(); i++)
		{
			ossCollectList << "|" << stQueryFeedListCollectRes.collect_list(i);
		}

		LOG4CPLUS_TRACE(logger, "Collect_list = " << ossCollectList.str() );
			
		//4.Build strResponse Json from responseMsg protobuf
		GetAnyValue()["Collect_list"] = ossCollectList.str();
		
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}
		
};

int main()
{
	CgiFeedListCollectCheck cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
