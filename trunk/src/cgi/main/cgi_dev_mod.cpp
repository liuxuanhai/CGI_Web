#include "main_cgi.h"

class CgiDevMod: public MainCgi
{
     public:
        CgiDevMod() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }
        
		bool InnerProcess()
		{
			if(m_user_info.m_dev_id.empty())
			{
				DoReply(CGI_RET_CODE_NOT_DEV);
				LOG4CPLUS_ERROR(logger, "m_user_info.m_dev_id is empty(), not dev, can not mod devinfo");
				return true;
			}

			string strErrMsg = "";
			DeveloperInfo stDevInfo(m_table_name_dev_info);
			stDevInfo.m_id = m_user_info.m_dev_id;
			if(stDevInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stDevInfo.SelectFromDB failed, dev_id="<<stDevInfo.m_id);
				return true;
			}

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
			
			bool bChanged = false;
			if(!strRegionArea.empty() && strRegionArea != stDevInfo.m_region_area)
			{
				bChanged = true;
				stDevInfo.m_region_area = strRegionArea;
			}

			if(!strIntroduction.empty() && strIntroduction != stDevInfo.m_introduction)
			{
				bChanged = true;
				stDevInfo.m_introduction = strIntroduction;
			}

			if(qwDevField != 0 && qwDevField != stDevInfo.m_dev_field)
			{
				bChanged = true;
				stDevInfo.m_dev_field = qwDevField;
			}

			if(qwDevLanguage != 0 && qwDevLanguage != stDevInfo.m_dev_language)
			{
				bChanged = true;
				stDevInfo.m_dev_language= qwDevLanguage;
			}

			if(!strShowCase.empty() && strShowCase != stDevInfo.m_show_case)
			{
				bChanged = true;
				stDevInfo.m_show_case = strShowCase;  
			}

			if(!strCompanyName.empty() && strCompanyName != stDevInfo.m_company_name)
			{
				bChanged = true;
				stDevInfo.m_company_name = strCompanyName;
			}

			if(!strCompanyDetailAddress.empty() && strCompanyDetailAddress != stDevInfo.m_company_detail_address)
			{
				bChanged = true;
				stDevInfo.m_company_detail_address = strCompanyDetailAddress;
			}

			if(!strCompanyPhoneNO.empty() && strCompanyPhoneNO != stDevInfo.m_company_phone_no)
			{
				bChanged = true;
				stDevInfo.m_company_phone_no = strCompanyPhoneNO;
			}

			if(!strCompanyEmail.empty() && strCompanyEmail != stDevInfo.m_company_email)
			{
				bChanged = true;
				stDevInfo.m_company_email = strCompanyEmail;
			}

			if(!strCompanyBusinessLicencePicUrl.empty() && strCompanyBusinessLicencePicUrl != stDevInfo.m_company_business_licence_pic_url)
			{
				bChanged = true;
				stDevInfo.m_company_business_licence_pic_url = strCompanyBusinessLicencePicUrl;
			}

			if(!strCompanyCode.empty() && strCompanyCode != stDevInfo.m_company_code)
			{
				bChanged = true;
				stDevInfo.m_company_code = strCompanyCode;
			}

			if(!strCompanyCorporationName.empty() && strCompanyCorporationName != stDevInfo.m_company_corporation_name)
			{
				bChanged = true;
				stDevInfo.m_company_corporation_name = strCompanyCorporationName;
			}
			
			if(bChanged)
			{
				//×´Ì¬¸ÄÎª´ýÉóºË
				stDevInfo.m_status = DEVELOPER_STATUS_VERIFY_PENDING;
				
		        if(stDevInfo.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
		        {
		            DoReply(CGI_RET_CODE_SERVER_BUSY);
					LOG4CPLUS_ERROR(logger, "stDevInfo.UpdateToDB failed, errmsg="<<strErrMsg);
					return true;
		        }
			}

			lce::cgi::CAnyValue new_dev_info;
			stDevInfo.ToAnyValue(new_dev_info);
			GetAnyValue()["dev_info"] = new_dev_info;
		    
            DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiDevMod cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


