#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiUserInfoGet: public CgiServer
{
public:
	CgiUserInfoGet(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		std::string strUseridList= (std::string)GetInput().GetValue("user_id_list");

		EMPTY_STR_RETURN(strCode);

		std::vector<uint64_t> vecUseridList;
		lce::util::StringOP::SplitInt(strUseridList, "|", vecUseridList);

		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::S_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::s::MsgReq* pSMsgReq = stRequestMsg.mutable_s_msg_req();
		pSMsgReq->set_sub_cmd(::hoosho::msg::s::GET_USER_INFO_REQ);
		pSMsgReq->set_code(strCode);

		::hoosho::msg::s::GetUserInfoReq* pGetUserInfoReq = pSMsgReq->mutable_get_user_info_req();
		for(size_t i=0; i<vecUseridList.size(); ++i)
		{
			pGetUserInfoReq->add_user_id_list(vecUseridList[i]);
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
		if(stSMsgRes.sub_cmd() != ::hoosho::msg::s::GET_USER_INFO_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stSMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//5.feedback		
		for(int i=0; i<stSMsgRes.user_info_list_size(); ++i)
		{
			const ::hoosho::msg::s::UserInfo& stUserInfo = stSMsgRes.user_info_list(i);
			GetAnyValue()["user_info_list"].push_back(UserInfoPB2Any(stUserInfo));						
		}				


		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main(int argc, char **argv)
{
	CgiUserInfoGet cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

