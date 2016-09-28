#include "manager_cgi.h"

class CgiManagerListGet: public ManagerCgi
{
     public:
        CgiManagerListGet() : ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

        int FetchFeedInfo(const vector<uint64_t>& vecFeedId, vector<hoosho::commstruct::FeedInfo>& vecFeedInfo
        	, const uint64_t& qwPaAppidMd5, const uint64_t& qwOpenidMd5)
        {
        	vecFeedInfo.clear();

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

			for(size_t i=0; i != vecFeedId.size(); ++i)
			{
				stQueryFeedDetailReq->add_feed_id_list( vecFeedId[i] );
			}


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

			lce::cgi::CAnyValue avFeedList;

			for(int i = 0; i < stQueryFeedDetailRes.feed_list_size(); i++)
			{
				/*
				if( stQueryFeedDetailRes.feed_list(i).del_ts() )
				{
					LOG4CPLUS_ERROR(logger, "feed_id = " << stQueryFeedDetailRes.feed_list(i).feed_id() << " has deleted");
					continue;
				}
				*/
				vecFeedInfo.push_back( stQueryFeedDetailRes.feed_list(i) );
			}

			return 0;
        }

		bool InnerProcess()
		{
			uint64_t qwOpenidMd5 = strtoul( ((string)GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
			ZERO_INT_RETURN(qwOpenidMd5);
			
		    string strErrMsg = "";
			int iRet;

			FeedReportInfoListIndexByOpenid stFeedReportInfoListIndexByOpenid(m_table_name_feed_report);
			stFeedReportInfoListIndexByOpenid.m_pa_appid_md5 = m_manager_info.m_pa_appid_md5;
			stFeedReportInfoListIndexByOpenid.m_openid_md5_to = qwOpenidMd5;
			
			iRet = stFeedReportInfoListIndexByOpenid.SelectFromDB(strErrMsg);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "stFeedReportInfoListIndexByOpenid.SelectFromDB failed, errmsg = " << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			vector<hoosho::commstruct::FeedInfo> vecFeedInfo;
			if(FetchFeedInfo(stFeedReportInfoListIndexByOpenid.m_feed_id_list, vecFeedInfo
				, m_manager_info.m_pa_appid_md5, m_manager_info.m_openid_md5))
			{
				LOG4CPLUS_ERROR(logger, "fecth user info failed");
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			for(size_t i = 0; i < vecFeedInfo.size(); i++)
			{
				GetAnyValue()["feed_list"].push_back( ToAnyValue(vecFeedInfo[i]) );
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


