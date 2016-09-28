#include "main_cgi.h"

class CgiOperApply: public MainCgi
{
     public:
		CgiOperApply() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			if(!m_user_info.m_oper_id.empty())
			{
				DoReply(CGI_RET_CODE_ALREADY_EXISTS);
				LOG4CPLUS_ERROR(logger, "apply oper failed, already is oper, oper_id="<<m_user_info.m_oper_id);
				return true;
			}

			if(m_user_info.m_user_type != USER_TYPE_SERVICE_SIDE)
			{
				DoReply(CGI_RET_CODE_NO_PREVILEDGES);
				LOG4CPLUS_ERROR(logger, "apply oper failed, no priviledges, user_type="<<m_user_info.m_user_type);
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

			EMPTY_STR_RETURN(strIntroduction);
			EMPTY_STR_RETURN(strCompanyName);
			EMPTY_STR_RETURN(strCompanyDetailAddress);
			EMPTY_STR_RETURN(strCompanyPhoneNO);
			EMPTY_STR_RETURN(strCompanyEmail);
			EMPTY_STR_RETURN(strCompanyBusinessLicencePicUrl);
			EMPTY_STR_RETURN(strCompanyCode);
			EMPTY_STR_RETURN(strCompanyCorporationName);

			
			string strErrMsg = "";
            OperInfo stOperInfo(m_table_name_oper_info);
			stOperInfo.m_id = GenerateTransID(ID_TYPE_OPER);
			stOperInfo.m_uin = m_user_info.m_uin;
			
			stOperInfo.m_status = OPERATOR_STATUS_VERIFY_PENDING;
			stOperInfo.m_expire_ts = ONE_YEAR_LATER;
			stOperInfo.m_company_introduction = strIntroduction;
			stOperInfo.m_company_name = strCompanyName;
			stOperInfo.m_company_detail_address = strCompanyDetailAddress;
			stOperInfo.m_company_phone_no = strCompanyPhoneNO;
			stOperInfo.m_company_email = strCompanyEmail;
			stOperInfo.m_company_business_licence_pic_url = strCompanyBusinessLicencePicUrl;
			stOperInfo.m_company_code = strCompanyCode;
			stOperInfo.m_company_corporation_name = strCompanyCorporationName;

			if(stOperInfo.m_id.empty())
			{
				LOG4CPLUS_ERROR(logger, "generate oper id failed, fuck, apply failed");
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			
			if(stOperInfo.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "stOperInfo.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			m_user_info.m_oper_id = stOperInfo.m_id;
			if(m_user_info.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{                           
					LOG4CPLUS_ERROR(logger, "m_user_info.UpdateToDB failed, errmsg="<<strErrMsg);
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
			}

			GetAnyValue()["oper_id"] = stOperInfo.m_id;
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiOperApply cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




