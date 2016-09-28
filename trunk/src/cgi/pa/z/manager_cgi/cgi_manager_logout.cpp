#include "cgi_manager_base.h"

class CgiManagerLogout:public CgiManagerBase
{
public:
	CgiManagerLogout():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{
        //clear cookie & reply
		lce::cgi::CHttpHeader& stHttpRspHeader =  GetHeader();
		stHttpRspHeader.SetCookie(m_cookie_name_uin, "", m_cookie_domain, 0, m_cookie_path, false, true);
		stHttpRspHeader.SetCookie(m_cookie_name_key, "", m_cookie_domain, 0, m_cookie_path, false, true);

		LOG4CPLUS_INFO(logger, "phone=" << m_login_phone << " login out");

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiManagerLogout cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
