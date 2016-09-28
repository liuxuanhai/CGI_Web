#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXCategoryQuery: public HooshoCgiCard
{
public:
	CgiWXCategoryQuery() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{
		string strErrMsg = "";
		string strQuery = "";
		int iRet=0;
		string strMyAccessToken="";
		for (int i = 0; i < 2; i++)
		{
			iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "my access_token="<<strMyAccessToken);
			strQuery = "";
			iRet = WXHttpsReq::WXAPICategoryQuery(strMyAccessToken, strQuery, strErrMsg);
			if (iRet != 0)
			{
				if (iRet == 40001)
				{
					if (!WXHttpsReq::AccessTokenDelete(m_cache_ip, m_cache_port_vc, strErrMsg))
					{
						LOG4CPLUS_ERROR(logger, "WXHttpsReq::AccessTokenDelete failed, errmsg = " << strErrMsg);
						DoReply(CGI_RET_CODE_SERVER_BUSY);
						return true;
					}
					continue;
				}
				strErrMsg = "WXHttpsReq::CategoryQuery failed, errmsg=" + strErrMsg;

				LOG4CPLUS_ERROR(logger, strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return -1;
			}
			break;
		}
		//GetAnyValue()["card_info"] = strQuery;
		SetOutputJson(strQuery);
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXCategoryQuery cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}




