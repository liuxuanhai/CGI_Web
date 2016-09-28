#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXCreateEntityShop: public HooshoCgiCard
{
public:
	CgiWXCreateEntityShop() :
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

		string tmpPostData = (string) GetInput().GetValue("entityshop_info");
		// std::string tmpPostData = GetInput().GetPostData();

		if (tmpPostData.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid tmpPostData="<<tmpPostData);
			return true;
		}

		// change the sid in the json from FE
		std::string tmp_sid = GenerateTransID(ID_TYPE_ENTITY_SHOP);

		Json::Value tmp_oJson;
		Json::Reader reader;

		if (!reader.parse(tmpPostData, tmp_oJson, false))
		{
			strErrMsg = "parse json error, FE gave tmpPostData=" + tmpPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		if (!tmp_oJson.hasKey("sid"))
		{
			strErrMsg = "parse sid error ,  tmpPostData=" + tmpPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		if (!tmp_oJson.hasKey("business_name") || !tmp_oJson.hasKey("branch_name") || !tmp_oJson.hasKey("province") || !tmp_oJson.hasKey("city")
				|| !tmp_oJson.hasKey("district") || !tmp_oJson.hasKey("address"))
		{
			strErrMsg = "parse business_name,branch_name,province,city,district,address error ,  tmpPostData=" + tmpPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		tmp_oJson["sid"] = tmp_sid;

		Json::Value base_oJson;
		base_oJson["base_info"] = tmp_oJson;
		Json::Value oJson;
		oJson["business"] = base_oJson;

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

			iRet = WXHttpsReq::WXAPIEntityShop(oJson.toJsonString(), "addpoi", strMyAccessToken, strRspBody, boolRet, strErrMsg);
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

		// success , save to DB
		EntityShopInfo stEntityShopInfo(m_table_name_entity_shop_info);
		stEntityShopInfo.m_sid = tmp_sid;

		stEntityShopInfo.m_uin = m_user_info.m_uin;
		stEntityShopInfo.m_PoiId = "";
		stEntityShopInfo.m_CreateTime =time(0);
		stEntityShopInfo.m_update_status = 1;
		stEntityShopInfo.m_err_msg = "";
		stEntityShopInfo.m_business_name = tmp_oJson["business_name"].asString();
		stEntityShopInfo.m_branch_name = tmp_oJson["branch_name"].asString();
		stEntityShopInfo.m_province = tmp_oJson["province"].asString();
		stEntityShopInfo.m_city = tmp_oJson["city"].asString();
		stEntityShopInfo.m_district = tmp_oJson["district"].asString();
		stEntityShopInfo.m_address = tmp_oJson["address"].asString();
		stEntityShopInfo.m_available_state = ENTITYSHOP_NOT_VERIFY;
		stEntityShopInfo.m_extra_data = oJson.toJsonString();

		if (stEntityShopInfo.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "stEntityShopInfo.UpdateToDB failed, errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "stEntityShopInfo="<<stEntityShopInfo.ToString());

		LOG4CPLUS_DEBUG(logger, "create_entity_shop done");
		GetAnyValue()["wx_result"] = strRspBody;
		//GetAnyValue()["wx_result"] = {"errcode":65109,"errmsg":"invalid business name hint: [Os0313vr23]"}
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXCreateEntityShop cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

