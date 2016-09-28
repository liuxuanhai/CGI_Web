#include "main_cgi.h"
class CgiTest302: public MainCgi
{
     public:
        CgiTest302() : MainCgi(0, "config.ini", "logger.properties", CGI_NEED_LOGIN_NO)
        {

        }
        
		bool InnerProcess()
		{
			lce::cgi::CHttpHeader header = GetHeader();
			header.AddHeader("Status: 302");
			//header.AddHeader("Location: http://weixin.qq.com/r/0Tr17XPEUi4trTeO92-D");
			//header.AddHeader("Location: weixin://profile/gh_34bd692a9835");
			header.AddHeader("Location: http://baidu.com");
            //succ
            header.Output();
            return true;
		}
};


int main()
{
    CgiTest302 cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




