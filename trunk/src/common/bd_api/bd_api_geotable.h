#ifndef _COMMON_BDAPI_BD_API_GEOTABLE_H_
#define _COMMON_BDAPI_BD_API_GEOTABLE_H_

#include "bd_api.h"
#include <map>

namespace common
{
	namespace bdapi
	{
		class BDAPIGeotable: public BDAPI
		{
		public:
			//使用ip白名单模式时，无需sn校验参数
			//使用校验方式时，必传sn 【sn通过sk和请求参数计算得到】
			//在"百度开放平台-应用列表"(http://lbsyun.baidu.com/apiconsole/key)设置
			//可选参数，不需要用到时，传空字符串或零值

			int CreateGeotable(const std::string& strName	//geotable的(中文)名称	string(45)	必选
					, uint32_t  dwGeotype					//geotable持有数据的类型	int32	必选
															//1：点；2：线；3：面。默认为1（当前不支持“线”）
					, uint32_t dwIsPublished				//是否发布到检索	int32	必选
															//0：未自动发布到云检索，
															//1：自动发布到云检索；
															//注：1）通过URL方式创建表时只有is_published=1时 在云检索时表内的数据才能被检索到。
															//2）可通过数据管理模块设置，在设置中将是否发送到检索一栏中选定为是即可。
					, const std::string& strBaiduAPIAK  	//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK 		//用户的私钥，用于生成权限签名	string(50)	可选
					, uint32_t dwTimestamp					//时间戳	uint32	可选，配合sn使用，增加时间戳安全验证)
					, uint32_t &dwID						//新增表id  FUCK BD
			);

			int GetGeotableList(const std::string& strName	//geotable的名字	string(45)	可选
					, const std::string& strBaiduAPIAK  	//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK 		//用户的私钥，用于生成权限签名	string(50)	可选
					, std::vector<GeotableInfo>& vecGeotableInfoList //GeotableInfo列表
			);

			int GetGeotableDetail(uint32_t dwID				//geotable的id	uint32	必选
					, const std::string& strBaiduAPIAK  	//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK 		//用户的私钥，用于生成权限签名	string(50)	可选
					, GeotableInfo& stGeotableInfo			//GeotableInfo
			);

			int UpdateGeotable(uint32_t  dwID				//geotable主键	uint32	必选
					, uint32_t  dwIsPublished				//是否发布到检索	int32	会引起批量操作
					, const std::string& strName			//geotable的中文名称	string(45)	可选
					, const std::string& strBaiduAPIAK  	//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK 		//用户的私钥，用于生成权限签名	string(50)	可选
			);

			int DeleteGeotable(uint32_t  dwID				//geotable主键	uint32	必选
					, const std::string& strBaiduAPIAK 		//用户的访问权限key	string(50)	必选
					, const std::string& strBaiduAPISK 		//用户的私钥，用于生成权限签名	string(50)	可选
			);

		public:
			int CreateGeotable(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK, uint32_t &dwID);
			int GetGeotableList(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK, Json::Value& jsonGeotableList);
			int GetGeotableDetail(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK, Json::Value& jsonGeotableInfo);
			int UpdateGeotable(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK);
			int DeleteGeotable(const std::map<std::string, std::string>& mapReqParam, const std::string& strSK);
		};
	}
}


#endif
