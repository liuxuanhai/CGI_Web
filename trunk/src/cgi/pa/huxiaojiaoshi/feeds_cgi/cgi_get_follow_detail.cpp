#include "cgi_feeds_base.h"

class CgiGetFollowDetail: public CgiFeedsBase
{
public:
	CgiGetFollowDetail():
		CgiFeedsBase(0, "config.ini", "logger.properties", true)
	{

	}


	bool InnerProcess()
	{
		//get HTTP params
		uint64_t qwFollowId = strtoul( ((string)GetInput().GetValue("follow_id")).c_str(), NULL, 10);
		ZERO_INT_RETURN(qwFollowId)

		uint64_t qwErrcode = 0;
		std::string strErrMsg = "";
		std::vector<uint64_t> vecFollowIdList;
		std::vector<hoosho::j::commstruct::FollowInfo> vecFollowInfo;
		vecFollowIdList.push_back(qwFollowId);
		if(FetchFollowInfo(m_feeds_server_ip, m_feeds_server_port, vecFollowIdList,
						 qwErrcode, strErrMsg, vecFollowInfo) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		SERVER_NOT_OK_RETURN(qwErrcode);

		for(size_t i = 0; i < vecFollowInfo.size(); i++)
		{
			lce::cgi::CAnyValue any;
			FollowInfoPB2Any(vecFollowInfo[i], any);
			GetAnyValue()["follow_list"].push_back(any);
		}
		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiGetFollowDetail cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
