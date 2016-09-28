#include "main_cgi.h"

class CgiOperMod: public MainCgi
{
     public:
        CgiOperMod() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }
        
		bool InnerProcess()
		{
			if(m_user_info.m_oper_id.empty())
			{
				DoReply(CGI_RET_CODE_NOT_OPER);
				LOG4CPLUS_ERROR(logger, "m_user_info.m_dev_id is empty(), not dev, can not mod devinfo");
				return true;
			}

			string strErrMsg = "";
			OperInfo stOperInfo(m_table_name_oper_info);
			stOperInfo.m_id = m_user_info.m_oper_id;
			if(stOperInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stOperInfo.SelectFromDB failed, dev_id="<<stOperInfo.m_id);
				return true;
			}

			std::string strIntroduction =(string)GetInput().GetValue("introduction");
			std::string strCompanyName =(string)GetInput().GetValue("company_name");
			std::string strCompanyDetailAddress = (string)GetInput().GetValue("company_detail_address");
			std::string strCompanyPhoneNO = (string)GetInput().GetValue("company_phone_no");
			std::string strCompanyEmail = (string)GetInput().GetValue("company_email");
			std::string strCompanyBusinessLicencePicUrl = (string)GetInput().GetValue("company_business_licence_pic_url");
			std::string strCompanyCode = (string)GetInput().GetValue("company_code");
			std::string strCompanyCorporationName = (string)GetInput().GetValue("company_corporation_name");
			
			bool bChanged = false;
			if(!strIntroduction.empty() && strIntroduction != stOperInfo.m_company_introduction)
			{
				bChanged = true;
				stOperInfo.m_company_introduction = strIntroduction;
			}

			if(!strCompanyName.empty() && strCompanyName != stOperInfo.m_company_name)
			{
				bChanged = true;
				stOperInfo.m_company_name = strCompanyName;
			}

			if(!strCompanyDetailAddress.empty() && strCompanyDetailAddress != stOperInfo.m_company_detail_address)
			{
				bChanged = true;
				stOperInfo.m_company_detail_address = strCompanyDetailAddress;
			}

			if(!strCompanyPhoneNO.empty() && strCompanyPhoneNO != stOperInfo.m_company_phone_no)
			{
				bChanged = true;
				stOperInfo.m_company_phone_no = strCompanyPhoneNO;
			}

			if(!strCompanyEmail.empty() && strCompanyEmail != stOperInfo.m_company_email)
			{
				bChanged = true;
				stOperInfo.m_company_email = strCompanyEmail;
			}

			if(!strCompanyBusinessLicencePicUrl.empty() && strCompanyBusinessLicencePicUrl != stOperInfo.m_company_business_licence_pic_url)
			{
				bChanged = true;
				stOperInfo.m_company_business_licence_pic_url = strCompanyBusinessLicencePicUrl;
			}

			if(!strCompanyCode.empty() && strCompanyCode != stOperInfo.m_company_code)
			{
				bChanged = true;
				stOperInfo.m_company_code = strCompanyCode;
			}

			if(!strCompanyCorporationName.empty() && strCompanyCorporationName != stOperInfo.m_company_corporation_name)
			{
				bChanged = true;
				stOperInfo.m_company_corporation_name = strCompanyCorporationName;
			}
			
			if(bChanged)
			{
				//×´Ì¬¸ÄÎª´ýÉóºË
				stOperInfo.m_status = OPERATOR_STATUS_VERIFY_PENDING;
				
		        if(stOperInfo.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
		        {
		            DoReply(CGI_RET_CODE_SERVER_BUSY);
					LOG4CPLUS_ERROR(logger, "stOperInfo.UpdateToDB failed, errmsg="<<strErrMsg);
					return true;
		        }

				lce::cgi::CAnyValue new_oper_info;
				stOperInfo.ToAnyValue(new_oper_info);
				GetAnyValue()["oper_info"] = new_oper_info;
			}
		    
            DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiOperMod cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


