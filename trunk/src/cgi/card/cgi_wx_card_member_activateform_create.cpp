#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWxCardMemberCreate: public HooshoCgiCard
{
public:
	CgiWxCardMemberCreate() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{

		const string strPostData = (string) GetInput().GetValue("activate_form");
		LOG4CPLUS_DEBUG(logger, "postdata="<<strPostData);
		if (strPostData.empty())
		{
			LOG4CPLUS_ERROR(logger, "ActivateForm Postdata error!!");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		string strErrMsg = "";
		std::string strMyAccessToken = "";
		string strReturn = "";
		for (int i = 0; i < 2; i++)
		{
			int iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "my access_token="<<strMyAccessToken);
			//https://api.weixin.qq.com/card/get?access_token=TOKEN

			iRet = WXHttpsReq::WXAPICardActivateFormCreate(strMyAccessToken, strPostData, strReturn, strErrMsg);
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
				strErrMsg = "WXHttpsReq::WXAPICardMemberCreate failed, errmsg=" + strErrMsg;
				LOG4CPLUS_DEBUG(logger, "WXHttpsReq::WXAPICardMemberCreate failed, errmsg=" + strErrMsg);
				return -1;
			}
			break;
		}

		Json::Value oJson;
		Json::Reader reader;
		LOG4CPLUS_DEBUG(logger, "activateform Json prase");
		if (!reader.parse(strPostData, oJson, false))
		{
			strErrMsg = "parse json error, resp_body=" + strPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}
		if (!oJson.hasKey("card_id"))
		{
			strErrMsg = "parse card error, resp_body=" + strPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			return -1;
		}

		CardInfo stCardInfo(m_table_name_card_info);
		stCardInfo.m_id = oJson["card_id"].asString();

		if (stCardInfo.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			DoReply(CGI_RET_CODE_ALREADY_EXISTS);
			LOG4CPLUS_ERROR(logger, "db card_info already exists cardid="<<stCardInfo.m_id<<","<<strErrMsg);
			return true;
		}

		stCardInfo.m_activate_form = strPostData;

		if (stCardInfo.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "stCardInfo.UpdateToDB failed, errmsg="<<strErrMsg);
			return true;
		}

		DoReply(CGI_RET_CODE_OK);

		return true;
	}
};

int main()
{
	CgiWxCardMemberCreate cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
