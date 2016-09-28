#include "manager_cgi.h"

class CgiManagerLogin: public ManagerCgi
{
     public:
        CgiManagerLogin() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_NO)
        {

        }

		bool InnerProcess()
		{
		    string strErrMsg = "";
			string strUin = (string)GetInput().GetValue("uin");
            string strPasswd = (string)GetInput().GetValue("passwd");

			if(strUin.empty() || strPasswd.empty())
			{
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "empty uin = " << strUin << ", or passwd = " << strPasswd);
				return true;
			}

            //get password
            ManagerInfo stManagerInfo(m_table_name_manager);
			stManagerInfo.m_uin = strUin;
			int iRet = stManagerInfo.SelectFromDB(strErrMsg);
			if(iRet == TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				DoReply(CGI_RET_CODE_MANAGER_NOT_EXIST);
				LOG4CPLUS_ERROR(logger, "not exists uin = " << strUin);
				return true;
			}

			if(iRet != TableBase::TABLE_BASE_RET_OK)
			{
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stManagerInfo.SelectFromDB failed , errmsg = " << strErrMsg);
				return true;
			}

            //check password
			if(stManagerInfo.m_passwd != strPasswd)
			{
                DoReply(CGI_RET_CODE_INVALID_PASSWORD);
				LOG4CPLUS_ERROR(logger, "check password failed, real_passwd = " << stManagerInfo.m_passwd
                                << ", req_passwd = " << strPasswd);
				return true;
			}

		    //check ok,  generate session
		    lce::cgi::CHttpHeader& stHttpRspHeader =  GetHeader();
		    if(MakeAddReplyCookie(strUin, stHttpRspHeader, strErrMsg) < 0)
		    {

                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "makeaddreply cookie failed, errmsg = " << strErrMsg);
				return true;
		    }

			LOG4CPLUS_DEBUG(logger, "one manager login succ, uin = " << strUin);
			
			GetAnyValue()["manager_info"] = stManagerInfo.ToAnyValue();
            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiManagerLogin cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


