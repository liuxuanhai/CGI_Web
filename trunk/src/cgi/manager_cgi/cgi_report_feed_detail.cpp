#include "manager_cgi.h"

class CgiManagerListGet: public ManagerCgi
{
     public:
        CgiManagerListGet() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

        int FetchFeedInfo(const uint64_t& qwFeedId, hoosho::commstruct::FeedInfo& stFeedInfo
        	, const uint64_t& qwPaAppidMd5, const uint64_t& qwOpenidMd5)
        {

        	//2.Build requestMsg protobuf
			string strErrMsg = "";
			::hoosho::msg::Msg stRequestMsg;
			::hoosho::msg::Msg stResponseMsg;

			::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
			stRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_DETAIL_REQ);
			stRequestMsgHead->set_seq(time(NULL));

			::hoosho::feeds::QueryFeedDetailReq* stQueryFeedDetailReq = stRequestMsg.mutable_query_feed_detail_req();
			stQueryFeedDetailReq->set_pa_appid_md5(qwPaAppidMd5);
			stQueryFeedDetailReq->set_openid_md5(qwOpenidMd5);

			
			stQueryFeedDetailReq->add_feed_id_list( qwFeedId );
			


			//3.send to server, and recv responseMsg protobuf
			::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);

			int iRet;
			iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
			if(iRet != 0)
			{
				return -1;
			}
			LOG4CPLUS_TRACE(logger, "resp = " << stResponseMsg.Utf8DebugString());
			iRet = stResponseMsg.head().result();

			if(iRet != 0)
			{
				return -1;
			}

			const hoosho::feeds::QueryFeedDetailRes& stQueryFeedDetailRes = stResponseMsg.query_feed_detail_res();

			if( stQueryFeedDetailRes.feed_list_size() == 0)
			{
				return -1;
			}
		
			stFeedInfo = stQueryFeedDetailRes.feed_list(0);

			

			return 0;
        }

        int FecthUserInfo(const set<uint64_t>& setOpenidMd5, vector<hoosho::commstruct::UserDetailInfo>& vecUserInfo)
		{
				//-2- get user_info		
			::hoosho::msg::Msg stUserInfoRequest;
			::hoosho::msg::MsgHead* pHead = stUserInfoRequest.mutable_head();
			pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
			pHead->set_seq(time(0));
			::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stUserInfoRequest.mutable_query_user_detail_info_req();

			for(set<uint64_t>::iterator iter = setOpenidMd5.begin(); iter != setOpenidMd5.end(); iter++)
			{
				LOG4CPLUS_TRACE(logger, "openid_md5 = " << *iter);
				pQueryUserDetailInfoReq->add_openid_md5_list(*iter);
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
			uint64_t qwFeedId = strtoul( ((string)GetInput().GetValue("feed_id")).c_str(), NULL, 10);
			ZERO_INT_RETURN(qwFeedId);

			hoosho::commstruct::FeedInfo stFeedInfo;
			if(FetchFeedInfo(qwFeedId, stFeedInfo, m_manager_info.m_pa_appid_md5, m_manager_info.m_openid_md5) < 0)
			{
				LOG4CPLUS_ERROR(logger, "fecthFeedInfo failed");
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			GetAnyValue()["feed_info"] = ToAnyValue(stFeedInfo);

			int iRet;
		    string strErrMsg = "";

			FeedReportInfoListIndexByFeedid stFeedReportInfoListIndexByFeedid(m_table_name_feed_report);
			stFeedReportInfoListIndexByFeedid.m_pa_appid_md5 = m_manager_info.m_pa_appid_md5;
			
			stFeedReportInfoListIndexByFeedid.m_feed_id = qwFeedId;
			iRet = stFeedReportInfoListIndexByFeedid.SelectFromDB(strErrMsg);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "stFeedReportInfoListIndexByFeedid.SelectFromDB failed, errmsg = " << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			set<uint64_t> setOpenidMd5;

			const vector<FeedReportInfo>& vecFeedReport = stFeedReportInfoListIndexByFeedid.m_feed_report_info_list;
			for(size_t i = 0; i < vecFeedReport.size(); i++)
			{
				GetAnyValue()["feed_report"].push_back(vecFeedReport[i].ToAnyValue() );
				setOpenidMd5.insert( vecFeedReport[i].m_openid_md5_from );
			}
			vector<hoosho::commstruct::UserDetailInfo> vecUserInfo;
			if(FecthUserInfo(setOpenidMd5, vecUserInfo) < 0)
			{
				LOG4CPLUS_ERROR(logger, "FecthUserInfo failed");
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			for(size_t i = 0; i < vecUserInfo.size(); i++)
			{
				lce::cgi::CAnyValue avUserInfo;
				UserInfoPB2Any(vecUserInfo[i], avUserInfo);
				GetAnyValue()["user_info"].push_back( avUserInfo );
			}
            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiManagerListGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


