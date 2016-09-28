#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXDeleteEntityShop: public HooshoCgiCard
{
public:
	CgiWXDeleteEntityShop() :
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
			LOG4CPLUS_ERROR(logger,
					"Invalid uin="<<m_user_info.m_uin<<"(supposed to be "<<stEntityShopsInfoIndexByPoiId.m_entityshop_info.m_uin<<" ), errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}


		EntityShopInfo stEntityShopInfo(m_table_name_entity_shop_info);
		stEntityShopInfo = stEntityShopsInfoIndexByPoiId.m_entityshop_info;

		stEntityShopInfo.m_available_state = ENTITYSHOP_DELETE;
		if (stEntityShopInfo.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "stEntityShopInfo.DeleteFromDB failed, errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
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

			iRet = WXHttpsReq::WXAPIEntityShop(PoiId.toJsonString(), "delpoi", strMyAccessToken, strRspBody, boolRet, strErrMsg);
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

		LOG4CPLUS_DEBUG(logger, "delete_entity_shop done");

		GetAnyValue()["wx_result"] = strRspBody;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXDeleteEntityShop cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

