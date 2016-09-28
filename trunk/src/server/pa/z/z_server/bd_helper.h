#ifndef _Z_SERVER_BD_HELPER_H_
#define _Z_SERVER_BD_HELPER_H_

#include "bd_api_poi.h"
#include "bd_api_geosearch.h"

class BDHelper
{

public:
	BDHelper(const std::string& strBDApiAK, const std::string& strBDApiSK, const uint64_t& qwBDApiGeotableId):
			m_bd_api_ak(strBDApiAK), m_bd_api_sk(strBDApiSK), m_bd_api_geotable_id(qwBDApiGeotableId)
	{
	}

	//增加热点, 经纬度使用“百度加密经纬度坐标”，前端通过百度js获取, orderid为自定义unique_key
	//http://lbsyun.baidu.com/index.php?title=lbscloud/api/geodata#.E4.BD.8D.E7.BD.AE.E6.95.B0.E6.8D.AE.EF.BC.88poi.EF.BC.89.E7.AE.A1.E7.90.86
	int CreatePoi(double fLongitude, double fLatitude,
				   uint64_t qwOrderid, uint32_t dwOrderType);

	//删除热点
	int DeletePoi(uint64_t qwOrderid);

	//查询热点
	//http://lbsyun.baidu.com/index.php?title=lbscloud/api/geosearch#poi.E5.91.A8.E8.BE.B9.E6.90.9C.E7.B4.A2
	int Geosearch(double fLongitude, double fLatitude, uint32_t dwRadius,
				  uint32_t dwPageIndex, uint32_t dwPageSize,
	              std::vector<uint64_t>& vecOrderidList, uint32_t& dwSize, uint32_t& dwTotal);

public:
	std::string m_bd_api_ak;
	std::string m_bd_api_sk;
	uint64_t m_bd_api_geotable_id;

	DECL_LOGGER(logger);
};

#endif
