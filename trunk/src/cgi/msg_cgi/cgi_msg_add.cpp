#include "cgi_msg_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"

class CgiMsgAdd: public CgiMsgServer
{
public:
	CgiMsgAdd():
		CgiMsgServer(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		//1.get HTTP params
		std::string strMsgContent = (std::string)GetInput().GetValue("content");
		uint64_t openid_md5_from = strtoul(((string)GetInput().GetValue("openid_md5_from")).c_str(), NULL, 10);
		uint64_t openid_md5_to = strtoul(((string)GetInput().GetValue("openid_md5_to")).c_str(), NULL, 10);

		if(strMsgContent.empty() || isAllSpace(strMsgContent.c_str()) || strMsgContent.size()>lenLimit)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID MSG CONTENT");
			return true;	
		}

		if(0 == openid_md5_from || 0 == openid_md5_to)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID OPENID MD5");
			return true;	
		}
		
		//2.Build requestMsg protobuf
		std::string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::ADD_MSG_REQ);		
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::sixin::AddSixinReq* stMsgAddReq = stRequestMsg.mutable_msg_add_req();
		::hoosho::commstruct::MsgContent* stMsgContent = stMsgAddReq->mutable_msg_content();
		stMsgContent->set_content(strMsgContent);
		stMsgContent->set_openid_md5_from(openid_md5_from);
		stMsgContent->set_openid_md5_to(openid_md5_to);

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
		const ::hoosho::msg::MsgHead& stHead = stResponseMsg.head();
		if(stHead.cmd() != ::hoosho::msg::ADD_MSG_RES)
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

		//4.Build strResponse Json from responseMsg protobuf
		const ::hoosho::sixin::AddSixinRes& stMsgAddRes = stResponseMsg.msg_add_res();
		GetAnyValue()["msg_id"] = int_2_str(stMsgAddRes.msg_id());
		GetAnyValue()["create_ts"] = int_2_str(stMsgAddRes.create_ts());
		
		DoReply(CGI_RET_CODE_OK);	
		
		return true;		
	}
		
};

int main()
{
	CgiMsgAdd cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
