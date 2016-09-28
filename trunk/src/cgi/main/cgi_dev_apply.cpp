#include "main_cgi.h"

class CgiDevApply: public MainCgi
{
     public:
		CgiDevApply() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			if(!m_user_info.m_dev_id.empty())
			{
				DoReply(CGI_RET_CODE_ALREADY_EXISTS);
				LOG4CPLUS_ERROR(logger, "apply dev failed, already is dev, dev_id="<<m_user_info.m_dev_id);
				return true;
			}

			if(m_user_info.m_user_type != USER_TYPE_SERVICE_SIDE)
			{
				DoReply(CGI_RET_CODE_NO_PREVILEDGES);
				LOG4CPLUS_ERROR(logger, "apply dev failed, no priviledges, user_type="<<m_user_info.m_user_type);
				return true;
			}
		
			uint32_t dwDevType =(uint32_t)GetInput().GetValue("dev_type");
			std::string strRegionArea =(string)GetInput().GetValue("region_area");
			std::string strIntroduction =(string)GetInput().GetValue("introduction");
			uint64_t qwDevField =(uint64_t)GetInput().GetValue("dev_field");
			uint64_t qwDevLanguage =(uint64_t)GetInput().GetValue("dev_language");
			std::string strShowCase =(string)GetInput().GetValue("show_case");
			
			std::string strCompanyName =(string)GetInput().GetValue("company_name");
			std::string strCompanyDetailAddress = (string)GetInput().GetValue("company_detail_address");
			std::string strCompanyPhoneNO = (string)GetInput().GetValue("company_phone_no");
			std::string strCompanyEmail = (string)GetInput().GetValue("company_email");
			std::string strCompanyBusinessLicencePicUrl = (string)GetInput().GetValue("company_business_licence_pic_url");
			std::string strCompanyCode = (string)GetInput().GetValue("company_code");
			std::string strCompanyCorporationName = (string)GetInput().GetValue("company_corporation_name");

			if(dwDevType != DEVELOPER_TYPE_PERSON && dwDevType != DEVELOPER_TYPE_COMPANY)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid dwDevType="<<dwDevType);
				return true;
			}
			
			ZERO_INT_RETURN(qwDevField);
			ZERO_INT_RETURN(qwDevLanguage);
			EMPTY_STR_RETURN(strRegionArea);
			EMPTY_STR_RETURN(strIntroduction);

			if(DEVELOPER_TYPE_COMPANY == dwDevType)
			{
				EMPTY_STR_RETURN(strCompanyName);
				EMPTY_STR_RETURN(strCompanyDetailAddress);
				EMPTY_STR_RETURN(strCompanyPhoneNO);
				EMPTY_STR_RETURN(strCompanyEmail);
				EMPTY_STR_RETURN(strCompanyBusinessLicencePicUrl);
				EMPTY_STR_RETURN(strCompanyCode);
				EMPTY_STR_RETURN(strCompanyCorporationName);
			}
			
			string strErrMsg = "";
            DeveloperInfo stDeveloperInfo(m_table_name_dev_info);
			stDeveloperInfo.m_id = GenerateTransID(ID_TYPE_DEV);
			stDeveloperInfo.m_uin = m_user_info.m_uin;
			stDeveloperInfo.m_type = dwDevType;
			stDeveloperInfo.m_status = DEVELOPER_STATUS_VERIFY_PENDING;
			stDeveloperInfo.m_expire_ts = ONE_YEAR_LATER;
			stDeveloperInfo.m_evaluate_active = 30;
			stDeveloperInfo.m_evaluate_quality = 30;
			stDeveloperInfo.m_evaluate_ontime = 30;
			stDeveloperInfo.m_region_area = strRegionArea;
			stDeveloperInfo.m_introduction = strIntroduction;
			stDeveloperInfo.m_dev_field = qwDevField;
			stDeveloperInfo.m_dev_language = qwDevLanguage;
			stDeveloperInfo.m_show_case = strShowCase;
			stDeveloperInfo.m_company_name = strCompanyName;
			stDeveloperInfo.m_company_detail_address = strCompanyDetailAddress;
			stDeveloperInfo.m_company_phone_no = strCompanyPhoneNO;
			stDeveloperInfo.m_company_email = strCompanyEmail;
			stDeveloperInfo.m_company_business_licence_pic_url = strCompanyBusinessLicencePicUrl;
			stDeveloperInfo.m_company_code = strCompanyCode;
			stDeveloperInfo.m_company_corporation_name = strCompanyCorporationName;
			stDeveloperInfo.m_deal_num = 0;

			if(stDeveloperInfo.m_id.empty())
			{
				LOG4CPLUS_ERROR(logger, "generate dev id failed, fuck, apply failed");
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			
			if(stDeveloperInfo.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "stDeveloperInfo.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			m_user_info.m_dev_id = stDeveloperInfo.m_id;
			if(m_user_info.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "m_user_info.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			GetAnyValue()["dev_id"] = stDeveloperInfo.m_id;
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiDevApply cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




