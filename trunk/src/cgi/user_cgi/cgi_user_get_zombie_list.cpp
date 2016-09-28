#ifndef _HOOSHO_CGI_USER_GET_ZOMBIE_LIST_H_
#define _HOOSHO_CGI_USER_GET_ZOMBIE_LIST_H_

#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"
#include <sstream>
#include <stdio.h>


class CgiUserGetZombieList: public lce::cgi::Cgi
{
public:
	 CgiUserGetZombieList(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{		
		GetConfig().GetValue("user_server", "ip", m_user_server_ip, "");
		GetConfig().GetValue("user_server", "port", m_user_server_port, 0);
		return true;
	}
	int FetchUserInfo(set<uint64_t>& setOpenidMd5)
	{
		::hoosho::msg::Msg stUserInfoRequest;
		::hoosho::msg::MsgHead* pHead = stUserInfoRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stUserInfoRequest.mutable_query_user_detail_info_req();
		for (std::set<uint64_t>::iterator iter = setOpenidMd5.begin(); iter != setOpenidMd5.end(); ++iter)
		{
			pQueryUserDetailInfoReq->add_openid_md5_list(*iter);
		}

		//io
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);

		std::string strErrMsg = "";
		int iRet = stProtoIOTcpClient.io(stUserInfoRequest, stResponse, strErrMsg);
		if (iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "FetchUserInfo IO failed, strErrMsg="<<strErrMsg);
			return -1;
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if (stHead.cmd() != ::hoosho::msg::QUERY_USER_DETAIL_INFO_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "QUERY_USER_DETAIL_INFO_RES.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return -1;
		}

		if (stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "QUERY_USER_DETAIL_INFO_RES.result != E_OK, "<<stHead.result());
			return -1;
		}

		LOG4CPLUS_DEBUG(logger, "QUERY_USER_DETAIL_INFO_RES Msg: \n" << stResponse.Utf8DebugString());

		setOpenidMd5.clear();
		for (int i = 0; i < stResponse.query_user_detail_info_res().user_detail_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
			UserInfoPB2Any(stUserDetailInfo, stAnyValue);
			GetAnyValue()["user_info_list"][int_2_str(stUserDetailInfo.openid_md5())] = stAnyValue;

			setOpenidMd5.insert(stUserDetailInfo.openid_md5());
		}

		return 0;
	}
	
	virtual bool Process()
	{
		std::string strUin = (std::string)GetInput().GetValue("uin");
		if(strUin.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "strUin is empty");
			return true;	
		}

		LOG4CPLUS_DEBUG(logger, "strUin="<<strUin);

		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_ZOMBIE_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserZombieReq* pQueryUserZombieReq = stRequest.mutable_query_user_zombie_req();
		pQueryUserZombieReq->set_uin(strUin);

		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return true;	
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_ZOMBIE_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;	
		}

		set<uint64_t> setOpenidMd5;
		for(int i=0; i<stResponse.query_user_zombie_res().openid_md5_list_size(); i++)
		{
			uint64_t openid_md5 =  stResponse.query_user_zombie_res().openid_md5_list(i);
			setOpenidMd5.insert(openid_md5);
		}

		//Get UserInfo
		if(FetchUserInfo(setOpenidMd5) < 0)
			return true;

		
		lce::cgi::CAnyValue stAnyValue;
		for(int i=0; i<stResponse.query_user_zombie_res().openid_md5_list_size(); i++)
		{
			uint64_t openid_md5 =  stResponse.query_user_zombie_res().openid_md5_list(i);
			if(setOpenidMd5.find(openid_md5) == setOpenidMd5.end())
				continue;
				
			stAnyValue.push_back(int_2_str(openid_md5));
		}
			
		GetAnyValue()["openid_md5_list"] = stAnyValue;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	string m_user_server_ip;
	int m_user_server_port;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUserGetZombieList, logger);

int main(int argc, char** argv)
{
	CgiUserGetZombieList cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

