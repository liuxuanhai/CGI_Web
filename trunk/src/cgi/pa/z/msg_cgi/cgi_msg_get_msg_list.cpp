#include "cgi_msg_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"

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
		std::string strCode = (std::string)GetInput().GetValue("code");
		uint64_t qwMsgId = strtoul(((std::string)GetInput().GetValue("msg_id")).c_str(), NULL, 10);
		uint32_t iLen = (uint32_t)GetInput().GetValue("len"); 		
		std::string strOpenidTo = (std::string)GetInput().GetValue("openid_to");

		if(strOpenidTo.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID OPENID_TO");
			return true;
		}

		if(iLen > getMsgAmountLimit)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "len="<<iLen<<" > getMsgAmountLimit:"<<getMsgAmountLimit);
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
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_MSG_LIST_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetMsgListReq* pGetMsgListReq = pZMsgReq->mutable_get_msg_list_req();		
		pGetMsgListReq->set_begin_msg_id(qwMsgId);
		pGetMsgListReq->set_len(iLen);
		pGetMsgListReq->set_openid_to(strOpenidTo);

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
		if(stZMsgRes.sub_cmd() != ::hoosho::msg::z::GET_MSG_LIST_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "respons.sub_cmd="<<stZMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//4.Build strResponse Json from responseMsg protobuf	
		const ::hoosho::msg::z::GetMsgListRes& stGetMsgListRes = stZMsgRes.get_msg_list_res();
		lce::cgi::CAnyValue msg_content_list;		
		uint64_t preTime = 0;
		int count = stGetMsgListRes.msg_content_list_size()-1;
		for(int i=0; i<=count; i++)
		{
			lce::cgi::CAnyValue item;
			const ::hoosho::msg::z::MsgContent& msgContent = stGetMsgListRes.msg_content_list(i);
			item["id"] = int_2_str(msgContent.msg_id());			
			item["content"] = msgContent.content();
			item["openid_from"] = msgContent.openid_from();
			item["openid_to"] = msgContent.openid_to();
			
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

		for(int i=0; i<stZMsgRes.user_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue item, stAnyValue;
			const ::hoosho::msg::z::UserInfo& stUserInfo = stZMsgRes.user_info_list(i);
			UserInfoPB2Any(stUserInfo, stAnyValue);	
			item[stUserInfo.openid()] = stAnyValue;
			GetAnyValue()["user_info_list"].push_back(item);						
		}	
		
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
