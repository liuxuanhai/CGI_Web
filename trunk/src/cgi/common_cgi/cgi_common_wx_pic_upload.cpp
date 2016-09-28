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

class CgiCommonWXPicUpload: public lce::cgi::Cgi
{
public:
	 CgiCommonWXPicUpload(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
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
		GetConfig().GetValue("pa_server", "ip", m_pa_server_ip, "");
		GetConfig().GetValue("pa_server", "port", m_pa_server_port, 0);

		m_mysql.Init(m_db_ip, m_db_name_pic, m_db_user, m_db_pass);
		return true;
	}

	int ReqWxPic(const std::string strMediaId, std::string& strData)
	{
		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::DOWNLOAD_PIC_RESOURCE_REQ);
		pHead->set_seq(time(0));
		::hoosho::pa::DownLoadPicResourceReq* pDownLoadPicResourceReq = stRequest.mutable_download_pic_resource_req();
		pDownLoadPicResourceReq->set_media_id(strMediaId);

		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_pa_server_ip, m_pa_server_port);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "ReqWxPic failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return -1;	
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::DOWNLOAD_PIC_RESOURCE_RES)
		{
			LOG4CPLUS_ERROR(logger, "ReqWxPic failed , response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return -1;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "ReqWxPic failed, response.result="<<stHead.result());
			return -1;	
		}

		strData = stResponse.download_pic_resource_res().media_data();

		LOG4CPLUS_DEBUG(logger, "ReqWxPic succ, pic.size="<<strData.size());
		
		return 0;
	}
	
	virtual bool Process()
	{
		std::string strOpenidMd5 = GetInput().GetValue("openid_md5");
		std::string strMediaIdList = (std::string)GetInput().GetValue("media_id_list");

		LOG4CPLUS_DEBUG(logger, "strOpenidMd5="<<strOpenidMd5<<", strMediaIdList="<<strMediaIdList);
		
		uint64_t qwOpenidMD5 = strtoul(strOpenidMd5.c_str(), NULL, 10);
		if(0 == qwOpenidMD5)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid req param, qwOpenidMD5="<<qwOpenidMD5);
			return true;
		}

		std::vector<std::string> vecMediaId;
		lce::cgi::Split(strMediaIdList, "|", vecMediaId);
		if(vecMediaId.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid req param, empty vecMediaId");
			return true;
		}

		if(vecMediaId.size() > 10)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "too much mediaid, size="<<vecMediaId.size());
			return true;
		}

		std::map<std::string, std::string> mapMediaId2PidId;
		for(size_t i=0; i<vecMediaId.size(); ++i)
		{	
			const std::string& strMediaId = vecMediaId[i];
			LOG4CPLUS_DEBUG(logger, "one mediaid="<<strMediaId);
			
			//req wx pic
			std::string strPicData = "";
			if(ReqWxPic(strMediaId, strPicData) < 0)
			{
				mapMediaId2PidId[strMediaId] = "0";
				LOG4CPLUS_ERROR(logger, "ReqWxPic failed, strMediaId="<<strMediaId);		
				continue;
			}

			LOG4CPLUS_DEBUG(logger, "ReqWxPic succ, strPicData.size="<<strPicData.size());		

			//ignore first, for add more platform data
			uint64_t qwPicId = ::common::util::generate_unique_id();
			LOG4CPLUS_DEBUG(logger, "generate qwPicId="<<qwPicId);		

			
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
				mapMediaId2PidId[strMediaId] = "0";
				LOG4CPLUS_ERROR(logger, "save pic failed, mysql query error, sql="<<ossSql<<", msg="<<m_mysql.GetErrMsg());
				continue;
			}

			LOG4CPLUS_DEBUG(logger, "save db succ, table_name="<<aczTableName);		

			mapMediaId2PidId[strMediaId] = int_2_str(qwPicId);
		}

		for(std::map<std::string, std::string>::iterator iter = mapMediaId2PidId.begin();
			iter != mapMediaId2PidId.end();
			iter++)
		{
			lce::cgi::CAnyValue one;
			one["media_id"] = iter->first;
			one["pic_id"] = iter->second;
			GetAnyValue()["pic_id_list"].push_back(one);
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
	string m_pa_server_ip;
	int m_pa_server_port;

	lce::cgi::CMysql m_mysql;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiCommonWXPicUpload, logger);

int main(int argc, char** argv)
{
	CgiCommonWXPicUpload cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

