#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXSearchEntityShop: public HooshoCgiCard
{
public:
	CgiWXSearchEntityShop() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{
		int iRet = 0;
		std::string strErrMsg = "";
		std::string MyAccessToken = "";
		bool boolRet = false;
		std::string strRspBody = "";

		std::string strPoiId = GetInput().GetValue("poi_id");
		EMPTY_STR_RETURN(strPoiId);


		EntityShopsInfoIndexByPoiId stEntityShopsInfoIndexByPoiId(m_table_name_entity_shop_info);
		stEntityShopsInfoIndexByPoiId.m_poiId = strPoiId;
		if (stEntityShopsInfoIndexByPoiId.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "Invalid poiId="<<strPoiId<<",uin="<<m_user_info.m_uin<<", errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		if (stEntityShopsInfoIndexByPoiId.m_entityshop_info.m_uin != m_user_info.m_uin)
		{
			LOG4CPLUS_ERROR(logger, "Invalid uin="<<m_user_info.m_uin<<"(supposed to be "<<stEntityShopsInfoIndexByPoiId.m_entityshop_info.m_uin<<" ), errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		Json::Value PoiId;
		PoiId["poi_id"] = strPoiId;
		std::string strMyAccessToken = "";

		for (int i = 0; i < 2; i++)
		{
			iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc,
					strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			iRet = WXHttpsReq::WXAPIEntityShop(PoiId.toJsonString(), "getpoi", strMyAccessToken, strRspBody, boolRet, strErrMsg);
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
				LOG4CPLUS_ERROR(logger, "WXAPIEntityShop failed , errmsg="<<strErrMsg);
				DoReply(iRet, strErrMsg);
				return true;
			}
			break;
		}

		if (!boolRet)
		{
			LOG4CPLUS_DEBUG(logger, "edit_entity_shop errcode != 0");
			GetAnyValue()["wx_result"] = strRspBody;
			DoReply(CGI_RET_CODE_OK);
			return true;
		}

		// check return json
		Json::Value ret_oJson;
		Json::Reader ret_reader;

		EntityShopInfo stEntityShopInfo(m_table_name_entity_shop_info);
		stEntityShopInfo = stEntityShopsInfoIndexByPoiId.m_entityshop_info;
		if (!ret_reader.parse(strRspBody, ret_oJson, false))
		{
			strErrMsg = "parse ret_oJson error, strRspBody=" + strRspBody;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		if (!ret_oJson.hasKey("business") || !ret_oJson["business"].isObject())
		{
			strErrMsg = "parse ret_oJson(business) error, strRspBody=" + strRspBody;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		// get "business" ojson
		strRspBody = ret_oJson["business"].toJsonString();

		if (!ret_reader.parse(strRspBody, ret_oJson, false))
		{
			strErrMsg = "parse ret_oJson(business) error, strRspBody=" + strRspBody;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "ret_oJson[business]="<<ret_oJson);

		if (!ret_oJson.hasKey("base_info") || !ret_oJson["base_info"].isObject())
		{
			strErrMsg = "parse ret_oJson(base_info) error, strRspBody=" + strRspBody;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		// get "base_info" ojson
		strRspBody = ret_oJson["base_info"].toJsonString();

		if (!ret_reader.parse(strRspBody, ret_oJson, false))
		{
			strErrMsg = "parse ret_oJson(business) error, strRspBody=" + strRspBody;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "ret_oJson[base_info]="<<ret_oJson);

		if (!ret_oJson.hasKey("update_status"))
		{
			strErrMsg = "parse update_status error, strRspBody=" + strRspBody;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "ret_oJson[update_status]="<<ret_oJson["update_status"]);

		if (ret_oJson["update_status"] == 1) // no need to update, return the old info
		{
			stEntityShopInfo.m_update_status = 1;
		}
		else if (ret_oJson["update_status"] == 0) // update db
		{
			stEntityShopInfo.m_update_status = 0;
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "wx_search_entityshop failed, update_status in db !0 && !1");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		uint32_t istatus = ret_oJson["available_state"].asInt();
	/*	switch (istatus)
		{
		case 1:
			stEntityShopInfo.m_available_state = ENTITYSHOP_SYSTEM_ERROR;
			break;
		case 2:
			stEntityShopInfo.m_available_state = ENTITYSHOP_NOT_VERIFY;
			break;
		case 3:
			stEntityShopInfo.m_available_state = ENTITYSHOP_PASS;
			break;
		case 4:
			stEntityShopInfo.m_available_state = ENTITYSHOP_NOT_PASS;
			break;
		default:
			stEntityShopInfo.m_available_state = istatus;
			LOG4CPLUS_ERROR(logger, "wx entityshop detail available_state error, available_state="<<istatus);
			break;

		}*/
		stEntityShopInfo.m_available_state = istatus;
		stEntityShopInfo.m_business_name = ret_oJson["business_name"].asString();
		stEntityShopInfo.m_branch_name = ret_oJson["branch_name"].asString();
		stEntityShopInfo.m_province = ret_oJson["province"].asString();
		stEntityShopInfo.m_city = ret_oJson["city"].asString();
		stEntityShopInfo.m_district = ret_oJson["district"].asString();
		stEntityShopInfo.m_address = ret_oJson["address"].asString();

		//	stEntityShopInfo.m_available_state = ret_oJson["available_state"].asInt();
		stEntityShopInfo.m_extra_data = ret_oJson.toJsonString();

		LOG4CPLUS_DEBUG(logger, "entity shop update db");
		UPDATE_DB(stEntityShopInfo,strErrMsg);
		lce::cgi::CAnyValue stWxResult;
		stEntityShopInfo.ToAnyValue(stWxResult);
		GetAnyValue()["wx_result"] = stWxResult;
		DoReply(CGI_RET_CODE_OK);
		return true;

	}

};

int main()
{
	CgiWXSearchEntityShop cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

