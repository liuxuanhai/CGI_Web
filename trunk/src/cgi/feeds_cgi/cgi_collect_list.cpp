#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiCollectList: public CgiFeedsServer
{
public:
	CgiCollectList():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	int FetchFeedInfo(const set<uint64_t>& setFeedId, vector<hoosho::commstruct::FeedInfo>& vecFeedInfo,
		const uint64_t& qwPaAppidMd5, const uint64_t& qwOpenidMd5)
	{

		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_DETAIL_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFeedDetailReq* stQueryFeedDetailReq = stRequestMsg.mutable_query_feed_detail_req();
		stQueryFeedDetailReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFeedDetailReq->set_openid_md5(qwOpenidMd5);


		for(set<uint64_t>::iterator iter = setFeedId.begin(); iter != setFeedId.end(); iter++)
		{
			stQueryFeedDetailReq->add_feed_id_list( *iter );
		}


		//3.send to server, and recv responseMsg protobuf
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
		if(iRet != 0)
		{
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
//			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return CGI_RET_CODE_SERVER_BUSY;
		}
		LOG4CPLUS_TRACE(logger, "resp = " << stResponseMsg.Utf8DebugString());
		iRet = stResponseMsg.head().result();

//		SERVER_NOT_OK_RETURN(iRet);
		if(iRet != hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "response.result="<<stResponseMsg.head().result());
			return CGI_RET_CODE_SERVER_BUSY;
		}

		const hoosho::feeds::QueryFeedDetailRes& stQueryFeedDetailRes = stResponseMsg.query_feed_detail_res();

		vecFeedInfo.clear();

		for(int i = 0; i < stQueryFeedDetailRes.feed_list_size(); i++)
		{
			vecFeedInfo.push_back( stQueryFeedDetailRes.feed_list(i) );
		}

		return CGI_RET_CODE_OK;
	}

	int FetchUserInfo(const set<uint64_t>& setOpenidMd5, vector<hoosho::commstruct::UserDetailInfo>& vecUserInfo)
	{
		int iRet;
		string strErrMsg;
		::hoosho::msg::Msg stUserInfoRequest;
		::hoosho::msg::MsgHead* pHead = stUserInfoRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stUserInfoRequest.mutable_query_user_detail_info_req();
		for(std::set<uint64_t>::iterator iter=setOpenidMd5.begin(); iter!=setOpenidMd5.end(); ++iter)
		{
			pQueryUserDetailInfoReq->add_openid_md5_list(*iter);
		}

		//io
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		iRet = stProtoIOTcpClient.io(stUserInfoRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
//			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return CGI_RET_CODE_SERVER_BUSY;
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_DETAIL_INFO_RES)
		{
//			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return CGI_RET_CODE_SERVER_BUSY;
		}

		if(stHead.result() != ::hoosho::msg::E_OK)
		{
//			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return CGI_RET_CODE_SERVER_BUSY;
		}

		LOG4CPLUS_DEBUG(logger, "size = " << stResponse.query_user_detail_info_res().user_detail_info_list_size());
		for(int i=0; i<stResponse.query_user_detail_info_res().user_detail_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
			LOG4CPLUS_DEBUG(logger, "one detailuserinfo="<<stUserDetailInfo.Utf8DebugString());

			vecUserInfo.push_back( stUserDetailInfo );
		}

		return CGI_RET_CODE_OK;
	}

	int FecthFavoriteCheck(const set<uint64_t>& setFeedId, vector<uint64_t>& vecFavoriteList
		, const uint64_t& qwPaAppidMd5, const uint64_t& qwOpenidMd5)
	{

		::hoosho::msg::Msg stFeedListFavoriteRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stFeedListFavoriteRequestMsgHead = stFeedListFavoriteRequestMsg.mutable_head();
		stFeedListFavoriteRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_LIST_FAVORITE_REQ);
		stFeedListFavoriteRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFeedListFavoriteReq* stQueryFeedListFavoriteReq = stFeedListFavoriteRequestMsg.mutable_query_feed_list_favorite_req();
		stQueryFeedListFavoriteReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFeedListFavoriteReq->set_openid_md5(qwOpenidMd5);

		for(set<uint64_t>::iterator iter = setFeedId.begin(); iter != setFeedId.end(); iter++)
		{
			stQueryFeedListFavoriteReq->add_feed_id_list( *iter );
		}

		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);
		string strErrMsg;
		int iRet;
		iRet = ioclient.io(stFeedListFavoriteRequestMsg, stResponseMsg, strErrMsg);
		vecFavoriteList.clear();
		if(iRet != 0 || stResponseMsg.head().result() != hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "iRet = " << iRet << ", result = " << stResponseMsg.head().result() );
			return -1;
		}
		else
		{
			::hoosho::feeds::QueryFeedListFavoriteRes stQueryFeedListFavoriteRes = stResponseMsg.query_feed_list_favorite_res();

			LOG4CPLUS_DEBUG(logger, "size = " << stQueryFeedListFavoriteRes.favorite_list_size());
			for(int i = 0; i < stQueryFeedListFavoriteRes.favorite_list_size(); i++)
			{
				vecFavoriteList.push_back( stQueryFeedListFavoriteRes.favorite_list(i) );
			}
		}



		return CGI_RET_CODE_OK;
	}


	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		uint64_t qwBeginCreateTs = strtoul( ((string) GetInput().GetValue("begin_create_ts")).c_str(), NULL, 10);
		uint64_t qwLimit = strtoul( ((string) GetInput().GetValue("limit")).c_str(), NULL, 10);

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		ZERO_INT_RETURN(qwLimit);


		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_COLLECT_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryCollectReq* stQueryCollectReq = stRequestMsg.mutable_query_collect_req();
		stQueryCollectReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryCollectReq->set_openid_md5(qwOpenidMd5);
		stQueryCollectReq->set_begin_create_ts(qwBeginCreateTs);
		stQueryCollectReq->set_limit(qwLimit);

		//3.send to server, and recv responseMsg protobuf
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
		if(iRet != 0)
		{
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		LOG4CPLUS_TRACE(logger, "resp = " << stResponseMsg.Utf8DebugString());
		iRet = stResponseMsg.head().result();

		SERVER_NOT_OK_RETURN(iRet);

		const hoosho::feeds::QueryCollectRes& stQueryCollectRes = stResponseMsg.query_collect_res();

		vector<uint64_t> vecFeedIdList;
		vector<uint64_t> vecCreateTsList;
        map<uint64_t, bool> mapFeedValid;
		//-0- init openid_md5 set
		set<uint64_t> setOpenidMd5;
		set<uint64_t> setFeedId;

		for(int i = 0; i < stQueryCollectRes.feed_id_list_size(); i++)
		{
			vecFeedIdList.push_back( stQueryCollectRes.feed_id_list(i) );
			vecCreateTsList.push_back( stQueryCollectRes.create_ts_list(i) );
            mapFeedValid[ vecFeedIdList[i] ] = 0;
			setFeedId.insert( stQueryCollectRes.feed_id_list(i) );
		}



		//4.Build strResponse Json from responseMsg protobuf


		vector<hoosho::commstruct::FeedInfo> vecFeedInfo;
		vector<hoosho::commstruct::UserDetailInfo> vecUserInfo;
		vector<uint64_t> vecFavoriteList;

		//-1- get feed_info
		iRet = FetchFeedInfo(setFeedId, vecFeedInfo, qwPaAppidMd5, qwOpenidMd5);
		if(iRet != CGI_RET_CODE_OK)
		{
			DoReply(iRet);
			return true;
		}
		for(size_t i = 0; i < vecFeedInfo.size(); i++)
		{
		    mapFeedValid[ vecFeedInfo[i].feed_id() ] = 1;
			GetAnyValue()["feed_info"].push_back( ToAnyValue( vecFeedInfo[i] ) );
			setOpenidMd5.insert( vecFeedInfo[i].openid_md5() );
		}

        //
        for(size_t i = 0; i < vecFeedIdList.size(); i++)
        {
            if(mapFeedValid[vecFeedIdList[i]] == 0)
            {
                continue;
            }
            GetAnyValue()["feed_id_list"].push_back( int_2_str(vecFeedIdList[i]) );
            GetAnyValue()["create_ts_list"].push_back( vecCreateTsList[i] );
        }
        //

		//-2- get user_info
		iRet = FetchUserInfo(setOpenidMd5, vecUserInfo);
		if(iRet != CGI_RET_CODE_OK)
		{
			DoReply(iRet);
			return true;
		}
		for(size_t i = 0; i < vecUserInfo.size(); i++)
		{
			lce::cgi::CAnyValue any;
			UserInfoPB2Any(vecUserInfo[i], any);
			GetAnyValue()["user_info"][ int_2_str(vecUserInfo[i].openid_md5())] = any;
		}

		//-3- get favorite_check
		iRet = FecthFavoriteCheck(setFeedId, vecFavoriteList, qwPaAppidMd5, qwOpenidMd5);
		if(iRet != CGI_RET_CODE_OK)
		{
			DoReply(iRet);
			return true;
		}
		set<uint64_t>::iterator iter = setFeedId.begin();
		for(size_t i = 0; iter != setFeedId.end() && i < vecFavoriteList.size(); iter++, i++)
		{
			LOG4CPLUS_DEBUG(logger, "feedid = " << *iter << " , favor = " << vecFavoriteList[i]);
			GetAnyValue()["favorite_check"][ int_2_str(*iter) ] = vecFavoriteList[i];
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiCollectList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
