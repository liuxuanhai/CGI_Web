#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiFeedList: public CgiFeedsServer
{
public:
	CgiFeedList():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	int FetchNumFeed(uint64_t qwPaAppidMd5, uint64_t& qwNumTopicFeed, uint64_t& qwNumBannerFeed)
	{
		std::ostringstream ossSql;
		ossSql.str("");
		ossSql << "SELECT num_topic_feed, num_banner_feed FROM " << m_table_name_pa_info << " WHERE appid_md5 = " << qwPaAppidMd5;
		if(!m_pa_mysql.Query(ossSql.str()))
		{
			LOG4CPLUS_ERROR(logger, "mysql query error, sql = "<< ossSql <<", msg = " << m_pa_mysql.GetErrMsg());
			return CGI_RET_CODE_SERVER_BUSY;
		}
		if(m_pa_mysql.GetRowCount() && m_pa_mysql.Next())
		{
			qwNumTopicFeed = strtoul( m_pa_mysql.GetRow(0), NULL, 10);
			qwNumBannerFeed = strtoul( m_pa_mysql.GetRow(1), NULL, 10);

			LOG4CPLUS_DEBUG(logger, "num_topic_feed = "<< qwNumTopicFeed <<", num_banner_feed = "<<qwNumBannerFeed);
			return true;
		}
		return CGI_RET_CODE_OK;
	}

	int FetchCollectCheck(vector<uint64_t>& qwVecFeedId, vector<uint64_t>& vecCollect,
		uint64_t qwPaAppidMd5, uint64_t qwOpenidMd5)
	{
		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_LIST_COLLECT_REQ);		
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFeedListCollectReq* stQueryFeedListCollectReq = stRequestMsg.mutable_query_feed_list_collect_req();
		stQueryFeedListCollectReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFeedListCollectReq->set_openid_md5(qwOpenidMd5);


		for(size_t i=0; i != qwVecFeedId.size(); ++i)
		{
			stQueryFeedListCollectReq->add_feed_id_list( qwVecFeedId[i] );
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
		SERVER_NOT_OK_RETURN(iRet);

		const hoosho::feeds::QueryFeedListCollectRes& stQueryFeedListCollectRes = stResponseMsg.query_feed_list_collect_res();

		vecCollect.clear();
		for(int i = 0; i < stQueryFeedListCollectRes.collect_list_size(); i++)
		{
			vecCollect.push_back( stQueryFeedListCollectRes.collect_list(i) );
		}
		
		return CGI_RET_CODE_OK;
	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		uint64_t qwBeginFeedId = strtoul( ((string) GetInput().GetValue("begin_feed_id")).c_str(), NULL, 10);
		uint64_t qwLimit = strtoul( ((string) GetInput().GetValue("limit")).c_str(), NULL, 10);
		uint64_t qwType = strtoul( ((string) GetInput().GetValue("type")).c_str(), NULL, 10);;
		uint64_t qwFeedType = strtoul( ((string) GetInput().GetValue("feed_type")).c_str(), NULL, 10);
		uint64_t qwOriginFeedId = strtoul( ((string) GetInput().GetValue("origin_feed_id")).c_str(), NULL, 10);
		uint64_t qwOrderType = strtoul( ((string) GetInput().GetValue("order_type")).c_str(), NULL, 10);
		uint64_t qwOffset = strtoul( ((string)GetInput().GetValue("offset")).c_str(), NULL, 10);
		
		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		ZERO_INT_RETURN(qwLimit);

		if(qwOrderType == 0)
		{
			qwOrderType = ::hoosho::commenum::FEEDS_FEED_LIST_ORDER_BY_TIME;
		}

		if(qwType <= ::hoosho::commenum::FEEDS_FEED_LIST_MIN ||
			qwType >= ::hoosho::commenum::FEEDS_FEED_LIST_MAX)
		{
			LOG4CPLUS_ERROR(logger, "unknow type = " << qwType);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		
		if( (qwFeedType & ::hoosho::commenum::FEEDS_FEED_TYPE_BASE_FEED) == 0 
			&& (qwFeedType & ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED) == 0 
			&& (qwFeedType & ::hoosho::commenum::FEEDS_FEED_TYPE_FEED_OF_TOPIC) == 0 
			&& (qwFeedType & ::hoosho::commenum::FEEDS_FEED_TYPE_NICE_FEED) == 0 
			&& (qwFeedType & ::hoosho::commenum::FEEDS_FEED_TYPE_BANNER_FEED) == 0
			)
		{
			LOG4CPLUS_ERROR(logger, "unknow feed_type = " << qwFeedType);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_FEED_OF_TOPIC)
		{
			ZERO_INT_RETURN(qwOriginFeedId);
		}
			
		//2.Build requestMsg protobuf
		string strErrMsg = "";
		lce::cgi::CAnyValue avFeedList;
		vector<uint64_t> qwVecFeedId;
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_REQ);		
		stRequestMsgHead->set_seq(time(NULL));
		
		::hoosho::feeds::QueryFeedReq* stQueryFeedReq = stRequestMsg.mutable_query_feed_req();

		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);
		int iRet;

		uint64_t qwNumBannerFeed = m_default_show_banner_feed_num;
		uint64_t qwNumTopicFeed = m_default_show_topic_feed_num;

		//拉取"精选-B2C"顶部feed列表[精华feed]）时，会将最近的X条banner feed，一并返回。
		if(qwFeedType == (::hoosho::commenum::FEEDS_FEED_TYPE_NICE_FEED)
			&& qwType == ::hoosho::commenum::FEEDS_FEED_LIST_BY_APPID
			&& qwBeginFeedId == 0)
		{
			FetchNumFeed(qwPaAppidMd5, qwNumTopicFeed, qwNumBannerFeed);
			stQueryFeedReq->set_pa_appid_md5(qwPaAppidMd5);
			stQueryFeedReq->set_openid_md5(qwOpenidMd5);
			stQueryFeedReq->set_begin_feed_id( 0 );
			stQueryFeedReq->set_limit( qwNumBannerFeed );
			stQueryFeedReq->set_type(qwType);
			stQueryFeedReq->set_feed_type( ::hoosho::commenum::FEEDS_FEED_TYPE_BANNER_FEED);
			stQueryFeedReq->set_order_type( ::hoosho::commenum::FEEDS_FEED_LIST_ORDER_BY_TIME );
			stQueryFeedReq->set_offset(0);
			
			iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
			if(iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_TRACE(logger, "resp = " << stResponseMsg.Utf8DebugString());
			iRet = stResponseMsg.head().result();

			if(iRet != hoosho::msg::E_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "server return error, ret = " << iRet);
				return true;
			}

			const hoosho::feeds::QueryFeedRes& stQueryFeedRes = stResponseMsg.query_feed_res();
			
			
			for(int i = 0; i < stQueryFeedRes.feed_list_size(); i++)
			{
				qwVecFeedId.push_back( stQueryFeedRes.feed_list(i).feed_id() );
				avFeedList.push_back(ToAnyValue(stQueryFeedRes.feed_list(i)));
			}
			
		}

		//拉取"精选-B2C"顶部feed列表[精华feed]）时，会将最近的Y条topic feed，一并返回。
		if(qwFeedType == (::hoosho::commenum::FEEDS_FEED_TYPE_NICE_FEED)
			&& qwType == ::hoosho::commenum::FEEDS_FEED_LIST_BY_APPID
			&& qwBeginFeedId == 0)
		{
			stQueryFeedReq->set_pa_appid_md5(qwPaAppidMd5);
			stQueryFeedReq->set_openid_md5(qwOpenidMd5);
			stQueryFeedReq->set_begin_feed_id( 0 );
			stQueryFeedReq->set_limit( qwNumTopicFeed );
			stQueryFeedReq->set_type(qwType);
			stQueryFeedReq->set_feed_type( ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED);
			stQueryFeedReq->set_order_type( ::hoosho::commenum::FEEDS_FEED_LIST_ORDER_BY_TIME );
			stQueryFeedReq->set_offset(0);
			
			iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
			if(iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_TRACE(logger, "resp = " << stResponseMsg.Utf8DebugString());
			iRet = stResponseMsg.head().result();

			if(iRet != hoosho::msg::E_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "server return error, ret = " << iRet);
				return true;
			}

			const hoosho::feeds::QueryFeedRes& stQueryFeedRes = stResponseMsg.query_feed_res();
			
			
			for(int i = 0; i < stQueryFeedRes.feed_list_size(); i++)
			{
				qwVecFeedId.push_back( stQueryFeedRes.feed_list(i).feed_id() );
				avFeedList.push_back(ToAnyValue(stQueryFeedRes.feed_list(i)));
			}
			
		}
		
		stQueryFeedReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFeedReq->set_openid_md5(qwOpenidMd5);
		stQueryFeedReq->set_begin_feed_id(qwBeginFeedId);
		stQueryFeedReq->set_limit(qwLimit);
		stQueryFeedReq->set_type(qwType);
		stQueryFeedReq->set_feed_type(qwFeedType);
		stQueryFeedReq->set_origin_feed_id(qwOriginFeedId);
		stQueryFeedReq->set_order_type(qwOrderType);
		stQueryFeedReq->set_offset(qwOffset);
			
		//3.send to server, and recv responseMsg protobuf
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


		const hoosho::feeds::QueryFeedRes& stQueryFeedRes = stResponseMsg.query_feed_res();

//		lce::cgi::CAnyValue avFeedList;
//		vector<uint64_t> qwVecFeedId;

		LOG4CPLUS_TRACE(logger, "feed_list_size = " << stQueryFeedRes.feed_list_size());
		for(int i = 0; i < stQueryFeedRes.feed_list_size(); i++)
		{
			qwVecFeedId.push_back( stQueryFeedRes.feed_list(i).feed_id() );
			avFeedList.push_back(ToAnyValue(stQueryFeedRes.feed_list(i)));
		}


		//4.Build strResponse Json from responseMsg protobuf
		GetAnyValue()["feed_list"] = avFeedList;

		/****************************/
		//获取参与话题讨论数
		GetAnyValue()["total"] = stQueryFeedRes.total();


		
		//-0- init openid_md5 set
		std::set<uint64_t> setOpenidMd5;
		LOG4CPLUS_TRACE(logger, "feed_list_size = " << stQueryFeedRes.feed_list_size());
		for(int i = 0; i < stQueryFeedRes.feed_list_size(); i++)
		{
			setOpenidMd5.insert( stQueryFeedRes.feed_list(i).openid_md5() );
		}
		LOG4CPLUS_TRACE(logger, "set_size = " << setOpenidMd5.size());
		
		//-1- get favor_check
		::hoosho::msg::Msg stFeedListFavoriteRequestMsg;
		
		::hoosho::msg::MsgHead* stFeedListFavoriteRequestMsgHead = stFeedListFavoriteRequestMsg.mutable_head();
		stFeedListFavoriteRequestMsgHead->set_cmd(hoosho::msg::QUERY_FEED_LIST_FAVORITE_REQ);		
		stFeedListFavoriteRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFeedListFavoriteReq* stQueryFeedListFavoriteReq = stFeedListFavoriteRequestMsg.mutable_query_feed_list_favorite_req();
		stQueryFeedListFavoriteReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFeedListFavoriteReq->set_openid_md5(qwOpenidMd5);

		for(size_t i = 0; i < qwVecFeedId.size(); i++)
		{
			stQueryFeedListFavoriteReq->add_feed_id_list( qwVecFeedId[i] );
		}
		
		iRet = ioclient.io(stFeedListFavoriteRequestMsg, stResponseMsg, strErrMsg);
		if(iRet != 0 || stResponseMsg.head().result() != hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "iRet = " << iRet << ", result = " << stResponseMsg.head().result() );
		}
		else
		{
			::hoosho::feeds::QueryFeedListFavoriteRes stQueryFeedListFavoriteRes = stResponseMsg.query_feed_list_favorite_res();

			LOG4CPLUS_DEBUG(logger, "size = " << stQueryFeedListFavoriteRes.favorite_list_size());				
			for(int i = 0; i < stQueryFeedListFavoriteRes.favorite_list_size(); i++)
			{
				GetAnyValue()["favorite_check"][ qwVecFeedId[i] ] = stQueryFeedListFavoriteRes.favorite_list(i);
			}		
		}
		
	
		//-2- get user_info		
		::hoosho::msg::Msg stUserInfoRequest;
		::hoosho::msg::MsgHead* pHead = stUserInfoRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stUserInfoRequest.mutable_query_user_detail_info_req();
		for(std::set<uint64_t>::iterator iter=setOpenidMd5.begin(); iter!=setOpenidMd5.end(); ++iter)
		{
			LOG4CPLUS_TRACE(logger, "openid_md5 = " << *iter);
			pQueryUserDetailInfoReq->add_openid_md5_list(*iter);
		}

		//io
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		iRet = stProtoIOTcpClient.io(stUserInfoRequest, stResponse, strErrMsg);

		if(iRet != 0 || stResponse.head().result() != hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "iRet = " << iRet << ", result = " << stResponse.head().result() );
		}
		else
		{
			LOG4CPLUS_DEBUG(logger, "userinfo size = " << stResponse.query_user_detail_info_res().user_detail_info_list_size());
			for(int i=0; i<stResponse.query_user_detail_info_res().user_detail_info_list_size(); ++i)
			{
				lce::cgi::CAnyValue stAnyValue;
				const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
				LOG4CPLUS_DEBUG(logger, "one detailuserinfo="<<stUserDetailInfo.Utf8DebugString());
				UserInfoPB2Any(stUserDetailInfo, stAnyValue);	
				GetAnyValue()["user_info"][ int_2_str(stUserDetailInfo.openid_md5()) ] = stAnyValue;	
			}
		}
		
		//-3- get collect_check
		vector<uint64_t> vecCollect;
		iRet = FetchCollectCheck(qwVecFeedId, vecCollect, qwPaAppidMd5, qwOpenidMd5);
		if(iRet != CGI_RET_CODE_OK)
		{
			LOG4CPLUS_ERROR(logger, "iRet = " << iRet);
		}
		else
		{
			for(size_t i = 0; i < vecCollect.size(); i++)
			{
				GetAnyValue()["collect_check"][ int_2_str(qwVecFeedId[i]) ] = vecCollect[i];
			}
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
