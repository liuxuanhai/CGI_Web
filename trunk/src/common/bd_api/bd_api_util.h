#ifndef _COMMON_BDAPI_BD_API_UTIL_H_
#define _COMMON_BDAPI_BD_API_UTIL_H_

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include "util/logger.h"
#include <arpa/inet.h>

namespace common
{
	namespace bdapi
	{
		#define MAKE_SURE_KEY_EXIST(jValue, key)  \
		if(!jValue.hasKey(key)) \
		{ \
			LOG4CPLUS_ERROR(logger, "key "#key" not found"); \
			return -1; \
		}

		//http://lbsyun.baidu.com/index.php?title=lbscloud/api/geodata#.E6.95.B0.E6.8D.AE.E5.88.97.EF.BC.88column.EF.BC.89.E7.AE.A1.E7.90.86
		enum GEOTABLE_GEOTYPE
		{
			GEOTABLE_GEOTYPE_POINT = 1, 	//点
//			GEOTABLE_GEOTYPE_LINE = 2,  	//线, 目前百度暂不支持
			GEOTABLE_GEOTYPE_SURFACE = 3, 	//面
		};

		enum COLUMN_KEY_TYPE
		{
			COLUMN_KEY_TYPE_INT = 1, 	// int  【百度支持int64。(jsoncpp/json.h)Json::Value只支持int32、uint32】
									// 仅当使用int32和uint32时使用INT类型
									// 若需int64或uint64，应当在百度中存储为STRING类型，在自己的代码自行转换
			COLUMN_KEY_TYPE_DOUBLE = 2,	// double
			COLUMN_KEY_TYPE_STRING = 3,	// string
			COLUMN_KEY_TYPE_URL = 4,		// 在线图片URL
		};

		//http://lbsyun.baidu.com/index.php?title=lbscloud/api/geosearch
		//百度云搜索相关接口，只能使用【百度加密经纬度坐标】和【百度加密墨卡托坐标】
		enum COORD_TYPE
		{
			COORD_TYPE_GPS = 1, 				//GPS经纬度坐标
			COORD_TYPE_GCJ_ENC = 2,				//国测局加密经纬度坐标
			COORD_TYPE_BD_ENC = 3,				//百度加密经纬度坐标
			COORD_TYPE_BD_ENC_MERCATOR = 4,		//百度加密墨卡托坐标
		};
		
		std::string BuildSN(const std::string& strAPI, const std::string& strQueryString, const std::string& strSK);
		std::string ReqParamToQueryString(const std::map<std::string, std::string>& mapReqParam);
		std::string UrlEncode(const std::string& strSrc);
		std::string Char2Hex(const char c);

		template <typename T>
		static std::string TypeToStr(const T &n)
		{
			std::ostringstream stream;
			stream << n;
			return stream.str();
		}
	}
}



#endif
