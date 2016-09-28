#include "cgi_feeds_base.h"

class CgiGetUserHistoryList: public CgiFeedsBase
{
public:

	CgiGetUserHistoryList():
		CgiFeedsBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{

		//get HTTP params
		std::string strOpenid = GetInput().GetValue("openid");
		if(strOpenid.empty())
		{
			strOpenid = m_cookie_value_key;
		}
		uint64_t qwBeginTs = strtoul( ((string) GetInput().GetValue("begin_ts")).c_str(), NULL, 10);
		uint64_t qwLimit = strtoul( ((string) GetInput().GetValue("limit")).c_str(), NULL, 10);

		EMPTY_STR_RETURN(strOpenid);
		ZERO_INT_RETURN(qwLimit);

		uint64_t qwErrcode = 0;
		std::string strErrMsg = "";
		std::vector<hoosho::j::commstruct::FeedInfo> vecFeedInfo;
		if(FetchHistoryList(m_feeds_server_ip, m_feeds_server_port, qwBeginTs, qwLimit, strOpenid, qwErrcode,
						strErrMsg, vecFeedInfo) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		SERVER_NOT_OK_RETURN(qwErrcode);

		for(size_t i = 0; i < vecFeedInfo.size(); i++)
		{
			lce::cgi::CAnyValue any;
			FeedInfoPB2Any(vecFeedInfo[i], any);
			GetAnyValue()["feed_list"].push_back(any);
		}
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}

};

int main()
{
	CgiGetUserHistoryList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
