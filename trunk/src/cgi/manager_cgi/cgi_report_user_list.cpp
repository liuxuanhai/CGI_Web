#include "manager_cgi.h"

class CgiFeedList: public ManagerCgi
{
public:
	CgiFeedList():
		ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
	{

	}

	int FecthUserInfo(const vector<UserForbidInfo>& vecUserForbidInfo, vector<::hoosho::commstruct::UserDetailInfo>& vecUserInfo)
	{
			//-2- get user_info		
		::hoosho::msg::Msg stUserInfoRequest;
		::hoosho::msg::MsgHead* pHead = stUserInfoRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stUserInfoRequest.mutable_query_user_detail_info_req();

		for(size_t i = 0; i < vecUserForbidInfo.size(); i++)
		{
			LOG4CPLUS_TRACE(logger, "openid_md5 = " << vecUserForbidInfo[i].m_openid_md5);
			pQueryUserDetailInfoReq->add_openid_md5_list(vecUserForbidInfo[i].m_openid_md5);
		}

		//io
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		int iRet;
		string strErrMsg;
		iRet = stProtoIOTcpClient.io(stUserInfoRequest, stResponse, strErrMsg);

		vecUserInfo.clear();
		if(iRet != 0 || stResponse.head().result() != hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "iRet = " << iRet << ", result = " << stResponse.head().result() );
			return -1;
		}
		else
		{
			LOG4CPLUS_DEBUG(logger, "userinfo size = " << stResponse.query_user_detail_info_res().user_detail_info_list_size());
			for(int i=0; i<stResponse.query_user_detail_info_res().user_detail_info_list_size(); ++i)
			{
				const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
				LOG4CPLUS_DEBUG(logger, "one detailuserinfo="<<stUserDetailInfo.Utf8DebugString());
				vecUserInfo.push_back( stUserDetailInfo );
			}
		}
		return 0;
	}

	bool InnerProcess()
	{

		UserForbidInfoList stUserForbidInfoList(m_table_name_user_forbid);
		stUserForbidInfoList.m_pa_appid_md5 = m_manager_info.m_pa_appid_md5;
		int iRet;
		string strErrMsg;
		iRet = stUserForbidInfoList.SelectFromDB(strErrMsg);
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "stUserForbidInfoList.SelectFromDB failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		vector<hoosho::commstruct::UserDetailInfo> vecUserInfo;
		if(FecthUserInfo(stUserForbidInfoList.m_user_forbid_info_list, vecUserInfo))
		{
			LOG4CPLUS_ERROR(logger, "fecth user info failed");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		size_t i, j;
		//skip invalid user_info
		for(i = 0, j = 0; i < stUserForbidInfoList.m_user_forbid_info_list.size() && j < vecUserInfo.size(); i++)
		{
			if(stUserForbidInfoList.m_user_forbid_info_list[i].m_openid_md5 != vecUserInfo[j].openid_md5())
			{
				LOG4CPLUS_ERROR(logger, "openid = " << stUserForbidInfoList.m_user_forbid_info_list[i].m_openid_md5 << " not found");
				continue;
			}
			lce::cgi::CAnyValue avUserInfo;
			UserInfoPB2Any(vecUserInfo[j], avUserInfo);
			avUserInfo["until_ts"] = stUserForbidInfoList.m_user_forbid_info_list[i].m_until_ts;
			GetAnyValue()["user_list"].push_back(avUserInfo);
			j++;
		}
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}
		
};

int main()
{
	CgiFeedList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
