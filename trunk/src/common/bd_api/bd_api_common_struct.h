#ifndef _COMMON_BDAPI_BD_API_COMMON_STRUCT_H_
#define _COMMON_BDAPI_BD_API_COMMON_STRUCT_H_

#include "jsoncpp/json.h"
#include "bd_api_util.h"

namespace common
{
	namespace bdapi
	{
		class GeotableInfo
		{
		public:
			uint32_t _id;				//geotable的主键	uint32	FUCK BD
			uint32_t _geotype;			//geo类型
			std::string _name;			//geotable的中文名称	string
			uint32_t _is_published;		//是否发布到检索	int32
			uint32_t _create_time;		//创建日期		FUCK BD
			uint32_t _modify_time;		//最近一次修改时间	FUCK BD

		public:
			int ParseFromJson(const Json::Value jsonGetotableInfo);
			std::string ToString()const;

			DECL_LOGGER(logger);
		};

		class ColumnInfo
		{
		public:
			uint32_t _id;					//id  uint32		FUCK BD
			uint32_t _geotable_id;			//所属于的geotable_id  FUCK BD
			std::string _name;				//中文名称	string
			std::string _key;				//存储的属性key	string
			uint32_t _type;					//存储的值的类型	uint32
			uint32_t _max_length;			//列的最大长度	uint32
			std::string _default_value;		//列的默认值	string(32)
			std::string _create_time;		//列的创建时间
			std::string _modify_time;		//列的修改时间
			uint32_t _is_sortfilter_field;	//是否检索端排序字段	uint32
			uint32_t _is_search_field;		//是否检索端文本检索字段	uint32
			uint32_t _is_index_field;		//是否存储管理索引字段	uint32
			uint32_t _is_unique_field;		//是否存储管理唯一索引字段	uint32

		public:
			int ParseFromJson(const Json::Value& jsonColumnInfo);
			std::string ToString() const;

			DECL_LOGGER(logger);
		};
	}
}

#endif
