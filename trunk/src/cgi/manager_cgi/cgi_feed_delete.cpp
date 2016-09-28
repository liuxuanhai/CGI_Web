#include "manager_cgi.h"
#include <time.h>
#include <sstream>

class CgiFeedDelete: public ManagerCgi
{
public:
	CgiFeedDelete():
		ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
	{

	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = m_manager_info.m_pa_appid_md5;
		uint64_t qwOpenidMd5 = m_manager_info.m_openid_md5;
		uint64_t qwFeedId = strtoul( ((string) GetInput().GetValue("feed_id")).c_str(), NULL, 10);

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		ZERO_INT_RETURN(qwFeedId);

		LOG4CPLUS_DEBUG(logger, "$" << m_table_name_feed_info << "$ $" << m_table_name_feed_index_on_appid << "$");

		FeedInfo stFeedInfo(m_table_name_feed_info);
		stFeedInfo.m_feed_id = qwFeedId;
		int iRet;
		string strErrMsg;
		iRet = stFeedInfo.SelectFromDB(strErrMsg);
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "stFeedInfo.SelectFromDB failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		if(iRet == TableBase::TABLE_BASE_RET_NOT_EXIST)
		{
			LOG4CPLUS_ERROR(logger, "feedid = " << qwFeedId << " not found");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		stFeedInfo.m_del_ts = (uint64_t)time(0);
		iRet = stFeedInfo.UpdateToDB(strErrMsg);
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "stFeedInfo.UpdateToDB failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		FeedIndexOnAppid stFeedIndexOnAppid(m_table_name_feed_index_on_appid);
		stFeedIndexOnAppid.m_pa_appid_md5 = qwPaAppidMd5;
		stFeedIndexOnAppid.m_feed_id = qwFeedId;

		iRet = stFeedIndexOnAppid.DeleteFromDB(strErrMsg);
		
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "stFeedIndexOnAppid.DeleteFromDB failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		
		LOG4CPLUS_TRACE(logger, "DELETE FEED, appid = " << qwPaAppidMd5 << ", uin = " << m_manager_info.m_uin << " , feedid = " << qwFeedId);
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
