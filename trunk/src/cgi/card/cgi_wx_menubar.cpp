#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXMenubar: public HooshoCgiCard
{
public:
	CgiWXMenubar() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", false)
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");
		string strState = GetInput().GetValue("state");

		LOG4CPLUS_DEBUG(logger, "code = " << strCode << ", state = " << strState);

		string strAccessToken;
		string strRefreshToken;
		string strOpenid;
		string strScope;
		string strErrMsg;
		int iRet;
		iRet = WXHttpsReq::WXAPIGetOAuthAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, strCode, strAccessToken, strRefreshToken, strOpenid, strScope, strErrMsg);
		if(iRet != 0)
		{
			LOG4CPLUS_DEBUG(logger, "GetOAuthAccessToken failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		LOG4CPLUS_DEBUG(logger, "access_token = " << strAccessToken << "refresh_token = " << strRefreshToken << ", openid = " << strOpenid << ", scope = " << strScope);
		LOG4CPLUS_DEBUG(logger, "strRspBody = " << strErrMsg);


		string strLang = "zh_CN";
		string strInfo;
		iRet = WXHttpsReq::WXAPIGetUserInfo(strAccessToken, strOpenid, strLang, strInfo, strErrMsg);
		if(iRet != 0)
		{

			LOG4CPLUS_DEBUG(logger, "GetOAuthAccessToken failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		
		LOG4CPLUS_DEBUG(logger, "info = " << strInfo);


		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXMenubar cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
