#include "cgi_msg_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"
#include "user_util.h"

class CgiMsgGetListBySession: public CgiMsgServer
{
public:
	CgiMsgGetListBySession():
		CgiMsgServer(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		//1.get HTTP params
		uint64_t msg_id = strtoul(((string)GetInput().GetValue("msg_id")).c_str(), NULL, 10);
		uint32_t amount = (uint32_t)GetInput().GetValue("amount"); 
		uint64_t openid_md5_from = strtoul(((string)GetInput().GetValue("openid_md5_from")).c_str(), NULL, 10);
		uint64_t openid_md5_to = strtoul(((string)GetInput().GetValue("openid_md5_to")).c_str(), NULL, 10);

		if(0 == openid_md5_from || 0 == openid_md5_to)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID OPENID MD5");
			return true;
		}

		if(amount > getMsgAmountLimit)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "amount="<<amount<<" > getMsgAmountLimit:"<<getMsgAmountLimit);
			return true;
		}
		
				
		//2.Build requestMsg protobuf
		std::string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::QUERY_MSG_REQ);		
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::sixin::QuerySixinReq* stMsgQueryReq = stRequestMsg.mutable_msg_query_req();
		stMsgQueryReq->set_msg_id(msg_id);
		stMsgQueryReq->set_amount(amount);
		stMsgQueryReq->set_openid_md5_from(openid_md5_from);
		stMsgQueryReq->set_openid_md5_to(openid_md5_to);

		//3.send to server, and recv responseMsg protobuf
		common::protoio::ProtoIOTcpClient ioclient(msgServerIP, msgServerPort);

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
		const ::hoosho::msg::MsgHead& stResponseMsgHead = stResponseMsg.head();
		if(stResponseMsgHead.cmd() != ::hoosho::msg::QUERY_MSG_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd="<<stResponseMsgHead.cmd()<<", unknown, fuck!!!");
			return true;	
		}

		if(stResponseMsgHead.result() != ::hoosho::msg::E_OK)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.result="<<stResponseMsgHead.result());
			return true;	
		}

		//4.Build strResponse Json from responseMsg protobuf		
		const ::hoosho::sixin::QuerySixinRes& stMsgQueryRes = stResponseMsg.msg_query_res();
		lce::cgi::CAnyValue msg_content_list;		
		uint64_t preTime = 0;
		int count = stMsgQueryRes.msg_content_size()-1;
		for(int i=0; i<=count; i++)
		{
			lce::cgi::CAnyValue item;
			const ::hoosho::commstruct::MsgContent& msgContent = stMsgQueryRes.msg_content(i);
			item["id"] = int_2_str(msgContent.id());			
			item["content"] = msgContent.content();
			item["openid_md5_from"] = int_2_str(msgContent.openid_md5_from());
			item["openid_md5_to"] = int_2_str(msgContent.openid_md5_to());
			
			if(i == 0)
			{
				item["create_ts"] = int_2_str(msgContent.create_ts());				
			}			
			else
			{				
				if(msgContent.create_ts() - preTime >= interval)
				{					
					item["create_ts"] = int_2_str(msgContent.create_ts());
				}
				else
				{
					item["create_ts"] = "0";
				}
			}
			preTime = msgContent.create_ts();

			msg_content_list.push_front(item);
		}


		//5.Get Userinfo by openid from user server		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(NULL));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stRequest.mutable_query_user_detail_info_req();
		pQueryUserDetailInfoReq->add_openid_md5_list(openid_md5_from);
		pQueryUserDetailInfoReq->add_openid_md5_list(openid_md5_to);
		
		//io
		strErrMsg = "";
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(userServerIP, userServerPort);
		iRet = stProtoIOTcpClient.io(stRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return true;	
		}

		LOG4CPLUS_DEBUG(logger, "response Msg: \n" << stResponse.Utf8DebugString());	
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

		lce::cgi::CAnyValue stHostUserInfo;
		if(stResponse.query_user_detail_info_res().user_detail_info_list_size() > 0)
		{
			const ::hoosho::commstruct::UserDetailInfo& stHostUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(0);
			UserInfoPB2Any(stHostUserDetailInfo, stHostUserInfo);
		}
		

		lce::cgi::CAnyValue stGuestUserInfo;
		if(stResponse.query_user_detail_info_res().user_detail_info_list_size() > 1)
		{
			const ::hoosho::commstruct::UserDetailInfo& stGuestUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(1);
			UserInfoPB2Any(stGuestUserDetailInfo, stGuestUserInfo);
		}		

		GetAnyValue()["userinfo_from"] = stHostUserInfo;
		GetAnyValue()["userinfo_to"] = stGuestUserInfo;
		GetAnyValue()["msg_content_list"] = msg_content_list;
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}
		
};

int main()
{
	CgiMsgGetListBySession cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
