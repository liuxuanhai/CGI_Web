#include "cgi_msg_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"

class CgiMsgNoticeRecordGet: public CgiMsgServer
{
public:
	CgiMsgNoticeRecordGet():
		CgiMsgServer(0, "config.ini", "logger.properties")
	{

	}
	
	bool InnerProcess()
	{		
		//1.Get HTTP params
		std::string strCode = (std::string)GetInput().GetValue("code");
		uint32_t iType = (uint32_t) GetInput().GetValue("type");
		uint64_t qwBeginTs = strtoul(((string) GetInput().GetValue("begin_ts")).c_str(), NULL, 10);
		uint32_t iLen = (uint32_t) GetInput().GetValue("len");

		if (iType != 1 && iType != 2)	//only support favorite and comment
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid iType="<<iType);
			return true;
		}
		
		//2.Build requestMsg protobuf
		std::string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);		
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stRequestMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_NOTICE_REQ);	
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetNoticeReq* pGetNoticeReq = pZMsgReq->mutable_get_notice_req();
		pGetNoticeReq->set_type(iType);
		pGetNoticeReq->set_begin_ts(qwBeginTs);
		pGetNoticeReq->set_len(iLen);

		//3.send to server, and recv responseMsg protobuf
		::common::protoio::ProtoIOTcpClient ioclient(ZServerIP, ZServerPort);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

		if(iRet != 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "response Msg: \n" << stResponseMsg.Utf8DebugString());
		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponseMsg.head();
		if(stHead.cmd() != ::hoosho::msg::Z_PROJECT_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
			return true;	
		}
		SERVER_NOT_OK_RETURN(stHead.result());

		const ::hoosho::msg::z::MsgRes& stZMsgRes = stResponseMsg.z_msg_res();
		if(stZMsgRes.sub_cmd() != ::hoosho::msg::z::GET_NOTICE_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "respons.sub_cmd="<<stZMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//4.Build strResponse Json from responseMsg protobuf
		const ::hoosho::msg::z::GetNoticeRes& stGetNoticeRes = stZMsgRes.get_notice_res();
		for(int i=stGetNoticeRes.notice_info_list_size()-1; i>=0; i--)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::msg::z::NoticeInfo& stNoticeInfo = stGetNoticeRes.notice_info_list(i);
			NoticeInfoPB2Any(stNoticeInfo, stAnyValue);	
			GetAnyValue()["notice_info_list"].push_back(stAnyValue);	
		}
		for(int i=stGetNoticeRes.order_follow_info_list_size()-1; i>=0; i--)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::msg::z::OrderFollowInfo& stOrderFollowInfo = stGetNoticeRes.order_follow_info_list(i);
			OrderFollowInfoPB2Any(stOrderFollowInfo, stAnyValue);			
			GetAnyValue()["order_follow_info_list"].push_back(stAnyValue);	
		}
		for(int i=0; i<stGetNoticeRes.order_info_list_size(); i++)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::msg::z::OrderInfo& stOrderInfo = stGetNoticeRes.order_info_list(i);
			OrderInfoPB2Any(stOrderInfo, stAnyValue);			
			GetAnyValue()["order_info_list"][stOrderInfo.order_id()] = stAnyValue;	
		}

		for(int i=0; i<stZMsgRes.user_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue item, stAnyValue;
			const ::hoosho::msg::z::UserInfo& stUserInfo = stZMsgRes.user_info_list(i);
			UserInfoPB2Any(stUserInfo, stAnyValue);	
			item[stUserInfo.openid()] = stAnyValue;
			GetAnyValue()["user_info_list"].push_back(item);						
		}	

		for(int i=0; i<stGetNoticeRes.user_extra_info_list_size(); i++)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::msg::z::UserExtraInfo& stUserExtraInfo = stGetNoticeRes.user_extra_info_list(i);
			UserExtraInfoPB2Any(stUserExtraInfo, stAnyValue);			
			GetAnyValue()["user_extra_info_list"][stUserExtraInfo.openid()] = stAnyValue;	
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}
	
};

int main()
{
	CgiMsgNoticeRecordGet cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}


