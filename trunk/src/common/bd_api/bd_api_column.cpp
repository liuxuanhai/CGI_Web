#include "bd_api_column.h"

namespace common
{
	namespace bdapi
	{
		int BDAPIColumn::CreateColumn(const std::string &strName
				, const std::string &strKey
				, uint32_t dwType
				, uint32_t dwMaxLength
				, const std::string &strDefaultValue
				, uint32_t dwIsSortfilterField
				, uint32_t dwIsSearchField
				, uint32_t dwIsIndexField
				, uint32_t dwIsUniqueField
				, uint32_t dwGeotableID
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK
				, uint32_t &dwID)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("name", strName));
			mapReqParam.insert(std::make_pair("key", strKey));
			mapReqParam.insert(std::make_pair("type", TypeToStr(dwType)));
			if(dwType == COLUMN_KEY_TYPE_STRING) //type=3,表示该字段类型为string，max_length字段必填
			{
				mapReqParam.insert(std::make_pair("max_length", TypeToStr(dwMaxLength)));
			}
			mapReqParam.insert(std::make_pair("default_value", TypeToStr(strDefaultValue)));
			mapReqParam.insert(std::make_pair("is_sortfilter_field", TypeToStr(dwIsSortfilterField)));
			mapReqParam.insert(std::make_pair("is_search_field", TypeToStr(dwIsSearchField)));
			mapReqParam.insert(std::make_pair("is_index_field", TypeToStr(dwIsIndexField)));
			mapReqParam.insert(std::make_pair("is_unique_field", TypeToStr(dwIsUniqueField)));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableID)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));

			return CreateColumn(mapReqParam, strBaiduAPISK, dwID);
		}

		int BDAPIColumn::GetColumnList(const std::string strName
				, const std::string &strKey
				, uint32_t dwGeotableID
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK
				, std::vector<ColumnInfo> &vecColumnInfoList)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("name", strName));
			mapReqParam.insert(std::make_pair("key", strKey));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableID)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));

			Json::Value jsonColumnList;
			int iRet;
			iRet = GetColumnList(mapReqParam, strBaiduAPISK, jsonColumnList);
			if(iRet < 0)
			{
				return iRet;
			}

			ColumnInfo stColumnInfo;
			for(size_t i = 0; i < jsonColumnList.size(); i++)
			{
				if(stColumnInfo.ParseFromJson(jsonColumnList[i]))
				{
					return -1;
				}
				vecColumnInfoList.push_back(stColumnInfo);
			}
			return 0;
		}

		int BDAPIColumn::GetColumnDetail(uint32_t dwID
				, uint32_t dwGeotableID
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK
				, ColumnInfo& stColumnInfo)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("id", TypeToStr(dwID)));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableID)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));
			Json::Value jsonColumnInfo;
			int iRet;
			iRet = GetColumnDetail(mapReqParam, strBaiduAPISK, jsonColumnInfo);
			if(iRet < 0)
			{
				return iRet;
			}
			return stColumnInfo.ParseFromJson(jsonColumnInfo);
		}

		int BDAPIColumn::UpdateColumn(uint32_t dwID
					, uint32_t dwGeotableID
					, const std::string& strName
					, const std::string& strDefaultValue
					, uint32_t dwMaxLength
					, uint32_t dwIsSortfilterField
					, uint32_t dwIsSearchField
					, uint32_t dwIsIndexField
					, uint32_t dwIsUniqueField
					, const std::string& strBaiduAPIAK
					, const std::string& strBaiduAPISK)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("id", TypeToStr(dwID)));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableID)));
			mapReqParam.insert(std::make_pair("name", strName));
			mapReqParam.insert(std::make_pair("default_value", strDefaultValue));
			if(dwMaxLength)
			{
				mapReqParam.insert(std::make_pair("max_length", TypeToStr(dwMaxLength)));
			}
			mapReqParam.insert(std::make_pair("is_sortfilter_field", TypeToStr(dwIsSortfilterField)));
			mapReqParam.insert(std::make_pair("is_search_field", TypeToStr(dwIsSearchField)));
			mapReqParam.insert(std::make_pair("is_index_field", TypeToStr(dwIsIndexField)));
			mapReqParam.insert(std::make_pair("is_unique_field", TypeToStr(dwIsUniqueField)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));

			return UpdateColumn(mapReqParam, strBaiduAPISK);
		}

		int BDAPIColumn::DeleteColumn(uint32_t dwID
				, uint32_t dwGeotableID
				, const std::string &strBaiduAPIAK
				, const std::string &strBaiduAPISK)
		{
			std::map<std::string, std::string> mapReqParam;
			mapReqParam.insert(std::make_pair("id", TypeToStr(dwID)));
			mapReqParam.insert(std::make_pair("geotable_id", TypeToStr(dwGeotableID)));
			mapReqParam.insert(std::make_pair("ak", strBaiduAPIAK));

			return DeleteColumn(mapReqParam, strBaiduAPISK);
		}

		int BDAPIColumn::CreateColumn(const std::map<std::string, std::string> &mapReqParam, const std::string& strSK, uint32_t & dwId)
		{
			//build
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/column/create // POST请求
			std::string strAPI = "geodata/v3/column/create";
		
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

		int BDAPIColumn::GetColumnList(const std::map<std::string, std::string> &mapReqParam, const std::string& strSK, Json::Value& jsonColumnList)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/column/list // GET请求
			std::string strAPI = "geodata/v3/column/list";
		
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

			MAKE_SURE_KEY_EXIST(jValue, "columns");
			jsonColumnList = jValue["columns"];
			return 0;
		}
		
		int BDAPIColumn::GetColumnDetail(const std::map<std::string, std::string> &mapReqParam, const std::string &strSK, Json::Value& jsonColumnInfo)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/column/detail // GET请求
			std::string strAPI = "geodata/v3/column/detail";
		
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

			MAKE_SURE_KEY_EXIST(jValue, "column");
			jsonColumnInfo = jValue["column"];
		
			return 0;
		}
		
		int BDAPIColumn::UpdateColumn(const std::map<std::string, std::string> &mapReqParam, const std::string &strSK)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/column/update // POST请求
			std::string strAPI = "geodata/v3/column/update";
		
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

		int BDAPIColumn::DeleteColumn(const std::map<std::string, std::string> &mapReqParam, const std::string &strSK)
		{
			std::string strQueryString = ReqParamToQueryString(mapReqParam);
			LOG4CPLUS_DEBUG(logger, "query string = " << strQueryString);
			//http://api.map.baidu.com/geodata/v3/column/delete // POST请求
			std::string strAPI = "geodata/v3/column/delete";

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
