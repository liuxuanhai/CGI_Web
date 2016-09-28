#include "manager_cgi.h"

class CgiManagerAdd: public ManagerCgi
{
     public:
        CgiManagerAdd() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
		    
			GetAnyValue()["manager_info"] = m_manager_info.ToAnyValue();
            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiManagerAdd cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


