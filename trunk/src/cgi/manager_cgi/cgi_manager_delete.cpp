#include "manager_cgi.h"

class CgiManagerDelete: public ManagerCgi
{
     public:
        CgiManagerDelete() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
		    string strErrMsg = "";
			string strUin = (string)GetInput().GetValue("uin");

			if(strUin.empty())
			{
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "empty uin = " << strUin);
				return true;
			}

			if(m_manager_info.m_admin != MANAGER_TYPE_OWNER)
			{
				DoReply(CGI_RET_CODE_INVALID_OP);
				LOG4CPLUS_ERROR(logger, "uin = " << m_manager_info.m_uin << " has not power to delete manager");
				return true;
			}

			if(strUin == m_manager_info.m_uin)
			{
				DoReply(CGI_RET_CODE_INVALID_OP);
				LOG4CPLUS_ERROR(logger, "uin = " << m_manager_info.m_uin << " is owner, can't delete ");
				return true;
			}
			
            //del db
            ManagerInfo stManagerInfo(m_table_name_manager);
			stManagerInfo.m_uin = strUin;
			
			int iRet = stManagerInfo.DeleteFromDB(strErrMsg);

			if(iRet == TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				DoReply(CGI_RET_CODE_NOT_EXIST);
				LOG4CPLUS_ERROR(logger, "not found uin = " << strUin);
			}
			
			if(iRet != TableBase::TABLE_BASE_RET_OK)
			{
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stManagerInfo.DeleteFromDB failed , errmsg = " << strErrMsg);
				return true;
			}

			LOG4CPLUS_DEBUG(logger, "one manager delete, uin = " << strUin);

            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiManagerDelete cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


