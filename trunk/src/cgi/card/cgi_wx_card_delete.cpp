#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXCardDelete: public HooshoCgiCard
{
public:
	CgiWXCardDelete() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}

	bool InnerProcess()
	{
		string strErrMsg = "";
		std::string strMyAccessToken = "";
		string strQuery = "";
		string strCardid = (string) GetInput().GetValue("card_id");
		string strUin = m_user_info.m_uin;
		int iRet = 0;
		bool owned = false;
		int iIndex = 0;
		EMPTY_STR_RETURN(strCardid);
		EMPTY_STR_RETURN(strUin);

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
				iIndex = i;
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
			iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "my access_token="<<strMyAccessToken);
			strQuery = "";
			iRet = WXHttpsReq::WXAPICardDelete(strCardid, strMyAccessToken, strQuery, strErrMsg);
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
				strErrMsg = "WXHttpsReq::WXAPICardDelete failed, errmsg=" + strErrMsg;
				DoReply(iRet, strErrMsg);
				LOG4CPLUS_ERROR(logger, strErrMsg);
				return -1;
			}
			break;
		}


		CardInfo stCardInfo(m_table_name_card_info);

		stCardInfo = stCardInfoIndexByUin.m_card_info_list[iIndex];
		stCardInfo.m_status = CARD_DELETE;
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
	CgiWXCardDelete cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
