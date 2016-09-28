#include "cgi_feeds_base.h"

class CgiGetCommentFollow: public CgiFeedsBase
{
public:
	CgiGetCommentFollow():
		CgiFeedsBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{

		//get HTTP params
		std::string strOpenid = m_cookie_value_key;
		std::string strFollowIdList = GetInput().GetValue("follow_id_list");

		std::vector<string> vecFollowIdList;
		lce::cgi::Split(strFollowIdList, "|", vecFollowIdList);

		uint64_t qwErrcode = 0;
		std::string strErrMsg = "";
		std::vector<hoosho::j::commstruct::CommentFollowInfo> vecCommentFollow;
		::hoosho::j::commstruct::CommentFollowInfo stCommentFollowInfo;
		stCommentFollowInfo.set_openid(strOpenid);
		stCommentFollowInfo.set_comment_type(::hoosho::j::commenum::COMMENT_TYPE_NULL);
		for(size_t i = 0; i < vecFollowIdList.size(); i++)
		{
			stCommentFollowInfo.set_follow_id(strtoul(vecFollowIdList[i].c_str(), NULL, 10));
			vecCommentFollow.push_back(stCommentFollowInfo);
		}

		if(FetchCommnetFollow(m_feeds_server_ip, m_feeds_server_port, vecCommentFollow,
							  qwErrcode, strErrMsg) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		SERVER_NOT_OK_RETURN(qwErrcode);

		for(size_t i = 0; i < vecCommentFollow.size(); i++)
		{
			GetAnyValue()[int_2_str(vecCommentFollow[i].follow_id())] = vecCommentFollow[i].comment_type();
		}
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}

};

int main()
{
	CgiGetCommentFollow cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
