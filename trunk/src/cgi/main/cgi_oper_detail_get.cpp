#include "main_cgi.h"

class CgiOperDetailGet: public MainCgi
{
     public:
        CgiOperDetailGet() : MainCgi(0, "config.ini", "logger.properties", CGI_NEED_LOGIN_NO)
        {

        }
        
		bool InnerProcess()
		{
            string strOperIDList = (string)GetInput().GetValue("oper_id_list");
            EMPTY_STR_RETURN(strOperIDList);

            lce::cgi::CAnyValue mapOperInfoAnyValue;
            lce::cgi::CAnyValue mapContractorInfoAnyValue;
            vector<string> vecOperId;
            lce::cgi::Split(strOperIDList, "|", vecOperId);
            for(size_t i=0; i!=vecOperId.size(); ++i)
            {
				string strErrMsg = "";
				OperInfo stOperInfo(m_table_name_oper_info);
				stOperInfo.m_id = vecOperId[i];
				if(stOperInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
            	{
        			LOG4CPLUS_ERROR(logger, "stOperInfo.SelectFromDB failed , errmsg="<<strErrMsg);
        			continue;
            	}

            	UserInfo stUserInfo(m_table_name_user_info);
            	stUserInfo.m_uin = stOperInfo.m_uin;
            	if(stUserInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
            	{
            		LOG4CPLUS_ERROR(logger, "stUserInfo.SelectFromDB failed , errmsg="<<strErrMsg);
            		continue;
            	}

				lce::cgi::CAnyValue oneAnyValue;
				
				stOperInfo.ToAnyValue(oneAnyValue);
            	mapOperInfoAnyValue[stOperInfo.m_id] = oneAnyValue;

            	stUserInfo.ToSimpleAnyValue(oneAnyValue);
            	mapContractorInfoAnyValue[stOperInfo.m_id] = oneAnyValue;
			}

			GetAnyValue()["oper_map"] = mapOperInfoAnyValue;
			GetAnyValue()["contractor_map"] = mapContractorInfoAnyValue;
            DoReply(CGI_RET_CODE_OK);
            
			return true;
		}
};


int main()
{
    CgiOperDetailGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

