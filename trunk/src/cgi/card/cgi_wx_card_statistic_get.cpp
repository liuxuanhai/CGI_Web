#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXCardCardinfoGet: public HooshoCgiCard
{
public:
	CgiWXCardCardinfoGet() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", true)
	{

	}

	bool InnerProcess()
	{
		string strPostData = (string) GetInput().GetPostData();
		EMPTY_STR_RETURN(strPostData);
		string strMyAccessToken;
		string strErrMsg;
		string strCardinfo;
		int iRet;

		for(int i = 0; i < 2; i++)
		{
			iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if(iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed, errmsg = " << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "my access_token = " << strMyAccessToken);
			iRet = WXHttpsReq::WXAPICardCardinfoGet(strMyAccessToken, strPostData, strCardinfo, strErrMsg);
			if(iRet != 0)
			{
				if(iRet == 40001)
				{
					if(!WXHttpsReq::AccessTokenDelete(m_cache_ip, m_cache_port_vc, strErrMsg))
					{
						LOG4CPLUS_ERROR(logger, "WXHttpsReq::AccessTokenDelete failed, errmsg = " << strErrMsg);
						DoReply(CGI_RET_CODE_SERVER_BUSY);
						return true;
					}
					continue;
				}
				LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXAPICardCardinfoGet failed, errmsg = " << strErrMsg);
				DoReply(iRet, strErrMsg);
				return -1;
			}
			break;
		}

		Json::Value oJson;
		Json::Reader reader;
		if(!reader.parse(strCardinfo, oJson, false))
		{
			LOG4CPLUS_ERROR(logger, "parse json error,  json = " << strCardinfo);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		GetAnyValue()["list"] = oJson["list"].toJsonString();
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXCardCardinfoGet cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
