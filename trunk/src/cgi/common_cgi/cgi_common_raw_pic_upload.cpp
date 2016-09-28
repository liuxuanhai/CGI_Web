#ifndef _HOOSHO_CGI_COMMON_PIC_UPLOAD_H_
#define _HOOSHO_CGI_COMMON_PIC_UPLOAD_H_

#include "cgi/cgi.h"
#include "jsoncpp/json.h"
#include "common_util.h"
#include "proto_io_tcp_client.h"
#include "util/lce_util.h"
#include "../cgi_ret_code_def.h"
#include <sstream>
#include <stdio.h>

class CgiCommonRawPicUpload: public lce::cgi::Cgi
{
public:
	 CgiCommonRawPicUpload(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("db", "db_ip", m_db_ip, "");
		GetConfig().GetValue("db", "db_user", m_db_user, "");
		GetConfig().GetValue("db", "db_passwd", m_db_pass, "");		
		GetConfig().GetValue("db", "db_name_pic", m_db_name_pic, "");
		GetConfig().GetValue("db", "table_name_pic_prefix", m_table_pic_name_prefix, "");

		m_mysql.Init(m_db_ip, m_db_name_pic, m_db_user, m_db_pass);
		return true;
	}

	virtual bool Process()
	{
		std::string strOpenidMd5 = (std::string)GetInput().GetValue("openid_md5");
		uint32_t dwPicNum = (uint32_t)GetInput().GetValue("pic_num");
		
		LOG4CPLUS_DEBUG(logger, "strOpenidMd5="<<strOpenidMd5<<", dwPicNum="<<dwPicNum);
		
		uint64_t qwOpenidMD5 = strtoul(strOpenidMd5.c_str(), NULL, 10);
		if(0 == qwOpenidMD5)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid req param, qwOpenidMD5="<<qwOpenidMD5);
			return true;
		}

		if(0 == dwPicNum)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid req param, dwPicNum="<<dwPicNum);
			return true;
		}

		if(dwPicNum > 10)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "too large dwPicNum="<<dwPicNum);
			return true;
		}

		std::vector<std::string> vecPicId;
		for(uint32_t i=0; i<dwPicNum; ++i)
		{
			std::string strPicParamName = "pic_data_" + int_2_str(i);
			std::string strPicData = (std::string)GetInput().GetFileData(strPicParamName);
			if(strPicData.empty())
			{
				LOG4CPLUS_ERROR(logger, "empty data for "<<strPicParamName);
				vecPicId.push_back("0");
				continue;
			}

			LOG4CPLUS_ERROR(logger, "pic.size="<<strPicData.size()<<", for "<<strPicParamName);

			//ignore first, for add more platform data
			uint64_t qwPicId = ::common::util::generate_unique_id();
			char aczTableName[32] = {0};
			::snprintf(aczTableName, 32, "%s%02x", m_table_pic_name_prefix.c_str(), (uint8_t)(qwPicId % 256));		
			std::ostringstream ossSql;
			ossSql.str("");
			ossSql<<"insert into "<<aczTableName
						<<" set pic_id="<<qwPicId
						<<", openid_md5="<<qwOpenidMD5
						<<", data='"<<lce::cgi::CMysql::MysqlEscape(strPicData)<<"'"
						<<", create_ts=unix_timestamp()";
			if(!m_mysql.Query(ossSql.str()))
			{
				LOG4CPLUS_ERROR(logger, "save pic failed, mysql query error, sql="<<ossSql<<", msg="<<m_mysql.GetErrMsg());
				vecPicId.push_back("0");
				continue;
			}

			vecPicId.push_back(int_2_str(qwPicId));
		}

		for(size_t i=0; i<vecPicId.size(); ++i)
		{	
			GetAnyValue()["pic_id_list"].push_back(vecPicId[i]);			
		}

		
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}

protected:
	//DB
	string m_db_ip;
	string m_db_user;
	string m_db_pass;	
	string m_db_name_pic;
	string m_table_pic_name_prefix;

	lce::cgi::CMysql m_mysql;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiCommonRawPicUpload, logger);

int main(int argc, char** argv)
{
	CgiCommonRawPicUpload cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

