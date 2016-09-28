#ifndef _COMMON_BDAPI_BD_API_POI_H_
#define _COMMON_BDAPI_BD_API_POI_H_

#include "bd_api.h"

namespace common
{
	namespace bdapi
	{
		class BDAPIPoi: public BDAPI
		{
		public:
			//使用ip白名单模式时，无需sn校验参数
			//使用校验方式时，必传sn【sn通过sk和请求参数计算得到】
			//在"百度开放平台-应用列表"(http://lbsyun.baidu.com/apiconsole/key)设置
			//可选参数，不需要用到时，传空字符串或零值

			/*
			 * Poi Info
			 *
			 * id				数据id	 						uint32 FUCK BD
			 * geotable_id		geotable_id						uint32 FUCK BD
			 * title			poi名称							string
			 * location			百度系坐标						float	格式为：[经度,纬度]
			 * gcj_location		国测局坐标						float	格式为：[经度,纬度]
			 * address			地址							string
			 * tags				标签							string
			 * create_time		用户创建日期					string
			 * modify_time		用户修改时间					string
			 * province			所在的省份						string
			 * district			所在的区						string
			 * {column key}		用户在column定义的key/value对	用户自定义的类类别
			 */

			int CreatePoi(const std::string& strTitle	//poi名称	string(256)	可选
					, const std::string& strAddress		//地址	string(256)	可选
					, const std::string& strTags		//tags	string(256)	可选
					, double dLatitude					//用户上传的纬度	double	必选
					, double dLongitude					//用户上传的经度	double	必选
					, uint32_t dwCoordType				//用户上传的坐标的类型	uint32	必选
														//1：GPS经纬度坐标
														//2：国测局加密经纬度坐标
														//3：百度加密经纬度坐标
														//4：百度加密墨卡托坐标
					, uint32_t dwGeotableId				//记录关联的geotable的标识	 FUCK BD	必选
					, const std::string& strBaiduAPIAK	//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK	//用户私钥，用于生成权限签名	string(50)	可选
					, std::map<std::string, std::string> mapCustomColumn
														//{column key}	用户在column定义的key/value对
														// 开发者自定义的类型（string、int、double）
														// 唯一索引字段必选，且需要保证唯一，否则会创建失败
					, uint32_t &dwId					//新增数据的id 	FUCK BD
			);
			int GetPoiList(std::map<std::string, std::string> mapCustomColumn
														//{index key}	用户在column定义的key/value对
														// column需要设置了is_index_field=1。
														// 对于string，是前缀匹配，如需精确匹配请在末尾加$。
														// 对于int或者double，则是范围查找，传递的格式为最小值,最大值。
														// 当无最小值或者最大值时，用-代替，同时，此字段最大长度不超过50，最小值与最大值都是整数
														//例：如加入一个命名为color数据类型为string的column，
														// 在检索是可设置为“color=red”的形式来检索color字段为red的POI
					, const std::string& strTitle		//记录（数据）名称	string(256)	可选
					, const std::string& strTags		//记录的标签（用于检索筛选）	string(256)	可选
					, const std::string& strBounds		//查询的矩形区域	string(100)	格式x1,y1;x2,y2分别代表矩形的左上角和右下角，可选
					, uint32_t dwGeotableId				//geotable_id	FUCK BD	必选
					, uint32_t dwPageIndex				//分页索引	uint32	默认为0
					, uint32_t dwPageSize				//分页数目	uint32	默认为10，上限为200
					, const std::string& strBaiduAPIAK	//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK	//用户的私钥，用于生成权限签名	string(50)	可选
					, Json::Value& jsonPoiList			//查询结果， PoiInfo列表
			);

			int GetPoiDetail(uint32_t dwId				//poi主键	uint64	必选
					, uint32_t dwGeotableId				//表主键	int32	必选
					, const std::string& strBaiduAPIAK	//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK	//用户的私钥，用于生成权限签名	string(50)	可选
					, Json::Value& jsonPoiInfo			//查询结果， PoiInfo
			);

