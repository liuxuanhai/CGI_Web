#include "cgi_msg_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"

class CgiMsgDelById: public CgiMsgServer
{
public:
	CgiMsgDelById():
		CgiMsgServer(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		//1.get HTTP params
		uint64_t msg_id = strtoul(((string)GetInput().GetValue("msg_id")).c_str(), NULL, 10);
		uint64_t openid_md5_from = strtoul(((string)GetInput().GetValue("openid_md5_from")).c_str(), NULL, 10);
		uint64_t openid_md5_to = strtoul(((string)GetInput().GetValue("openid_md5_to")).c_str(), NULL, 10);

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
		stRequestMsgHead->set_cmd(::hoosho::msg::DELETE_MSG_REQ);		
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::sixin::DeleteSixinReq* stMsgDelReq = stRequestMsg.mutable_msg_del_req();
		stMsgDelReq->set_msg_id(msg_id);
		stMsgDelReq->set_openid_md5_from(openid_md5_from);
		stMsgDelReq->set_openid_md5_to(openid_md5_to);

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
		if(stHead.cmd() != ::hoosho::msg::DELETE_MSG_RES)
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
		
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}
		
};

int main()
{
	CgiMsgDelById cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
