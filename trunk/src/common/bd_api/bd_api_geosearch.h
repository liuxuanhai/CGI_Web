#ifndef _COMMON_BDAPI_BD_API_GEOSEARCH_H_
#define _COMMON_BDAPI_BD_API_GEOSEARCH_H_

#include "bd_api.h"

namespace common
{
	namespace bdapi
	{
		class BDAPIGeosearch:public BDAPI
		{
		public:
			//使用ip白名单模式时，无需sn校验参数
			//使用校验方式时，必传sn【sn通过sk和请求参数计算得到】
			//在"百度开放平台-应用列表"(http://lbsyun.baidu.com/apiconsole/key)设置
			//可选参数，不需要用到时，传空字符串或零值

			int GeosearchNearby(const std::string& strQeuryStr	//检索关键字	string(45)	任意汉字或数字，英文字母，可以为空字符	可选
					, const std::string& strLocation			//检索的中心点	string(25)	逗号分隔的经纬度	必须样例：116.4321,38.76623
					, uint32_t dwCoordType						//坐标系	uint32	数字	可选
																//3代表百度经纬度坐标系统 4代表百度墨卡托系统
					, uint32_t dwRadius							//检索半径	uint32	单位为米，默认为1000	可选
					, const std::string& strTags				//标签	string(45)	空格分隔的多字符串	可选 样例：美食 小吃
					
					, const std::string& strSortby				//排序字段	string	”分隔的多个检索条件。
					// 格式为sortby={key1}:value1|{key2:val2|key3:val3}。
					// 最多支持16个字段排序 {keyname}:1 升序 {keyname}:-1 降序
					// 以下keyname为系统预定义的： distance 距离排序 weight 权重排序
					// 可选
					// 默认为按weight排序 如果需要自定义排序则指定排序字段 样例：按照价格由便宜到贵排序 sortby=price:1
					
					, const std::string& strFilter				//过滤条件	string(50)	竖线分隔的多个key-value对
					// key为筛选字段的名称(存储服务中定义) 支持连续区间或者离散区间的筛选： a:连续区间 key:value1,value2 b:离散区间 key:[value1,value2,value3,...]
					// 可选
					// 样例:
					// a:连续区间
					// 筛选价格为9.99到19.99并且生产时间为2012的项 price:9.99,19.99|time:2012,2012
					// b:离散区间
					// 筛选价格为8,9,13，并且生产时间为2012年的项 price:[8,9,13]|time:2012,2012
					// 注：符号为英文半角中括号

					, uint32_t dwPageIndex						//分页索引	uint32	当前页标，从0开始	可选 默认为0
					, uint32_t dwPageSize						//分页数量	uint32	当前页面最大结果数	可选 默认为10，最多为50
					, const std::string& strCallback			//回调函数	string(20)	js回调函数	可选
					, uint32_t dwGeotableID						//所属于的geotable_id	 uint32  FUCK BD
					, const std::string& strBaiduAPIAK			//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK			//用户的私钥，用于生成权限签名	string(50)	可选
					, Json::Value& jsonPoiList					//搜索结果，PoiInfo列表
					, uint32_t& dwSize							//分页参数，当前页返回数量
					, uint32_t& dwTotal							//分页参数，所有召回数量
			);

		public:
			//size	分页参数，当前页返回数量	int32
			//total	分页参数，所有召回数量	int32
			int GeosearchNearby(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK, Json::Value& jsonPoiList, uint32_t& size, uint32_t& total);

		};
	}
}

#endif
