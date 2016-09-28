#include "cgi_feeds_base.h"

class CgiGetFeedDetail: public CgiFeedsBase
{
public:
	CgiGetFeedDetail():
		CgiFeedsBase(0, "config.ini", "logger.properties", true)
	{

	}


	bool InnerProcess()
	{
		//get HTTP params
		string strOpenid = m_cookie_value_key;
		uint64_t qwFeedId = strtoul( ((string)GetInput().GetValue("feed_id")).c_str(), NULL, 10);

		EMPTY_STR_RETURN(strOpenid);
		ZERO_INT_RETURN(qwFeedId)

		uint64_t qwErrcode = 0;
		std::string strErrMsg = "";
		std::vector<uint64_t> vecFeedIdList;
		std::vector<hoosho::j::commstruct::FeedInfo> vecFeedInfo;
		vecFeedIdList.push_back(qwFeedId);
		if(FetchFeedInfo(m_feeds_server_ip, m_feeds_server_port, vecFeedIdList, strOpenid,
						 qwErrcode, strErrMsg, vecFeedInfo) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		SERVER_NOT_OK_RETURN(qwErrcode);

		set<string> setOpenid;
		map<string, lce::cgi::CAnyValue> mapRet;

		for(size_t i = 0; i < vecFeedInfo.size(); i++)
		{
			lce::cgi::CAnyValue any;
			FeedInfoPB2Any(vecFeedInfo[i], any);
			GetAnyValue()["feed_list"].push_back(any);

			setOpenid.insert(vecFeedInfo[i].openid());
		}



		qwErrcode = ::hoosho::msg::E_OK;
		vector<hoosho::j::commstruct::FollowInfo> vecFollowInfo;

		vector<hoosho::j::commstruct::CheckListenInfo> vecCheckListen;
		hoosho::j::commstruct::CheckListenInfo stCheckListenInfo;
		stCheckListenInfo.set_openid(strOpenid);
		stCheckListenInfo.set_listen(false);

		vector<hoosho::j::commstruct::CommentFollowInfo> vecCommentFollow;
		hoosho::j::commstruct::CommentFollowInfo stCommentFollowInfo;
		stCommentFollowInfo.set_openid(strOpenid);
		stCommentFollowInfo.set_comment_type(hoosho::j::commenum::COMMENT_TYPE_NULL);

		if(!FetchFollowList(m_feeds_server_ip, m_feeds_server_port, 0, m_default_show_follow_num,
							::hoosho::j::commenum::GET_LIST_SCENE_TYPE_MAIN_PAGE, qwFeedId, strOpenid, qwErrcode,
							strErrMsg, vecFollowInfo) && qwErrcode == ::hoosho::msg::E_OK)
		{
			for(size_t i = 0; i < vecFollowInfo.size(); i++)
			{
				lce::cgi::CAnyValue any;
				FollowInfoPB2Any(vecFollowInfo[i], any);
				GetAnyValue()["follow_list"].push_back(any);

				setOpenid.insert(vecFollowInfo[i].openid());

				stCheckListenInfo.set_follow_id(vecFollowInfo[i].follow_id());
				vecCheckListen.push_back(stCheckListenInfo);

				stCommentFollowInfo.set_follow_id(vecFollowInfo[i].follow_id());
				vecCommentFollow.push_back(stCommentFollowInfo);
			}
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
	CgiGetFeedDetail cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
