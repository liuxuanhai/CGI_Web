#include "hoosho_cgi_card.h"

class CgiEntityShopInfoListGetByPoiId: public HooshoCgiCard
{
public:
	CgiEntityShopInfoListGetByPoiId() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool ProcessByPoiID()
	{
		string strPoiId = (string) GetInput().GetValue("poi_id");
		string strUin = m_user_info.m_uin;
//		EMPTY_STR_RETURN(strPoiId);
		if(strPoiId.empty())
		{
			GetAnyValue()["entity_shops_list"] = "";
			DoReply(CGI_RET_CODE_OK);
			return true;
		}

		EMPTY_STR_RETURN(strUin);
		string strErrMsg = "";
		int iRet = 0;
		vector<string> vecQueryString;
		lce::cgi::Split(strPoiId, "|", vecQueryString);
		if(!vecQueryString.size())
		{
			GetAnyValue()["entity_shops_list"] = "";
			DoReply(CGI_RET_CODE_OK);
			return true;
		}

		EntityShopsInfoIndexByUin stEntityShopsInfoIndexByPoiId(m_table_name_entity_shop_info);

		EntityShopsInfoIndexByUin stEntityShopsInfoIndexByUin(m_table_name_entity_shop_info);
		stEntityShopsInfoIndexByUin.m_uin = strUin;
		iRet = stEntityShopsInfoIndexByUin.SelectFromDB(strErrMsg);
		if (iRet != TableBaseCard::TABLE_BASE_RET_OK && iRet != TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			GetAnyValue()["entity_shops_list"] = "";
			LOG4CPLUS_ERROR(logger, "stEntityShopsInfoIndexByUin.SelectFromDB failed , errmsg="<<strErrMsg);
			return true;
		}

		for (uint32_t i = 0; i < vecQueryString.size(); i++)
		{
			for (uint32_t j = 0; j < stEntityShopsInfoIndexByUin.m_entityshops_info_list.size(); j++)
			{
				if (stEntityShopsInfoIndexByUin.m_entityshops_info_list[j].m_PoiId == vecQueryString[i])
				{
					stEntityShopsInfoIndexByPoiId.m_entityshops_info_list.push_back(stEntityShopsInfoIndexByUin.m_entityshops_info_list[j]);
				}
			}
		}

		for (size_t i = 0; i != stEntityShopsInfoIndexByPoiId.m_entityshops_info_list.size(); ++i)
		{
			const EntityShopInfo& stEntityShopInfo = stEntityShopsInfoIndexByPoiId.m_entityshops_info_list[i];
			lce::cgi::CAnyValue stAnyValue;
			stEntityShopInfo.ToAnyValue(stAnyValue);
			GetAnyValue()["entity_shops_list"].push_back(stAnyValue);
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
	bool ProcessBySid()
	{
		string strSid = (string) GetInput().GetValue("sid");
		string strUin = m_user_info.m_uin;
		EMPTY_STR_RETURN(strSid);
		EMPTY_STR_RETURN(strUin);
		string strErrMsg = "";
		int iRet = 0;
		vector<string> vecQueryString;
		lce::cgi::Split(strSid, "|", vecQueryString);

		EntityShopsInfoIndexByUin stEntityShopsInfoIndexBySid(m_table_name_entity_shop_info);

		EntityShopsInfoIndexByUin stEntityShopsInfoIndexByUin(m_table_name_entity_shop_info);
		stEntityShopsInfoIndexByUin.m_uin = strUin;
		iRet = stEntityShopsInfoIndexByUin.SelectFromDB(strErrMsg);
		if (iRet != TableBaseCard::TABLE_BASE_RET_OK && iRet != TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "stEntityShopsInfoIndexByUin.SelectFromDB failed , errmsg="<<strErrMsg);
			return true;
		}

		for (uint32_t i = 0; i < vecQueryString.size(); i++)
		{
			for (uint32_t j = 0; j < stEntityShopsInfoIndexByUin.m_entityshops_info_list.size(); j++)
			{
				if (stEntityShopsInfoIndexByUin.m_entityshops_info_list[j].m_sid == vecQueryString[i])
				{
					stEntityShopsInfoIndexBySid.m_entityshops_info_list.push_back(stEntityShopsInfoIndexByUin.m_entityshops_info_list[j]);
				}
			}
		}

		for (size_t i = 0; i != stEntityShopsInfoIndexByUin.m_entityshops_info_list.size(); ++i)
		{
			const EntityShopInfo& stEntityShopInfo = stEntityShopsInfoIndexByUin.m_entityshops_info_list[i];
			lce::cgi::CAnyValue stAnyValue;
			stEntityShopInfo.ToAnyValue(stAnyValue);
			GetAnyValue()["entity_shops_list"].push_back(stAnyValue);
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

	bool InnerProcess()
	{

		return ProcessByPoiID();
	/*	uint32_t dwReqType = (uint32_t) GetInput().GetValue("req_type"); //1: by poi_id;  2:by sid

		if (dwReqType != 1 )
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid req_type="<<dwReqType);
			return true;
		}

		if (1 == dwReqType)
		{
			return ProcessByPoiID();
		}*/

	/*	if (2 == dwReqType)
		{
			return ProcessBySid();
		}*/

		return true;
	}
};

int main()
{
	CgiEntityShopInfoListGetByPoiId cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
