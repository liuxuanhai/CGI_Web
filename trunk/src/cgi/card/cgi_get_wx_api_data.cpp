#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXDataGet: public HooshoCgiCard
{
     public:
		CgiWXDataGet() : HooshoCgiCard(0, "config_card.ini", "logger.properties_card", USER_LOGIN)
        {

        }

		bool InnerProcess()
		{
			int iRet = 0;
			std::string strErrMsg = "";
			string strAccountAppid = (string)GetInput().GetValue("account_appid");
			string strBeginDate = (string)GetInput().GetValue("begin_date");
			string strEndDate = (string)GetInput().GetValue("end_date");
			string strDataName = (string)GetInput().GetValue("data_name");
			if(strAccountAppid.empty())
			{
				LOG4CPLUS_ERROR(logger, "empty strAccountAppid");
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				return true;
			}

			WxAccountAuthInfo stWxAccountAuthInfo(m_table_name_wx_account_auth_info);
			stWxAccountAuthInfo.m_auth_appid = strAccountAppid;
			iRet = stWxAccountAuthInfo.SelectFromDB(strErrMsg);
			if(iRet != TableBaseCard::TABLE_BASE_RET_OK && iRet != TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
			{
				LOG4CPLUS_ERROR(logger, "WxAccountAuthInfo.SelectFromDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			if(iRet == TableBaseCard::TABLE_BASE_RET_NOT_EXIST)
			{
				DoReply(CGI_RET_CODE_OK);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "stWxAccountAuthInfo="<<stWxAccountAuthInfo.ToString());


			OptionsInfo stOptionsInfo(m_table_name_options_info);
			stOptionsInfo.m_id = OPTIONS_ID_VERIFY_TICKET;
			iRet = stOptionsInfo.SelectFromDB(strErrMsg);
			if(iRet != TableBaseCard::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "OptionsInfo.SelectFromDB failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "verify_ticket="<< stOptionsInfo.m_value);


			std::string strComponentAccessToken = "";
			iRet = WXHttpsReq::WXAPIGetComponentAccessToken(WX_THIRD_PLATFORM_DEV_APPID
							, WX_THIRD_PLATFORM_DEV_SECRET
							, stOptionsInfo.m_value
							, strComponentAccessToken
							, strErrMsg);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "WXAPIGetComponentAccessToken failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_DEBUG(logger, "component_access_token="<<strComponentAccessToken);

			std::string strNewAuthAccessToken = "";
			std::string strNewAuthRefreshToken = "";
			iRet = WXHttpsReq::WXAPIRefreshAuthInfo(WX_THIRD_PLATFORM_DEV_APPID
												, strComponentAccessToken
												, stWxAccountAuthInfo.m_auth_appid
												, stWxAccountAuthInfo.m_auth_refresh_token
												, strNewAuthAccessToken
												, strNewAuthRefreshToken
												, strErrMsg);
			if(iRet < 0)
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
			if(iRet != TableBaseCard::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "WxAccountAuthInfo.UpdateToDB failed , errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}


		    std::string strRspBody = "";
			std::string strInnerErrMsg = "";
//			std::string strBeginDate = "2015-11-10";
//			std::string strEndDate = "2015-11-10";
			iRet = WXHttpsReq::WXAPIGetStatData(strBeginDate,strEndDate,strDataName,strNewAuthAccessToken, strRspBody,strInnerErrMsg);
			if(iRet < 0)
			{
				strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
				return -1;
			}


			GetAnyValue()["WXAPI_INFO"] = strRspBody;

            DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiWXDataGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


