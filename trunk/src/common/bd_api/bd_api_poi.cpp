#include "bd_api_poi.h"
#include <cstdio>

namespace common
{
	namespace bdapi
	{
		int BDAPIPoi::CreatePoi(const std::string &strTitle
				, const std::string &strAddress
				, const std::string &strTags
				, double dLatitude
				, double dLongitude
				, uint32_t dwCoordType
				, uint32_t dwGeotableId
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK
				, std::map<std::string, std::string> mapCustomColumn
				, uint32_t &dwId)
		{
			mapCustomColumn.insert(std::make_pair("title", strTitle));
			mapCustomColumn.insert(std::make_pair("address", strAddress));
			mapCustomColumn.insert(std::make_pair("tags", strTags));
			char buf[20];
			sprintf(buf, "%.9f", dLatitude);
			mapCustomColumn.insert(std::make_pair("latitude", buf));
			sprintf(buf, "%.9f", dLongitude);
			mapCustomColumn.insert(std::make_pair("longitude", buf));
			mapCustomColumn.insert(std::make_pair("coord_type", TypeToStr(dwCoordType)));
			mapCustomColumn.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableId)));
			mapCustomColumn.insert(std::make_pair("ak", strBaiduAPIAK));

			return CreatePoi(mapCustomColumn, strBaiduAPISK, dwId);
		}

		int BDAPIPoi::GetPoiList(std::map<std::string, std::string> mapCustomColumn
					, const std::string& strTitle
					, const std::string& strTags
					, const std::string& strBounds
					, uint32_t dwGeotableId
					, uint32_t dwPageIndex
					, uint32_t dwPageSize
					, const std::string& strBaiduAPIAK
					, const std::string& strBaiduAPISK
					, Json::Value& jsonPoiList
			)
		{
			mapCustomColumn.insert(std::make_pair("title", strTitle));
			mapCustomColumn.insert(std::make_pair("tags", strTags));
			mapCustomColumn.insert(std::make_pair("bounds", strBounds));
			mapCustomColumn.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableId)));
			mapCustomColumn.insert(std::make_pair("page_index", TypeToStr(dwPageIndex)));
			mapCustomColumn.insert(std::make_pair("page_size", TypeToStr(dwPageSize)));
			mapCustomColumn.insert(std::make_pair("ak", strBaiduAPIAK));
			
			return GetPoiList(mapCustomColumn, strBaiduAPISK, jsonPoiList);
		}

		int BDAPIPoi::GetPoiDetail(uint32_t dwId
				, uint32_t dwGeotableId
				, const std::string& strBaiduAPIAK
				, const std::string& strBaiduAPISK
				, Json::Value& jsonPoiInfo
				)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("id", TypeToStr(dwId)));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableId)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));

			return GetPoiDetail(mapReqParam, strBaiduAPISK, jsonPoiInfo);
		}

		int BDAPIPoi::UpdatePoi(uint32_t dwId
					, const std::string strUniqueKey
					, const std::string strUniqueValue
					, const std::string& strTitle
					, const std::string& strAddress
					, const std::string& strTags
					, double dLatitude
					, double dLongitude
					, uint32_t dwCoordType
					, uint32_t dwGeotableId
					, const std::string& strBaiduAPIAK
					, const std::string& strBaiduAPISK
					, std::map<std::string, std::string> mapCustomColumn
			)
		{
			if(dwId)
			{
				mapCustomColumn.insert(std::make_pair("id", TypeToStr(dwId)));
			}
			mapCustomColumn.insert(std::make_pair(strUniqueKey, strUniqueValue));
			mapCustomColumn.insert(std::make_pair("title", strTitle));
			mapCustomColumn.insert(std::make_pair("address", strAddress));
			mapCustomColumn.insert(std::make_pair("tags", strTags));
			char buf[20];
			sprintf(buf, "%.9f", dLatitude);
			mapCustomColumn.insert(std::make_pair("latitude", buf));
			sprintf(buf, "%.9f", dLongitude);
			mapCustomColumn.insert(std::make_pair("longitude", buf));
			mapCustomColumn.insert(std::make_pair("coord_type", TypeToStr(dwCoordType)));
			mapCustomColumn.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableId)));
			mapCustomColumn.insert(std::make_pair("ak", strBaiduAPIAK));

			return UpdatePoi(mapCustomColumn, strBaiduAPISK);
		}

		int BDAPIPoi::DeletePoiByID(uint32_t dwID
				, uint32_t dwGeotableId
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("id", TypeToStr(dwID)));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableId)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));
			
			return DeletePoi(mapReqParam, strBaiduAPISK);
		}

		int BDAPIPoi::DeletePoiByUniqueKey(const std::string &strUniqueKey
				, const std::string &strUniqueValue
				, uint32_t dwGeotableId
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair(strUniqueKey, strUniqueValue));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableId)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));
			
			return DeletePoi(mapReqParam, strBaiduAPISK);
		}

		int BDAPIPoi::DeletePoiByIds(const std::string &strIds
				, uint32_t dwGeotableId
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK)
		{
			if(strIds.empty())
			{
				LOG4CPLUS_ERROR(logger, "ids can't be empty, call DeleteAllPoi to del all poi if need!!!");
				return -1;
			}
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("ids", strIds));
			mapReqParam.insert(std::make_pair("is_total_del", "1"));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableId)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));
			
			return DeletePoi(mapReqParam, strBaiduAPISK);
		}


		int BDAPIPoi::DeletePoiByBounds(const std::string &strBounds
				, uint32_t dwGeotableId
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("bounds", strBounds));
			mapReqParam.insert(std::make_pair("is_total_del", "1"));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableId)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));
			
			return DeletePoi(mapReqParam, strBaiduAPISK);
		}

		int BDAPIPoi::DeleteALlPoi(uint32_t dwGeotableId
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("is_total_del", "1"));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableId)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));

			return DeletePoi(mapReqParam, strBaiduAPISK);
		}

		int BDAPIPoi::CreatePoi(const std::map<std::string, std::string> &mapReqParam, const std::string& strSK, uint32_t & dwId)
		{
			//build
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/poi/create // POST请求
			std::string strAPI = "geodata/v3/poi/create";

			std::string strSN = BuildSN(strAPI, strQueryString, strSK);
			LOG4CPLUS_DEBUG(logger, "build sn succ, sn = " << strSN);
			strQueryString += "&sn=" + strSN;

			LOG4CPLUS_DEBUG(logger, "api = " << strAPI);
			LOG4CPLUS_DEBUG(logger, "query_string = " << strQueryString);
			int iRet;
			std::string strRspBody;
			iRet = HttpPost("api.map.baidu.com", 80, strAPI, strQueryString, strRspBody);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "HttpPost failed");
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

			MAKE_SURE_KEY_EXIST(jValue, "message");
			strMessage = jValue["message"].asString();

			LOG4CPLUS_DEBUG(logger, strRspBody);

			if(dwStatus)
			{
				LOG4CPLUS_ERROR(logger, "baidu api return failed, status = " << dwStatus << ", msg = " << strMessage);
				return -1;
			}

			MAKE_SURE_KEY_EXIST(jValue, "id");
			dwId = jValue["id"].asUInt();

			return 0;
		}

		int BDAPIPoi::GetPoiList(const std::map<std::string, std::string> &mapReqParam, const std::string& strSK, Json::Value& jsonPoiList)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/poi/list // GET请求
			std::string strAPI = "geodata/v3/poi/list";

			std::string strSN = BuildSN(strAPI, strQueryString, strSK);
			LOG4CPLUS_DEBUG(logger, "build sn succ, sn = " << strSN);
			strQueryString += "&sn=" + strSN;

			LOG4CPLUS_DEBUG(logger, "api = " << strAPI);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
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

			MAKE_SURE_KEY_EXIST(jValue, "message");
			strMessage = jValue["message"].asString();

			LOG4CPLUS_DEBUG(logger, strRspBody);

			if(dwStatus)
			{
				LOG4CPLUS_ERROR(logger, "baidu api return failed, status = " << dwStatus << ", msg = " << strMessage);
				return -1;
			}

			MAKE_SURE_KEY_EXIST(jValue, "pois");
			jsonPoiList = jValue["pois"];

			return 0;
		}

		int BDAPIPoi::GetPoiDetail(const std::map<std::string, std::string> &mapReqParam, const std::string &strSK, Json::Value& jsonPoiInfo)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/poi/detail // GET请求
			std::string strAPI = "geodata/v3/poi/detail";

			std::string strSN = BuildSN(strAPI, strQueryString, strSK);
			LOG4CPLUS_DEBUG(logger, "build sn succ, sn = " << strSN);
			strQueryString += "&sn=" + strSN;

			LOG4CPLUS_DEBUG(logger, "api = " << strAPI);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
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

			MAKE_SURE_KEY_EXIST(jValue, "message");
			strMessage = jValue["message"].asString();

			LOG4CPLUS_DEBUG(logger, strRspBody);

			if(dwStatus)
			{
				LOG4CPLUS_ERROR(logger, "baidu api return failed, status = " << dwStatus << ", msg = " << strMessage);
				return -1;
			}

			MAKE_SURE_KEY_EXIST(jValue, "poi");
			jsonPoiInfo = jValue["poi"];

			return 0;
		}

		int BDAPIPoi::UpdatePoi(const std::map<std::string, std::string> &mapReqParam, const std::string &strSK)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/poi/update // POST请求
			std::string strAPI = "geodata/v3/poi/update";

			std::string strSN = BuildSN(strAPI, strQueryString, strSK);
			LOG4CPLUS_DEBUG(logger, "build sn succ, sn = " << strSN);
			strQueryString += "&sn=" + strSN;

			LOG4CPLUS_DEBUG(logger, "api = " << strAPI);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			int iRet;
			std::string strRspBody;
			iRet = HttpPost("api.map.baidu.com", 80, strAPI, strQueryString, strRspBody);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "HttpPost failed");
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

			MAKE_SURE_KEY_EXIST(jValue, "message");
			strMessage = jValue["message"].asString();

			LOG4CPLUS_DEBUG(logger, strRspBody);

			if(dwStatus)
			{
				LOG4CPLUS_ERROR(logger, "baidu api return failed, status = " << dwStatus << ", msg = " << strMessage);
				return -1;
			}

			return 0;
		}

		int BDAPIPoi::DeletePoi(const std::map<std::string, std::string> &mapReqParam, const std::string &strSK)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/poi/delete // POST请求
			std::string strAPI = "geodata/v3/poi/delete";

			std::string strSN = BuildSN(strAPI, strQueryString, strSK);
			LOG4CPLUS_DEBUG(logger, "build sn succ, sn = " << strSN);
			strQueryString += "&sn=" + strSN;

			LOG4CPLUS_DEBUG(logger, "api = " << strAPI);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			int iRet;
			std::string strRspBody;
			iRet = HttpPost("api.map.baidu.com", 80, strAPI, strQueryString, strRspBody);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "HttpPost failed");
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

			MAKE_SURE_KEY_EXIST(jValue, "message");
			strMessage = jValue["message"].asString();

			LOG4CPLUS_DEBUG(logger, strRspBody);

			if(dwStatus == BD_API_STATUS_BULL_OPERATION)
			{
				LOG4CPLUS_DEBUG(logger, "bull operation, need time to finish");
				return 0;
			}
			if(dwStatus)
			{
				LOG4CPLUS_ERROR(logger, "baidu api return failed, status = " << dwStatus << ", msg = " << strMessage);
				return -1;
			}

			return 0;
		}
	}
}
