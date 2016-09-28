#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWxSubmerchantGet: public HooshoCgiCard
{
public:
	CgiWxSubmerchantGet() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", true)
	{

	}

	bool SubmerchantInfoUpdate(SubmerchantInfo &stSubmerchantInfo)
	{

		time_t tNow = time(0);
		time_t tTime = stSubmerchantInfo.m_expire_timestamp;
		if (tNow > tTime)
		{

			int iRet;
			string strMyAccessToken;
			string strPostData = "{\"merchant_id\": " + stSubmerchantInfo.m_id + "}";
			string strErrMsg;
			string strSubmerchantInfo;
			for (int i = 0; i < 2; i++)
			{
				iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc,
						strMyAccessToken, strErrMsg);
				if (iRet != 0)
				{
					LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed, errmsg = " << strErrMsg);
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}
				LOG4CPLUS_DEBUG(logger, "my access_token = " << strMyAccessToken);
				iRet = WXHttpsReq::WXAPISubmerchantGet(strMyAccessToken, strPostData, strSubmerchantInfo, strErrMsg);
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
					LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXAPISubmerchantGet failed, errmsg = " << strErrMsg);
					DoReply(iRet, strErrMsg);
					return true;
				}
				break;
			}

			Json::Value oJson;
			Json::Reader reader;
			if (!reader.parse(strSubmerchantInfo, oJson, false))
			{
				LOG4CPLUS_ERROR(logger, "parse json error, merchant_info = " << strSubmerchantInfo);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			stSubmerchantInfo.m_brand_name = oJson["brand_name"].asString();
			stSubmerchantInfo.m_logo_url = oJson["logo_url"].asString();

			if (oJson["status"].asString() == "CHECKING")
			{
				stSubmerchantInfo.m_status = SUBMERCHANT_NOT_VERIFY;
			}
			else if (oJson["status"].asString() == "APPROVED")
			{
				stSubmerchantInfo.m_status = SUBMERCHANT_PASS;
			}
			else if (oJson["status"].asString() == "REJECTED")
			{
				stSubmerchantInfo.m_status = SUBMERCHANT_NOT_PASS;
			}
			else if (oJson["status"].asString() == "EXPIRED")
			{
				stSubmerchantInfo.m_status = SUBMERCHANT_EXPIRED;
			}
			else
			{
				LOG4CPLUS_ERROR(logger, "status = " << oJson["status"] << " not define");
			}
			stSubmerchantInfo.m_expire_timestamp = tNow + m_submerchantlist_life;
			stSubmerchantInfo.m_extra_data = strSubmerchantInfo;
			UPDATE_DB(stSubmerchantInfo, strErrMsg);

		}

		return true;
	}

	bool InnerProcess()
	{

		SubmerchantInfoIndexByUin stSubmerchantInfoIndexByUin(m_table_name_submerchant_info);
		stSubmerchantInfoIndexByUin.m_uin = m_user_info.m_uin;
		LOG4CPLUS_DEBUG(logger, "uin = " << m_user_info.m_uin);
		string strErrMsg;
		if (stSubmerchantInfoIndexByUin.SelectFromDB(strErrMsg) == TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_OK);
			GetAnyValue()["merchant_lsit"] = "";
			LOG4CPLUS_DEBUG(logger, "not submerchant found");
			return true;
		}



		for (int i = stSubmerchantInfoIndexByUin.m_submerchant_info_list.size()-1; i >=0; i--)
		{

			SubmerchantInfo stSubmerchantInfo(m_table_name_submerchant_info);
			stSubmerchantInfo.m_agreement = stSubmerchantInfo.m_operator = "";
			stSubmerchantInfo = stSubmerchantInfoIndexByUin.m_submerchant_info_list[i];

			if (!SubmerchantInfoUpdate(stSubmerchantInfo))
			{
				continue;
			}

			lce::cgi::CAnyValue anyValue;
			stSubmerchantInfo.ToAnyValue(anyValue);
			GetAnyValue()["merchant_list"].push_back(anyValue);
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWxSubmerchantGet cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
