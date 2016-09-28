#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiGetUser: public CgiServer
{
public:
	CgiGetUser(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		std::string strFeedidList = (std::string)GetInput().GetValue("feed_id_list");

		EMPTY_STR_RETURN(strCode);
		EMPTY_STR_RETURN(strFeedidList);

		vector<uint64_t> vecFeedid;
		lce::cgi::SplitInt(strFeedidList, "|", vecFeedid);

		ZERO_INT_RETURN(vecFeedid.size());
		LOG4CPLUS_DEBUG(logger, "size=" << vecFeedid.size());

		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::S_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::s::MsgReq* pSMsgReq = stRequestMsg.mutable_s_msg_req();
		pSMsgReq->set_sub_cmd(::hoosho::msg::s::GET_FEED_DETAIL_REQ);
		pSMsgReq->set_code(strCode);

		::hoosho::msg::s::GetFeedDetailReq *pGetFeedDetailReq = pSMsgReq->mutable_get_feed_detail_req();
		for(size_t i = 0; i < vecFeedid.size(); i++)
		{
			pGetFeedDetailReq->add_feed_id_list(vecFeedid[i]);
		}

		//3.Send to SServer, and recv responseMsg
		common::protoio::ProtoIOTcpClient ioclient(SServerIP, SServerPort);

		std::string strErrMsg = "";
		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

		if(iRet != 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = "<<strErrMsg);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "response Msg: \n"<<stResponseMsg.Utf8DebugString());

		//4.parse responseMsg
		const ::hoosho::msg::MsgHead& stHead = stResponseMsg.head();
		if(stHead.cmd() != ::hoosho::msg::S_PROJECT_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;
		}
		SERVER_NOT_OK_RETURN(stHead.result());
		
		const ::hoosho::msg::s::MsgRes& stSMsgRes = stResponseMsg.s_msg_res();
		if(stSMsgRes.sub_cmd() != ::hoosho::msg::s::GET_FEED_DETAIL_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stSMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//5.feedback		
		set<uint64_t> setUserid;
		const ::hoosho::msg::s::GetFeedDetailRes stGetFeedDetailRes = stSMsgRes.get_feed_detail_res();
		for(int i = 0; i < stGetFeedDetailRes.feed_info_list_size(); i++)
		{
			GetAnyValue()["feed_info_list"].push_back(FeedInfoPB2Any(stGetFeedDetailRes.feed_info_list(i)));	
			if(stGetFeedDetailRes.feed_info_list(i).luck_user_id() != 0)
			{
				setUserid.insert(stGetFeedDetailRes.feed_info_list(i).luck_user_id());
			}
		}
		
		for(int i = 0; i < stGetFeedDetailRes.good_info_list_size(); i++)
		{
			GetAnyValue()["good_info_list"][int_2_str(stGetFeedDetailRes.good_info_list(i).good_id())] 
				= GoodInfoPB2Any(stGetFeedDetailRes.good_info_list(i));
		}
		
		LOG4CPLUS_DEBUG(logger, "size=" << vecFeedid.size());
		vector<hoosho::msg::s::FeedContendInfo> vecFeedContendInfo;
		if(!GetUserFeedContendIdList(ioclient, strCode, 0, vecFeedid, vecFeedContendInfo))
		{
			for(size_t i = 0; i < vecFeedContendInfo.size(); i++)
			{
				for(int j = 0; j < vecFeedContendInfo[i].contend_id_list_size(); j++)
				{
					GetAnyValue()["feed_contend_info_list"][int_2_str(vecFeedContendInfo[i].feed_id())].push_back(
							int_2_str(vecFeedContendInfo[i].contend_id_list(j)) );
				}
			}
		}
		
		vector<hoosho::msg::s::UserInfo> vecUserInfo;
		//user_info
		if(setUserid.size() && !GetUserInfoList(ioclient, strCode, setUserid, vecUserInfo))
		{
			for(size_t i = 0; i < vecUserInfo.size(); i++)
			{
				GetAnyValue()["user_info_list"][int_2_str(vecUserInfo[i].user_id())] = 
					UserInfoPB2Any(vecUserInfo[i]);
			}
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main(int argc, char **argv)
{
	CgiGetUser cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

