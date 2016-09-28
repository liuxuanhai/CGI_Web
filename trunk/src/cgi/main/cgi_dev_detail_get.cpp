#include "main_cgi.h"

class CgiDevDetailGet: public MainCgi
{
     public:
        CgiDevDetailGet() : MainCgi(0, "config.ini", "logger.properties", CGI_NEED_LOGIN_NO)
        {

        }
        
		bool InnerProcess()
		{
            string strDevIDList = (string)GetInput().GetValue("dev_id_list");
            EMPTY_STR_RETURN(strDevIDList);

            lce::cgi::CAnyValue mapDevInfoAnyValue;
            lce::cgi::CAnyValue mapContractorInfoAnyValue;
            vector<string> vecDevId;
            lce::cgi::Split(strDevIDList, "|", vecDevId);
            for(size_t i=0; i!=vecDevId.size(); ++i)
            {
				string strErrMsg = "";
				DeveloperInfo stDeveloperInfo(m_table_name_dev_info);
				stDeveloperInfo.m_id = vecDevId[i];
				if(stDeveloperInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
            	{
        			LOG4CPLUS_ERROR(logger, "stDeveloperInfo.SelectFromDB failed , errmsg="<<strErrMsg);
        			continue;
            	}

				UserInfo stUserInfo(m_table_name_user_info);
            	stUserInfo.m_uin = stDeveloperInfo.m_uin;
            	if(stUserInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
            	{
            		LOG4CPLUS_ERROR(logger, "stUserInfo.SelectFromDB failed , errmsg="<<strErrMsg);
            		continue;
            	}

				lce::cgi::CAnyValue oneAnyValue;
				
				stDeveloperInfo.ToAnyValue(oneAnyValue);
            	mapDevInfoAnyValue[stDeveloperInfo.m_id] = oneAnyValue;

            	stUserInfo.ToSimpleAnyValue(oneAnyValue);
            	mapContractorInfoAnyValue[stDeveloperInfo.m_id] = oneAnyValue;
			}

			GetAnyValue()["dev_map"] = mapDevInfoAnyValue;
			GetAnyValue()["contractor_map"] = mapContractorInfoAnyValue;
			
            DoReply(CGI_RET_CODE_OK);
            
			return true;
		}
};


int main()
{
    CgiDevDetailGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

