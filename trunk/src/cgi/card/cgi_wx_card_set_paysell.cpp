#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWxCardMemberSetPaysell: public HooshoCgiCard
{
public:
	CgiWxCardMemberSetPaysell() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{

		string strCardid = (string) GetInput().GetValue("card_id");
		string strIsopen = (string) GetInput().GetValue("is_open");
		if (strIsopen != "false")
		{
			strIsopen = "true";
		}
		LOG4CPLUS_DEBUG(logger, "postdata="<<strCardid);
		if (strCardid.empty())
		{
			LOG4CPLUS_ERROR(logger, "Paysell Cardid error!! "<<strCardid);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		string strErrMsg = "";
		std::string strMyAccessToken = "";
		string strReturn = "";
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

			iRet = WXHttpsReq::WXAPICardSetPaysell(strMyAccessToken, strCardid, strIsopen, strReturn, strErrMsg);
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
				strErrMsg = "WXHttpsReq::WXAPICardMemberSetPaysell failed, errmsg=" + strErrMsg;
				LOG4CPLUS_ERROR(logger, strErrMsg);
				return -1;
			}
			break;
		}
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWxCardMemberSetPaysell cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

