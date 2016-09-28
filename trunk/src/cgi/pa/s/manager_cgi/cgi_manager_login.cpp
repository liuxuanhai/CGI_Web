#include "cgi_manager_base.h"

class CgiManagerLogin:public CgiManagerBase
{
public:
	CgiManagerLogin():CgiManagerBase(0, "config.ini", "logger.properties", false)
	{

	}

	bool InnerProcess()
	{
		std::string strPhone = GetInput().GetValue("phone");
		std::string strPasswd = GetInput().GetValue("passwd");
		string strVC = GetInput().GetValue("vc");

		EMPTY_STR_RETURN(strPhone);
		EMPTY_STR_RETURN(strVC);

		if(!IsValidPhone(strPhone))
		{
			LOG4CPLUS_ERROR(logger, "phone=" << strPhone << " invalid");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		std::string strErrMsg;
		//check
		if(!CheckPasswd(strPhone, strPasswd, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "passwd check failed, errmsg=" << strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PASSWORD);
			return true;
		}

		if(!VerifyCodeCheck(strPhone, strVC, strErrMsg))
		{
			LOG4CPLUS_ERROR(logger, "vc check failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		//set cookie
		lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
		if(MakeAddReplyCookie(strPhone, stHttpRspHeader, strErrMsg) < 0)
		{
			LOG4CPLUS_ERROR(logger, "add reply cookie failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		LOG4CPLUS_INFO(logger, "phone=" << strPhone << " login in");

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiManagerLogin cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
