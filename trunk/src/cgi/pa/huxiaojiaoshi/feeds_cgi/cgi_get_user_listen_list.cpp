#include "cgi_feeds_base.h"

class CgiGetUserListenList: public CgiFeedsBase
{
public:
	CgiGetUserListenList():
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
		std::vector<hoosho::j::commstruct::FollowInfo> vecFollowInfo;
		if(FetchListenList(m_feeds_server_ip, m_feeds_server_port, qwBeginTs, qwLimit,
						 ::hoosho::j::commenum::GET_LIST_SCENE_TYPE_PERSON_PAGE, 0, strOpenid, qwErrcode,
						strErrMsg, vecFollowInfo) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		SERVER_NOT_OK_RETURN(qwErrcode);

		vector<uint64_t> vecFeedId;
		vector<hoosho::j::commstruct::FeedInfo> vecFeedInfo;

		set<string> setOpenid;
		map<string, lce::cgi::CAnyValue> mapRet;

		vector<hoosho::j::commstruct::CheckListenInfo> vecCheckListen;
		hoosho::j::commstruct::CheckListenInfo stCheckListenInfo;
		stCheckListenInfo.set_openid(m_cookie_value_key);
		stCheckListenInfo.set_listen(false);

		vector<hoosho::j::commstruct::CommentFollowInfo> vecCommentFollow;
		hoosho::j::commstruct::CommentFollowInfo stCommentFollowInfo;
		stCommentFollowInfo.set_openid(m_cookie_value_key);
		stCommentFollowInfo.set_comment_type(hoosho::j::commenum::COMMENT_TYPE_NULL);

		for(size_t i = 0; i < vecFollowInfo.size(); i++)
		{
			lce::cgi::CAnyValue any;
			FollowInfoPB2Any(vecFollowInfo[i], any);
			GetAnyValue()["follow_list"].push_back(any);

			vecFeedId.push_back(vecFollowInfo[i].feed_id());
			setOpenid.insert(vecFollowInfo[i].openid());

			stCheckListenInfo.set_follow_id(vecFollowInfo[i].follow_id());
			vecCheckListen.push_back(stCheckListenInfo);

			stCommentFollowInfo.set_follow_id(vecFollowInfo[i].follow_id());
			vecCommentFollow.push_back(stCommentFollowInfo);

		}

		if(!CheckListen(m_feeds_server_ip, m_feeds_server_port, vecCheckListen, qwErrcode, strErrMsg)
		   && qwErrcode == hoosho::msg::E_OK)
		{
			for(size_t i = 0; i < vecCheckListen.size(); i++)
			{
				GetAnyValue()["check_listen"][int_2_str(vecCheckListen[i].follow_id())] = vecCheckListen[i].listen();
			}
		}

		if(!FetchCommnetFollow(m_feeds_server_ip, m_feeds_server_port, vecCommentFollow, qwErrcode, strErrMsg)
		   && qwErrcode == hoosho::msg::E_OK)
		{
			for(size_t i = 0; i < vecCommentFollow.size(); i++)
			{
				GetAnyValue()["comment_follow"][int_2_str(vecCommentFollow[i].follow_id())] = vecCommentFollow[i].comment_type();
			}
		}


		if(!FetchFeedInfo(m_feeds_server_ip, m_feeds_server_port, vecFeedId, "0",
						 qwErrcode, strErrMsg, vecFeedInfo))
		{
			for(size_t i = 0; i < vecFeedInfo.size(); i++)
			{
				lce::cgi::CAnyValue any;
				FeedInfoPB2Any(vecFeedInfo[i], any);
				GetAnyValue()["feed_info"][int_2_str(vecFeedInfo[i].feed_id())] = any;

				setOpenid.insert(vecFeedInfo[i].openid());
			}
		}

		if(!FetchUserInfo(setOpenid, mapRet, m_user_server_ip, m_user_server_port, strErrMsg))
		{
			map<string, lce::cgi::CAnyValue>::iterator iter;
			for(iter = mapRet.begin(); iter != mapRet.end(); iter++)
			{
				GetAnyValue()["user_info"][iter->first] = iter->second;
			}
		}
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}

};

int main()
{
	CgiGetUserListenList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
