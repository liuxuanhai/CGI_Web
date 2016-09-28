#include "hoosho_cgi_card.h"

class CgiCardUserInfoGet: public HooshoCgiCard
{
     public:
        CgiCardUserInfoGet() : HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", USER_LOGIN)
        {

        }
        
		bool InnerProcess()
		{
		    lce::cgi::CAnyValue stAnyValue;
		    m_user_info.ToAnyValue(stAnyValue);
			GetAnyValue()["user_info"] = stAnyValue;
		    DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiCardUserInfoGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


