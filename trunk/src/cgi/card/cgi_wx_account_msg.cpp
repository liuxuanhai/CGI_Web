#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXAccountMsg: public HooshoCgiCard
{
     public:
        CgiWXAccountMsg() : HooshoCgiCard(0, "config_card.ini", "logger.properties_card", NO_LOGIN)
        {

        }
        
		bool InnerProcess()
		{
		    //account_appid=/$APPID$&a=b&c=d
            int iRet = 0;
			std::string strErrMsg = "";
			string strQueryString = (string)GetInput().GetQueryString();
			vector<string> vecQueryString;
			lce::cgi::Split(strQueryString, "/", vecQueryString);
			if(vecQueryString.size() != 2)
			{
                LOG4CPLUS_ERROR(logger, "vecQueryString.size() != 2");
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			string strTmp1 = vecQueryString[1].substr(0,vecQueryString[1].find_first_of('&'));
			string strTmp2 = vecQueryString[1].substr(vecQueryString[1].find_first_of('&')+1);

			LOG4CPLUS_ERROR(logger, "tmp1="<<strTmp1<<",tmp2="<<strTmp2);
            //account appid


            string strAccountAppid = strTmp1;
            LOG4CPLUS_DEBUG(logger, "account_appid="<<strAccountAppid);	
            
            //msg decrypt param
            string strMsgSignature = "";
            string strNonce = "";
            string strTimeStamp = "";
            vector<string> vecParseTmp;
            vecParseTmp.clear();
            lce::cgi::Split(strTmp2, "&", vecParseTmp);
			for(size_t i=0; i!=vecParseTmp.size(); ++i)
			{
				vector<string> vecParams;
				lce::cgi::Split(vecParseTmp[i], "=", vecParams);
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
    
            //msg body
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

            //deal msg
            string strMsgType = mapMsg["MsgType"];
            if(strMsgType == "")
            {
                LOG4CPLUS_ERROR(logger, "invalid wx msg_type="<<strMsgType);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;   
            }

            string strNow = int_2_str(time(0));
            static char aczMsgBuffer[2048];

            // event msg
			if(strMsgType == "event")
			{
                
                string strEvent = mapMsg["Event"];
                string strToUserName = mapMsg["ToUserName"];
                string strFromUserName = mapMsg["FromUserName"];
                string strCreateTime = mapMsg["CreateTime"];

                LOG4CPLUS_DEBUG(logger, "event msg, event="<<strEvent
                                                   <<", strToUserName="<<strToUserName
                                                   <<", strFromUserName="<<strFromUserName
                                                   <<", strCreateTime="<<strCreateTime);

                bzero(aczMsgBuffer, 2048);
                snprintf(aczMsgBuffer, 2048, "<xml><ToUserName><![CDATA[%s]]></ToUserName>"
                                             "<FromUserName><![CDATA[%s]]></FromUserName>"
                                             "<CreateTime>%s</CreateTime>"
                                             "<MsgType><![CDATA[text]]></MsgType>"
                                             "<Content><![CDATA[%s]]></Content></xml>"
                                            , strFromUserName.c_str()
                                            , strToUserName.c_str()
                                            , strNow.c_str()
                                            , (strEvent+"from_callback").c_str());

                 string strEncryptMsg = "";
                 iRet = WXHttpsReq::WXMsgEncrypt(aczMsgBuffer, strNow, strNonce
                		 ,WX_THIRD_PLATFORM_DEV_MSG_CHECK_TOKEN
     					,WX_THIRD_PLATFORM_DEV_MSG_CRYPT_KEY
     					,WX_THIRD_PLATFORM_DEV_APPID
     					,strEncryptMsg);
                 if(iRet < 0)
                 {
                    LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXMsgEncrypt failed!!");
    				DoReply(CGI_RET_CODE_SERVER_BUSY);
    				return true;   
                 }

                 SetOutputJson(strEncryptMsg);
                DoReply(CGI_RET_CODE_OK);
    			return true;
			}

            //text msg
            if(strMsgType == "text")
            {
                string strToUserName = mapMsg["ToUserName"];
                string strFromUserName = mapMsg["FromUserName"];
                string strCreateTime = mapMsg["CreateTime"];
                string strContent = mapMsg["Content"];
                string strMsgId = mapMsg["MsgId"];

                LOG4CPLUS_DEBUG(logger, "strToUserName="<<strToUserName
                                           <<", strFromUserName="<<strFromUserName
                                           <<", strCreateTime="<<strCreateTime
                                           <<", strContent="<<strContent
                                           <<", strMsgId="<<strMsgId);

                if(strContent == "TESTCOMPONENT_MSG_TYPE_TEXT")
                {
                    bzero(aczMsgBuffer, 2048);
                    snprintf(aczMsgBuffer, 2048, "<xml><ToUserName><![CDATA[%s]]></ToUserName>"
                                                 "<FromUserName><![CDATA[%s]]></FromUserName>"
                                                 "<CreateTime>%s</CreateTime>"
                                                 "<MsgType><![CDATA[text]]></MsgType>"
                                                 "<Content><![CDATA[%s]]></Content></xml>"
                                                , strFromUserName.c_str()
                                                , strToUserName.c_str()
                                                , strNow.c_str()
                                                , "TESTCOMPONENT_MSG_TYPE_TEXT_callback");

                     string strEncryptMsg = "";
                     iRet = WXHttpsReq::WXMsgEncrypt(aczMsgBuffer, strNow, strNonce
                    		,WX_THIRD_PLATFORM_DEV_MSG_CHECK_TOKEN
         					,WX_THIRD_PLATFORM_DEV_MSG_CRYPT_KEY
         					,WX_THIRD_PLATFORM_DEV_APPID
         					,strEncryptMsg);
                     if(iRet < 0)
                     {
                        LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXMsgEncrypt failed!!");
        				DoReply(CGI_RET_CODE_SERVER_BUSY);
        				return true;   
                     }

                     SetOutputJson(strEncryptMsg);
                     DoReply(CGI_RET_CODE_OK);
            		 return true;
                }

                
                string strTarget = "QUERY_AUTH_CODE:";
                size_t pos = strContent.find(strTarget);
                if(pos == string::npos)
                {
                    LOG4CPLUS_ERROR(logger, "QUERY_AUTH_CODE not found!!");
    				DoReply(CGI_RET_CODE_SERVER_BUSY);
    				return true;  
                }

                //rsp empty body to wx
                {
                    GetHeader().Output();
                }
                
                //send kf msg
                string strAuthCode = strContent.substr(pos + strTarget.size());
                LOG4CPLUS_DEBUG(logger, "strAuthCode="<<strAuthCode);

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

    			
    			std::string strAuthAppid = "";
    			std::string strAuthAccessToken = "";
    			std::string strAuthRefreshToken = "";
    			iRet = WXHttpsReq::WXAPIGetAuthInfo(WX_THIRD_PLATFORM_DEV_APPID
    											, strComponentAccessToken
    											, strAuthCode
    											, strAuthAppid
    											, strAuthAccessToken
    											, strAuthRefreshToken
    											, strErrMsg);
    			if(iRet < 0)
    			{
    				LOG4CPLUS_ERROR(logger, "WXAPIGetAuthInfo failed , errmsg="<<strErrMsg);
    				DoReply(CGI_RET_CODE_SERVER_BUSY);
    				return true;
    			}
    			LOG4CPLUS_DEBUG(logger, "auth_appid="<<strAuthAppid
    								<<", auth_access_token=<<"<<strAuthAccessToken
    								<<", auth_refresh_token="<<strAuthRefreshToken);

                string strTextContent = strAuthCode+"_from_api";
                iRet = WXHttpsReq::WXAPISendKFMsg(strAuthAccessToken, strFromUserName, strTextContent, strErrMsg);
                if(iRet < 0)
                {
                    LOG4CPLUS_ERROR(logger, "WXAPISendKFMsg failed , errmsg="<<strErrMsg);
    				DoReply(CGI_RET_CODE_SERVER_BUSY);
    				return true;
                }
                
                return true;
            }
            
			return true;
		}
};


int main()
{
    CgiWXAccountMsg cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

