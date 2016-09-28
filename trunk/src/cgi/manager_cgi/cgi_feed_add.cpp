#include "manager_cgi.h"
#include <time.h>
#include <sstream>

class CgiFeedAdd: public ManagerCgi
{
public:
	CgiFeedAdd():
		ManagerCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
	{

	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = m_manager_info.m_pa_appid_md5;
		uint64_t qwOpenidMd5 = m_manager_info.m_openid_md5;
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

		if(qwFeedType == ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED)
		{

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
