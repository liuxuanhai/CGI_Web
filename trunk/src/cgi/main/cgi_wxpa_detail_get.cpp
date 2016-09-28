#include "main_cgi.h"

class CgiWXPADetailGet: public MainCgi
{
     public:
        CgiWXPADetailGet() : MainCgi(0, "config.ini", "logger.properties", CGI_NEED_LOGIN_NO)
        {

        }
        
		bool InnerProcess()
		{
            string strWXPAList = (string)GetInput().GetValue("wxpa_list");
            EMPTY_STR_RETURN(strWXPAList);

            lce::cgi::CAnyValue mapAnyValue;
            vector<string> vecWXPA;
            lce::cgi::Split(strWXPAList, "|", vecWXPA);
            for(size_t i=0; i!=vecWXPA.size(); ++i)
            {
				string strErrMsg = "";
				WXPublicAccount stWXPublicAccount(m_table_name_wx_public_account);
				stWXPublicAccount.m_wx = vecWXPA[i];
				if(stWXPublicAccount.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
            	{
        			LOG4CPLUS_ERROR(logger, "stWXPublicAccount.SelectFromDB failed , errmsg="<<strErrMsg);
        			continue;
            	}

				lce::cgi::CAnyValue oneAnyValue;
				stWXPublicAccount.ToAnyValue(oneAnyValue);
            	mapAnyValue[stWXPublicAccount.m_wx] = oneAnyValue;
			}

			GetAnyValue()["wxpa_map"] = mapAnyValue;
            DoReply(CGI_RET_CODE_OK);
            
			return true;
		}
};


int main()
{
    CgiWXPADetailGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

