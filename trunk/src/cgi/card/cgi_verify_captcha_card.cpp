#include "hoosho_cgi_card.h"
#include "wx_https_req.h"
#include <stdio.h>

class CgiVerifyCaptchaCard: public HooshoCgiCard
{
     public:
		CgiVerifyCaptchaCard() : HooshoCgiCard(0, "config_card.ini", "logger.properties_card", NO_LOGIN)
        {

        }

		bool InnerProcess()
		{
			string token = (string)GetInput().GetValue("captcha_token");
			string ans = (string)GetInput().GetValue("captcha_ans");
			string strErrMsg = "";

            if(token.empty())
            {
                DoReply(CGI_RET_CODE_INVALID_PARAM);
                LOG4CPLUS_ERROR(logger, "invalid captcha token");
                return true;
            }

            if(ans.empty())
            {
                DoReply(CGI_RET_CODE_INVALID_PARAM);
                LOG4CPLUS_ERROR(logger, "invalid captcha answer");
                return true;
            }

            if(CaptchaCheck(token, ans, strErrMsg) == -1)
            {
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "check captcha token failed, errmsg="<<strErrMsg);
				return true;
            }

            if (CaptchaCheck(token, ans, strErrMsg))
            {
            	// delete in memcached
                if(!MemcachedDelete(token, strErrMsg))
                {
                    DoReply(CGI_RET_CODE_INVALID_PARAM);
    		    LOG4CPLUS_ERROR(logger, "delete captcha token failed, errmsg="<<strErrMsg);
    		    return true;
                }

            	GetAnyValue()["captcha_result"] = true;
            	DoReply(CGI_RET_CODE_OK);
    			return true;
            }

            GetAnyValue()["captcha_result"] = false;
            DoReply(CGI_RET_CODE_OK);
    		return true;



		}
};


int main()
{
	CgiVerifyCaptchaCard cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

