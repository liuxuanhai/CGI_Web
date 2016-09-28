#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXUploadVideo: public HooshoCgiCard
{
public:
	CgiWXUploadVideo() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", false)
	{

	}

	bool InnerProcess()
	{
		string strPostData = (string) GetInput().GetPostData();
		LOG4CPLUS_DEBUG(logger, "POSTDATA="<<strPostData);

		if (strPostData.empty())
		{
			LOG4CPLUS_ERROR(logger, "req data error!!");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		string strErrMsg = "";
		string strMyAccessToken = "";
		string strMediaid = "";
		for (int i = 0; i < 2; i++)
		{
			int iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "my access_token="<<strMyAccessToken);


			iRet = WXHttpsReq::WXAPIUploadVideo(strMyAccessToken, strPostData, strMediaid, strErrMsg);
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
				strErrMsg = "WXHttpsReq::WXAPIUploadVideo failed, errmsg=" + strErrMsg;
				LOG4CPLUS_ERROR(logger, strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return -1;
			}
			break;
		}

		LOG4CPLUS_DEBUG(logger, "media_id="<<strMediaid);

		GetAnyValue()["media_id"] = strMediaid;

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXUploadVideo cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
