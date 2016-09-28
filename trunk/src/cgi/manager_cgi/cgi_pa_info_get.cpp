#include "manager_cgi.h"

class CgiPaInfoGet: public ManagerCgi
{
public:
    CgiPaInfoGet() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
    {

    }

    bool InnerProcess()
    {
        string strErrMsg = "";
        uint64_t qwPaAppidMd5 = m_manager_info.m_pa_appid_md5;

        PaInfo stPaInfo = PaInfo(m_table_name_pa_info);
        stPaInfo.m_appid_md5 = qwPaAppidMd5;
        int iRet = stPaInfo.SelectFromDB(strErrMsg);

        if(iRet < 0)
        {
            DoReply(CGI_RET_CODE_SERVER_BUSY);
            LOG4CPLUS_ERROR(logger, "stPaInfo.SelectFromDB failed , errmsg = " << strErrMsg);
            return true;
        }
        if(iRet == TableBase::TABLE_BASE_RET_NOT_EXIST)
        {
            LOG4CPLUS_ERROR(logger, "appid_md5 = " << qwPaAppidMd5 << " not found, insert now");
            stPaInfo.m_appid_md5 = qwPaAppidMd5;
            stPaInfo.m_appid = "";
            stPaInfo.m_name = "";
            stPaInfo.m_simple_desc = "";
            stPaInfo.m_portrait_pic_id = 0;
            stPaInfo.m_create_ts = time(NULL);
            stPaInfo.m_num_topic_feed = 1;
            stPaInfo.m_num_banner_feed = 5;

            iRet = stPaInfo.UpdateToDB(strErrMsg);
            if(iRet < 0)
            {
                DoReply(CGI_RET_CODE_SERVER_BUSY);
                LOG4CPLUS_ERROR(logger, "stPaInfo.UpdateToDB failed , errmsg = " << strErrMsg);
                return true;

            }
        }
        LOG4CPLUS_DEBUG(logger, stPaInfo.ToString());
        GetAnyValue()["pa_info"] = stPaInfo.ToAnyValue();
        DoReply(CGI_RET_CODE_OK);
        return true;
    }
};


int main()
{
    CgiPaInfoGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


