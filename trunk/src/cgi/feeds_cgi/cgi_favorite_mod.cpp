#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiFavoriteMod: public CgiFeedsServer
{
public:
	CgiFavoriteMod():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		uint64_t qwFeedId = strtoul( ((string) GetInput().GetValue("feed_id")).c_str(), NULL, 10);
		uint32_t dwOper = (uint32_t) GetInput().GetValue("oper");
		uint64_t qwFeedOwnerOpenidMd5 = strtoul( ((string) GetInput().GetValue("feed_owner_openid_md5")).c_str(), NULL, 10);

		
		
		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		ZERO_INT_RETURN(qwFeedId);
		ZERO_INT_RETURN(dwOper);
		
		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
				
		stRequestMsgHead->set_seq(time(NULL));

		//////////
		if(dwOper == 1)
		{
			ZERO_INT_RETURN(qwFeedOwnerOpenidMd5);
			
			stRequestMsgHead->set_cmd(hoosho::msg::ADD_FAVORITE_REQ);
			::hoosho::feeds::AddFavoriteReq* stAddFavoriteReq = stRequestMsg.mutable_add_favorite_req();

			stAddFavoriteReq->set_pa_appid_md5(qwPaAppidMd5);
			stAddFavoriteReq->set_openid_md5(qwOpenidMd5);
			stAddFavoriteReq->set_feed_id(qwFeedId);
			stAddFavoriteReq->set_feed_owner_openid_md5(qwFeedOwnerOpenidMd5);
		}
		else if(dwOper == 2)
		{
			stRequestMsgHead->set_cmd(hoosho::msg::DELETE_FAVORITE_REQ);		
			::hoosho::feeds::DeleteFavoriteReq* stDeleteFavoriteReq = stRequestMsg.mutable_delete_favorite_req();

			stDeleteFavoriteReq->set_pa_appid_md5(qwPaAppidMd5);
			stDeleteFavoriteReq->set_openid_md5(qwOpenidMd5);
			stDeleteFavoriteReq->set_feed_id(qwFeedId);

		}
		else
		{
			LOG4CPLUS_ERROR(logger, "unknown oper = " << dwOper);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

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
		

		//4.Build strResponse Json from responseMsg protobuf
		DoReply(CGI_RET_CODE_OK);
		
		return true;		
	}
		
};

int main()
{
	CgiFavoriteMod cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
