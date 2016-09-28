#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXAuthEvent: public HooshoCgiCard
{
     public:
        CgiWXAuthEvent() : HooshoCgiCard(0, "config_card.ini", "logger.properties_card", NO_LOGIN)
        {

        }
        
		bool InnerProcess()
		{
			int iRet = 0;
			std::string strErrMsg = "";
			std::string strMsgSignature = "";
			std::string strTimeStamp = "";
			std::string strNonce = "";
			string strQueryString = (string)GetInput().GetQueryString();
			vector<string> vecQueryString;
			lce::cgi::Split(strQueryString, "&", vecQueryString);
			for(size_t i=0; i!=vecQueryString.size(); ++i)
			{
				vector<string> vecParams;
				lce::cgi::Split(vecQueryString[i], "=", vecParams);
				if(vecParams.size() != 2)
				{
					continue;
				}

				string strKey = vecParams[0];
				string strValue = vecParams[1];
				if(strKey == "msg_signature") strMsgSignature = strValue;
				if(strKey == "nonce") strNonce = strValue;
				if(strKey == "timestamp") strTimeStamp = strValue;
			}
			LOG4CPLUS_DEBUG(logger, "strMsgSignature="<<strMsgSignature
								<<", strTimeStamp="<<strTimeStamp
								<<", strNonce="<<strNonce);	

			string strPostData = GetInput().GetPostData();
			std::map<std::string, std::string> mapMsg;

			iRet = WXHttpsReq::WXMsgDecrypt(strMsgSignature, strTimeStamp, strNonce, strPostData
					,WX_THIRD_PLATFORM_DEV_MSG_CHECK_TOKEN
					,WX_THIRD_PLATFORM_DEV_MSG_CRYPT_KEY
					,WX_THIRD_PLATFORM_DEV_APPID
					,mapMsg);

			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "WXMsgDecrypt failed");
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			std::string strInfoType = mapMsg["InfoType"];
			if("component_verify_ticket" == strInfoType)
			{
				std::string strComponentVerifyTicket = mapMsg["ComponentVerifyTicket"];
				if(strComponentVerifyTicket.empty())
				{
					LOG4CPLUS_ERROR(logger, "ComponentVerifyTicket is empty!");
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				OptionsInfo stOptionsInfo(m_table_name_options_info);
				stOptionsInfo.m_id = OPTIONS_ID_VERIFY_TICKET;
				stOptionsInfo.m_value = strComponentVerifyTicket;
				iRet = stOptionsInfo.UpdateToDB(strErrMsg);
				if(iRet != TableBaseCard::TABLE_BASE_RET_OK)
				{
					LOG4CPLUS_ERROR(logger, "OptionsInfo.UpdateToDB failed , errmsg="<<strErrMsg);
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				//succ
			}
			else if("unauthorized" == strInfoType)
			{
				std::string strAuthorizerAppid = mapMsg["AuthorizerAppid"];
				if(strAuthorizerAppid.empty())
				{
					LOG4CPLUS_ERROR(logger, "strAuthorizerAppid is empty!");
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				WxAccountAuthInfo stWxAccountAuthInfo(m_table_name_wx_account_auth_info);
				stWxAccountAuthInfo.m_auth_appid = strAuthorizerAppid;
				iRet = stWxAccountAuthInfo.DeleteFromDB(strErrMsg);
				if(iRet < 0)
				{
					LOG4CPLUS_ERROR(logger, "stWxAccountAuthInfo.DeleteFromDB failed , errmsg="<<strErrMsg);
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				//succ
			}
			else
			{
				LOG4CPLUS_ERROR(logger, "unknown InfoType="<<strInfoType);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}


		    SetOutputJson("success");
            DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiWXAuthEvent cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


