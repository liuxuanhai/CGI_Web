#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWxSubmerchantCreate: public HooshoCgiCard
{
public:
	CgiWxSubmerchantCreate() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", true)
	{

	}

	bool InnerProcess()
	{
		string strPostData = (string) GetInput().GetPostData();
		LOG4CPLUS_DEBUG(logger, "postdata="<<strPostData);
		EMPTY_STR_RETURN(strPostData);
		string strErrMsg = "";
		std::string strMyAccessToken = "";
		string strMerchantid = "";



		//parse json
		string strParseData = strPostData;
		Json::Value oJson;
		Json::Reader reader;
		LOG4CPLUS_DEBUG(logger, "Json prase");
		if (!reader.parse(strParseData, oJson, false))
		{
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return true;
		}
		if (!oJson.hasKey("info") || !oJson["info"].isObject())
		{
			strErrMsg = "parse card error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return true;
		}

		strParseData = oJson["info"].toJsonString();

		if (!reader.parse(strParseData, oJson, false))
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			strErrMsg = "parse json error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return true;
		}
		if (!oJson.hasKey("brand_name") || !oJson.hasKey("protocol") || !oJson.hasKey("logo_url"))
		{
			strErrMsg = "no brand_name ,protocol or logo_url,parse card error, resp_body=" + strParseData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return true;
		}
		SubmerchantInfo stSubmerchantInfo(m_table_name_submerchant_info);
		time_t tNow = time(0);
		string strProtocol = oJson["protocol"].asString();
		std::string strPicData = "";
		for (int i = 0; i < 2; i++)
		{
			int iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc,
					strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			iRet = WXHttpsReq::WXAPIDownloadMedia(strProtocol, strMyAccessToken, strPicData, strErrMsg);
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
		CardPicture stCardPictureProtocol(m_table_name_card_picture);
		stCardPictureProtocol.m_id = GenerateTransID(ID_TYPE_PICTURE);
		if ("" == stCardPictureProtocol.m_id)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "GenerateTransID failed!");
			return true;
		}
		stCardPictureProtocol.m_data = strPicData;
		LOG4CPLUS_DEBUG(logger, "protocal pic.size="<<strPicData.size());
		stCardPictureProtocol.m_status = PICTURE_STATUS_PUBLIC;
		stCardPictureProtocol.m_uin = m_user_info.m_uin;
		if (stCardPictureProtocol.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "stCardPicture.UpdateToDB failed, errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		strProtocol = stCardPictureProtocol.m_id;

		if (oJson.hasKey("agreement_media_id") && oJson.hasKey("operator_media_id"))
		{
			std::string strAgreementId = oJson["agreement_media_id"].asString();
			std::string strOperatorId = oJson["operator_media_id"].asString();

			for (int i = 0; i < 2; i++)
			{
				int iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc,
						strMyAccessToken, strErrMsg);
				if (iRet != 0)
				{
					LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				iRet = WXHttpsReq::WXAPIDownloadMedia(strAgreementId, strMyAccessToken, strPicData, strErrMsg);
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
			CardPicture stCardPictureAgreement(m_table_name_card_picture);
			stCardPictureAgreement.m_id = GenerateTransID(ID_TYPE_PICTURE);
			if ("" == stCardPictureAgreement.m_id)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "GenerateTransID failed!");
				return true;
			}
			stCardPictureAgreement.m_data = strPicData;
			LOG4CPLUS_DEBUG(logger, "Agreement pic.size="<<strPicData.size());
			stCardPictureAgreement.m_status = PICTURE_STATUS_PUBLIC;
			stCardPictureAgreement.m_uin = m_user_info.m_uin;
			stCardPictureAgreement.m_createtimestamp = tNow;
			if (stCardPictureAgreement.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "stCardPicture.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			strAgreementId = stCardPictureAgreement.m_id;

			for (int i = 0; i < 2; i++)
			{
				int iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc,
						strMyAccessToken, strErrMsg);
				if (iRet != 0)
				{
					LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				iRet = WXHttpsReq::WXAPIDownloadMedia(strOperatorId, strMyAccessToken, strPicData, strErrMsg);
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
			CardPicture stCardPictureOperator(m_table_name_card_picture);
			stCardPictureOperator.m_id = GenerateTransID(ID_TYPE_PICTURE);
			if ("" == stCardPictureOperator.m_id)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "GenerateTransID failed!");
				return true;
			}
			stCardPictureOperator.m_data = strPicData;
			LOG4CPLUS_DEBUG(logger, "Operator pic.size="<<strPicData.size());
			stCardPictureOperator.m_status = PICTURE_STATUS_PUBLIC;
			stCardPictureOperator.m_uin = m_user_info.m_uin;
			stCardPictureOperator.m_createtimestamp = tNow;
			if (stCardPictureOperator.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "stCardPicture.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			strOperatorId = stCardPictureOperator.m_id;

			stSubmerchantInfo.m_agreement = strAgreementId;
			stSubmerchantInfo.m_operator = strOperatorId;
		}

		// call wx api
		for (int i = 0; i < 2; i++)
		{
			int iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc,
					strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "my access_token="<<strMyAccessToken);

			iRet = WXHttpsReq::WXAPISubmerchantCreate(strMyAccessToken, strPostData, strMerchantid, strErrMsg);
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
				LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXAPISubmerchantCreate failed, errmsg=" << strErrMsg);
				DoReply(iRet, strErrMsg);
				return true;
			}
			break;
		}


		//update to db
		stSubmerchantInfo.m_id = strMerchantid;
		CHECK_DB_NOT_EXIST(stSubmerchantInfo, strErrMsg, strMerchantid);

		stSubmerchantInfo.m_brand_name = oJson["brand_name"].asString();
		stSubmerchantInfo.m_logo_url = oJson["logo_url"].asString();
		stSubmerchantInfo.m_protocol = strProtocol;
		stSubmerchantInfo.m_uin = m_user_info.m_uin;
		stSubmerchantInfo.m_status = SUBMERCHANT_NOT_VERIFY;
		stSubmerchantInfo.m_wx_msg = "";
		stSubmerchantInfo.m_extra_data = strParseData;
		stSubmerchantInfo.m_create_timestamp = time(0);

		UPDATE_DB(stSubmerchantInfo, strErrMsg);

		GetAnyValue()["merchant_id"] = strMerchantid;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWxSubmerchantCreate cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
