#ifndef _COMMON_BDAPI_BD_API_H_
#define _COMMON_BDAPI_BD_API_H_

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include "util/logger.h"
#include <arpa/inet.h>
#include <errno.h>
#include "bd_api_util.h"
#include "bd_api_common_struct.h"
#include "jsoncpp/json.h"

namespace common
{
	namespace bdapi
	{
		class BDAPI
		{
		public:
			//百度api返回码值定义
			//http://lbsyun.baidu.com/index.php?title=lbscloud/api/geodata#.E8.BF.94.E5.9B.9E.E7.A0.81.E5.AE.9A.E4.B9.89.E5.80.BC.E8.A1.A8
			enum BD_API_STATUS_CODE
			{
				BD_API_STATUS_BULL_OPERATION = 21, //请求的API为批量操作，请求操作已完成，需要一段时间才能完全执行完毕
			};

			int32_t GetStatus()
			{
				return dwStatus;
			}
			std::string GetMessage()
			{
				return strMessage;
			}
			
		protected:
			int HttpGet(const std::string& strHost, uint16_t wPort, const std::string& strAPI, std::string& strRespBody);
			int HttpPost(const std::string& strHost, uint16_t wPort, const std::string& strAPI,
						 const std::string& strReqBody, std::string& strRespBody, const std::string& strContentType = "application/x-www-form-urlencoded");
			in_addr_t DNSParse(const std::string& strHost);
			
			int32_t dwStatus; // 百度API 状态码
			std::string strMessage; // 状态码描述
			
			DECL_LOGGER(logger);
			
		private:
			char _aczRequest[2048]; // request buffer
			char _aczResponse[2048]; //response buffer
			char _aczHostentTempBuffer[8*1024]; //gethostbyname use tmp buffer  8K
		
		};
	}
}
#endif

