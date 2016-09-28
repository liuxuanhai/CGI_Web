#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiCheckUserCollect: public CgiServer
{
public:
	CgiCheckUserCollect(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		std::string strFeedidList = (std::string) GetInput().GetValue("feed_id_list");

		EMPTY_STR_RETURN(strCode);
		EMPTY_STR_RETURN(strFeedidList);

		vector<uint64_t> vecFeedid;
		lce::cgi::SplitInt(strFeedidList, "|", vecFeedid);

		ZERO_INT_RETURN(vecFeedid.size());


		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::S_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::s::MsgReq* pSMsgReq = stRequestMsg.mutable_s_msg_req();
		pSMsgReq->set_sub_cmd(::hoosho::msg::s::CHECK_USER_COLLECT_REQ);
		pSMsgReq->set_code(strCode);

		::hoosho::msg::s::CheckUserCollectReq *pCheckUserCollectReq = pSMsgReq->mutable_check_user_collect_req();
		for(size_t i = 0; i < vecFeedid.size(); i++)
		{
			pCheckUserCollectReq->add_feed_id_list(vecFeedid[i]);
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
		if(stSMsgRes.sub_cmd() != ::hoosho::msg::s::CHECK_USER_COLLECT_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stSMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//5.feedback		
		const ::hoosho::msg::s::CheckUserCollectRes stCheckUserCollectRes = stSMsgRes.check_user_collect_res();
		for(int i = 0; i< stCheckUserCollectRes.user_collect_flag_list_size(); i++)
		{
			GetAnyValue()[int_2_str(stCheckUserCollectRes.user_collect_flag_list(i).feed_id())] = 
					stCheckUserCollectRes.user_collect_flag_list(i).flag();
		}


		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main(int argc, char **argv)
{
	CgiCheckUserCollect cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

