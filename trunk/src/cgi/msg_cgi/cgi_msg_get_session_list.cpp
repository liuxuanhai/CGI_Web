#include "cgi_msg_server.h"
#include "proto_io_tcp_client.h"
#include <time.h>
#include "msg.pb.h"
#include "user_util.h"

class CgiMsgGetSessionList: public CgiMsgServer
{
public:
	CgiMsgGetSessionList():
		CgiMsgServer(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		//1.get HTTP params
		uint64_t openid_md5 = strtoul(((string)GetInput().GetValue("openid_md5")).c_str(), NULL, 10);

		if(0 == openid_md5)
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
		stRequestMsgHead->set_cmd(::hoosho::msg::QUERY_MSG_SESSION_LIST_REQ);		
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::sixin::QuerySixinSessionListReq* stMsgQueryReq = stRequestMsg.mutable_msg_query_session_list_req();
		stMsgQueryReq->set_openid_md5(openid_md5);		

		//3.send to server, and recv responseMsg protobuf
		common::protoio::ProtoIOTcpClient ioclient(msgServerIP, msgServerPort);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

		if(iRet != 0)
		{
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "response Msg: \n" << stResponseMsg.Utf8DebugString());		
		//parse response
		const ::hoosho::msg::MsgHead& stResponseMsgHead = stResponseMsg.head();
		if(stResponseMsgHead.cmd() != ::hoosho::msg::QUERY_MSG_SESSION_LIST_RES)
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
		const ::hoosho::sixin::QuerySixinSessionListRes& stMsgQueryRes = stResponseMsg.msg_query_session_list_res();


		//5.Get Userinfo by openid from user server		
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::MsgHead* pHead = stRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(NULL));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stRequest.mutable_query_user_detail_info_req();
		pQueryUserDetailInfoReq->add_openid_md5_list(openid_md5);
		for(int i=0; i<stMsgQueryRes.session_size(); i++)
		{
			const ::hoosho::commstruct::Session& session = stMsgQueryRes.session(i);	
			if(openid_md5 == session.openid_md5_from())
			{
				pQueryUserDetailInfoReq->add_openid_md5_list(session.openid_md5_to());
			}
			else
			{
				pQueryUserDetailInfoReq->add_openid_md5_list(session.openid_md5_from());
			}
		}
		

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
		const ::hoosho::commstruct::UserDetailInfo& stHostUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(0);
		UserInfoPB2Any(stHostUserDetailInfo, stHostUserInfo);
		
		lce::cgi::CAnyValue session_list;		
		for(int i=stMsgQueryRes.session_size()-1; i>=0; i--)
		{			
			const ::hoosho::commstruct::Session& session = stMsgQueryRes.session(i);	
			bool flag = false;
			
			lce::cgi::CAnyValue stGuestUserInfo;
			for(int j=stResponse.query_user_detail_info_res().user_detail_info_list_size()-1; j>=0; j--)
			{
				const ::hoosho::commstruct::UserDetailInfo& stGuestUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(j);
				if(openid_md5 == session.openid_md5_from())
				{
					if(stGuestUserDetailInfo.openid_md5() != session.openid_md5_to())
						continue;
					else
					{
						UserInfoPB2Any(stGuestUserDetailInfo, stGuestUserInfo);	
						flag = true;
						break;
					}	
				}
				else if(openid_md5 == session.openid_md5_to())
				{
					if(stGuestUserDetailInfo.openid_md5() != session.openid_md5_from())
						continue;
					else
					{
						UserInfoPB2Any(stGuestUserDetailInfo, stGuestUserInfo);	
						flag = true;
						break;
					}	
				}								
			}

			if(!flag)
				continue;			

			lce::cgi::CAnyValue item;

			item["openid_md5_from"] = int_2_str(session.openid_md5_from());
			item["openid_md5_to"] = int_2_str(session.openid_md5_to());
			
			if(openid_md5 == session.openid_md5_from())
			{
				item["userinfo_from"] = stHostUserInfo;
				item["userinfo_to"] = stGuestUserInfo;
			}
			else
			{
				item["userinfo_from"] = stGuestUserInfo;
				item["userinfo_to"] = stHostUserInfo;
			}
						
			item["content"] = session.content();
			item["create_ts"] = int_2_str(session.create_ts());
			item["newmsg_status"] = session.newmsg_status();

			session_list.push_back(item);			
		}
		GetAnyValue()["session_list"] = session_list;		
		
		DoReply(CGI_RET_CODE_OK);
		return true;		
	}
		
};

int main()
{
	CgiMsgGetSessionList cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