			int UpdatePoi(uint32_t dwId					//poi的id	uint64	当不存在唯一索引字段时必须，存在唯一索引字段可选

					// !!!
					, const std::string strUniqueKey
					, const std::string strUniqueValue
					// !!!! *****自定义唯一索引key	Value	用户自定义类型	可选，
					// 若自定义索引字段和id共存时，将以id为准，且自定义索引key将被修改为新的value

					, const std::string& strTitle		//poi名称	string(256)
					, const std::string& strAddress		//地址
					, const std::string& strTags		//tags
					, double dLatitude					//用户上传的纬度	double
					, double dLongitude					//用户上传的经度	double
					, uint32_t dwCoordType				//用户上传的坐标的类型	uint32	必选
														//1．GPS经纬度坐标
														//2．测局加密经纬度坐标
														//3．百度加密经纬度坐标
														//4．百度加密墨卡托坐标
					, uint32_t dwGeotableId				//记录关联的geotable的标识	 FUCK BD	必选
					, const std::string& strBaiduAPIAK	//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK	//用户的私钥，用于生成权限签名	string(50)	可选
					, std::map<std::string, std::string> mapCustomColumn
														//{column key}	用户在column定义的key/value对	用户自定义的的列类别
			);


			//单条删除
			//根据 百度生成的数据id 来对数据进行删除
			int DeletePoiByID(uint32_t dwID
					, uint32_t dwGeotableId				//记录关联的geotable的标识	 FUCK BD	必选
					, const std::string& strBaiduAPIAK	//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK	//用户的私钥，用于生成权限签名	string(50)	可选
			);

			//单条删除
			//根据 自定义的主键 来对数据进行删除
			int DeletePoiByUniqueKey(const std::string& strUniqueKey	//自定义的主键 名字
					, const std::string& strUniqueValue					//要删除数据的主键 值
					, uint32_t dwGeotableId								//记录关联的geotable的标识	 FUCK BD	必选
					, const std::string& strBaiduAPIAK					//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK					//用户的私钥，用于生成权限签名	string(50)	可选
			);

			//批量删除
			//根据多个 百度生成的数据id 来对数据进行删除,
			int DeletePoiByIds(const std::string& strIds				//将id拼接成字符串形式，以“,”分割，最多1000个id
					, uint32_t dwGeotableId								//记录关联的geotable的标识	 FUCK BD	必选
					, const std::string& strBaiduAPIAK					//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK					//用户的私钥，用于生成权限签名	string(50)	可选
			);

			//批量删除
			//删除坐标范围在指定矩形区域内的数据
			int DeletePoiByBounds(const std::string& strBounds			//查询的矩形区域	string(100)	格式x1,y1;x2,y2分别代表矩形的左上角和右下角
					, uint32_t dwGeotableId								//记录关联的geotable的标识	 FUCK BD	必选
					, const std::string& strBaiduAPIAK					//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK					//用户的私钥，用于生成权限签名	string(50)	可选
			);

			//删除所有数据
			int DeleteALlPoi(uint32_t dwGeotableId						//记录关联的geotable的标识	 FUCK BD	必选
					, const std::string& strBaiduAPIAK					//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK					//用户的私钥，用于生成权限签名	string(50)	可选
			);

			//更多删除poi方式:
			//http://lbsyun.baidu.com/index.php?title=lbscloud/api/geodata#.E4.BD.8D.E7.BD.AE.E6.95.B0.E6.8D.AE.EF.BC.88poi.EF.BC.89.E7.AE.A1.E7.90.86


		public:
			int CreatePoi(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK, uint32_t& dwId);
			int GetPoiList(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK, Json::Value& jsonPoiList);
			int GetPoiDetail(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK, Json::Value& jsonPoiInfo);
			int UpdatePoi(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK);
			int DeletePoi(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK);
		};
	}
}

#endif
