#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXEntityShopInfoGet: public HooshoCgiCard
{
public:
	CgiWXEntityShopInfoGet() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{
		string strUin = m_user_info.m_uin;
		EMPTY_STR_RETURN(strUin);

		std::string strErrMsg = "";
		EntityShopsInfoIndexByUin stEntityShopsInfoIndexByUin(m_table_name_entity_shop_info);
		stEntityShopsInfoIndexByUin.m_uin = strUin;
		int iRet = stEntityShopsInfoIndexByUin.SelectFromDB(strErrMsg);
		if (iRet == TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			LOG4CPLUS_ERROR(logger, "uin = " << strUin << " has none entityshop");
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
		if (iRet != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "stEntityShopsInfoIndexByUin.SelectFromDB failed, strErrMsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		WXEntityShopInfo stWXEntityShopInfo(m_table_name_wx_entity_shop_info);

		//check cache expired or not
		time_t cur_ts;
		time(&cur_ts);
		bool bNeedGetFromWX = false;

		for (uint32_t i = 0; i < stEntityShopsInfoIndexByUin.m_entityshops_info_list.size(); i++)
		{
			if(stEntityShopsInfoIndexByUin.m_entityshops_info_list[i].m_available_state == ENTITYSHOP_PASS)
			{
				continue;
			}
			stWXEntityShopInfo.m_sid = stEntityShopsInfoIndexByUin.m_entityshops_info_list[i].m_sid;
			if (stWXEntityShopInfo.SelectFromDB(strErrMsg) == TableBaseCard::TABLE_BASE_RET_NOT_EXIST
					|| ( stWXEntityShopInfo.m_available_state != ENTITYSHOP_PASS && stWXEntityShopInfo.m_expire_timestamp_cached < (uint64_t) cur_ts) )
			{
				bNeedGetFromWX = true;
				LOG4CPLUS_DEBUG(logger, "stEntityShopsInfoIndexByUin.SelectFromDB failed, strErrMsg = " << strErrMsg);
				break;
			}
		}

		if (bNeedGetFromWX)
		{
			time_t expire_timestamp = cur_ts + m_entityshop_life;
			std::string strMyAccessToken = "";
			std::string strRspBody;
			bool boolRet;
			Json::Value jsonPost;
			Json::Value jsonRsp;
			Json::Reader reader;
			int iSum = 0;

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

				int iBegin = 0;
				int iLimit = m_entityshop_limit;
				int iCount = 0;
				jsonPost["limit"] = iLimit;
//				int iMaxloop = 10000;
				do
				{
					jsonPost["begin"] = iBegin;

					iRet = WXHttpsReq::WXAPIEntityShop(jsonPost.toJsonString(), "getpoilist", strMyAccessToken, strRspBody, boolRet, strErrMsg);
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

//					LOG4CPLUS_ERROR(logger, "rspbody=" << strRspBody);
					if (!reader.parse(strRspBody, jsonRsp, false))
					{
						LOG4CPLUS_ERROR(logger, "parse rspbody error = " << strRspBody);
						DoReply(CGI_RET_CODE_SERVER_BUSY);
						return true;
					}
					strRspBody = jsonRsp["business_list"].toJsonString();
					if (!reader.parse(strRspBody, jsonRsp, false))
					{
						LOG4CPLUS_ERROR(logger, "parse rspbody error = " << strRspBody);
						DoReply(CGI_RET_CODE_SERVER_BUSY);
						return true;
					}

					iCount = jsonRsp.size();
					LOG4CPLUS_DEBUG(logger, "count = " << iCount);

//					LOG4CPLUS_DEBUG(logger, "jsonRsp = " << jsonRsp.toJsonString());
					for (int i = 0; i < iCount; i++)
					{
						string strBase = jsonRsp[i].toJsonString();
						Json::Value jsonBase;
						if (!reader.parse(strBase, jsonBase, false))
						{
							LOG4CPLUS_ERROR(logger, "parse json error, i = " << i << " , parse data = " << strBase);
							continue;
						}
						strBase = jsonBase["base_info"].toJsonString();
						if (!reader.parse(strBase, jsonBase, false))
						{
							LOG4CPLUS_ERROR(logger, "parse json error, i = " << i << " , parse data = " << strBase);
							continue;
						}
						if (!jsonBase.hasKey("sid"))
						{
							LOG4CPLUS_ERROR(logger, "sid not found, base_json = " << jsonBase.toJsonString());
							continue;
						}
						stWXEntityShopInfo.m_sid = jsonBase["sid"].asString();
						stWXEntityShopInfo.SelectFromDB(strErrMsg);

						stWXEntityShopInfo.m_PoiId = jsonBase["poi_id"].asString();
						stWXEntityShopInfo.m_update_status = jsonBase["update_status"].asInt();
						stWXEntityShopInfo.m_extra_data = jsonBase.toJsonString();
						stWXEntityShopInfo.m_business_name = jsonBase["business_name"].asString();
						stWXEntityShopInfo.m_branch_name = jsonBase["branch_name"].asString();
						stWXEntityShopInfo.m_province = jsonBase["province"].asString();
						stWXEntityShopInfo.m_city = jsonBase["city"].asString();
						stWXEntityShopInfo.m_district = jsonBase["district"].asString();
						stWXEntityShopInfo.m_address = jsonBase["address"].asString();
						stWXEntityShopInfo.m_available_state = jsonBase["available_state"].asInt();
						stWXEntityShopInfo.m_expire_timestamp_cached = expire_timestamp;
						if (stWXEntityShopInfo.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
						{
							LOG4CPLUS_ERROR(logger, "stWXEntityShopInfo.UpdateToDB failed, errmsg = " << strErrMsg);
							continue;
						}

					}

					iBegin += iLimit;
					iSum += iCount;
//					iMaxloop--;
				} while (iCount >= iLimit);
//				while(iCount >= iLimit && iMaxloop);
				break;
			}
			LOG4CPLUS_DEBUG(logger, "sum = " << iSum);
		}

		for (int i = stEntityShopsInfoIndexByUin.m_entityshops_info_list.size()-1; i >=0; i--)
		{
			stWXEntityShopInfo.m_sid = stEntityShopsInfoIndexByUin.m_entityshops_info_list[i].m_sid;
			CHECK_DB(stWXEntityShopInfo, strErrMsg);

			EntityShopInfo stEntityShopInfo(m_table_name_entity_shop_info);
			stEntityShopInfo.m_sid = stWXEntityShopInfo.m_sid;
			CHECK_DB(stEntityShopInfo, strErrMsg);

			stEntityShopInfo.m_PoiId = stWXEntityShopInfo.m_PoiId;
			if(stEntityShopInfo.m_available_state != ENTITYSHOP_DELETE)
			{
				stEntityShopInfo.m_available_state = stWXEntityShopInfo.m_available_state;
			}
			stEntityShopInfo.m_update_status = stWXEntityShopInfo.m_update_status;
			UPDATE_DB(stEntityShopInfo, strErrMsg);

			lce::cgi::CAnyValue stAnyValue;
			stWXEntityShopInfo.ToAnyValue(stAnyValue);
			GetAnyValue()["entity_shops_list"].push_back(stAnyValue);
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXEntityShopInfoGet cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

