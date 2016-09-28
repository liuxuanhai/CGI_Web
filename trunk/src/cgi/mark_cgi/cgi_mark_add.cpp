#include "cgi_mark_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"

class CgiMarkAdd: public CgiMarkServer
{
public:
	CgiMarkAdd():
		CgiMarkServer(0, "config.ini", "logger.properties")
	{		
	}
	
	bool InnerProcess()
	{
		//1.get HTTP params
		uint64_t openid_md5 = strtoul(((string)GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		uint64_t pa_appid_md5 = strtoul(((string)GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t pa_openid = strtoul(((string)GetInput().GetValue("pa_openid")).c_str(), NULL, 10);
		uint64_t is_follow = strtoul(((string)GetInput().GetValue("is_follow")).c_str(), NULL, 10);	

		if(0 == openid_md5 || 0 == pa_appid_md5 || 0 == pa_openid)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID PARAM");
			return true;
		}

		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::ADD_USER_MARK_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::usermark::AddUserMarkReq* stMarkAddReq = stRequestMsg.mutable_usermark_add_req();
		::hoosho::commstruct::UserMark* stUserMark = stMarkAddReq->mutable_usermark();
		stUserMark->set_openid_md5(openid_md5);
		stUserMark->set_pa_appid_md5(pa_appid_md5);
		stUserMark->set_pa_openid(pa_openid);
		stUserMark->set_is_follow(is_follow);

		//3.send to server, and recv responseMsg protobuf
		common::protoio::ProtoIOTcpClient ioclient(markServerIP, markServerPort);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

		if(iRet != 0)
		{			
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			return true;
		}

		//LOG4CPLUS_DEBUG(logger, "response Msg: \n" << stResponseMsg.Utf8DebugString());	
		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponseMsg.head();
		if(stHead.cmd() != ::hoosho::msg::ADD_USER_MARK_RES)
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

int main(int argc, char** argv)
{
	CgiMarkAdd cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

