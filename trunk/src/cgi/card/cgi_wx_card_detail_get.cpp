#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXCardQuery: public HooshoCgiCard
{
public:
	CgiWXCardQuery() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{
		string strErrMsg = "";
		std::string strMyAccessToken = "";
		string strQuery = "";
		string strCardid = (string) GetInput().GetValue("card_id");
		string strUin = m_user_info.m_uin;
		int iRet = 0;
		bool owned = false;
		int iIndex = 0;
		EMPTY_STR_RETURN(strCardid);
		EMPTY_STR_RETURN(strUin);

		CardInfoIndexByUin stCardInfoIndexByUin(m_table_name_card_info, m_table_name_submerchant_info);
		stCardInfoIndexByUin.m_uin = strUin;
		stCardInfoIndexByUin.m_merchant_id = "";
		iRet = stCardInfoIndexByUin.SelectFromDB(strErrMsg);
		if (iRet != TableBaseCard::TABLE_BASE_RET_OK && iRet != TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "stCardInfoIndexByUin.SelectFromDB failed , errmsg="<<strErrMsg);
			return true;
		}
		if (iRet == TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "fatal error,the card does not exist,uin="<<strUin<<", cardid="<<strCardid);
			return true;
		}
		for (size_t i = 0; i != stCardInfoIndexByUin.m_card_info_list.size(); ++i)
		{
			if (strCardid == stCardInfoIndexByUin.m_card_info_list[i].m_id)
			{
				owned = true;
				iIndex = i;
			}
		}
		if (!owned)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "fatal error,the card is not belong to "<<strUin<<", cardid="<<strCardid);
			return true;
		}

