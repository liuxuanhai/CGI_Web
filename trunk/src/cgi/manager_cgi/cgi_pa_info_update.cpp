#include "manager_cgi.h"

class CgiPaInfoUpdate: public ManagerCgi
{
public:
    CgiPaInfoUpdate() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
    {

    }

    bool InnerProcess()
    {
        string strErrMsg = "";
        uint64_t qwPaAppidMd5 = m_manager_info.m_pa_appid_md5;
        uint64_t qwNumTopicFeed = strtoul( ((string)GetInput().GetValue("num_topic_feed")).c_str(), NULL, 10);
        uint64_t qwNumBannerFeed = strtoul( ((string)GetInput().GetValue("num_banner_feed")).c_str(), NULL, 10);

        if(qwNumTopicFeed > m_feeds_limit_num)
        {
            DoReply(CGI_RET_CODE_INVALID_PARAM);
            LOG4CPLUS_ERROR(logger, "num_topic_feed = " << qwNumTopicFeed << ", too larger");
            return true;
        }
        if(qwNumBannerFeed > m_feeds_limit_num)
        {
            DoReply(CGI_RET_CODE_INVALID_PARAM);
            LOG4CPLUS_ERROR(logger, "num_banner_feed = " << qwNumBannerFeed << ", too larger");
            return true;
        }

        if(qwNumBannerFeed == 0 && qwNumTopicFeed == 0)
        {
            DoReply(CGI_RET_CODE_INVALID_PARAM);
            LOG4CPLUS_ERROR(logger, "both num_banner_feed and num_bannner_feed = 0, not need to update");
            return true;
        }

        PaInfo stPaInfo = PaInfo(m_table_name_pa_info);
        stPaInfo.m_appid_md5 = qwPaAppidMd5;

        int iRet = stPaInfo.SelectFromDB(strErrMsg);
        if(iRet != TableBase::TABLE_BASE_RET_OK)
        {
            DoReply(CGI_RET_CODE_SERVER_BUSY);
            LOG4CPLUS_ERROR(logger, "stPaInfo.SelectFromDB failed , errmsg = " << strErrMsg);
            return true;
        }
        LOG4CPLUS_DEBUG(logger, stPaInfo.ToString());

        if(qwNumTopicFeed > 0)
        {
            stPaInfo.m_num_topic_feed = qwNumTopicFeed;
        }
        if(qwNumBannerFeed > 0)
        {
            stPaInfo.m_num_banner_feed = qwNumBannerFeed;
        }
        iRet = stPaInfo.UpdateToDB(strErrMsg);
        if(iRet != TableBase::TABLE_BASE_RET_OK)
        {
            DoReply(CGI_RET_CODE_SERVER_BUSY);
            LOG4CPLUS_ERROR(logger, "stPaInfo.UpdateToDB failed , errmsg = " << strErrMsg);
            return true;
        }

        DoReply(CGI_RET_CODE_OK);
        return true;
    }
};


int main()
{
    CgiPaInfoUpdate cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


