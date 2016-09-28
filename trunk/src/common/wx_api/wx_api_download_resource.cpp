#include "wx_api_download_resource.h"
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

				int WXAPIDownloadResource::DownloadResource(const std::string& strPAAccessToken, const std::string& strMediaId, std::string& strResource)
				{
						//https://file.api.weixin.qq.com/cgi-bin/media/get?access_token=ACCESS_TOKEN&media_id=MEDIA_ID
						std::string strAPI = "cgi-bin/media/get";
						strAPI += "?access_token=" + strPAAccessToken;
						strAPI += "&media_id=" + strMediaId;

						std::string strRspBody = "";
						int iRet = HttpGet("file.api.weixin.qq.com", 80, strAPI, strRspBody);
						if(iRet < 0)
						{
								LOG4CPLUS_ERROR(logger, "DownloadResource failed, msg=HttpGet failed");
								return -1;
						}

						if(strRspBody.empty())
						{
								LOG4CPLUS_ERROR(logger, "DownloadResource failed, msg=strRspBody is empty, cao ta da ye");
								return -1;
						}

						if(strRspBody[0] == '{')
						{
								if(strRspBody.find("errcode") != std::string::npos)
								{
										LOG4CPLUS_ERROR(logger, "DownloadResource failed, errcode found, cao ta da ye, strRspBody="<<strRspBody);
										return -1;
								}
						}

						strResource = strRspBody;
						LOG4CPLUS_DEBUG(logger, "DownloadResource succ, strResource.size="<<strResource.size());

						return 0;
				}

				int WXAPIDownloadResource::DownloadResource(const std::string& strAppid, const std::string& strAppscret, const std::string& strMediaId, std::string& strResource)
				{
						std::string strPAAccessToken = "";
						int iRet = PAGetAccessToken(strAppid, strAppscret, strPAAccessToken);
						if(iRet < 0)
						{
								LOG4CPLUS_ERROR(logger, "DownloadResource failed, msg=PAGetAccessToken failed");
								return -1;
						}

						iRet = DownloadResource(strPAAccessToken, strMediaId, strResource);
						if(iRet < 0)
						{
								LOG4CPLUS_ERROR(logger, "DownloadResource failed, msg=raw DownloadResource failed");
								return -1;
						}

						return 0;
				}

		}
}