//		SetOutputJson(stCardInfoIndexByUin.m_card_info_list[iIndex].m_extra_data);
		lce::cgi::CAnyValue stAnyValue;
		stCardInfoIndexByUin.m_card_info_list[iIndex].ToAnyValue(stAnyValue);
		GetAnyValue()["card_info"] = stAnyValue;
		DoReply(CGI_RET_CODE_OK);
		return true;
		/*	for (int i = 0; i < 2; i++)
		 {
		 iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
		 if (iRet != 0)
		 {
		 LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
		 DoReply(CGI_RET_CODE_SERVER_BUSY);
		 return true;
		 }
		 LOG4CPLUS_DEBUG(logger, "my access_token="<<strMyAccessToken);
		 strQuery = "";
		 iRet = WXHttpsReq::WXAPICardQuery(strCardid, strMyAccessToken, strQuery, strErrMsg);
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
		 strErrMsg = "WXHttpsReq::WXAPICardQuery failed, errmsg=" + strErrMsg;

		 LOG4CPLUS_ERROR(logger, strErrMsg);
		 DoReply(CGI_RET_CODE_SERVER_BUSY);
		 return -1;
		 }
		 break;
		 }
		 std::string strParseData = strQuery;
		 std::string strMerchantid = "";
		 std::string strCardType = "";
		 std::string strLogoUrl = "";
		 std::string strTitle = "";
		 std::string strStatus = "";
		 std::string strDateInfo = "";
		 CardInfo stCardInfo(m_table_name_card_info);
		Json::Value oJson;
		 Json::Reader reader;
		 if (!reader.parse(strParseData, oJson, false) || !oJson.hasKey("card_type"))
		 {
		 strErrMsg = "parse json error, resp_body=" + strParseData;
		 LOG4CPLUS_ERROR(logger, strErrMsg);
		 DoReply(CGI_RET_CODE_INVALID_PARAM);
		 return true;
		 }

		 // card_type
		 strCardType = oJson["card_type"].asString();

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
		 //base _info
		 strParseData = oJson["base_info"].toJsonString();

		 if (!reader.parse(strParseData, oJson, false) || !oJson.hasKey("logo_url") || !oJson.hasKey("title") || !oJson.hasKey("status") || !oJson.hasKey("date_info"))
		 {
		 strErrMsg = "parse json error, resp_body=" + strParseData;
		 LOG4CPLUS_ERROR(logger, strErrMsg);
		 DoReply(CGI_RET_CODE_INVALID_PARAM);
		 return true;
		 }
		 // logo url stauts
		 strLogoUrl = oJson["logo_url"].asString();
		 strTitle = oJson["title"].asString();
		 strStatus = oJson["status"].asString();
		 strDateInfo = oJson["date_info"].toJsonString();
		 //		std::string strTemp = strParseData;
		 if (!oJson.hasKey("sub_merchant_info") || !oJson["sub_merchant_info"].isObject())
		 {
		 strErrMsg = "parse error, resp_body=" + strParseData;
		 LOG4CPLUS_ERROR(logger, strErrMsg);
		 DoReply(CGI_RET_CODE_INVALID_PARAM);
		 return true;
		 }
		 if (!oJson.hasKey("sku") || !oJson["sku"].isObject())
		 {
		 strErrMsg = "parse error, resp_body=" + strParseData;
		 LOG4CPLUS_ERROR(logger, strErrMsg);
		 DoReply(CGI_RET_CODE_INVALID_PARAM);
		 return true;
		 }

		 strParseData = oJson["sub_merchant_info"].toJsonString();
		 string strAnoParseData = oJson["sku"].toJsonString();
		 if (!reader.parse(strParseData, oJson, false) || !oJson.hasKey("merchant_id"))
		 {
		 strErrMsg = "parse json error, resp_body=" + strParseData;
		 LOG4CPLUS_ERROR(logger, strErrMsg);
		 DoReply(CGI_RET_CODE_INVALID_PARAM);
		 return true;
		 }
		 ostringstream oss;
		 oss.str("");
		 oss << oJson["merchant_id"].asInt();
		 strMerchantid = oss.str();

		 //		strParseData = strTemp;
		 //		strParseData = oJson["sku"].toJsonString();
		 if (!reader.parse(strAnoParseData, oJson, false) || !oJson.hasKey("quantity") ||!oJson.hasKey("total_quantity"))
		 {
		 strErrMsg = "parse json error, resp_body=" + strParseData;
		 LOG4CPLUS_ERROR(logger, strErrMsg);
		 DoReply(CGI_RET_CODE_INVALID_PARAM);
		 return true;
		 }
		 uint64_t iQuantity = oJson["quantity"].asInt();
		 uint64_t iTotalquantity = oJson["total_quantity"].asInt();

		 stCardInfo = stCardInfoIndexByUin.m_card_info_list[iIndex];

		 stCardInfo.m_logourl = strLogoUrl;
		 stCardInfo.m_title = strTitle;
		 stCardInfo.m_extra_data = strQuery;
		 stCardInfo.m_date_info = strDateInfo;
		 stCardInfo.m_quantity = iQuantity;
		 stCardInfo.m_total_quantity = iTotalquantity;

		 if (strStatus == "CARD_STATUS_NOT_VERIFY")
		 {
		 stCardInfo.m_status = CARD_NOT_VERIFY;
		 }
		 else if ("CARD_STATUS_VERIFY_OK" == strStatus)
		 {
		 stCardInfo.m_status = CARD_PASS;
		 }
		 else if ("CARD_STATUS_VERIFY_FAIL" == strStatus)
		 {
		 stCardInfo.m_status = CARD_NOT_PASS;
		 }
		 else if ("CARD_STATUS_DELETE" == strStatus)
		 {
		 stCardInfo.m_status = CARD_DELETE;
		 }
		 else if ("CARD_STATUS_DISPATCH" == strStatus)
		 {
		 stCardInfo.m_status = CARD_DISPATCH;
		 }
		 else
		 {
		 DoReply(CGI_RET_CODE_SERVER_BUSY);
		 LOG4CPLUS_ERROR(logger, "wx stauts type error,stauts="<<strStatus);
		 return true;
		 }

		 if (stCardInfo.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		 {
		 DoReply(CGI_RET_CODE_SERVER_BUSY);
		 LOG4CPLUS_ERROR(logger, "stCardInfo.UpdateToDB failed, errmsg="<<strErrMsg);
		 return true;
		 }*/

		//GetAnyValue()["card_info"] = strQuery;
		/*	SetOutputJson(strQuery);
		 DoReply(CGI_RET_CODE_OK);
		 return true;*/
	}
};

int main()
{
	CgiWXCardQuery cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
