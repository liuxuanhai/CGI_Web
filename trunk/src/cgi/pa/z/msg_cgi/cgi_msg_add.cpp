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
		std::string strCode = (std::string)GetInput().GetValue("code");
		std::string strMsgContent = (std::string)GetInput().GetValue("content");
		std::string strOpenidTo = (std::string)GetInput().GetValue("openid_to");

		if(strMsgContent.empty() || isAllSpace(strMsgContent.c_str()) || strMsgContent.size()>lenLimit)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID MSG CONTENT");
			return true;	
		}

		if(strOpenidTo.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID OPENID_TO");
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
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::ADD_NEW_MSG_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::AddNewMsgReq* pAddNewMsgReq = pZMsgReq->mutable_add_new_msg_req();		
		::hoosho::msg::z::MsgContent* pMsgContent = pAddNewMsgReq->mutable_msg_content();
		pMsgContent->set_content(strMsgContent);
		pMsgContent->set_openid_to(strOpenidTo);

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
		if(stZMsgRes.sub_cmd() != ::hoosho::msg::z::ADD_NEW_MSG_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "respons.sub_cmd="<<stZMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//4.Build strResponse Json from responseMsg protobuf
		const ::hoosho::msg::z::AddNewMsgRes& stAddNewMsgRes = stZMsgRes.add_new_msg_res();
		GetAnyValue()["msg_id"] = int_2_str(stAddNewMsgRes.msg_id());
		GetAnyValue()["create_ts"] = int_2_str(stAddNewMsgRes.create_ts());
		
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
