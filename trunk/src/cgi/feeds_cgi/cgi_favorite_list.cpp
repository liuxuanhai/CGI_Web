#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiFavoriteList: public CgiFeedsServer
{
public:
	CgiFavoriteList():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		uint64_t qwFeedId = strtoul( ((string) GetInput().GetValue("feed_id")).c_str(), NULL, 10);
		uint64_t qwBeginCreateTs = strtoul( ((string) GetInput().GetValue("begin_create_ts")).c_str(), NULL, 10);
		uint64_t qwLimit = strtoul( ((string) GetInput().GetValue("limit")).c_str(), NULL, 10);

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		ZERO_INT_RETURN(qwFeedId);
		ZERO_INT_RETURN(qwLimit);


		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_FAVORITE_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFavoriteReq* stQueryFavoriteReq = stRequestMsg.mutable_query_favorite_req();
		stQueryFavoriteReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFavoriteReq->set_openid_md5(qwOpenidMd5);
		stQueryFavoriteReq->set_feed_id(qwFeedId);
		stQueryFavoriteReq->set_begin_create_ts(qwBeginCreateTs);
		stQueryFavoriteReq->set_limit(qwLimit);

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

		const hoosho::feeds::QueryFavoriteRes& stQueryFavoriteRes = stResponseMsg.query_favorite_res();

		vector<uint64_t> vecOpenidMd5List;
		vector<uint64_t> vecCreateTsList;
        map<uint64_t, bool> mapOpenidValid;
		//-0- init openid_md5 set
		set<uint64_t> setOpenidMd5;

		for(int i = 0; i < stQueryFavoriteRes.openid_md5_list_size(); i++)
		{
			vecOpenidMd5List.push_back( stQueryFavoriteRes.openid_md5_list(i) );
			vecCreateTsList.push_back( stQueryFavoriteRes.create_ts_list(i) );
            mapOpenidValid[ vecOpenidMd5List[i] ] = 0;
			setOpenidMd5.insert( stQueryFavoriteRes.openid_md5_list(i) );
		}



		//4.Build strResponse Json from responseMsg protobuf



		//-1- get user_info

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
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return true;
		}

		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponse.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_USER_DETAIL_INFO_RES)
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

		LOG4CPLUS_DEBUG(logger, "size = " << stResponse.query_user_detail_info_res().user_detail_info_list_size());
		for(int i=0; i<stResponse.query_user_detail_info_res().user_detail_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
			LOG4CPLUS_DEBUG(logger, "one detailuserinfo="<<stUserDetailInfo.Utf8DebugString());
			UserInfoPB2Any(stUserDetailInfo, stAnyValue);
			GetAnyValue()["user_info"][ int_2_str(stUserDetailInfo.openid_md5()) ] = stAnyValue;
			mapOpenidValid[ stUserDetailInfo.openid_md5() ] = 1;
		}

        for(size_t i = 0; i < vecOpenidMd5List.size(); i++)
        {
            if(mapOpenidValid[ vecOpenidMd5List[i] ] == 0)
            {
                continue;
            }
            GetAnyValue()["openid_md5_list"].push_back(int_2_str( vecOpenidMd5List[i] ));
            GetAnyValue()["create_ts_list"].push_back(vecCreateTsList[i]);
        }

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiFavoriteList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
