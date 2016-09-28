#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWxSubmerchantUpdate: public HooshoCgiCard
{
public:
	CgiWxSubmerchantUpdate() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", false)
	{

	}

	bool InnerProcess()
	{
		string strPostData = (string) GetInput().GetPostData();
		LOG4CPLUS_DEBUG(logger, "postdata="<<strPostData);
		if (strPostData.empty())
		{
			LOG4CPLUS_ERROR(logger, "req data error!!");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		string strErrMsg = "";
		std::string strMyAccessToken = "";
		string strMerchantid = "";

		string strParseData = strPostData;
		//check duplicate
		Json::Value oJson;
		Json::Reader reader;
		LOG4CPLUS_DEBUG(logger, "Json prase");
		if (!reader.parse(strParseData, oJson, false))
		{
			strErrMsg = "parse json error, postdata=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		if (!oJson.hasKey("info") || !oJson["info"].isObject())
		{
			strErrMsg = "parse card error, postdata=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		strParseData = oJson["info"].toJsonString();

		if (!reader.parse(strParseData, oJson, false))
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		if (!oJson.hasKey("merchant_id"))
		{
			strErrMsg = "parse card_type error , resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		strMerchantid = oJson["merchant_id"].asString();
		SubmerchantInfo stSubmerchantInfo(m_table_name_submerchant_info);
		stSubmerchantInfo.m_id = strMerchantid;
		if(stSubmerchantInfo.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "no data with merchant_id = " << strMerchantid << " in db, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		if(stSubmerchantInfo.m_uin != m_user_info.m_uin)
		{
			LOG4CPLUS_ERROR(logger, "merchant_id = " << strMerchantid  
							<< " belong to uin = " << stSubmerchantInfo.m_uin
							<< ", not belong to uin = " << m_user_info.m_uin);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		if(stSubmerchantInfo.m_status != SUBMERCHANT_NOT_PASS)
		{
			LOG4CPLUS_ERROR(logger, "submerchant is not verify or pass, can't update");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		if(!oJson.hasKey("brand_name") || !oJson.hasKey("logo_url"))
		{
			LOG4CPLUS_ERROR(logger, "no brand_name or logo_url in postdata, postdata = " << strPostData);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		stSubmerchantInfo.m_brand_name = oJson["brand_name"].asString();
		stSubmerchantInfo.m_logo_url = oJson["logo_url"].asString();
		stSubmerchantInfo.m_status = SUBMERCHANT_NOT_VERIFY;
		stSubmerchantInfo.m_wx_msg = "";
		stSubmerchantInfo.m_extra_data = strPostData;

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

			iRet = WXHttpsReq::WXAPISubmerchantUpdate(strMyAccessToken, strPostData, strErrMsg);
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
				LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXAPISubmerchantUpdate failed, errmsg=" << strErrMsg);
				DoReply(iRet, strErrMsg);
				return true;
			}
			break;
		}
		
		if(stSubmerchantInfo.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "stSubmerchantInfo.UpdateToDB failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		GetAnyValue()["merchant_id"] = strMerchantid;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWxSubmerchantUpdate cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
