#include "cgi_manager_base.h"
#include "sms_https_req.h"

class CgiManagerGetVC:public CgiManagerBase
{
public:
	CgiManagerGetVC():CgiManagerBase(0, "config.ini", "logger.properties", false)
	{

	}

	bool InnerProcess()
	{
		std::string strPhone = GetInput().GetValue("phone");

		EMPTY_STR_RETURN(strPhone);

		if(!IsValidPhone(strPhone))
		{
			LOG4CPLUS_ERROR(logger, "phone=" << strPhone << " invalid");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		std::string strVC = "";
		std::string strErrMsg = "";
		//get old
		if(!VerifyCodeQuery(strPhone, strVC, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "vc query failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		//add new
		if(strVC.empty())
		{
			strVC = int_2_str(common::util::GetRandNum(100000, 999999));
			LOG4CPLUS_INFO(logger, "phone=" << strPhone << ", vc=" << strVC);

			if(!VerifyCodeAdd(strPhone, strVC, strErrMsg))
			{
				LOG4CPLUS_ERROR(logger, "vc add failed, errmsg = " << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
		}

		//send sms
		::common::smsapi::SMSHttpsReq smsSend(m_sms_api_host
	 				, m_sms_api_port
	 				, m_sms_api_account
	 				, m_sms_api_accountpwd
	 				, m_sms_api_appid
	 				, m_sms_api_version
	 				, m_sms_templateid);

		if(smsSend.SMSUnifiedPost(strPhone, strVC, strErrMsg)< 0)
		{	      
			LOG4CPLUS_ERROR(logger, "SendRegisterSMS failed, errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiManagerGetVC cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
