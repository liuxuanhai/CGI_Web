#ifndef _COMMON_BDAPI_BD_API_GEOTABLE_H_
#define _COMMON_BDAPI_BD_API_GEOTABLE_H_

#include "bd_api.h"
#include <map>

namespace common
{
	namespace bdapi
	{
		class BDAPIColumn: public BDAPI
		{
		public:
			//使用ip白名单模式时，无需sn校验参数
			//使用校验方式时，必传sn【sn通过sk和请求参数计算得到】
			//在"百度开放平台-应用列表"(http://lbsyun.baidu.com/apiconsole/key)设置
			//可选参数，不需要用到时，传空字符串或零值

			int CreateColumn(const std::string& strName			//column的属性(中文)名称	string(45)	必选
					, const std::string& strKey					//column存储的属性key	string(45)	必选，同一个geotable内的名字不能相同
					, uint32_t dwType							//存储的值的类型	uint32	必选，枚举值1:Int64, 2:double, 3:string, 4:在线图片url
					, uint32_t dwMaxLength						//最大长度	uint32	最大值2048，最小值为1。
																// 当type为string该字段有效，此时该字段必填。此值代表utf8的汉字个数，不是字节个数
					, const std::string& strDefaultValue		//默认值	string(45)	设置默认值
																//type为4种类型中的任何一种都可设置默认值，但需转成string以增加通用性
					, uint32_t dwIsSortfilterField				//是否检索引擎的数值排序筛选字段	uint32	必选
																//1代表是，0代表否。设置后，在请求 LBS云检索时可针对该字段进行排序。该字段只能为int或double类型，最多设置15个
					, uint32_t dwIsSearchField					//是否检索引擎的文本检索字段	uint32	必选
																//1代表支持，0为不支持。只有type为string时可以设置检索字段，只能用于字符串类型的列且最大长度不能超过512个字节
					, uint32_t dwIsIndexField					//是否存储引擎的索引字段	uint32	必选
																//用于存储接口查询:1代表支持，0为不支持
																//注：is_index_field=1 时才能在根据该列属性值检索时检索到数据
					, uint32_t dwIsUniqueField					//是否云存储唯一索引字段，方便更新，删除，查询	uint32	可选，1代表是，0代表否。
																// 设置后将在数据创建和更新时进行该字段唯一性检查，并可以以此字段为条件进行数据的更新、删除和查询。最多设置1个
					, uint32_t dwGeotableID						//所属于的geotable_id	 uint32  FUCK BD
					, const std::string& strBaiduAPIAK			//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK			//用户的私钥，用于生成权限签名	string(50)	可选
					, uint32_t& dwID							//新增的数据id	uint32  FUCK BD
			);

			int GetColumnList(const std::string strName			//geotable meta的属性中文名称	string(45)	可选
					, const std::string& strKey 				//geotable meta存储的属性key	string(45)	可选
					, uint32_t dwGeotableID						//所属于的geotable_id	 uint32	必选 FUCK BD
					, const std::string& strBaiduAPIAK			//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK			//用户的私钥，用于生成权限签名	string(50)	可选
					, std::vector<ColumnInfo>& vecColumnInfoList	//返回的ColumnInfo列表
			);

			int GetColumnDetail(uint32_t dwID				//列的id	uint32	必选
					, uint32_t dwGeotableID					//表的id	uint32	必选
					, const std::string& strBaiduAPIAK		//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK		//用户的私钥，用于生成权限签名	string(50)	可选
					, ColumnInfo& stColumnInfo				//ColumnInfo
			);

			int UpdateColumn(uint32_t dwID					//列主键	uint32	必选
					, uint32_t dwGeotableID					//所属表主键	uint32	必选
					, const std::string& strName			//geotable的中文名称	string(45)	可选
					, const std::string& strDefaultValue	//默认值	string	可选
					, uint32_t dwMaxLength					//文本最大长度	int32	字符串最大长度，只能改大，不能改小
					, uint32_t dwIsSortfilterField			//是否检索引擎的数值排序字段	uint32	1代表是，0代表否，如果poi表中已经有数据，并且发布到了 LBS云检索，修改此字段，会引起批量操作
					, uint32_t dwIsSearchField				//是否检索引擎的文本检索字段	uint32	1代表是，0代表否，如果poi表中已经有数据，并且发布到了 LBS云检索，修改此字段，会引起批量操作
					, uint32_t dwIsIndexField				//是否存储引擎的索引字段	uint32	1代表是，0代表否
					, uint32_t dwIsUniqueField				//是否存储索引的唯一索引字段	uint32	1代表是，0代表否。设置后将在数据创建和更新时进行该字段唯一性检查，并可以以此字段为条件进行数据的更新、删除和查询。最多设置1个
					, const std::string& strBaiduAPIAK		//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK		//用户的私钥，用于生成权限签名	string(50)	可选
			);

			int DeleteColumn(uint32_t dwID					//列的id	int32	必选
					, uint32_t dwGeotableID					//所属表主键	uint32	必选
					, const std::string& strBaiduAPIAK		//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK		//用户的权限签名	string(50)	可选
			);

		public:
			int CreateColumn(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK, uint32_t & dwId);
			int GetColumnList(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK, Json::Value& jsonColumnList);
			int GetColumnDetail(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK, Json::Value& jsonColumnInfo);
			int UpdateColumn(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK);
			int DeleteColumn(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK);
		};
	}
}


#endif
