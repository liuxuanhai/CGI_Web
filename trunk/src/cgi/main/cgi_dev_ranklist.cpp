#include "main_cgi.h"

class CgiDevRankList: public MainCgi
{
     public:
		CgiDevRankList() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_NO)
        {

        }

		bool InnerProcess()
		{
			//param check
			uint64_t qwDevField = (uint64_t)GetInput().GetValue("dev_field");
			uint64_t qwDevLanguage = (uint64_t)GetInput().GetValue("dev_language");
			uint32_t dwDevType = (uint32_t)GetInput().GetValue("dev_type");
			uint32_t dwOrderType = (uint32_t)GetInput().GetValue("order_type");
			uint32_t dwOffset = (uint32_t)GetInput().GetValue("offset");
			uint32_t dwLen = (uint32_t)GetInput().GetValue("len");

			ZERO_INT_RETURN(qwDevField);
			ZERO_INT_RETURN(qwDevLanguage);
			ZERO_INT_RETURN(dwLen);
			if(dwDevType!=0 && dwDevType != DEVELOPER_TYPE_PERSON && dwDevType != DEVELOPER_TYPE_COMPANY)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid dwDevType="<<dwDevType);
				return true;
			}

			if(dwOrderType > DeveloperInfoWithFieldLanguageTypeByOrder::DeveloperInfoOrderBy_STAR)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid dwOrderType="<<dwOrderType);
				return true;
			}

			uint32_t dwTotalNum = 0;
			std::string strErrMsg = "";
			DeveloperInfoWithFieldLanguageTypeByOrder stDeveloperInfoWithFieldLanguageTypeByOrder(m_table_name_dev_info);
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_dev_field = qwDevField;
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_dev_language = qwDevLanguage;
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_type = dwDevType;
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_order_by = dwOrderType;
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_limit_from = dwOffset;
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_limit_len = dwLen;
			int iRet = stDeveloperInfoWithFieldLanguageTypeByOrder.SelectFromDB(dwTotalNum, strErrMsg);
			if(iRet != TableBase::TABLE_BASE_RET_OK && iRet != TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				LOG4CPLUS_ERROR(logger, "stDeveloperInfoWithFieldLanguageTypeByOrder.SelectFromDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			GetAnyValue()["total_num"] = dwTotalNum;	
			GetAnyValue()["offset"] = dwOffset;	
			GetAnyValue()["len"] = dwLen;	

			for(size_t i=0; i<stDeveloperInfoWithFieldLanguageTypeByOrder.m_developer_info_list.size(); ++i)
			{
				lce::cgi::CAnyValue stAnyValue;
				stDeveloperInfoWithFieldLanguageTypeByOrder.m_developer_info_list[i].ToAnyValue(stAnyValue);
				GetAnyValue()["dev_list"].push_back(stAnyValue);	
			}
			
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiDevRankList cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




