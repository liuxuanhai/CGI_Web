#include "wx_api_ticket.h"
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
#include <openssl/sha.h>
#include <sstream>

namespace common
{
		namespace wxapi
		{

				int WXAPITicket::GetJSAPITicket(const std::string& strPAAccessToken, std::string& strJSAPITicket)
				{
						//https://api.weixin.qq.com/cgi-bin/ticket/getticket?access_token=ACCESS_TOKEN&type=jsapi
						std::string strAPI = "cgi-bin/ticket/getticket";
						strAPI += "?access_token=" + strPAAccessToken;
						strAPI += "&type=jsapi";

						std::string strRspBody = "";
						int iRet = HttpsGet("api.weixin.qq.com", strAPI, strRspBody);
						if(iRet < 0)
						{
								LOG4CPLUS_ERROR(logger, "GetJSAPITicket failed, msg=HttpsGet failed");
								return -1;
						}

						Json::Value oJson;
						Json::Reader reader;
						if(!reader.parse(strRspBody, oJson, false))
						{
								LOG4CPLUS_ERROR(logger, "GetJSAPITicket failed, parse json error");
								return -1;
						}

						if(oJson.hasKey("errcode") && oJson["errcode"].asInt() != 0)
						{
								LOG4CPLUS_ERROR(logger, "GetJSAPITicket failed, wx return errcode");
								return -1;
						}

						if(!oJson.hasKey("ticket"))
						{
								LOG4CPLUS_ERROR(logger, "GetJSAPITicket failed, wx return invalid, ticket not found!!");
								return -1;
						}

						strJSAPITicket = oJson["ticket"].asString();
						LOG4CPLUS_DEBUG(logger, "GetJSAPITicket succ, strJSAPITicket="<<strJSAPITicket);

						/*
						   std::string _req_url; 
						   std::string _pa_appid;
						   std::string _ticket;
						   std::string _nonce;
						   uint64_t    _timestamp;
						   std::string _signature;*/


						return 0;
				}

				int WXAPITicket::GetJSAPITicket(const std::string& strAppid, const std::string& strAppscret, std::string& strJSAPITicket)
				{
						std::string strPAAccessToken = "";
						int iRet = PAGetAccessToken(strAppid, strAppscret, strPAAccessToken);
						if(iRet < 0)
						{
								LOG4CPLUS_ERROR(logger, "GetJSAPITicket failed, msg=PAGetAccessToken failed");
								return -1;
						}

						iRet = GetJSAPITicket(strPAAccessToken, strJSAPITicket);
						if(iRet < 0)
						{
								LOG4CPLUS_ERROR(logger, "GetJSAPITicket failed, msg=raw GetJSAPITicket failed");
								return -1;
						}

						return 0;
				}

		}
}
