#ifndef _HOOSHO_CGI_UE_RESOURCE_UPLOAD_H_
#define _HOOSHO_CGI_UE_RESOURCE_UPLOAD_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "common_util.h"
#include "proto_io_tcp_client.h"
#include "util/lce_util.h"
#include "cgi_ue_common.h"
#include <sstream>
#include <fstream>
#include <stdio.h>


class CgiUEResourceUpload: public lce::cgi::Cgi
{
public:
	 CgiUEResourceUpload(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }


public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("db", "db_ip", m_db_ip, "");
		GetConfig().GetValue("db", "db_user", m_db_user, "");
		GetConfig().GetValue("db", "db_passwd", m_db_pass, "");		
		GetConfig().GetValue("db", "db_name_ue", m_db_name_ue, "");
		GetConfig().GetValue("db", "table_name_resource_prefix", m_table_resource_name_prefix, "");

		GetConfig().GetValue("ue", "config_json", m_ue_config_json, "");

		m_mysql.Init(m_db_ip, m_db_name_ue, m_db_user, m_db_pass);
		return true;
	}


	bool FailReply(const std::string& strReplyState)
	{
		std::ostringstream oss;
		oss.str("");
		oss<<"{\"state\":\""<<strReplyState<<"\"}";

		SetOutputJson(oss.str());
		DoReply(0);
		return true;
	}

	uint64_t generate_unique_id()
	{
		timeval tv;
		gettimeofday(&tv, NULL);
		uint64_t qwMS = (uint64_t)tv.tv_sec * 1000;
		qwMS += ((uint64_t)tv.tv_usec / 1000);
		srand(tv.tv_usec ^ 77);
		uint16_t randNum = rand() % 65536;

		//高6字节为毫秒， 低2字节为随机数
		uint64_t qwRetId = (qwMS << 16) | randNum;
		return qwRetId;	
	}

	virtual bool Process()
	{
		const std::string strQueryString = GetInput().GetQueryString();
		LOG4CPLUS_INFO(logger, "strQueryString="<<strQueryString);

		//1.parse action
		string strReqAction = "";
		std::vector<std::string> vecParam;
		lce::cgi::Split(strQueryString, "&", vecParam);
		for(size_t i=0; i<vecParam.size(); ++i)
		{
			const std::string& strPair = vecParam[i];
			std::vector<std::string> vecKeyValue;
			lce::cgi::Split(strPair, "=", vecKeyValue);
			if(2 == vecKeyValue.size() && "action" == vecKeyValue[0] && !vecKeyValue[1].empty())
			{
				strReqAction = vecKeyValue[1];
				break;
			}
		}

		if(strReqAction.empty())
		{
			LOG4CPLUS_ERROR(logger, "actionParam is empty!");
			return FailReply(REPLY_STATE_IVALID_PARAM);
		}

		//2. 配置检查 ue_config.json
		std::string strUEConfigJsonPath = GetConfigPath() + m_ue_config_json;
		std::ifstream fin(strUEConfigJsonPath.c_str());
		Json::Value oJson;
		Json::Reader reader;
		if(!reader.parse(fin, oJson, false))
		{
			LOG4CPLUS_ERROR(logger, "parse ue_config failed, strUEConfigJsonPath="<<strUEConfigJsonPath);
			return FailReply(REPLY_STATE_CONFIG_PARSE_FAILED);			
		}

		uint32_t dwResourceType = 0;
		std::string strUEConfigUploadFieldName = "";
		uint32_t dwUEConfigUploadMaxSize = 0;
		if(oJson.hasKey("imageActionName") && oJson["imageActionName"].asString() == strReqAction)
		{
			//上传图片请求
			if(!oJson.hasKey("imageFieldName") || !oJson.hasKey("imageMaxSize"))
			{
				LOG4CPLUS_ERROR("logger", "action="<<strReqAction<<", config absent in (imageFieldName,imageMaxSize)");
				return FailReply(REPLY_STATE_CONFIG_PARAM_ABSENT);
			}

			strUEConfigUploadFieldName = oJson["imageFieldName"].asString();
			dwUEConfigUploadMaxSize = oJson["imageMaxSize"].asInt();
			dwResourceType = UE_RESOURCE_TYPE_IMAGE;
		}
		else if(oJson.hasKey("videoActionName") && oJson["videoActionName"].asString() == strReqAction)
		{
			//上传视频请求 
			if(!oJson.hasKey("videoFieldName") || !oJson.hasKey("videoMaxSize"))
			{
				LOG4CPLUS_ERROR("logger", "action="<<strReqAction<<", config absent in (videoFieldName,videoMaxSize)");
				return FailReply(REPLY_STATE_CONFIG_PARAM_ABSENT);
			}

			strUEConfigUploadFieldName = oJson["videoFieldName"].asString();
			dwUEConfigUploadMaxSize = oJson["videoMaxSize"].asInt();
			dwResourceType = UE_RESOURCE_TYPE_VIDEO;
		}
		else if(oJson.hasKey("fileActionName") && oJson["fileActionName"].asString() == strReqAction)
		{
			//上传一般文件请求
			if(!oJson.hasKey("fileFieldName") || !oJson.hasKey("fileMaxSize"))
			{
				LOG4CPLUS_ERROR("logger", "action="<<strReqAction<<", config absent in (fileFieldName,fileMaxSize)");
				return FailReply(REPLY_STATE_CONFIG_PARAM_ABSENT);
			}

			strUEConfigUploadFieldName = oJson["fileFieldName"].asString();
			dwUEConfigUploadMaxSize = oJson["fileMaxSize"].asInt();
			dwResourceType = UE_RESOURCE_TYPE_FILE;
		}
		else if(strReqAction != REQ_ACTION_CONFIG)
		{
			//非上传请求， 也非config请求，无效的action
			LOG4CPLUS_ERROR(logger, "unknown actionParam="<<strReqAction);
			return FailReply(REPLY_STATE_IVALID_PARAM);
		}

		//3. config请求
		if(0 == dwResourceType)
		{
			SetOutputJson(oJson.toJsonString());
			return DoReply(0);	
		}

		//4. 上传类请求
		std::string strResourceData = (std::string)GetInput().GetFileData(strUEConfigUploadFieldName);		
		if(strResourceData.empty())
		{
			LOG4CPLUS_ERROR(logger, "empty data for "<<strUEConfigUploadFieldName);
			return FailReply(REPLY_STATE_UPLOAD_EMPTY);
		}
		if(strResourceData.size() > dwUEConfigUploadMaxSize)
		{
			LOG4CPLUS_ERROR(logger, "strResourceData.size="<<strResourceData.size()<<", too large for dwUEConfigUploadMaxSize="<<dwUEConfigUploadMaxSize);
			return FailReply(REPLY_STATE_SIZE_EXCEED);
		}

		/*
		+---------------+---------------------+------+-----+---------+-------+
		| Field         | Type                | Null | Key | Default | Extra |
		+---------------+---------------------+------+-----+---------+-------+
		| resource_id   | bigint(20) unsigned | NO   | PRI | 0       |       |
		| resource_type | int(10) unsigned    | NO   |     | 0       |       |
		| data          | mediumblob          | NO   |     | NULL    |       |
		| name          | blob                | NO   |     | NULL    |       |
		| create_ts     | bigint(20) unsigned | NO   |     | 0       |       |
		+---------------+---------------------+------+-----+---------+-------+	
		*/
		std::string strResourceName = GetInput().GetFileName(strUEConfigUploadFieldName);
		uint64_t qwResourceId = generate_unique_id();
		char aczTableName[32] = {0};
		::snprintf(aczTableName, 32, "%s%02x", m_table_resource_name_prefix.c_str(), (uint8_t)(qwResourceId % 256));		
		std::ostringstream ossSql;
		ossSql.str("");
		ossSql<<"insert into "<<aczTableName
					<<" set resource_id="<<qwResourceId
					<<", resource_type="<<dwResourceType
					<<", data='"<<lce::cgi::CMysql::MysqlEscape(strResourceData)<<"'"
					<<", name='"<<lce::cgi::CMysql::MysqlEscape(strResourceName)<<"'"
					<<", create_ts=unix_timestamp()";
		if(!m_mysql.Query(ossSql.str()))
		{
			LOG4CPLUS_ERROR(logger, "save resource failed, mysql query error, sql="<<ossSql<<", msg="<<m_mysql.GetErrMsg());
			return FailReply(REPLY_STATE_SERVER_BUSY);
		}


		//4. 回包成功
		//{"original":"demo.jpg","name":"demo.jpg","url":"\/server\/ueditor\/upload\/image\/demo.jpg","size":"99697","type":".jpg","state":"SUCCESS"}
		std::string strSuffix = ".fuck";
		size_t szPos = strResourceName.find_last_of(".");
		if(szPos != std::string::npos)
		{
			strSuffix = strResourceName.substr(szPos);
		}
		std::ostringstream output;
		output.str("");
		output<<"{"
			<<"\"state\":\""<<REPLY_STATE_SUCC<<"\","
			<<"\"original\":\""<<strResourceName<<"\","
			<<"\"name\":\""<<strResourceName<<"\","
			<<"\"url\":\"/cgi-bin/ue/cgi_ue_resource_download?resource_id="<<qwResourceId<<"\","
			<<"\"size\":\""<<strResourceData.size()<<"\","
			<<"\"type\":\""<<strSuffix<<"\""
			<<"}";

		SetOutputJson(output.str());
		return DoReply(0);
	}


protected:
	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;	
	string m_db_name_ue;
	string m_table_resource_name_prefix;

	//ue config
	string m_ue_config_json;

	lce::cgi::CMysql m_mysql;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUEResourceUpload, logger);

int main(int argc, char** argv)
{
	CgiUEResourceUpload cgi(0, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

