#include "wx_api_oauth2.h"
#include "jsoncpp/json.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

namespace common
{
		namespace wxapi
		{

				int WXAPIOAuth2::OAuth2GetAccessToken(const std::string& strAppid
								, const std::string& strAppsecret
								, const std::string& strPreAuthCode
								, std::string& strAccessToken
								, std::string& strRefreshToken
								, std::string& strOpenid
								, std::string& strScope)
				{
						//https://api.weixin.qq.com/sns/oauth2/access_token?appid=APPID&secret=SECRET&code=CODE&grant_type=authorization_code
						std::string strAPI = "sns/oauth2/access_token";
						strAPI += "?appid=" + strAppid;
						strAPI += "&secret=" + strAppsecret;
						strAPI += "&code=" + strPreAuthCode;
						strAPI += "&grant_type=authorization_code";

						std::string strRspBody = "";
						int iRet = HttpsGet("api.weixin.qq.com", strAPI, strRspBody);
						if(iRet < 0)
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetAccessToken failed, msg=HttpsGet failed");
								return -1;
						}

						Json::Value oJson;
						Json::Reader reader;
						if(!reader.parse(strRspBody, oJson, false))
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetAccessToken failed, parse json error");
								return -1;
						}

						if(oJson.hasKey("errcode"))
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetAccessToken failed, wx return errcode");
								return -1;
						}

						if(!oJson.hasKey("access_token"))
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetAccessToken failed, wx return invalid, access_token not found!!");
								return -1;
						}

						if(!oJson.hasKey("refresh_token"))
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetAccessToken failed, wx return invalid, refresh_token not found!!");
								return -1;
						}

						if(!oJson.hasKey("openid"))
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetAccessToken failed, wx return invalid, openid not found!!");
								return -1;
						}

						if(!oJson.hasKey("scope"))
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetAccessToken failed, wx return invalid, scope not found!!");
								return -1;
						}

						strAccessToken = oJson["access_token"].asString();
						strRefreshToken = oJson["refresh_token"].asString();
						strOpenid = oJson["openid"].asString();
						strScope = oJson["scope"].asString();

						LOG4CPLUS_DEBUG(logger, "OAuth2GetAccessToken succ, strAccessToken="<<strAccessToken
										<<", strRefreshToken="<<strRefreshToken
										<<", strOpenid="<<strOpenid
										<<", strScope="<<strScope);

						return 0;
				}

				int WXAPIOAuth2::OAuth2GetUserInfo(const std::string& strOpenid
												, const std::string& strUserInfoScoreAccessToken
												, WXUserInfo& stWXUserInfo)
				{
						//https://api.weixin.qq.com/sns/userinfo?access_token=ACCESS_TOKEN&openid=OPENID&lang=zh_CN
						std::string strAPI = "sns/userinfo";
						strAPI += "?access_token=" + strUserInfoScoreAccessToken;
						strAPI += "&openid=" + strOpenid;
						strAPI += "&lang=zh_CN";

						std::string strRspBody = "";
						int iRet = HttpsGet("api.weixin.qq.com", strAPI, strRspBody);
						if(iRet < 0)
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetUserInfo failed, msg=HttpsGet failed");
								return -1;
						}

						Json::Value oJson;
						Json::Reader reader;
						if(!reader.parse(strRspBody, oJson, false))
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetUserInfo failed, parse json error");
								return -1;
						}

						if(oJson.hasKey("errcode"))
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetUserInfo failed, wx return errcode");
								return -1;
						}

						if(!oJson.hasKey("openid"))
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetUserInfo failed, wx return invalid, openid not found!!");
								return -1;
						}

						if(strOpenid != oJson["openid"].asString())
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetUserInfo failed, wx return invalid, rsp_openid="<<oJson["openid"].asString()
												<<", while req_openid="<<strOpenid);
								return -1;
						}
						stWXUserInfo._openid = strOpenid;

						if(oJson.hasKey("nickname"))
						{
								stWXUserInfo._nick = oJson["nickname"].asString();
						}

						if(oJson.hasKey("sex"))
						{
								stWXUserInfo._sex = oJson["sex"].asInt();
						}

						if(oJson.hasKey("country"))
						{
								stWXUserInfo._country = oJson["country"].asString();
						}

						if(oJson.hasKey("province"))
						{
								stWXUserInfo._province = oJson["province"].asString();
						}

						if(oJson.hasKey("city"))
						{
								stWXUserInfo._city = oJson["city"].asString();
						}

						if(oJson.hasKey("headimgurl"))
						{
								stWXUserInfo._portrait_url = oJson["headimgurl"].asString();
						}

						LOG4CPLUS_DEBUG(logger, "OAuth2GetUserInfo succ, stWXUserInfo="<<stWXUserInfo.to_string());
						return 0;	
				}

				int WXAPIOAuth2::OAuth2GetUserInfo(const std::string& strAppid
								, const std::string& strAppsecret
								, const std::string& strPreAuthCode
								, WXUserInfo& stWXUserInfo)
				{
						std::string strAccessToken = "";
						std::string strRefreshToken = "";
						std::string strOpenid = "";
						std::string strScope = "";
						int iRet = OAuth2GetAccessToken(strAppid, strAppsecret, strPreAuthCode, strAccessToken, strRefreshToken, strOpenid, strScope);
						if(iRet < 0)
						{
								LOG4CPLUS_ERROR(logger, "OAuth2GetUserInfo failed, msg=OAuth2GetAccessToken failed");
								return -1;
						}

						return OAuth2GetUserInfo(strOpenid, strAccessToken, stWXUserInfo);
				}

		}
}


