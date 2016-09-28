#include "manager_cgi.h"

class CgiForbidUser: public ManagerCgi
{
     public:
        CgiForbidUser() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
		
		    string strErrMsg = "";
			
			uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
			uint64_t qwUntilTs = strtoul( ((string) GetInput().GetValue("until_ts")).c_str(), NULL, 10);

			ZERO_INT_RETURN(qwOpenidMd5);
			ZERO_INT_RETURN(qwUntilTs);
			
			UserForbidInfo stUserForbidInfo(m_table_name_user_forbid);
			stUserForbidInfo.m_pa_appid_md5 = m_manager_info.m_pa_appid_md5;
			stUserForbidInfo.m_openid_md5 = qwOpenidMd5;
			stUserForbidInfo.m_until_ts = qwUntilTs;

			int iRet;
			iRet = stUserForbidInfo.UpdateToDB(strErrMsg);
			LOG4CPLUS_TRACE(logger, strErrMsg);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "stUserForbidInfo.UpdateToDB failed, errmsg = " << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			
            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiForbidUser cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


