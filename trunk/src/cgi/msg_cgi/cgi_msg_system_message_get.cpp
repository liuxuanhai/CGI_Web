#include "cgi_msg_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"

class CgiMsgSystemMessageGet: public CgiMsgServer
{
public:
	CgiMsgSystemMessageGet():
		CgiMsgServer(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		//1.get HTTP params
		uint64_t openid_md5 = strtoul(((string)GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		uint64_t pa_appid_md5 = strtoul(((string)GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t begin_ts = strtoul(((string)GetInput().GetValue("begin_ts")).c_str(), NULL, 10);
		uint64_t limit = strtoul(((string)GetInput().GetValue("limit")).c_str(), NULL, 10);

		if(0 == openid_md5 || 0 == pa_appid_md5 || (limit <=0 || limit >= 100))
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID openid_md5="<<openid_md5<<", pa_appid_md5="<<pa_appid_md5<<", begin_ts="<<begin_ts<<", limit="<<limit);
			return true;	
		}		
				
		//2.Build requestMsg protobuf
		std::string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::QUERY_SYSTEM_MSG_REQ);		
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::sixin::QuerySystemMsgReq* stMsgQueryReq = stRequestMsg.mutable_system_msg_query_req();
		stMsgQueryReq->set_openid_md5(openid_md5);
		stMsgQueryReq->set_pa_appid_md5(pa_appid_md5);
		stMsgQueryReq->set_begin_ts(begin_ts);
		stMsgQueryReq->set_limit(limit);

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
		if(stHead.cmd() != ::hoosho::msg::QUERY_SYSTEM_MSG_RES)
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
		const ::hoosho::sixin::QuerySystemMsgRes& stMsgQueryRes = stResponseMsg.system_msg_query_res();
		for(int i=0; i<stMsgQueryRes.system_msg_list_size(); i++)
		{
			lce::cgi::CAnyValue stAnyValue;
			stAnyValue["pa_appid_md5"] = stMsgQueryRes.system_msg_list(i).pa_appid_md5();
			stAnyValue["create_ts"] = stMsgQueryRes.system_msg_list(i).create_ts();
			stAnyValue["title"] = stMsgQueryRes.system_msg_list(i).title();
			stAnyValue["content"] = stMsgQueryRes.system_msg_list(i).content();
			GetAnyValue()["system_msg_list"].push_back(stAnyValue);
		}
		
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}
	
};

int main()
{
	CgiMsgSystemMessageGet cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

