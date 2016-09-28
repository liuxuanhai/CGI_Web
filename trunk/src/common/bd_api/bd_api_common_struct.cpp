#include "bd_api_common_struct.h"

namespace common
{
	namespace bdapi
	{
		IMPL_LOGGER(GeotableInfo, logger);

		int GeotableInfo::ParseFromJson(const Json::Value jsonGetotableInfo)
		{
			MAKE_SURE_KEY_EXIST(jsonGetotableInfo, "id");
			MAKE_SURE_KEY_EXIST(jsonGetotableInfo, "geotype");
			MAKE_SURE_KEY_EXIST(jsonGetotableInfo, "name");
			MAKE_SURE_KEY_EXIST(jsonGetotableInfo, "is_published");
			MAKE_SURE_KEY_EXIST(jsonGetotableInfo, "create_time");
			MAKE_SURE_KEY_EXIST(jsonGetotableInfo, "modify_time");

			_id = jsonGetotableInfo["id"].asUInt();
			_geotype = jsonGetotableInfo["geotype"].asUInt();
			_name = jsonGetotableInfo["name"].asString();
			_is_published = jsonGetotableInfo["is_published"].asUInt();
			_create_time = jsonGetotableInfo["create_time"].asUInt();
			_modify_time = jsonGetotableInfo["modify_time"].asUInt();
			return 0;
		}

		std::string GeotableInfo::ToString()const
		{
			std::ostringstream oss;
			oss.str("");
			oss << "{ id: " << _id
			<< ", geotype: " << _geotype
			<< ", name: " << _name
			<< ", is_published: " << _is_published
			<< ", create_time: " << _create_time
			<< ", modify_time: " << _modify_time
			<< " }";
			return oss.str();
		}

		IMPL_LOGGER(ColumnInfo, logger);

		int ColumnInfo::ParseFromJson(const Json::Value& jsonColumnInfo)
		{
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "id");
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "geotable_id");
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "name");
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "key");
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "type");
			if(jsonColumnInfo["type"] == COLUMN_KEY_TYPE_STRING)
			{
				MAKE_SURE_KEY_EXIST(jsonColumnInfo, "max_length");
			}
//			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "default_value");
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "create_time");
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "modify_time");
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "is_sortfilter_field");
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "is_search_field");
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "is_index_field");
			MAKE_SURE_KEY_EXIST(jsonColumnInfo, "is_unique_field");

			_id = jsonColumnInfo["id"].asUInt();
			_geotable_id = jsonColumnInfo["geotable_id"].asUInt();
			_name = jsonColumnInfo["name"].asString();
			_key = jsonColumnInfo["key"].asString();
			_type = jsonColumnInfo["type"].asUInt();;
			if(_type == COLUMN_KEY_TYPE_STRING && jsonColumnInfo.hasKey("max_length"))
			{
				_max_length = jsonColumnInfo["max_length"].asUInt();
			}
			else
			{
				_max_length = 0;
			}
			if(jsonColumnInfo.hasKey("default_value"))
			{
				if(_type == COLUMN_KEY_TYPE_INT)
				{
					int64_t qwDefVal = jsonColumnInfo["default_value"].asInt();
					_default_value = TypeToStr(qwDefVal);
				}
				else if(_type == COLUMN_KEY_TYPE_DOUBLE)
				{
					double dDefVal = jsonColumnInfo["default_value"].asDouble();
					_default_value = TypeToStr(dDefVal);
				}
				else if(_type == COLUMN_KEY_TYPE_STRING || _type == COLUMN_KEY_TYPE_URL)
				{
					_default_value = jsonColumnInfo["default_value"].asString();
				}
				else
				{
					LOG4CPLUS_ERROR(logger, "invalid type = " << _type);
					return -1;
				}
			}
			else
			{
				_default_value = "";
			}
			_create_time = jsonColumnInfo["create_time"].asString();
			_modify_time = jsonColumnInfo["modify_time"].asString();
			_is_sortfilter_field = jsonColumnInfo["is_sortfilter_field"].asUInt();
			_is_search_field = jsonColumnInfo["is_search_field"].asUInt();
			_is_index_field = jsonColumnInfo["is_index_field"].asUInt();
			_is_unique_field = jsonColumnInfo["is_unique_field"].asUInt();
			return 0;
		}
		std::string ColumnInfo::ToString() const
		{
			std::ostringstream oss;
			oss.str("");
			oss << "{ id: " << _id
			<< ", geotable_id: " << _geotable_id
			<< ", name: " << _name
			<< ", key: " << _key
			<< ", type: " << _type
			<< ", max_length: " << _max_length
			<< ", default_value: " << _default_value
			<< ", create_time: " << _create_time
			<< ", modify_time: " << _modify_time
			<< ", is_sortfilter_field: " << _is_sortfilter_field
			<< ", is_search_field: " << _is_search_field
			<< ", is_index_field: " << _is_index_field
			<< ", is_unique_field: " << _is_unique_field
			<< " }";
			return oss.str();
		}
	}
}
