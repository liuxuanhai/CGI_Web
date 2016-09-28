#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXMemberCardUserInfoQuery: public HooshoCgiCard
{
public:
	CgiWXMemberCardUserInfoQuery() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", false)
	{

	}

	bool InnerProcess()
	{
		string strCardCode = (string) GetInput().GetValue("card_code");
		string strCardid = (string) GetInput().GetValue("card_id");
		EMPTY_STR_RETURN(strCardCode);
		EMPTY_STR_RETURN(strCardid);


		string strErrMsg = "";
		string strQuery = "";

		std::string strMyAccessToken = "";
		for (int i = 0; i < 2; i++)
		{
			int iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			iRet = WXHttpsReq::WXAPIMemberCardUserInfoQuery(strCardid, strCardCode, strMyAccessToken, strQuery, strErrMsg);
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
				LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXAPICardCodeQuery failed, errmsg=" << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			break;
		}

		GetAnyValue()["wx_result"] = strQuery;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXMemberCardUserInfoQuery cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
