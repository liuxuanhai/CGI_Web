#include "cgi_feeds_base.h"

class CgiGetFeedList: public CgiFeedsBase
{
public:
	CgiGetFeedList():
		CgiFeedsBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{

		//get HTTP params
		std::string strOpenid = m_cookie_value_key;
		uint64_t qwBeginFeedId = strtoul( ((string) GetInput().GetValue("begin_feed_id")).c_str(), NULL, 10);
		uint64_t qwLimit = strtoul( ((string) GetInput().GetValue("limit")).c_str(), NULL, 10);
		uint64_t qwNeedSex = strtoul( ((string) GetInput().GetValue("need_sex")).c_str(), NULL, 10);

		EMPTY_STR_RETURN(strOpenid);
		ZERO_INT_RETURN(qwLimit);
		if(!qwNeedSex)
		{
			qwNeedSex = 7;
		}

		uint64_t qwErrcode = 0;
		std::string strErrMsg = "";
		std::vector<hoosho::j::commstruct::FeedInfo> vecFeedInfo;
		if(FetchFeedList(m_feeds_server_ip, m_feeds_server_port, qwBeginFeedId, qwLimit,
						 ::hoosho::j::commenum::GET_LIST_SCENE_TYPE_MAIN_PAGE, qwNeedSex, strOpenid, qwErrcode,
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
	CgiGetFeedList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
