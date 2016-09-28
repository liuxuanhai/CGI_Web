#include "manager_cgi.h"

class CgiManagerAdd: public ManagerCgi
{
     public:
        CgiManagerAdd() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
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

			if(m_manager_info.m_admin != MANAGER_TYPE_OWNER)
			{
				DoReply(CGI_RET_CODE_INVALID_OP);
				LOG4CPLUS_ERROR(logger, "uin = " << m_manager_info.m_uin << " has not power to add manager");
				return true;
			}

			int iRet;
			
			//check uin
			ManagerInfo stManagerInfo(m_table_name_manager);
			stManagerInfo.m_uin = strUin;
			iRet = stManagerInfo.SelectFromDB(strErrMsg);
			if(iRet < 0)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stManagerInfo.SelectFromDB failed , errmsg = " << strErrMsg);
				return true;
			}
			if(iRet != TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				DoReply(CGI_RET_CODE_ALREADY_EXIST);
				LOG4CPLUS_ERROR(logger, "uin = " << strUin << " repeated");
				return true;
			}
			
			//check openid_md5
			ManagerInfoIndexByOpenidMd5 stManagerInfoIndexByOpenidMd5(m_table_name_manager);
			uint64_t qwOpenidMd5 = 0;
			do{
				qwOpenidMd5 = m_manager_info.m_openid_md5 / OPENID_PREFIX * OPENID_PREFIX + GetRandNum(100, OPENID_PREFIX);
				stManagerInfoIndexByOpenidMd5.m_openid_md5 = qwOpenidMd5;
				iRet = stManagerInfoIndexByOpenidMd5.SelectFromDB(strErrMsg);
				if(iRet < 0)
				{
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					LOG4CPLUS_ERROR(logger, "stManagerInfoIndexByOpenidMd5.SelectFromDB failed , errmsg = " << strErrMsg);
					return true;
				}
			}while(iRet != TableBase::TABLE_BASE_RET_NOT_EXIST);
			
			
            //save to db
            
			stManagerInfo.m_uin = strUin;
			stManagerInfo.m_passwd = strPasswd;
			stManagerInfo.m_pa_appid_md5 = m_manager_info.m_pa_appid_md5;
			stManagerInfo.m_openid_md5 = qwOpenidMd5;
			stManagerInfo.m_create_ts = time(0);
			stManagerInfo.m_admin = MANAGER_TYPE_NORMAL;
			
			iRet = stManagerInfo.UpdateToDB(strErrMsg);
			
			if(iRet != TableBase::TABLE_BASE_RET_OK)
			{
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stManager.UpdateToDB failed , errmsg = " << strErrMsg);
				return true;
			}

			LOG4CPLUS_DEBUG(logger, "one manager add, uin = " << strUin);

            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiManagerAdd cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


