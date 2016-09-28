#include "cgi_feeds_base.h"

class CgiCheckListen: public CgiFeedsBase
{
public:
	CgiCheckListen():
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
		std::vector<hoosho::j::commstruct::CheckListenInfo> vecCheckListen;
		::hoosho::j::commstruct::CheckListenInfo stCheckListenInfo;
		stCheckListenInfo.set_openid(strOpenid);
		stCheckListenInfo.set_listen(false);
		for(size_t i = 0; i < vecFollowIdList.size(); i++)
		{
			stCheckListenInfo.set_follow_id(strtoul(vecFollowIdList[i].c_str(), NULL, 10));
			vecCheckListen.push_back(stCheckListenInfo);
		}

		if(CheckListen(m_feeds_server_ip, m_feeds_server_port, vecCheckListen,
							  qwErrcode, strErrMsg) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		SERVER_NOT_OK_RETURN(qwErrcode);

		for(size_t i = 0; i < vecCheckListen.size(); i++)
		{
			GetAnyValue()[int_2_str(vecCheckListen[i].follow_id())] = vecCheckListen[i].listen();
		}
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}

};

int main()
{
	CgiCheckListen cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
