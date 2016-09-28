#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXCardCodeGet: public HooshoCgiCard
{
public:
	CgiWXCardCodeGet() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", NO_LOGIN)
	{

	}

	bool InnerProcess()
	{
		uint32_t iflag = (uint32_t) GetInput().GetValue("flag");
		if (iflag != 2)
		{
			iflag = 1;
		}

		string strCardid = (string) GetInput().GetValue("card_id");
		if (strCardid.empty())
		{
			LOG4CPLUS_ERROR(logger, "cgi_card_code_get card_id error , card_id="<<strCardid);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		uint32_t iExpireSeconds = (uint32_t) GetInput().GetValue("expire_seconds");
		if (iExpireSeconds != 0)
		{
			if (iExpireSeconds < 60 || iExpireSeconds > 1800)
			{
				LOG4CPLUS_ERROR(logger, "cgi_card_code_get iExpireSeconds error , iExpireSeconds="<<iExpireSeconds);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
		}
		////////
		iflag = 1;
		iExpireSeconds = 0;

		string strErrMsg = "";
		std::string strMyAccessToken = "";
		string strTicket = "";
		string strShowCodeUrl = "";

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
			//https://api.weixin.qq.com/card/get?access_token=TOKEN
			strTicket = "";
			strShowCodeUrl = "";
			iRet = WXHttpsReq::WXAPIGetCardCodeInfo(strCardid, strMyAccessToken, iExpireSeconds, iflag, strTicket, strShowCodeUrl, strErrMsg);

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
				strErrMsg = "WXHttpsReq::WXAPIGetCardCodeInfo failed, errmsg=" + strErrMsg;
				DoReply(iRet, strErrMsg);
				LOG4CPLUS_ERROR(logger, "WXAPIGetCardCodeInfo failed , errmsg="<<strErrMsg);
				return -1;
			}
			break;
		}

		GetAnyValue()["ticket"] = strTicket;
		GetAnyValue()["code_url"] = strShowCodeUrl;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXCardCodeGet cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
