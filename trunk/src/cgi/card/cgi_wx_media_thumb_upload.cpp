#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXMediaThumbUpload: public HooshoCgiCard
{
public:
	CgiWXMediaThumbUpload() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{
		string strMediaData = (string) GetInput().GetFileData("media_data");
		string strMediaName = (string) GetInput().GetFileName("media_data");

		EMPTY_STR_RETURN(strMediaData);
		EMPTY_STR_RETURN(strMediaName);


		string strErrMsg = "";
		string strMediaId = "";
		int intCreateTime = 0;

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
			iRet = WXHttpsReq::WXAPIUploadMedia(strMediaData, strMediaName, "thumb", strMyAccessToken, strMediaId, intCreateTime, strErrMsg);
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
				strErrMsg = "WXHttpsReq::WXAPIUploadMedia failed, errmsg=" + strErrMsg;
				LOG4CPLUS_ERROR(logger, strErrMsg);
				DoReply(iRet, strErrMsg);
				return -1;
			}
			break;
		}

		GetAnyValue()["thumb_media_id"] = strMediaId;
		GetAnyValue()["create_time"] = intCreateTime;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXMediaThumbUpload cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
