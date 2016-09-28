#ifndef _HOOSHO_CGI_USER_LOGIN_H_
#define _HOOSHO_CGI_USER_LOGIN_H_

#include "cgi/cgi.h"
#include "proto_io_tcp_client.h"
#include "../cgi_ret_code_def.h"
#include "user_util.h"
#include <sstream>
#include <stdio.h>

class CgiUserInfoUpdate: public lce::cgi::Cgi
{
public:
	 CgiUserInfoUpdate(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
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

	int GetUserDetailInfo(uint64_t qwOpenidMd5, ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo)
	{
		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stRequest.mutable_query_user_detail_info_req();
		pQueryUserDetailInfoReq->add_openid_md5_list(qwOpenidMd5);
		
		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "GetUserDetailInfo failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return -1;	
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_DETAIL_INFO_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "GetUserDetailInfo failed, response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return -1;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "GetUserDetailInfo failed, response.result="<<stHead.result());
			return -1;	
		}

		if(stResponse.query_user_detail_info_res().user_detail_info_list_size() == 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "GetUserDetailInfo failed, user not exists. for qwOpenidMd5="<<qwOpenidMd5);
			return -1;	
		}

		stUserDetailInfo.CopyFrom(stResponse.query_user_detail_info_res().user_detail_info_list(0));
		return 0;
	}

	int UpdateUserDetailInfo(::hoosho::commstruct::UserDetailInfo& stUserDetailInfo)
	{
		//request		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::UPDATE_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::UpdateUserDetailInfoReq* pUpdateUserDetailInfoReq = stRequest.mutable_update_user_detail_info_req();
		pUpdateUserDetailInfoReq->mutable_user_detail_info()->CopyFrom(stUserDetailInfo);
		
		//io
		std::string strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		int iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "UpdateUserDetailInfo failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return -1;	
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::UPDATE_USER_DETAIL_INFO_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "UpdateUserDetailInfo failed, response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return -1;	
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "UpdateUserDetailInfo failed, response.result="<<stHead.result());
			return -1;	
		}

		return 0;
	}
	
	virtual bool Process()
	{
		std::string strOpenidMd5 = GetInput().GetValue("openid_md5");
		uint64_t qwOpenidMd5 = strtoul(strOpenidMd5.c_str(), NULL, 10);
		if(0 == qwOpenidMd5)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "qwOpenidMd5 is zero , not found");
			return true;	
		}

		LOG4CPLUS_DEBUG(logger, "qwOpenidMd5="<<qwOpenidMd5);

		//1. query old info first
		::hoosho::commstruct::UserDetailInfo stUserDetailInfo;
		if(GetUserDetailInfo(qwOpenidMd5, stUserDetailInfo) < 0)
		{
			return true;
		}

		//2. check updated
		bool bChanged = false;
		if(GetInput().HasParam("phone_no"))
		{
			std::string strPhoneNo = GetInput().GetValue("phone_no");
			uint64_t qwPhoneNo = strtoul(strPhoneNo.c_str(), NULL, 10);	
			if(qwPhoneNo != stUserDetailInfo.phone_no())
			{
				stUserDetailInfo.set_phone_no(qwPhoneNo);
				bChanged = true;
			}
		}

		if(GetInput().HasParam("email"))
		{
			std::string strEmail = (std::string)GetInput().GetValue("email");	
			if(strEmail != stUserDetailInfo.email())
			{
				stUserDetailInfo.set_email(strEmail);
				bChanged = true;
			}
		}

		if(GetInput().HasParam("self_desc"))
		{
			std::string strSelfDesc = (std::string)GetInput().GetValue("self_desc");	
			if(strSelfDesc != stUserDetailInfo.self_desc())
			{
				stUserDetailInfo.set_self_desc(strSelfDesc);
				bChanged = true;
			}
		}

		if(GetInput().HasParam("main_page_cover_pic_id"))
		{
			std::string strCoverPicId = GetInput().GetValue("main_page_cover_pic_id");
			uint64_t qwCoverPicId = strtoul(strCoverPicId.c_str(), NULL, 10);
			if(qwCoverPicId != stUserDetailInfo.main_page_cover_pic_id())
			{
				stUserDetailInfo.set_main_page_cover_pic_id(qwCoverPicId);
				bChanged = true;
			}
		}

		if(GetInput().HasParam("birthday_ts"))
		{
			std::string strBirthday = GetInput().GetValue("birthday_ts");
			uint64_t qwBirthday = strtoul(strBirthday.c_str(), NULL, 10);
			if(qwBirthday != stUserDetailInfo.birthday_ts())
			{
				stUserDetailInfo.set_birthday_ts(qwBirthday);
				bChanged = true;
			}
		}

		//3. update user info
		if(bChanged)
		{
			if(UpdateUserDetailInfo(stUserDetailInfo) < 0)
			{
				return true;
			}
		}		

		lce::cgi::CAnyValue stAnyValue;
		UserInfoPB2Any(stUserDetailInfo, stAnyValue);

		GetAnyValue()["user_info"] = stAnyValue;	
		DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	string m_user_server_ip;
	int m_user_server_port;
protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiUserInfoUpdate, logger);

int main(int argc, char** argv)
{
	CgiUserInfoUpdate cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

