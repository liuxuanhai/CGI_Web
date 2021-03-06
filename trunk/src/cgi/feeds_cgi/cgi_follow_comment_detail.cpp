#include "cgi_feeds_server.h"
#include <time.h>
#include <sstream>

class CgiFollowCommentDetail: public CgiFeedsServer
{
public:
	CgiFollowCommentDetail():
		CgiFeedsServer(0, "config.ini", "logger.properties", false)
	{

	}

	bool InnerProcess()
	{

		//1.get HTTP params
		uint64_t qwPaAppidMd5 = strtoul( ((string) GetInput().GetValue("pa_appid_md5")).c_str(), NULL, 10);
		uint64_t qwOpenidMd5 = strtoul( ((string) GetInput().GetValue("openid_md5")).c_str(), NULL, 10);
		string strCommentIdList = (string) GetInput().GetValue("comment_id_list");

		ZERO_INT_RETURN(qwPaAppidMd5);
		ZERO_INT_RETURN(qwOpenidMd5);
		EMPTY_STR_RETURN(strCommentIdList);

			
		//2.Build requestMsg protobuf
		string strErrMsg = "";
		::hoosho::msg::Msg stRequestMsg;
		::hoosho::msg::Msg stResponseMsg;
		
		::hoosho::msg::MsgHead* stRequestMsgHead = stRequestMsg.mutable_head();
		stRequestMsgHead->set_cmd(hoosho::msg::QUERY_FOLLOW_COMMENT_DETAIL_REQ);		
		stRequestMsgHead->set_seq(time(NULL));

		::hoosho::feeds::QueryFollowCommentDetailReq* stQueryFollowCommentDetailReq = stRequestMsg.mutable_query_follow_comment_detail_req();
		stQueryFollowCommentDetailReq->set_pa_appid_md5(qwPaAppidMd5);
		stQueryFollowCommentDetailReq->set_openid_md5(qwOpenidMd5);

		vector<string> vecCommentId;
		lce::cgi::Split(strCommentIdList, "|", vecCommentId);
		for(size_t i=0; i != vecCommentId.size(); ++i)
		{
			stQueryFollowCommentDetailReq->add_comment_id_list( strtoul(vecCommentId[i].c_str(), NULL, 10) );
		}
		
		
		//3.send to server, and recv responseMsg protobuf
		::common::protoio::ProtoIOTcpClient ioclient(m_feeds_server_ip, m_feeds_server_port);

		int iRet;
		iRet = ioclient.io(stRequestMsg, stResponseMsg, strErrMsg);

		if(iRet != 0)
		{
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		LOG4CPLUS_TRACE(logger, "resp = " << stResponseMsg.Utf8DebugString());
		iRet = stResponseMsg.head().result();
		SERVER_NOT_OK_RETURN(iRet);


		const hoosho::feeds::QueryFollowCommentDetailRes& stQueryFollowCommentDetailRes = stResponseMsg.query_follow_comment_detail_res();
		lce::cgi::CAnyValue avCommentList;

		//-0- init openid_md5 set
		set<uint64_t> setOpenidMd5;

		LOG4CPLUS_TRACE(logger, "list size = " << stQueryFollowCommentDetailRes.comment_list_size());
		for(int i = 0; i < stQueryFollowCommentDetailRes.comment_list_size(); i++)
		{
			const hoosho::commstruct::FollowCommentInfo& stFollowCommentInfo = stQueryFollowCommentDetailRes.comment_list(i);
			
			LOG4CPLUS_TRACE(logger, "commnet_id = " << stFollowCommentInfo.comment().follow_id());

			lce::cgi::CAnyValue avCommentInfo;
			avCommentInfo["comment"] = ToAnyValue( stFollowCommentInfo.comment());
			avCommentInfo["reply_list"].clear();

			setOpenidMd5.insert( stFollowCommentInfo.comment().openid_md5_from() );
			for(int j = 0; j < stFollowCommentInfo.reply_list_size(); j++)
			{
				avCommentInfo["reply_list"].push_back( ToAnyValue(stFollowCommentInfo.reply_list(j) ) );
				LOG4CPLUS_TRACE(logger, "------ follow_id = " << stFollowCommentInfo.reply_list(j).follow_id());

				setOpenidMd5.insert( stFollowCommentInfo.reply_list(j).openid_md5_from() );
			}
			avCommentList.push_back(avCommentInfo);
		}


		//4.Build strResponse Json from responseMsg protobuf
		GetAnyValue()["comment_list"] = avCommentList;


	
		//-1- get user_info		
		::hoosho::msg::Msg stUserInfoRequest;
		::hoosho::msg::MsgHead* pHead = stUserInfoRequest.mutable_head();
		pHead->set_cmd(::hoosho::msg::QUERY_USER_DETAIL_INFO_REQ);
		pHead->set_seq(time(0));
		::hoosho::user::QueryUserDetailInfoReq* pQueryUserDetailInfoReq = stUserInfoRequest.mutable_query_user_detail_info_req();
		for(std::set<uint64_t>::iterator iter=setOpenidMd5.begin(); iter!=setOpenidMd5.end(); ++iter)
		{
			pQueryUserDetailInfoReq->add_openid_md5_list(*iter);
		}

		//io
		::hoosho::msg::Msg stResponse;
		::common::protoio::ProtoIOTcpClient stProtoIOTcpClient(m_user_server_ip, m_user_server_port);
		iRet = stProtoIOTcpClient.io(stUserInfoRequest, stResponse, strErrMsg);
		if(iRet < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Process failed, stProtoIOTcpClient.io, strErrMsg="<<strErrMsg);
			return true;	
		}
		

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

		LOG4CPLUS_DEBUG(logger, "size = " << stResponse.query_user_detail_info_res().user_detail_info_list_size());
		for(int i=0; i<stResponse.query_user_detail_info_res().user_detail_info_list_size(); ++i)
		{
			lce::cgi::CAnyValue stAnyValue;
			const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = stResponse.query_user_detail_info_res().user_detail_info_list(i);
			LOG4CPLUS_DEBUG(logger, "one detailuserinfo="<<stUserDetailInfo.Utf8DebugString());
			UserInfoPB2Any(stUserDetailInfo, stAnyValue);	
			GetAnyValue()["user_info"][ int_2_str(stUserDetailInfo.openid_md5()) ] = stAnyValue;	
		}

		DoReply(CGI_RET_CODE_OK);
		return true;		
	}
		
};

int main()
{
	CgiFollowCommentDetail cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
