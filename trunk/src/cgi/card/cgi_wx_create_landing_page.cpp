#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXCreateLandingPage: public HooshoCgiCard
{
public:
	CgiWXCreateLandingPage() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", false)
	{

	}

	bool InnerProcess()
	{
		int iRet = 0;
		std::string strErrMsg = "";
		std::string MyAccessToken = "";

		string strPostData = (string) GetInput().GetValue("landing_page_info");
		// std::string strPostData = GetInput().GetPostData();
		if (strPostData.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid strPostData="<<strPostData);
			return true;
		}
		string strPageUrl = "";
		int strPageId = 0;
		string retMsg = "";
		// get MyAccessToken
		for (int i = 0; i < 2; i++)
		{
			iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, MyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "MyAccessToken="<<MyAccessToken);

			iRet = WXHttpsReq::WXAPILandingPageCreate(MyAccessToken, strPostData, strPageUrl, strPageId, retMsg, strErrMsg);
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
				LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXAPILandingPageCreate failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			break;
		}

		LOG4CPLUS_DEBUG(logger, "WXAPILandingPageCreate strPageUrl="<<strPageUrl<<"strPageId="<<strPageId<<"retMsg="<<retMsg);

		if ((strPageUrl == "") && (strPageId == 0) && (retMsg != ""))
		{
			LOG4CPLUS_DEBUG(logger, "stLandingPageInfo return retMsg (errcode!=0)");
			GetAnyValue()["wx_result"] = retMsg;
			DoReply(CGI_RET_CODE_OK);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "stLandingPageInfo is ready to update db");

		LandingPageInfo stLandingPageInfo(m_table_name_landing_page_info);
		stLandingPageInfo.m_page_id = strPageId;
		stLandingPageInfo.m_uin = m_user_info.m_uin;
		stLandingPageInfo.m_url = strPageUrl;
		stLandingPageInfo.m_extra_data = strPostData;

		if (stLandingPageInfo.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "stLandingPageInfo.UpdateToDB failed, errmsg="<<strErrMsg);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "stLandingPageInfo done");

		GetAnyValue()["wx_result"] = strPageUrl;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXCreateLandingPage cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

