#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXMediaImageDownload: public HooshoCgiCard
{
public:
	CgiWXMediaImageDownload() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", false)
	{

	}

    void Return404()
        {
        	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
        	stHttpRspHeader.AddHeader("Status: 404");
        	stHttpRspHeader.Output();
        }

    void Return200(const string& strPicData)
    {
        GetHeader().AddHeader("Cache-Control: max-age=0, must-revalidate");
        GetHeader().SetContentType("image/png");

        GetHeader().Output();
        if(!std::cout.good())
        {
            std::cout.clear();
            std::cout.flush();
        }

        std::cout<<strPicData;

        return;
    }

	bool InnerProcess()
	{
		string strMediaId = (string) GetInput().GetValue("media_id");

		if (strMediaId.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "empty strMediaId, fuck out");
			return true;
		}

		std::string strErrMsg = "";
		std::string strPicData = "";
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

			iRet = WXHttpsReq::WXAPIDownloadMedia(strMediaId, strMyAccessToken, strPicData, strErrMsg);
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
				strErrMsg = "WXHttpsReq::WXAPIDownloadMedia failed, errmsg=" + strErrMsg;
				LOG4CPLUS_ERROR(logger, strErrMsg);
				DoReply(iRet, strErrMsg);
				return -1;
			}
			break;
		}

        Return200(strPicData);
		return true;
	}
};

int main()
{
	CgiWXMediaImageDownload cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
