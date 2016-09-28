#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXCardConsume: public HooshoCgiCard
{
public:
	CgiWXCardConsume() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", USER_LOGIN)
	{

	}
	bool InnerProcess()
	{
		string strCardCode = (string) GetInput().GetValue("card_code");
		if (strCardCode.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "CgiWXCardConsume invalid card code");
			return true;
		}
		string strUin = m_user_info.m_uin;
		string strErrMsg = "";
		string strCardid = "";
		std::string strMyAccessToken = "";
		int iRet = 0;
		bool owned = false;

		for (int i = 0; i < 2; i++)
		{
			iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			iRet = WXHttpsReq::WXAPICardCodeQuery(true,strCardCode, strMyAccessToken, strCardid, strErrMsg);
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
				/*
				else if(iRet==40056)   //已經贈送
				{
					DoReply(CGI_RET_CODE_WX_INVALID_SERIAL_CODE);
					return true;
				}
				else if(iRet==40099)  //已經使用
				{
					DoReply(CGI_RET_CODE_WX_INVALID_CODE);
					return true;
				}
				else if(iRet == 40079) //
				{
					DoReply(CGI_RET_CODE_WX_INVALID_TIME);
					return true;
				}
				*/
				LOG4CPLUS_ERROR(logger, "WXAPICardCodeQuery failed, errmsg=" << strErrMsg);
				DoReply(iRet, strErrMsg);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "WXAPICardCodeQuery   --  strCardid = "<<strCardid);
			break;
		}
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

		std::string strQuery="";
		for (int i = 0; i < 2; i++)
		{
			iRet = WXHttpsReq::WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID, WX_HX_PLATFORM_DEV_SECRET, m_cache_ip, m_cache_port_vc, strMyAccessToken, strErrMsg);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetMyAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			iRet = WXHttpsReq::WXAPICardConsume(strCardCode, strMyAccessToken, strQuery, strErrMsg);

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
				LOG4CPLUS_ERROR(logger, "WXAPICardConsume failed, errmsg=" << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			break;
		}

		LOG4CPLUS_DEBUG(logger, "WXAPICardConsume   --  strQuery = "<<strQuery);
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXCardConsume cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
