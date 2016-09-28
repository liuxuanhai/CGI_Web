#include "main_cgi.h"

class CgiUserInfoGet: public MainCgi
{
     public:
        CgiUserInfoGet() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }
        
		bool InnerProcess()
		{
		    lce::cgi::CAnyValue stAnyValue;
		    m_user_info.ToAnyValue(stAnyValue);
			GetAnyValue()["user_info"] = stAnyValue;

			string strErrMsg = "";
			if(!m_user_info.m_dev_id.empty())
			{
				DeveloperInfo stDevInfo(m_table_name_dev_info);
				stDevInfo.m_id = m_user_info.m_dev_id;
				if(stDevInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
				{
					LOG4CPLUS_ERROR(logger, "stDevInfo.SelectFromDB failed , errmsg="<<strErrMsg);
				}
				else
				{
					stAnyValue.clear();
					stDevInfo.ToAnyValue(stAnyValue);
					GetAnyValue()["dev_info"] = stAnyValue;
				}
			}

			if(!m_user_info.m_oper_id.empty())
			{
				OperInfo stOperInfo(m_table_name_oper_info);
				stOperInfo.m_id = m_user_info.m_oper_id;
				if(stOperInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
				{
					LOG4CPLUS_ERROR(logger, "stOperInfo.SelectFromDB failed , errmsg="<<strErrMsg);
				}
				else
				{
					stAnyValue.clear();
					stOperInfo.ToAnyValue(stAnyValue);
					GetAnyValue()["oper_info"] = stAnyValue;
				}

				WXPublicAccountWithOperId stWXPublicAccountWithOperId(m_table_name_wx_public_account);
				stWXPublicAccountWithOperId.m_oper_id = m_user_info.m_oper_id;
				if(stWXPublicAccountWithOperId.SelectFromDB(strErrMsg) == TableBase::TABLE_BASE_RET_OK)
				{
					for(size_t i=0; i<stWXPublicAccountWithOperId.m_wx_public_account_list.size(); ++i)
					{
						stAnyValue.clear();
						stWXPublicAccountWithOperId.m_wx_public_account_list[i].ToAnyValue(stAnyValue);
						GetAnyValue()["wxpa_list"].push_back(stAnyValue);
					}
				}
				
			}

            DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiUserInfoGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


