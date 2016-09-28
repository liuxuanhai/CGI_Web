#ifndef _HOOSHO_PA_SERVER_WX_API_H_
#define _HOOSHO_PA_SERVER_WX_API_H_

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include "util/logger.h"
#include <arpa/inet.h>

namespace common
{
	namespace wxapi
	{


			class WXAPI
			{
					public:
							int PAGetAccessToken(const std::string& strAppid, const std::string& strAppSecret, std::string& strPAAccessToken);

					protected:
							int HttpsGet(const std::string& strHost, const std::string& strAPI, std::string& strRespBody);
							int HttpsPost(const std::string& strHost, const std::string& strAPI, const std::string& strReqBody, std::string& strRespBody, const std::string& strContentType="text/plain", uint16_t wPort=443);							
							int HttpGet(const std::string& strHost, uint16_t wPort, const std::string& strAPI, std::string& strRespBody);
							int CurlSSLPost(const std::string& strUrl
											, const std::string& strReqBody
											, std::string& strRspBody
											, const std::string& strRootcaPath
											, const std::string& strAPIClientCertPath
											, const std::string& strAPIClientKeyPath
											);

					private:
							in_addr_t DNSParse(const std::string& strHost); //return ip_net_order_int if succ; 0 if error
							static size_t write_data(void *ptr, size_t sSize, size_t sNmemb, void *stream);
					

					private:
							char _aczRequest[2048]; // request buffer
							char _aczResponse[2048]; //response buffer
							char _aczHostentTempBuffer[8*1024]; //gethostbyname use tmp buffer  8K

					protected:
							DECL_LOGGER(logger);


					public:
						enum WX_RETURN_ERRCODE_TYPE
						{
							WX_RETURN_ERRCODE_SUCC = 0,
							WX_RETURN_ERRCODE_INVALID_BUTTON_SIZE = 40016,
							WX_RETURN_ERRCODE_MENU_NO_EXIST = 46003,
							WX_RETURN_ERRCODE_INVALID_MEDIA_ID = 40007,
						};
						std::string errcode2str(int errcode)
						{
							std::string strRet;
							if(errcode == WX_RETURN_ERRCODE_SUCC)
							{
								strRet = "succ";
							}
							else if(errcode == WX_RETURN_ERRCODE_INVALID_BUTTON_SIZE)
							{
								strRet = "invalid button size";
							}
							else if(errcode == WX_RETURN_ERRCODE_MENU_NO_EXIST)
							{
								strRet = "menu no exist";
							}
							else if(errcode == WX_RETURN_ERRCODE_INVALID_MEDIA_ID)
							{
								strRet = "invalid media_id";
							}
							else
							{
								strRet = "unknow";
							}
							return strRet;
						}
			};

	}
}

#endif













