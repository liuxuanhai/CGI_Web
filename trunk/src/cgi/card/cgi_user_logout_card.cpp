#include "hoosho_cgi_card.h"

class CgiUserLogoutCard: public HooshoCgiCard
{
     public:
        CgiUserLogoutCard() : HooshoCgiCard(0, "config_card.ini", "logger.properties_card", NO_LOGIN)
        {

        }
        
		bool InnerProcess()
		{
		    //clear cookie & reply
		    lce::cgi::CHttpHeader& stHttpRspHeader =  GetHeader();
        	stHttpRspHeader.SetCookie(m_cookie_name_uin, "", m_cookie_domain, 0, m_cookie_path, false, true);
        	stHttpRspHeader.SetCookie(m_cookie_name_key, "", m_cookie_domain, 0, m_cookie_path, false, true);
        	
            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiUserLogoutCard cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

