#include "bd_api_geosearch.h"

namespace common
{
	namespace bdapi
	{
		int BDAPIGeosearch::GeosearchNearby(const std::string& strQeuryStr
					, const std::string& strLocation
					, uint32_t dwCoordType
					, uint32_t dwRadius
					, const std::string& strTags
					, const std::string& strSortby
					, const std::string& strFilter
					, uint32_t dwPageIndex
					, uint32_t dwPageSize
					, const std::string& strCallback
					, uint32_t dwGeotableID
					, const std::string& strBaiduAPIAK
					, const std::string& strBaiduAPISK
					, Json::Value& jsonPoiList
					, uint32_t& dwSize
					, uint32_t& dwTotal
			)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("q", strQeuryStr));
			mapReqParam.insert(std::make_pair("location", strLocation));
			mapReqParam.insert(std::make_pair("coord_type", TypeToStr(dwCoordType)));
			mapReqParam.insert(std::make_pair("radius", TypeToStr(dwRadius)));
			mapReqParam.insert(std::make_pair("tags", strTags));
			mapReqParam.insert(std::make_pair("sortby", strSortby));
			mapReqParam.insert(std::make_pair("filter", strFilter));
			mapReqParam.insert(std::make_pair("page_index", TypeToStr(dwPageIndex)));
			if(dwPageSize)
			{
				mapReqParam.insert(std::make_pair("page_size", TypeToStr(dwPageSize)));
			}
			mapReqParam.insert(std::make_pair("callback", strCallback));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableID)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));

			return GeosearchNearby(mapReqParam, strBaiduAPISK, jsonPoiList, dwSize, dwTotal);
		}

		int BDAPIGeosearch::GeosearchNearby(const std::map<std::string, std::string> &mapReqParam, const std::string &strSK,
											Json::Value &jsonPoiList, uint32_t &dwSize, uint32_t &dwTotal)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query_string = " << strQueryString);

			//http://api.map.baidu.com/geosearch/v3/nearby // GET请求
			std::string strAPI = "geosearch/v3/nearby";

			std::string strSN = BuildSN(strAPI, strQueryString, strSK);
			strQueryString += "&sn=" + strSN;

			LOG4CPLUS_DEBUG(logger, "api = " << strAPI);
			LOG4CPLUS_DEBUG(logger, "query_string = " << strQueryString);

			int iRet;
			std::string strRspBody;
			iRet = HttpGet("api.map.baidu.com", 80, strAPI + "?" + strQueryString, strRspBody);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "HttpGet failed");
				return -1;
			}
			if(strRspBody.empty())
			{
				LOG4CPLUS_ERROR(logger, "RespBody empty");
				return -1;
			}

			Json::Value jValue;
			Json::Reader jReader;
			if(!jReader.parse(strRspBody, jValue, false))
			{
				LOG4CPLUS_ERROR(logger, "parse json error, data = " << strRspBody);
				return -1;
			}
			MAKE_SURE_KEY_EXIST(jValue, "status");
			dwStatus = jValue["status"].asInt();

			if(jValue.hasKey("message"))
			{
				strMessage = jValue["message"].asString();
			}
			else
			{
				strMessage = "";
			}

			LOG4CPLUS_DEBUG(logger, strRspBody);

			if(dwStatus)
			{
				LOG4CPLUS_ERROR(logger, "baidu api return failed, status = " << dwStatus << ", msg = " << strMessage);
				return -1;
			}

			MAKE_SURE_KEY_EXIST(jValue, "size");
			dwSize = jValue["size"].asUInt();

			MAKE_SURE_KEY_EXIST(jValue, "total");
			dwTotal = jValue["total"].asUInt();

			MAKE_SURE_KEY_EXIST(jValue, "contents");
			jsonPoiList = jValue["contents"];

			return 0;
		}

	}
}
