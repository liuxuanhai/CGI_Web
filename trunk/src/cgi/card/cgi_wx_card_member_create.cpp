#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWxCardMemberCreate: public HooshoCgiCard
{
public:
	CgiWxCardMemberCreate() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{
		string strPostData = (string) GetInput().GetValue("card_info");
		LOG4CPLUS_DEBUG(logger, "postdata="<<strPostData);
		if (strPostData.empty())
		{
			LOG4CPLUS_ERROR(logger, "req data error!!");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		string strErrMsg = "";
		std::string strMyAccessToken = "";
		string strCardid = "";
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


			iRet = WXHttpsReq::WXAPICardCreate(strMyAccessToken, strPostData, strCardid, strErrMsg);
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
				strErrMsg = "WXHttpsReq::WXAPICardMemberCreate failed, errmsg=" + strErrMsg;
				LOG4CPLUS_ERROR(logger, strErrMsg);
				return -1;
			}
			break;
		}
		string strParseData = strPostData;
		//check duplicate
		Json::Value oJson;
		Json::Reader reader;
		LOG4CPLUS_DEBUG(logger, "membercard Json prase");
		if (!reader.parse(strParseData, oJson, false))
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}
		if (!oJson.hasKey("card") || !oJson["card"].isObject())
		{
			strErrMsg = "parse card error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}

		strParseData = oJson["card"].toJsonString();

		if (!reader.parse(strParseData, oJson, false))
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}
		if (!oJson.hasKey("card_type"))
		{
			strErrMsg = "parse card_type error , resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}

		if (oJson["card_type"].asString() != "MEMBER_CARD")
		{
			strErrMsg = "card_type error not membercard, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}
		if (!oJson.hasKey("member_card") || !oJson["member_card"].isObject())
		{
			strErrMsg = "parse member_card error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}
		strParseData = oJson["member_card"].toJsonString();
		if (!reader.parse(strParseData, oJson, false))
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}

		strParseData = oJson["base_info"].toJsonString();

		if (!reader.parse(strParseData, oJson, false))
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}
		if (!oJson.hasKey("logo_url"))
		{
			strErrMsg = "parse logo_url error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}
		if (!oJson.hasKey("title"))
		{
			strErrMsg = "parse title error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}

		string strLogoUrl = oJson["logo_url"].asString();
		string strTitle = oJson["title"].asString();

		CardInfo stCardInfo(m_table_name_card_info);
		stCardInfo.m_id = strCardid;

		if (stCardInfo.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_ALREADY_EXISTS);
			LOG4CPLUS_ERROR(logger, "db card_info already exists cardid="<<strCardid<<","<<strErrMsg);
			return true;
		}
		stCardInfo.m_type = CARD_MEMBER;
		stCardInfo.m_status = CARD_NOT_VERIFY;
		stCardInfo.m_logourl = strLogoUrl;
		stCardInfo.m_title = strTitle;
		//		stCardInfo.m_uin ="hoosho";
		stCardInfo.m_extra_data = strPostData;

		if (stCardInfo.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "stCardInfo.UpdateToDB failed, errmsg="<<strErrMsg);
			return true;
		}

		GetAnyValue()["card_id"] = strCardid;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWxCardMemberCreate cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
