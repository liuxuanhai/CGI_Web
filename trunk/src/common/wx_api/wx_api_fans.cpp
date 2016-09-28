#include "wx_api_fans.h"
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

				int WXAPIFans::GetFansInfo(const std::string& strAppid
									, const std::string& strAppscret
									, const std::string& strOpenid
									, WXFansInfo& stWXFansInfo)
				{
					std::string strPAAccessToken = "";
					if(PAGetAccessToken(strAppid, strAppscret, strPAAccessToken) < 0)
					{
						LOG4CPLUS_ERROR(logger, "GetFansInfo failed, msg=PAGetAccessToken failed");
						return -1;
					}

					return GetFansInfo(strPAAccessToken, strOpenid, stWXFansInfo);
				}

				int WXAPIFans::GetFansInfo(const std::string& strAccessToken
									, const std::string& strOpenid
									, WXFansInfo& stWXFansInfo)
				{
					//https://api.weixin.qq.com/cgi-bin/user/info?access_token=ACCESS_TOKEN&openid=OPENID&lang=zh_CN
					std::string strAPI = "cgi-bin/user/info";
					strAPI += "?access_token=" + strAccessToken;
					strAPI += "&openid=" + strOpenid;
					strAPI += "&lang=zh_CN";

					std::string strRspBody = "";
					int iRet = HttpsGet("api.weixin.qq.com", strAPI, strRspBody);
					if(iRet < 0)
					{
						LOG4CPLUS_ERROR(logger, "GetFansInfo failed, msg=HttpsGet failed");
						return -1;
					}

					Json::Value oJson;
					Json::Reader reader;
					if(!reader.parse(strRspBody, oJson, false))
					{
							LOG4CPLUS_ERROR(logger, "GetFansInfo failed, parse json error");
							return -1;
					}

					if(!oJson.hasKey("subscribe"))
					{
							LOG4CPLUS_ERROR(logger, "GetFansInfo failed, wx returned error structure, key [subscribe] not found for strRspBody="<<strRspBody);
							return -1;
					}

					if(oJson["subscribe"].asInt() == 0)
					{
							LOG4CPLUS_ERROR(logger, "GetFansInfo not exists, [subscribe]=0, not pa fans for openid="<<strOpenid);
							return 0;
					}

					//parse user info
					if(oJson.hasKey("openid"))
					{
						stWXFansInfo._openid = oJson["openid"].asString();
					}

					if(oJson.hasKey("nickname"))
					{
						stWXFansInfo._nick = oJson["nickname"].asString();
					}

					if(oJson.hasKey("sex"))
					{
						stWXFansInfo._sex = oJson["sex"].asInt();
					}

					if(oJson.hasKey("province"))
					{
						stWXFansInfo._province= oJson["province"].asString();
					}

					if(oJson.hasKey("country"))
					{
						stWXFansInfo._country = oJson["country"].asString();
					}

					if(oJson.hasKey("city"))
					{
						stWXFansInfo._city = oJson["city"].asString();
					}

					if(oJson.hasKey("headimgurl"))
					{
						stWXFansInfo._portrait_url = oJson["headimgurl"].asString();
					}

					if(oJson.hasKey("subscribe_time"))
					{
						stWXFansInfo._qwSubscribeTime = oJson["subscribe_time"].asUInt();
					}
					
					LOG4CPLUS_DEBUG(logger, "GetFansInfo succ, strOpenid="<<strOpenid
									<<", WXFansInfo="<<stWXFansInfo.to_string());

					return 1;
				}

		}
}


