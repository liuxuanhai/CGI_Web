#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXCardLogoUpload: public HooshoCgiCard
{
public:
	CgiWXCardLogoUpload() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", NO_LOGIN)
	{

	}

	bool InnerProcess()
	{
		string strPicData = (string) GetInput().GetFileData("pic_data");
		string strFilename = (string) GetInput().GetFileName("pic_data");
		if (strPicData.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid pic data");
			return true;
		}

		string strErrMsg = "";
		std::string strMyAccessToken = "";
		std::string strImgurl = "";
		std::string strLogo = strPicData;

		for (int i = 0; i < 2; i++)
		{
			int iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "my access_token="<<strMyAccessToken);

			strImgurl = "";

			iRet = WXHttpsReq::WXAPIUploadLogo(strLogo, strFilename, strMyAccessToken, strImgurl, strErrMsg);
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
				strErrMsg = "WXHttpsReq::WXAPIUploadLogo failed, errmsg=" + strErrMsg;
				DoReply(iRet, strErrMsg);
				return -1;
			}
			break;
		}

		GetAnyValue()["img_url"] = strImgurl;
		DoReply(CGI_RET_CODE_OK);

		return true;
	}
};

int main()
{
	CgiWXCardLogoUpload cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
