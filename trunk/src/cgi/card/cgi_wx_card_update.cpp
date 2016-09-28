#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiCardUpdate: public HooshoCgiCard
{
public:
	CgiCardUpdate() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{

		string strPostData = (string) GetInput().GetValue("card_info");
		string strSendcheck = "";
		string strUin ="";
		string strErrMsg = "";
		string strCardid = "";
		string strMyAccessToken = "";
		int iRet = 0;
		bool owned = false;
		LOG4CPLUS_DEBUG(logger, "postdata="<<strPostData);

		Json::Value oJson;
		Json::Reader reader;
		if (!reader.parse(strPostData, oJson, false) || !oJson.hasKey("card_id"))
		{
			strErrMsg = "parse json error, resp_body=" + strPostData;
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		strCardid = oJson["card_id"].asString();
		strUin = m_user_info.m_uin;
		CardInfoIndexByUin stCardInfoIndexByUin(m_table_name_card_info, m_table_name_submerchant_info);
		stCardInfoIndexByUin.m_uin = strUin;
		stCardInfoIndexByUin.m_merchant_id = "";
		iRet = stCardInfoIndexByUin.SelectFromDB(strErrMsg);
		if (iRet != TableBaseCard::TABLE_BASE_RET_OK && iRet != TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "stCardInfoIndexByUin.SelectFromDB failed , errmsg="<<strErrMsg);
			return true;
		}
		if (iRet == TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "fatal error,the card does not exist,uin="<<strUin<<", cardid="<<strCardid);
			return true;
		}
		for (size_t i = 0; i != stCardInfoIndexByUin.m_card_info_list.size(); ++i)
		{
			if (strCardid == stCardInfoIndexByUin.m_card_info_list[i].m_id)
			{
				owned = true;

			}
		}
		if (!owned)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "fatal error,the card is not belong to "<<strUin<<", cardid="<<strCardid);
			return true;
		}

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
			//https://api.weixin.qq.com/card/get?access_token=TOKEN

			iRet = WXHttpsReq::WXAPICardUpdate(strMyAccessToken, strPostData, strSendcheck, strErrMsg);
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
				strErrMsg = "WXHttpsReq::WXAPICardMemberSetPaysell failed, errmsg=" + strErrMsg;
				LOG4CPLUS_ERROR(logger, strErrMsg);
				DoReply(iRet);
				return -1;
			}
			break;
		}

		GetAnyValue()["send_check"] = strSendcheck;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiCardUpdate cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

