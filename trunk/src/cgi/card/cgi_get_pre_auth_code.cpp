#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiGetPreAuthCode: public HooshoCgiCard
{
     public:
        CgiGetPreAuthCode() : HooshoCgiCard(0, "config_card.ini", "logger.properties_card", NO_LOGIN)
        {

        }
        
		bool InnerProcess()
		{
			int iRet = 0;
			std::string strErrMsg = "";
			OptionsInfo stOptionsInfo(m_table_name_options_info);
			stOptionsInfo.m_id = OPTIONS_ID_VERIFY_TICKET;
			iRet = stOptionsInfo.SelectFromDB(strErrMsg);
			if(iRet != TableBaseCard::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "OptionsInfo.SelectFromDB failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "verify_ticket="<< stOptionsInfo.m_value);

			std::string strComponentAccessToken = "";
			iRet = WXHttpsReq::WXAPIGetComponentAccessToken(WX_THIRD_PLATFORM_DEV_APPID
							, WX_THIRD_PLATFORM_DEV_SECRET
							, stOptionsInfo.m_value
							, strComponentAccessToken
							, strErrMsg);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetComponentAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "component_access_token="<<strComponentAccessToken);
			
			std::string strPreAuthCode = "";
			iRet = WXHttpsReq::WXAPIGetPreAuthCode(WX_THIRD_PLATFORM_DEV_APPID
											, strComponentAccessToken
											, strPreAuthCode
											, strErrMsg);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetPreAuthCode failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "pre_auth_code="<<strPreAuthCode);
			
		    GetAnyValue()["pre_auth_code"] = strPreAuthCode;
            DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiGetPreAuthCode cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


