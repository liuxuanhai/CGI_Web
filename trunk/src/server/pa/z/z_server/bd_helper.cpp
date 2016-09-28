#include "bd_helper.h"
#include "global_var.h"
#include "utility"
#include "util/str_op.h"

IMPL_LOGGER(BDHelper, logger);

int BDHelper::CreatePoi(double fLongitude, double fLatitude, uint64_t qwOrderid, uint32_t dwOrderType)
{
	std::map<std::string, std::string> mapReqParam;
	char buf[20];
	sprintf(buf, "%.9f", fLatitude);
	mapReqParam.insert(std::make_pair("latitude", buf));
	sprintf(buf, "%.9f", fLongitude);
	mapReqParam.insert(std::make_pair("longitude", buf));
	mapReqParam.insert(std::make_pair("coord_type", "3"));  //3：百度加密经纬度坐标
	mapReqParam.insert(std::make_pair("orderid", int_2_str(qwOrderid)));
	mapReqParam.insert(std::make_pair("order_type", int_2_str(dwOrderType)));
	mapReqParam.insert(std::make_pair("geotable_id", int_2_str(m_bd_api_geotable_id)));
	mapReqParam.insert(std::make_pair("ak", m_bd_api_ak));
	mapReqParam.insert(std::make_pair("title", int_2_str(qwOrderid)));

	common::bdapi::BDAPIPoi bdapiPoi;
	uint32_t dwID;
	int iRet = bdapiPoi.CreatePoi(mapReqParam, m_bd_api_sk, dwID);
	if(iRet < 0)
	{
		LOG4CPLUS_ERROR(logger, "BDAHelper.CreatePoi failed");
		return -1;
	}

	return 0;
}

int BDHelper::DeletePoi(uint64_t qwOrderid)
{
	std::map<std::string, std::string> mapReqParam;
	mapReqParam.insert(std::make_pair("geotable_id", int_2_str(m_bd_api_geotable_id)));
	mapReqParam.insert(std::make_pair("ak", m_bd_api_ak));
	mapReqParam.insert(std::make_pair("orderid", int_2_str(qwOrderid)));

	common::bdapi::BDAPIPoi bdapiPoi;
	int iRet = bdapiPoi.DeletePoi(mapReqParam, m_bd_api_sk);
	if(iRet < 0)
	{
		LOG4CPLUS_ERROR(logger, "BDHelper.DeletePoi failed");
		return -1;
	}
	return 0;
}

int BDHelper::Geosearch(double fLongitude, double fLatitude, uint32_t dwRadius,
						uint32_t dwPageIndex, uint32_t dwPageSize,
						std::vector<uint64_t>& vecOrderidList, uint32_t& dwSize, uint32_t& dwTotal)
{
	char cLocation[50];
	snprintf(cLocation, 50, "%.6f,%.6f", fLongitude, fLatitude);

	std::map<std::string, std::string> mapReqParam;
	mapReqParam.insert(std::make_pair("geotable_id", int_2_str(m_bd_api_geotable_id)));
	mapReqParam.insert(std::make_pair("ak", m_bd_api_ak));
	mapReqParam.insert(std::make_pair("location", cLocation));
	mapReqParam.insert(std::make_pair("coord_type", "3"));
	mapReqParam.insert(std::make_pair("radius", int_2_str(dwRadius)));
	mapReqParam.insert(std::make_pair("page_index", int_2_str(dwPageIndex)));
	mapReqParam.insert(std::make_pair("page_size", int_2_str(dwPageSize)));

	common::bdapi::BDAPIGeosearch bdapiGeosearch;
	Json::Value jsonPoiList;

	int iRet = bdapiGeosearch.GeosearchNearby(mapReqParam, m_bd_api_sk, jsonPoiList, dwSize, dwTotal);
	if(iRet < 0)
	{
		LOG4CPLUS_ERROR(logger, "BDHelper.Geosearch failed");
		return -1;
	}

	vecOrderidList.clear();
	std::string strOrderid;
	if(dwSize && jsonPoiList.isArray() && jsonPoiList.size())
	{
		for(size_t i = 0; i < jsonPoiList.size(); i++)
		{
			strOrderid = jsonPoiList[i]["orderid"].asString();
			vecOrderidList.push_back(strtoul(strOrderid.c_str(), NULL, 10));
		}
	}

	return 0;
}
