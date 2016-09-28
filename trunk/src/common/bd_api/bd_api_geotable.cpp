#include "bd_api_geotable.h"

namespace common
{
	namespace bdapi
	{
		int BDAPIGeotable::CreateGeotable(const std::string& strName
				, uint32_t dwGeotype
				, uint32_t dwIsPublished
				, const std::string& strBaiduAPIAK
				, const std::string &strBaiduAPISK
				, uint32_t dwTimestamp
				, uint32_t &dwID)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("name", strName));
			mapReqParam.insert(std::make_pair("geotype", TypeToStr(dwGeotype)));
			mapReqParam.insert(std::make_pair("is_published", TypeToStr(dwIsPublished)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));
			mapReqParam.insert(std::make_pair("timestamp", TypeToStr(dwTimestamp)));

			return CreateGeotable(mapReqParam, strBaiduAPISK, dwID);
		}

		int BDAPIGeotable::GetGeotableList(const std::string& strName
				, const std::string& strBaiduAPIAK
				, const std::string& strBaiduAPISK
				, std::vector<GeotableInfo>& vecGeotableInfo)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("name", strName));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));
			Json::Value jsonGeotableList;
			int iRet;
			iRet = GetGeotableList(mapReqParam, strBaiduAPISK, jsonGeotableList);
			if(iRet < 0)
			{
				return iRet;
			}
			GeotableInfo stGeotableInfo;
			for(size_t i = 0; i < jsonGeotableList.size(); i++)
			{
				if(stGeotableInfo.ParseFromJson(jsonGeotableList[i]) < 0)
				{
					return -1;
				}
				vecGeotableInfo.push_back(stGeotableInfo);
			}
			return 0;
		}

		int BDAPIGeotable::GetGeotableDetail(uint32_t dwID
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK
				, GeotableInfo &stGeotableInfo)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("id", TypeToStr(dwID)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));
			Json::Value jsonGeotableInfo;
			int iRet;
			iRet = GetGeotableDetail(mapReqParam, strBaiduAPISK, jsonGeotableInfo);
			if(iRet < 0)
			{
				return iRet;
			}
			return stGeotableInfo.ParseFromJson(jsonGeotableInfo);
		}

		int BDAPIGeotable::UpdateGeotable(uint32_t dwID
				, uint32_t dwIsPublished
				, const std::string &strName
			 	, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("id", TypeToStr(dwID)));
			mapReqParam.insert(std::make_pair("is_published", TypeToStr(dwIsPublished)));
			mapReqParam.insert(std::make_pair("name", strName));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));
			return UpdateGeotable(mapReqParam, strBaiduAPISK);
		}

		int BDAPIGeotable::DeleteGeotable(uint32_t dwID
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("id", TypeToStr(dwID)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));
			return DeleteGeotable(mapReqParam, strBaiduAPISK);
		}

		int BDAPIGeotable::CreateGeotable(const std::map<std::string, std::string> &mapReqParam, const std::string& strSK, uint32_t & dwID)
		{
			//check param
			std::string strName;
			std::map<std::string, std::string>::const_iterator iter;
			iter = mapReqParam.find("name");
			if(iter == mapReqParam.end())
			{
				LOG4CPLUS_DEBUG(logger, "need param 'name' not found");
				return -1;
			}
			strName = iter->second;
			if(strName.empty())
			{
				LOG4CPLUS_ERROR(logger, "value of name is empty");
				return -1;
			}
			if(strName.find(' ') != std::string::npos)
			{
				LOG4CPLUS_ERROR(logger, "invalid name = " << strName << ", contains SPACE, not allowed");
				return -1;
			}
			
			//build
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/geotable/create  //POST请求
			std::string strAPI = "geodata/v3/geotable/create";
		
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
			dwID = jValue["id"].asUInt();

			return 0;
		}
		int BDAPIGeotable::GetGeotableList(const std::map<std::string, std::string> &mapReqParam, const std::string& strSK, Json::Value& jsonGeotableList)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/geotable/list // GET请求
			std::string strAPI = "geodata/v3/geotable/list";
		
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

			MAKE_SURE_KEY_EXIST(jValue, "geotables");
			jsonGeotableList = jValue["geotables"];
		
			return 0;
		}
		
		int BDAPIGeotable::GetGeotableDetail(const std::map<std::string, std::string> &mapReqParam, const std::string &strSK, Json::Value& jsonGeotableInfo)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/geotable/detail // GET请求
			std::string strAPI = "geodata/v3/geotable/detail";
		
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

			MAKE_SURE_KEY_EXIST(jValue, "geotable");
			jsonGeotableInfo = jValue["geotable"];
		
			return 0;
		}
		
		int BDAPIGeotable::UpdateGeotable(const std::map<std::string, std::string> &mapReqParam, const std::string &strSK)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/geotable/update // POST请求
			std::string strAPI = "geodata/v3/geotable/update";
		
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

		int BDAPIGeotable::DeleteGeotable(const std::map<std::string, std::string> &mapReqParam, const std::string &strSK)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/geotable/delete // POST请求
			//注：当geotable里面没有有效数据时，才能删除geotable
			std::string strAPI = "geodata/v3/geotable/delete";

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
	}
}
