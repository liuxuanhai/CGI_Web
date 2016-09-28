#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXTicketGet: public HooshoCgiCard
{
public:
	CgiWXTicketGet() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{
		uint32_t iflag = (uint32_t) GetInput().GetValue("flag");
		if (iflag != 2)
		{
			iflag = 1;
		}
		int iRet = 0;
		string strAccountAppid = (string) GetInput().GetValue("account_appid");
		string strSceneStr = (string) GetInput().GetValue("scene_str");
		EMPTY_STR_RETURN(strAccountAppid);
		EMPTY_STR_RETURN(strSceneStr);
		////////

		string strErrMsg = "";
		std::string strMyAccessToken = "";
		string strTicket = "";
		string strShowCodeUrl = "";

		WxAccountAuthInfo stWxAccountAuthInfo(m_table_name_wx_account_auth_info);
		stWxAccountAuthInfo.m_auth_appid = strAccountAppid;
		iRet = stWxAccountAuthInfo.SelectFromDB(strErrMsg);
		if (iRet != TableBaseCard::TABLE_BASE_RET_OK && iRet != TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			LOG4CPLUS_ERROR(logger, "WxAccountAuthInfo.SelectFromDB failed, errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		if (iRet == TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
		LOG4CPLUS_DEBUG(logger, "stWxAccountAuthInfo="<<stWxAccountAuthInfo.ToString());

		OptionsInfo stOptionsInfo(m_table_name_options_info);
		stOptionsInfo.m_id = OPTIONS_ID_VERIFY_TICKET;
		iRet = stOptionsInfo.SelectFromDB(strErrMsg);
		if (iRet != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "OptionsInfo.SelectFromDB failed , errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		LOG4CPLUS_DEBUG(logger, "verify_ticket="<< stOptionsInfo.m_value);

		std::string strComponentAccessToken = "";
		iRet = WXHttpsReq::WXAPIGetComponentAccessToken(WX_THIRD_PLATFORM_DEV_APPID, WX_THIRD_PLATFORM_DEV_SECRET, stOptionsInfo.m_value,
				strComponentAccessToken, strErrMsg);
		if (iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "WXAPIGetComponentAccessToken failed , errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		LOG4CPLUS_DEBUG(logger, "component_access_token="<<strComponentAccessToken);

		std::string strNewAuthAccessToken = "";
		std::string strNewAuthRefreshToken = "";
		iRet = WXHttpsReq::WXAPIRefreshAuthInfo(WX_THIRD_PLATFORM_DEV_APPID, strComponentAccessToken, stWxAccountAuthInfo.m_auth_appid,
				stWxAccountAuthInfo.m_auth_refresh_token, strNewAuthAccessToken, strNewAuthRefreshToken, strErrMsg);
		if (iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "WXAPIRefreshAuthInfo failed , errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		LOG4CPLUS_DEBUG(logger, "NewAuthAccessToken="<<strNewAuthAccessToken<<", NewAuthRefreshToken="<<strNewAuthRefreshToken);

		//re save
		stWxAccountAuthInfo.m_auth_access_token = strNewAuthAccessToken;
		stWxAccountAuthInfo.m_auth_refresh_token = strNewAuthRefreshToken;
		iRet = stWxAccountAuthInfo.UpdateToDB(strErrMsg);
		if (iRet != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "WxAccountAuthInfo.UpdateToDB failed , errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		strTicket = "";
		strShowCodeUrl = "";
		iRet = WXHttpsReq::WXAPIGetSceneQrcode(1, strSceneStr, strNewAuthAccessToken, strTicket, strShowCodeUrl, strErrMsg);
		if (iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "WXAPIGetSceneQrcode failed , errmsg="<<strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		GetAnyValue()["ticket"] = strTicket;
		GetAnyValue()["code_url"] = strShowCodeUrl;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXTicketGet cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

