#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXEditEntityShop: public HooshoCgiCard
{
public:
	CgiWXEditEntityShop() :
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

		string strPostData = (string) GetInput().GetValue("entityshop_info");
		// std::string strPostData = GetInput().GetPostData();

		if (strPostData.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid strPostData="<<strPostData);
			return true;
		}

		// get sid & sendout in the json from FE
		Json::Value tmp_oJson;
		Json::Reader reader;
		if (!reader.parse(strPostData, tmp_oJson, false))
		{
			strErrMsg = "parse json error, strPostData=" + strPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		if (!tmp_oJson.hasKey("sid"))
		{
			strErrMsg = "parse sid error , FE gave strPostData=" + strPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		std::string str_sid = tmp_oJson["sid"].asString();
		std::string strPoid = "";
		// check whether db can be updated (check up update_status)
		EntityShopInfo stEntityShopInfo(m_table_name_entity_shop_info);
		stEntityShopInfo.m_sid = str_sid;
	//	stEntityShopInfo.m_uin = m_user_info.m_uin;
		if (stEntityShopInfo.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "Invalid sid="<<str_sid<<",uin="<<m_user_info.m_uin<<", errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		if (stEntityShopInfo.m_uin != m_user_info.m_uin)
		{
			LOG4CPLUS_ERROR(logger, "Invalid uin="<<m_user_info.m_uin<<"(supposed to be "<<stEntityShopInfo.m_uin<<" ), errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		strPoid = stEntityShopInfo.m_PoiId;

		LOG4CPLUS_DEBUG(logger, "str_sid="<<str_sid<<", stEntityShopInfo.m_update_status="<<stEntityShopInfo.m_update_status);

		if (stEntityShopInfo.m_update_status == 1)
		{
			LOG4CPLUS_ERROR(logger, "FE are you kidding me? it is updating");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		if (!tmp_oJson.hasKey("sendout") || !tmp_oJson["sendout"].isObject())
		{
			strErrMsg = "parse ret_oJson(business) error, strPostData=" + strPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		strPostData = tmp_oJson["sendout"].toJsonString();
		if (!reader.parse(strPostData, tmp_oJson, false))
		{
			strErrMsg = "parse json error, strPostData=" + strPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		if (!tmp_oJson.hasKey("business"))
		{
			strErrMsg = "parse sid error , FE gave strPostData=" + strPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		strPostData = tmp_oJson["base_info"].toJsonString();
		if (!reader.parse(strPostData, tmp_oJson, false) || !tmp_oJson.hasKey("poi_id"))
		{
			strErrMsg = "parse json error, strPostData=" + strPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		string strTmpPoid = tmp_oJson["poi_id"].asString();
		if (strPoid != strTmpPoid)
		{
			LOG4CPLUS_ERROR(logger,
					"Invalid poid="<<strPoid<<"(supposed to be "<<stEntityShopInfo.m_PoiId<<" )"<<",uin="<<m_user_info.m_uin<<", errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

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
			iRet = WXHttpsReq::WXAPIEntityShop(strPostData, "updatepoi", strMyAccessToken, strRspBody, boolRet, strErrMsg);
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

		LOG4CPLUS_DEBUG(logger, "wx returns edit_entity_shop msg end");

		// update db
		stEntityShopInfo.m_update_status = 1;

		if (stEntityShopInfo.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "stEntityShopInfo.UpdateToDB failed, errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "stEntityShopInfo="<<stEntityShopInfo.ToString());

		GetAnyValue()["wx_result"] = strRspBody;
		LOG4CPLUS_DEBUG(logger, "edit_entity_shop done");
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXEditEntityShop cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

