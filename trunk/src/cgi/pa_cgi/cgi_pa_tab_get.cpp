#include "cgi_pa_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"

class CgiPaTabGet: public CgiPaServer
{
public:
	CgiPaTabGet():
		CgiPaServer(0, "config.ini", "logger.properties")
	{		
	}
	
	bool InnerProcess()
	{	
		//1.get HTTP params
		uint64_t pa_appid_md5 = strtoul(((string)GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);	

		if(0 == pa_appid_md5)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "INVALID PA APPID MD5");
			return true;			
		}

		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;

		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(::hoosho::msg::QUERY_PA_TAB_REQ);
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::pa::QueryPaTabReq* stQueryPaTabReq = stRequestMsg.mutable_pa_tab_query_req();
		stQueryPaTabReq->set_appid_md5(pa_appid_md5);

		//3.send to server, and recv responseMsg protobuf
		common::protoio::ProtoIOTcpClient ioclient(paServerIP, paServerPort);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

		if(iRet != 0)
		{
			LOG4CPLUS_DEBUG(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		//LOG4CPLUS_DEBUG(logger, "response Msg: \n" << stResponseMsg.Utf8DebugString());	
		//parse response
		const ::hoosho::msg::MsgHead& stHead = stResponseMsg.head();
		if(stHead.cmd() != ::hoosho::msg::QUERY_PA_TAB_RES)
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
		lce::cgi::CAnyValue tab_info_list;
		const ::hoosho::pa::QueryPaTabRes& stQueryPaTabRes = stResponseMsg.pa_tab_query_res();
		for(int i=0; i<stQueryPaTabRes.tab_info_size(); i++)
		{
			const ::hoosho::commstruct::TabInfo& tabInfo = stQueryPaTabRes.tab_info(i);
			lce::cgi::CAnyValue item;		
			item["id"] = tabInfo.id();
			item["name"] = tabInfo.name();
			item["extra_data_0"] = tabInfo.extra_data_0();

			tab_info_list.push_back(item);
		}

		GetAnyValue()["tab_info_list"] = tab_info_list;
		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main(int argc, char** argv)
{
	CgiPaTabGet cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}

