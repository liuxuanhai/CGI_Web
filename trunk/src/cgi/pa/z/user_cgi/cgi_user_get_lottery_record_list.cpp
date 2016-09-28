#include "cgi_server.h"
#include "../cgi_common_util.h"

class CgiUserGetLotteryRecordList: public CgiServer
{
public:
	CgiUserGetLotteryRecordList(): CgiServer(0, "config.ini", "logger.properties")
	{
	}

	bool InnerProcess()
	{
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");		
		uint32_t iLen = (int)GetInput().GetValue("len");
		uint64_t qwBeginTs = strtoul(((std::string)GetInput().GetValue("begin_ts")).c_str(), NULL, 10);
		
		if(strCode.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "code empty!");
			return true;
		}

		if(iLen > 20)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "unavailable len");
			return true;
		}
		
		//2.Build requestMsg 
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stRequestMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_LOTTERY_RECORD_LIST_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetLotteryRecordListReq* pGetLotteryRecordListReq = pZMsgReq->mutable_get_lottery_record_list_req();
		pGetLotteryRecordListReq->set_len(iLen);
		pGetLotteryRecordListReq->set_begin_ts(qwBeginTs);

		//3.Send to ZServer, and recv responseMsg
		common::protoio::ProtoIOTcpClient ioclient(ZServerIP, ZServerPort);

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
		if(stHead.cmd() != ::hoosho::msg::Z_PROJECT_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;
		}
		if(stHead.result() != ::hoosho::msg::E_OK)
		{
			if(stHead.result() == ::hoosho::msg::E_LOGIN_INVALID)
				DoReply(CGI_RET_CODE_NO_LOGIN);			
			else
				DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stHead.result());
			return true;
		}
		
		const ::hoosho::msg::z::MsgRes& stZMsgRes = stResponseMsg.z_msg_res();
		if(stZMsgRes.sub_cmd() != ::hoosho::msg::z::GET_LOTTERY_RECORD_LIST_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "sub_cmd="<<stZMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//5.feedback
		const ::hoosho::msg::z::GetLotteryRecordListRes& stGetLotteryRecordListRes = stZMsgRes.get_lottery_record_list_res();
		for(int i=0; i < stGetLotteryRecordListRes.lottery_info_list().size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::msg::z::LotteryInfo& stLotteryInfo = stGetLotteryRecordListRes.lottery_info_list(i);
			LotteryInfoPB2Any(stLotteryInfo, stAnyValue);
			GetAnyValue()["lottery_info_list"].push_back(stAnyValue);
		}
		
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
	
};

int main(int argc, char **argv)
{
	CgiUserGetLotteryRecordList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

