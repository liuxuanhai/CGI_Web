#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXAuthResult: public HooshoCgiCard
{
     public:
        CgiWXAuthResult() : HooshoCgiCard(0, "config_card.ini", "logger.properties_card", NO_LOGIN)
        {

        }
        
		bool InnerProcess()
		{
			int iRet = 0;
			std::string strErrMsg = "";
			string strAuthCode = (string)GetInput().GetValue("auth_code");
			LOG4CPLUS_DEBUG(logger, "auth_code="<<strAuthCode);
			
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

			
			std::string strAuthAppid = "";
			std::string strAuthAccessToken = "";
			std::string strAuthRefreshToken = "";
			iRet = WXHttpsReq::WXAPIGetAuthInfo(WX_THIRD_PLATFORM_DEV_APPID
											, strComponentAccessToken
											, strAuthCode
											, strAuthAppid
											, strAuthAccessToken
											, strAuthRefreshToken
											, strErrMsg);
			if(iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetAuthInfo failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "auth_appid="<<strAuthAppid
								<<", auth_access_token=<<"<<strAuthAccessToken
								<<", auth_refresh_token="<<strAuthRefreshToken);
			
			//save auth info
			WxAccountAuthInfo stWxAccountAuthInfo(m_table_name_wx_account_auth_info);
			stWxAccountAuthInfo.m_auth_appid = strAuthAppid;
			stWxAccountAuthInfo.m_auth_access_token = strAuthAccessToken;
			stWxAccountAuthInfo.m_auth_refresh_token = strAuthRefreshToken;
			iRet = stWxAccountAuthInfo.UpdateToDB(strErrMsg);
			if(iRet != TableBaseCard::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "WxAccountAuthInfo.UpdateToDB failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			GetAnyValue()["appid"] =strAuthAppid ;
            DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiWXAuthResult cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


