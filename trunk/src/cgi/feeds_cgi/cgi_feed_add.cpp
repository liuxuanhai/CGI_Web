#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiFeedAdd: public CgiFeedsServer
{
public:
	CgiFeedAdd():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	int CheckUserForbid(uint64_t qwPaAppidMd5, uint64_t qwOpenidMd5)
	{
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);
		int iRet;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_USER_FORBID_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryUserForbidReq* stQueryUserForbidReq = stRequestMsg.mutable_query_user_forbid_req();
		stQueryUserForbidReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryUserForbidReq->set_openid_md5(qwOpenidMd5);

		//3.send to server, and recv responseMsg protobuf

		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);
		if(iRet != 0)
		{
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			return 1;
		}
		LOG4CPLUS_TRACE(logger, "resp = " << stResponseMsg.Utf8DebugString());
		iRet = stResponseMsg.head().result();
		if(iRet != hoosho::msg::E_OK)
		{
			LOG4CPLUS_ERROR(logger, "ret = " << iRet);
			return 1;
		}


		const hoosho::feeds::QueryUserForbidRes& stQueryUserForbidRes = stResponseMsg.query_user_forbid_res();
		const hoosho::commstruct::UserForbidInfo& stUserForbidInfo = stQueryUserForbidRes.user_forbid_info();
		
		LOG4CPLUS_TRACE(logger, "openid = " << qwOpenidMd5 << ", appid = " << qwPaAppidMd5 << ", until_ts = " << stUserForbidInfo.until_ts());

		LOG4CPLUS_TRACE(logger, "cur_ts = " << (uint64_t)(time(0)));
		if(stUserForbidInfo.until_ts() > (uint64_t)time(0))
		{
			return -1;
		}
		return 0;
	}
	
	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		uint64_t qwCoverPicId = strtoul( ((string) GetInput().GetValue("cover_pic_id")).c_str(), NULL, 10);
		string strPicIdList = (string) GetInput().GetValue("pic_id_list");
		string strContent = (string) GetInput().GetValue("content");
		uint64_t qwFeedType = strtoul( ((string) GetInput().GetValue("feed_type")).c_str(), NULL, 10);
		uint64_t qwOriginFeedId = strtoul( ((string) GetInput().GetValue("origin_feed_id")).c_str(), NULL, 10);

		uint64_t qwBannerFeedSubtype = strtoul( ((string) GetInput().GetValue("banner_feed_subtype")).c_str(), NULL, 10);
		string strUrl = (string) GetInput().GetValue("url");

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);

		if(qwCoverPicId == 0 && strContent.empty())
		{
			LOG4CPLUS_ERROR(logger, "cover_pic and content can't null at same time");
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_BASE_FEED)
		{

		}
		else if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED)
		{

		}
		else if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_FEED_OF_TOPIC)
		{
			ZERO_INT_RETURN(qwOriginFeedId);

		}
		else if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_NICE_FEED)
		{

		}
		else if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_BANNER_FEED)
		{
			if(qwBannerFeedSubtype == ::hoosho::commenum::FEEDS_BANNER_FEED_SUBTYPE_AD)
			{

			}
			else if(qwBannerFeedSubtype == ::hoosho::commenum::FEEDS_BANNER_FEED_SUBTYPE_PUSH)
			{

			}
			else
			{
				LOG4CPLUS_ERROR(logger, "unknow banner_feed_subtype = " << qwBannerFeedSubtype);
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				return true;
			}
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "unknow feed_type = " << qwFeedType);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		if(strContent.size() > m_feeds_limit_length)
		{
			LOG4CPLUS_ERROR(logger, "content length too large, = " << strContent.size());
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}
		if(qwCoverPicId == 0)
		{
            bool bAllSpace = true;
            for(size_t i = 0; i < strContent.size(); i++)
            {
                if(!isspace(strContent[i]))
                {
                    bAllSpace = false;
                    break;
                }
            }
            if(bAllSpace)
            {
                LOG4CPLUS_ERROR(logger, "content only contains space, = " << strContent.size());
                DoReply(CGI_RET_CODE_INVALID_PARAM);
                return true;
            }
        }

		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);
		int iRet;

		
		//check user forbid or not
		iRet = CheckUserForbid(qwPaAppidMd5, qwOpenidMd5);
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "openid = " << qwOpenidMd5 << " was forbid in appid = " << qwPaAppidMd5);
			DoReply(CGI_RET_CODE_USER_FORBID);
			return true;
		}
		else if(iRet > 0)
		{
			LOG4CPLUS_ERROR(logger, "ret = " << iRet);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		
		
		//check user has right to add topic_feed / nice_feed / banner_feed
		if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED
			|| qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_NICE_FEED
			|| qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_BANNER_FEED)
		{

			::hoosho::msg::Msg stUserServerRequestMsg;
			::hoosho::msg::Msg stUserServerResponseMsg;
			::common::protoio::ProtoIOTcpClient ioclientUserServer(m_user_server_ip, m_user_server_port);

			::hoosho::msg::MsgHead* stUserServerRequestMsgHead = stUserServerRequestMsg.mutable_head();
			stUserServerRequestMsgHead->set_cmd(hoosho::msg::QUERY_USER_POWER_REQ);
			stUserServerRequestMsgHead->set_seq(time(NULL));

			::hoosho::user::QueryUserPowerReq* stQueryUserPowerReq = stUserServerRequestMsg.mutable_query_user_power_req();
			stQueryUserPowerReq->set_appid_md5(qwPaAppidMd5);
			stQueryUserPowerReq->set_openid_md5(qwOpenidMd5);

			iRet = ioclientUserServer.io(stUserServerRequestMsg, stUserServerResponseMsg, strErrMsg);
			if(iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			LOG4CPLUS_TRACE(logger, "resp = " << stUserServerResponseMsg.Utf8DebugString());
			iRet = stUserServerResponseMsg.head().result();
			SERVER_NOT_OK_RETURN(iRet);

			const hoosho::user::QueryUserPowerRes& stQueryUserPowerRes = stUserServerResponseMsg.query_user_power_res();


			bool rightToAddTopicFeed = false;
			bool rightToAddNiceFeed = false;
			bool rightToAddBannerFeed = false;
			for(int i = 0; i < stQueryUserPowerRes.power_list_size(); i++)
			{
				if(stQueryUserPowerRes.power_list(i) == ::hoosho::commenum::USER_POWER_TYPE_CREATE_TOPIC)
				{
					rightToAddTopicFeed = true;
				}
				else if(stQueryUserPowerRes.power_list(i) == ::hoosho::commenum::USER_POWER_TYPE_CREATE_VIP_FEED)
				{
					rightToAddNiceFeed = true;
				}
				else if(stQueryUserPowerRes.power_list(i) == ::hoosho::commenum::USER_POWER_TYPE_BANNER_MANAGE)
				{
					rightToAddBannerFeed = true;
				}
			}
			if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED && !rightToAddTopicFeed)
			{
				LOG4CPLUS_ERROR(logger, "openid_md5 = " << qwOpenidMd5 << " has no right to add topic_feed in appid = " << qwPaAppidMd5);
				DoReply(CGI_RET_CODE_INVALID_OP);
				return true;
			}
			if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_NICE_FEED && !rightToAddNiceFeed)
			{
				LOG4CPLUS_ERROR(logger, "openid_md5 = " << qwOpenidMd5 << " has no right to add nice_feed in appid = " << qwPaAppidMd5);
				DoReply(CGI_RET_CODE_INVALID_OP);
				return true;
			}
			if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_BANNER_FEED && !rightToAddBannerFeed)
			{
				LOG4CPLUS_ERROR(logger, "openid_md5 = " << qwOpenidMd5 << " has no right to add banner_feed in appid = " << qwPaAppidMd5);
				DoReply(CGI_RET_CODE_INVALID_OP);
				return true;
			}
		}

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::ADD_FEED_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::AddFeedReq* stAddFeedReq = stRequestMsg.mutable_add_feed_req();
		stAddFeedReq->set_pa_appid_md5(qwPaAppidMd5);
		stAddFeedReq->set_openid_md5(qwOpenidMd5);
		stAddFeedReq->set_cover_pic_id(qwCoverPicId);
		stAddFeedReq->set_pic_id_list(strPicIdList);
		stAddFeedReq->set_content(strContent);
		stAddFeedReq->set_feed_type(qwFeedType);
		stAddFeedReq->set_origin_feed_id(qwOriginFeedId);

		stAddFeedReq->set_banner_feed_subtype(qwBannerFeedSubtype);
		stAddFeedReq->set_url(strUrl);

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


		const hoosho::feeds::AddFeedRes& stAddFeedRes = stResponseMsg.add_feed_res();

		LOG4CPLUS_TRACE(logger, "feed_id = " << stAddFeedRes.feed_id());


		//4.Build strResponse Json from responseMsg protobuf
		GetAnyValue()["feed_id"] = int_2_str(stAddFeedRes.feed_id());
		DoReply(CGI_RET_CODE_OK);

		return true;
	}

};

int main()
{
	CgiFeedAdd cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
