#include "manager_cgi.h"

class CgiModifyPasswd: public ManagerCgi
{
     public:
        CgiModifyPasswd() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
		    string strErrMsg = "";
            string strOldPasswd = (string)GetInput().GetValue("old_passwd");
            string strNewPasswd = (string)GetInput().GetValue("new_passwd");

			if(strOldPasswd.empty() || strNewPasswd.empty())
			{
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "empty old_passwd = " << strOldPasswd << " or new_passwd = " << strNewPasswd);
				return true;
			}

			//check password
			if(m_manager_info.m_passwd != strOldPasswd)
			{
				DoReply(CGI_RET_CODE_INVALID_PASSWORD);
				LOG4CPLUS_ERROR(logger, "check passwd error, real_passwd = " << m_manager_info.m_passwd 
				<< ", req_passwd = " << strOldPasswd);
				return true;
			}
			
            //set password
            m_manager_info.m_passwd = strNewPasswd;
			
			int iRet = m_manager_info.UpdateToDB(strErrMsg);

			if(iRet != TableBase::TABLE_BASE_RET_OK)
			{
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "m_manager_info.UpdateToDB failed , errmsg = " << strErrMsg);
				return true;
			}

			LOG4CPLUS_DEBUG(logger, "one manager modify passwd succ, uin = " << m_manager_info.m_uin);

            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiModifyPasswd cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


