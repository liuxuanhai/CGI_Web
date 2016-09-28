#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXApplyprotocolGet: public HooshoCgiCard
{
public:

	CgiWXApplyprotocolGet() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{
	}

	bool InnerProcess()
	{
		string strErrMsg = "";
		string strMyAccessToken = "";
		string strApplyprotocol = "";

		for (int i = 0; i < 2; i++)
		{
			int iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed, errmsg =" << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "my access_token=" << strMyAccessToken);

			iRet = WXHttpsReq::WXAPIApplyprotocolGet(strMyAccessToken, "", strApplyprotocol, strErrMsg);
			if (iRet != 0)
			{
				if (iRet == 40001)
				{
					if(!WXHttpsReq::AccessTokenDelete(m_cache_ip, m_cache_port_vc,strErrMsg))
					{
						LOG4CPLUS_ERROR(logger, "WXHttpsReq::AccessTokenDelete failed, errmsg = " << strErrMsg);
						DoReply(CGI_RET_CODE_SERVER_BUSY);
						return true;
					}
					LOG4CPLUS_DEBUG(logger, "get access token again.");
					continue;
				}
				LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXAPIApplyprotocolGet failed, errmsg = " << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			break;
		}
//		LOG4CPLUS_DEBUG(logger, "apply protocol = " << strApplyprotocol);

		Json::Value oJson;
		Json::Reader reader;
		if(!reader.parse(strApplyprotocol, oJson))
		{
				LOG4CPLUS_ERROR(logger, "parse json error");
				return true;
		}
//		oJson = oJson["category"];
//		oJson.toStyledString();
//		for(unsigned int i = 0; i < oJson.size(); i++)
//		{
//			LOG4CPLUS_DEBUG(logger, "" << oJson[i].toJsonString());
//			GetAnyValue()["category"].push_back(oJson[i].toJsonString());
//		}

//		GetAnyValue()["out"]["in"] = "test";
		GetAnyValue()["category"] = strApplyprotocol;



		strApplyprotocol = "{\"category\":" + strApplyprotocol + ", \"ec\": 0}";
		SetOutputJson(strApplyprotocol);
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXApplyprotocolGet cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

