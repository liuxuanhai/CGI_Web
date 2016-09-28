#include "manager_cgi.h"

class CgiManagerListGet: public ManagerCgi
{
     public:
        CgiManagerListGet() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
		    string strErrMsg = "";

			if(m_manager_info.m_admin != MANAGER_TYPE_OWNER)
			{
				DoReply(CGI_RET_CODE_INVALID_OP);
				LOG4CPLUS_ERROR(logger, "uin = " << m_manager_info.m_uin << " has not power to get manager list");
				return true;
			}

            //get
            ManagerInfoList stManagerInfoList(m_table_name_manager);
			stManagerInfoList.m_pa_appid_md5 = m_manager_info.m_pa_appid_md5;
			
			
			int iRet = stManagerInfoList.SelectFromDB(strErrMsg);
			
			if(iRet < 0)
			{
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stManagerInfoList.SelectFromDB failed , errmsg = " << strErrMsg);
				return true;
			}

			for(size_t i = 0; i < stManagerInfoList.m_manager_info_list.size(); i++)
			{
				GetAnyValue()["manager_list"].push_back(stManagerInfoList.m_manager_info_list[i].ToAnyValue());
			}
			

            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiManagerListGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


