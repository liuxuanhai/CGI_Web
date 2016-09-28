#include "main_cgi.h"

class CgiRecommendListGet: public MainCgi
{
     public:
		CgiRecommendListGet() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_NO)
        {

        }

		bool InnerProcess()
		{
			uint32_t dwTotalNum = 0;
			std::string strErrMsg = "";
			DeveloperInfoWithFieldLanguageTypeByOrder stDeveloperInfoWithFieldLanguageTypeByOrder(m_table_name_dev_info);
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_dev_field = -1;
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_dev_language = -1;
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_type = DEVELOPER_TYPE_COMPANY;
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_order_by = DeveloperInfoWithFieldLanguageTypeByOrder::DeveloperInfoOrderBy_NONE;
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_limit_from = 0;
			stDeveloperInfoWithFieldLanguageTypeByOrder.m_limit_len = 10;
			int iRet = stDeveloperInfoWithFieldLanguageTypeByOrder.SelectFromDB(dwTotalNum, strErrMsg);
			if(iRet != TableBase::TABLE_BASE_RET_OK && iRet != TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				LOG4CPLUS_ERROR(logger, "stDeveloperInfoWithFieldLanguageTypeByOrder.SelectFromDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			for(size_t i=0; i!=stDeveloperInfoWithFieldLanguageTypeByOrder.m_developer_info_list.size(); ++i)
			{
				const DeveloperInfo& stDeveloperInfo = stDeveloperInfoWithFieldLanguageTypeByOrder.m_developer_info_list[i];
				UserInfo stUserInfo(m_table_name_user_info);
				stUserInfo.m_uin = stDeveloperInfo.m_uin;
				iRet = stUserInfo.SelectFromDB(strErrMsg);
				if(iRet == TableBase::TABLE_BASE_RET_OK)
				{
					lce::cgi::CAnyValue one;
					one["dev_id"] = stDeveloperInfo.m_id;
					one["head_portait"] = stUserInfo.m_headportait_url;
					GetAnyValue()["recommend_dev_list"].push_back(one);
				}
			}
			

			WXPublicAccountWithTagAndCertifiedStatus stWXPublicAccountWithTagAndCetifiedStatus(m_table_name_wx_public_account);
			stWXPublicAccountWithTagAndCetifiedStatus.m_tag = -1;
			stWXPublicAccountWithTagAndCetifiedStatus.m_cetified_status = 1;
			stWXPublicAccountWithTagAndCetifiedStatus.m_limit_from = 0;
			stWXPublicAccountWithTagAndCetifiedStatus.m_limit_len = 10;
			iRet = stWXPublicAccountWithTagAndCetifiedStatus.SelectFromDB(dwTotalNum, strErrMsg);
			if(iRet != TableBase::TABLE_BASE_RET_OK && iRet != TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				LOG4CPLUS_ERROR(logger, "stWXPublicAccountWithTagAndCetifiedStatus.SelectFromDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			for(size_t i=0; i!=stWXPublicAccountWithTagAndCetifiedStatus.m_wx_public_account_list.size(); ++i)
			{
				const WXPublicAccount& stWXPublicAccount = stWXPublicAccountWithTagAndCetifiedStatus.m_wx_public_account_list[i];
				lce::cgi::CAnyValue one;
				one["wxpa"] = stWXPublicAccount.m_wx;
				one["head_portait"] = stWXPublicAccount.m_headportrait_url;
				GetAnyValue()["recommend_wxpa_list"].push_back(one);
			}
			
			
			
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiRecommendListGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




