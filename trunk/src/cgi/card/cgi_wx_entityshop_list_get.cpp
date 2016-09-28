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

		if (strUin.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid shops_uin="<<strUin);
			return true;
		}

		int iRet = 0;
		std::string strErrMsg = "";
		EntityShopsInfoIndexByUin stEntityShopsInfoIndexByUin(m_table_name_entity_shop_info);
		stEntityShopsInfoIndexByUin.m_uin = strUin;
		iRet = stEntityShopsInfoIndexByUin.SelectFromDB(strErrMsg);
		if (iRet != TableBaseCard::TABLE_BASE_RET_OK && iRet != TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "stEntityShopsInfoIndexByUin.SelectFromDB failed , errmsg="<<strErrMsg);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "stEntityShopsInfoIndexByUin.size="<<stEntityShopsInfoIndexByUin.m_entityshops_info_list.size());

		for (size_t i = 0; i != stEntityShopsInfoIndexByUin.m_entityshops_info_list.size(); ++i)
		{
			const EntityShopInfo& stEntityShopInfo = stEntityShopsInfoIndexByUin.m_entityshops_info_list[i];
			lce::cgi::CAnyValue stAnyValue;
			stEntityShopInfo.ToAnyValue(stAnyValue);


			GetAnyValue()["entity_shops_list"].push_back(stAnyValue);
		}

		LOG4CPLUS_DEBUG(logger, "entityshop_info_get done");

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

