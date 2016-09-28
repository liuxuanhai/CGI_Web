#include "wx_api_parse_ipaddr.h"
#include "jsoncpp/json.h"

namespace common
{
	namespace wxapi
	{
		int WXAPIParseIPAddr::ParseIPAddr(const std::string& strIP, std::string& strIPCity)
		{
			//[case 1] http://int.dpool.sina.com.cn/iplookup/iplookup.php?format=json&ip=58.62.52.61
			//[case 2] http://ip.taobao.com/service/getIpInfo.php?ip=58.62.52.61
			strIPCity = "Unidentified";
			std::string strAPI = "service/getIpInfo.php?ip=" + strIP;
			
			std::string strIPAddrJson = "";			
			uint32_t iRet = HttpGet("ip.taobao.com", 80, strAPI, strIPAddrJson);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "ParseIPAddr failed, msg=HttpGet failed");
				return -1;
			}

			if(strIPAddrJson.empty())
			{
				LOG4CPLUS_ERROR(logger, "ParseIPAddr failed, msg=strRspBody is empty, cao ta da ye");
				return -1;
			}

			LOG4CPLUS_DEBUG(logger, "strRspBody: "<<strIPAddrJson);
			
			Json::Reader jReader;
			Json::Value jValue;			
			if(!jReader.parse(strIPAddrJson, jValue, false))
			{
				LOG4CPLUS_ERROR(logger, "parse json error, data="<<strIPAddrJson);	
				return -1;
			}
			else
			{
				if(1 == jValue["code"].asInt()) //"invaild ip"
				{
					LOG4CPLUS_ERROR(logger, "respon data error: "<<jValue["data"].asString());
					return -1;
				}

				strIPCity = jValue["data"]["city"].asString();
				
				LOG4CPLUS_DEBUG(logger, "ParseIPAddr succ, ip: "<<strIP<<" -> "<<strIPCity);				
			}
			
			return 0;
		}
	}
}

