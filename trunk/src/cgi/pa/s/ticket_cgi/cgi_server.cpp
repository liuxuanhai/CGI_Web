#include "cgi_server.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

IMPL_LOGGER(CgiServer, logger);

bool CgiServer::DerivedInit()
{
	GetConfig().GetValue("s_server", "ip", SServerIP, "");
	GetConfig().GetValue("s_server", "port", SServerPort, 0);
	
	return true;
}

bool CgiServer::Process()
{
	LOG4CPLUS_INFO(logger, "BEGIN CGI ----------------- "<<GetCgiName()<<"----------------- ");

	if(strcmp(GetInput().GetQueryString(), "") != 0)
	{
		LOG4CPLUS_INFO(logger, "GET PARAM: \n"<<(GetInput().GetQueryString()));
	}
	if(strcmp(GetInput().GetPostData(), "") != 0)
	{
		LOG4CPLUS_INFO(logger, "POST PARAM: \n"<<(GetInput().GetPostData()));
	}	

	//derived Cgi  logic here!!!!!!!!!
	InnerProcess();

	LOG4CPLUS_INFO(logger, "END CGI \n");

	return true;
}

int CgiServer::GetUserFeedContendIdList(common::protoio::ProtoIOTcpClient &ioclient
								, const std::string &strCode, uint64_t qwUserid, const std::vector<uint64_t> &vecFeedidList
								, std::vector<hoosho::msg::s::FeedContendInfo> &vecFeedContendInfo)
{
	//2.Build requestMsg 
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
	pRequestMsgHead->set_cmd(::hoosho::msg::S_PROJECT_REQ);
	pRequestMsgHead->set_seq(time(NULL));

	::hoosho::msg::s::MsgReq* pSMsgReq = stRequestMsg.mutable_s_msg_req();
	pSMsgReq->set_sub_cmd(::hoosho::msg::s::GET_USER_FEED_CONTEND_ID_LIST_REQ);
	pSMsgReq->set_code(strCode);

	::hoosho::msg::s::GetUserFeedContendIdListReq *pGetUserFeedContendIdListReq = pSMsgReq->mutable_get_user_feed_contend_id_list_req();
	pGetUserFeedContendIdListReq->set_user_id(qwUserid);
	for(size_t i = 0; i < vecFeedidList.size(); i++)
	{
		pGetUserFeedContendIdListReq->add_feed_id_list(vecFeedidList[i]);
	}

	//3.Send to SServer, and recv responseMsg

	std::string strErrMsg = "";
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

	if(iRet != 0)
	{
		DoReply(CGI_RET_CODE_SERVER_BUSY);
		LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = "<<strErrMsg);
		return -1;
	}

	LOG4CPLUS_DEBUG(logger, "response Msg: \n"<<stResponseMsg.Utf8DebugString());

	//4.parse responseMsg
	const ::hoosho::msg::MsgHead& stHead = stResponseMsg.head();
	if(stHead.cmd() != ::hoosho::msg::S_PROJECT_RES)
	{
		DoReply(CGI_RET_CODE_SERVER_BUSY);
		LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
		return -1;
	}
	if(stHead.result() != hoosho::msg::E_OK)
	{
		LOG4CPLUS_ERROR(logger, "result=" << stHead.result());
		return -1;
	}
	
	const ::hoosho::msg::s::MsgRes& stSMsgRes = stResponseMsg.s_msg_res();
	if(stSMsgRes.sub_cmd() != ::hoosho::msg::s::GET_USER_FEED_CONTEND_ID_LIST_RES)
	{
		DoReply(CGI_RET_CODE_SERVER_BUSY);
		LOG4CPLUS_ERROR(logger, "sub_cmd="<<stSMsgRes.sub_cmd()<<", unknown, fuck!!!");
		return -1;
	}

	const hoosho::msg::s::GetUserFeedContendIdListRes stGetUserFeedContendIdListRes = stSMsgRes.get_user_feed_contend_id_list_res();
	for(int i = 0; i < stGetUserFeedContendIdListRes.feed_contend_info_list_size(); i++)
	{
		hoosho::msg::s::FeedContendInfo stFeedContendInfo;
		stFeedContendInfo.CopyFrom(stGetUserFeedContendIdListRes.feed_contend_info_list(i));
		vecFeedContendInfo.push_back(stFeedContendInfo);
	}
	return 0;
}

int CgiServer::GetUserInfoList(common::protoio::ProtoIOTcpClient &ioclient
								, const std::string &strCode, const std::set<uint64_t> &setUserid
								, std::vector<hoosho::msg::s::UserInfo> &vecUserInfo)
{
	//2.Build requestMsg 
	::hoosho::msg::Msg stRequestMsg;
	::hoosho::msg::Msg stResponseMsg;

	::hoosho::msg::MsgHead* pRequestMsgHead = stRequestMsg.mutable_head();
	pRequestMsgHead->set_cmd(::hoosho::msg::S_PROJECT_REQ);
	pRequestMsgHead->set_seq(time(NULL));

	::hoosho::msg::s::MsgReq* pSMsgReq = stRequestMsg.mutable_s_msg_req();
	pSMsgReq->set_sub_cmd(::hoosho::msg::s::GET_USER_INFO_REQ);
	pSMsgReq->set_code(strCode);

	::hoosho::msg::s::GetUserInfoReq *pGetUserInfoReq = pSMsgReq->mutable_get_user_info_req();
	std::set<uint64_t>::const_iterator it = setUserid.begin();
	for(;it != setUserid.end(); ++it)
	{
		pGetUserInfoReq->add_user_id_list(*it);
	}

	//3.Send to SServer, and recv responseMsg

	std::string strErrMsg = "";
	int iRet;
	iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

	if(iRet != 0)
	{
		DoReply(CGI_RET_CODE_SERVER_BUSY);
		LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = "<<strErrMsg);
		return -1;
	}

	LOG4CPLUS_DEBUG(logger, "response Msg: \n"<<stResponseMsg.Utf8DebugString());

	//4.parse responseMsg
	const ::hoosho::msg::MsgHead& stHead = stResponseMsg.head();
	if(stHead.cmd() != ::hoosho::msg::S_PROJECT_RES)
	{
		DoReply(CGI_RET_CODE_SERVER_BUSY);
		LOG4CPLUS_ERROR(logger, "response.cmd="<<stHead.cmd()<<", unknown, fuck!!!");
		return -1;
	}
	if(stHead.result() != hoosho::msg::E_OK)
	{
		LOG4CPLUS_ERROR(logger, "result=" << stHead.result());
		return -1;
	}
	
	const ::hoosho::msg::s::MsgRes& stSMsgRes = stResponseMsg.s_msg_res();
	if(stSMsgRes.sub_cmd() != ::hoosho::msg::s::GET_USER_INFO_RES)
	{
		DoReply(CGI_RET_CODE_SERVER_BUSY);
		LOG4CPLUS_ERROR(logger, "sub_cmd="<<stSMsgRes.sub_cmd()<<", unknown, fuck!!!");
		return -1;
	}

	for(int i = 0; i < stSMsgRes.user_info_list_size(); i++)
	{
		hoosho::msg::s::UserInfo stUserInfo;
		stUserInfo.CopyFrom(stSMsgRes.user_info_list(i));
		vecUserInfo.push_back(stUserInfo);
	}
	return 0;
}
