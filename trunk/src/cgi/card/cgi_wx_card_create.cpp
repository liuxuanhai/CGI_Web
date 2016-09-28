#include "hoosho_cgi_card.h"
#include "wx_https_req.h"
#include <iostream>

class CgiWxCardCreate: public HooshoCgiCard
{
public:
	CgiWxCardCreate() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", true)
	{

	}

	bool InnerProcess()
	{
//			string strPostData = (string)GetInput().GetValue("card_info");
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
		int iRet = 0;
		string strCardid = "";
		string strExtradata= "";
		string strParseData = strPostData;

		//parse json
		Json::Value oJson;
		Json::Reader reader;
		if (!reader.parse(strParseData, oJson, false) || !oJson.hasKey("card") || !oJson["card"].isObject())
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		strParseData = oJson["card"].toJsonString();
		strExtradata = strParseData;
		if (!reader.parse(strParseData, oJson, false) || !oJson.hasKey("card_type"))
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		std::string strCardType = oJson["card_type"].asString();
		CardInfo stCardInfo(m_table_name_card_info);
		if (strCardType == "MEMBER")
			stCardInfo.m_type = CARD_MEMBER;
		else if (strCardType == "GROUPON")
			stCardInfo.m_type = CARD_GROUPON;
		else if (strCardType == "CASH")
			stCardInfo.m_type = CARD_CASH;
		else if (strCardType == "DISCOUNT")
			stCardInfo.m_type = CARD_DISCOUNT;
		else if (strCardType == "GIFT")
			stCardInfo.m_type = CARD_GIFT;
		else if (strCardType == "GENERAL_COUPON")
			stCardInfo.m_type = CARD_GENERAL_COUPON;
		else
		{
			strErrMsg = "card_type error not exist, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, "cardtype = " << strCardType << ", errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		for (unsigned int i = 0; i < strCardType.length(); i++)
			if (strCardType[i] >= 'A' && strCardType[i] <= 'Z')
				strCardType[i] = strCardType[i] - 'A' + 'a';
		if (!oJson.hasKey(strCardType) || !oJson[strCardType].isObject())
		{
			strErrMsg = "parse " + strCardType + " error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		
		strParseData = oJson[strCardType].toJsonString();
		if (!reader.parse(strParseData, oJson, false) || !oJson.hasKey("base_info"))
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		strParseData = oJson["base_info"].toJsonString();
		if (!reader.parse(strParseData, oJson, false) || !oJson.hasKey("logo_url") || !oJson.hasKey("title") || !oJson.hasKey("date_info"))
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		string strLogoUrl = oJson["logo_url"].asString();
		string strTitle = oJson["title"].asString();
		string strDateInfo = oJson["date_info"].toJsonString();

		if(!oJson.hasKey("sub_merchant_info") || !oJson["sub_merchant_info"].isObject())
		{
			strErrMsg = "parse error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		if(!oJson.hasKey("sku") || !oJson["sku"].isObject())
		{
			strErrMsg = "parse error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		strParseData = oJson["sub_merchant_info"].toJsonString();
		string strAnoParseData = oJson["sku"].toJsonString();
        if(!reader.parse(strParseData, oJson, false) || !oJson.hasKey("merchant_id"))
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		ostringstream ossid;
		ossid.str("");
		LOG4CPLUS_DEBUG(logger, "get merchant_id");
		ossid << oJson["merchant_id"].asInt();
		string strMerchantid = ossid.str();
		LOG4CPLUS_DEBUG(logger, "get merchant_id = " << strMerchantid);

		if(!reader.parse(strAnoParseData, oJson, false) || !oJson.hasKey("quantity"))
		{
			LOG4CPLUS_ERROR(logger, "parse json error, parse data = " << strAnoParseData);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		uint32_t iQuantity = oJson["quantity"].asInt();

		for (int i = 0; i < 2; i++)
		{
			iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "my access_token="<<strMyAccessToken);

			//////////////////
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
				/*
				else if(iRet == 40079)
				{
					DoReply(CGI_RET_CODE_WX_INVALID_TIME);
					GetAnyValue()["errmsg"] = strErrMsg;
					return true;
				}
				else if(iRet == 45040)
				{
					DoReply(CGI_RET_CODE_WX_REACH_MAX_MONTH_LIMIT);
					GetAnyValue()["errmsg"] = strErrMsg;
					return true;
				}
				*/
				strErrMsg = "WXHttpsReq::WXAPICardCreate failed, errmsg=" + strErrMsg;
				DoReply(iRet, strErrMsg);	
				LOG4CPLUS_ERROR(logger, strErrMsg);
				return true;
			}
			break;
		}
		stCardInfo.m_id = strCardid;

		if (stCardInfo.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_ALREADY_EXISTS);
			LOG4CPLUS_ERROR(logger, "db card_info already exists cardid="<<strCardid<<", errmsg = "<<strErrMsg);
			return true;
		}
		stCardInfo.m_owned = CARD_OWNED_OURS;
		stCardInfo.m_status = CARD_NOT_VERIFY;
		stCardInfo.m_logourl = strLogoUrl;
		stCardInfo.m_title = strTitle;
		stCardInfo.m_merchant_id = strMerchantid;
		stCardInfo.m_extra_data = strExtradata;
		stCardInfo.m_quantity = stCardInfo.m_total_quantity = iQuantity;
		stCardInfo.m_create_timestamp = time(0);
		stCardInfo.m_expire_timestamp=time(0);
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
	CgiWxCardCreate cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
