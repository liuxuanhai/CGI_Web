#include "cgi_msg_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"

class CgiMsgGetRedPoint: public CgiMsgServer
{
public:
	CgiMsgGetRedPoint():
		CgiMsgServer(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		//1.get HTTP params		
		std::string strCode = (std::string)GetInput().GetValue("code");
				
		//2.Build requestMsg protobuf
		std::string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
		pRequestMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);		
		pRequestMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stRequestMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_RED_POINT_REQ);		
		pZMsgReq->set_code(strCode);

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
		if(stZMsgRes.sub_cmd() != ::hoosho::msg::z::GET_RED_POINT_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "respons.sub_cmd="<<stZMsgRes.sub_cmd()<<", unknown, fuck!!!");
			return true;
		}

		//4.Build strResponse Json from responseMsg protobuf		
		//init AnyValue
		for(int i=1; i<=4; i++)
		{
			GetAnyValue()["redpoint_info_list"][i] = 0;			
		}
		
		const ::hoosho::msg::z::GetRedPointRes& stGetRedPointRes = stZMsgRes.get_red_point_res();
		for(int i=0; i<stGetRedPointRes.red_point_info_list_size(); i++)
		{
			int index = stGetRedPointRes.red_point_info_list(i).type();
			GetAnyValue()["redpoint_info_list"][index] = stGetRedPointRes.red_point_info_list(i).value();
		}
		
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}
	
};

int main()
{
	CgiMsgGetRedPoint cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

