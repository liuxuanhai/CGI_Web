#include "notice_server_cgi.h"
#include "proto_io_tcp_client.h"

class CgiSysNoticeUpdate: public NoticeServerCgi
{
public:
	CgiSysNoticeUpdate() :
			NoticeServerCgi(0, "config.ini", "logger.properties")
	{

	}
	bool InnerProcess()
	{
		uint64_t iPaAppidMd5 = strtoul(((string)GetInput().GetValue("pa_appid_md5")).c_str(),NULL,10);
		//	uint64_t iOpenidMd5 = strtoul(((string)GetInput().GetValue("openid_md5")).c_str(),NULL,10);
		string strTitle = (string) GetInput().GetValue("title");
		string strContent = (string) GetInput().GetValue("content");
		uint64_t iCreateTs = strtoul(((string)GetInput().GetValue("create_ts")).c_str(),NULL,10);

		ZERO_INT_RETURN(iPaAppidMd5);
		ZERO_INT_RETURN(iCreateTs);
		EMPTY_STR_RETURN(strTitle);
		EMPTY_STR_RETURN(strContent);

		::common::protoio::ProtoIOTcpClient client(m_notice_server_ip, m_notice_server_port);
		std::string strErrMsg = "";
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::Msg stResponse;
		::hoosho::msg::MsgHead* header = stRequest.mutable_head();

		uint32_t iSeq = 1;
		header->set_seq(iSeq);
		header->set_cmd(hoosho::msg::UPDATE_SYS_NOTICE_REQ);
		::hoosho::sysnotice::UpdateSysNoticeReq* req = stRequest.mutable_sys_notice_update_req();
		hoosho::commstruct::SysNotice* pBodySysNotice = req->mutable_sys_notice();
		pBodySysNotice->set_pa_appid_md5(iPaAppidMd5);
		pBodySysNotice->set_title(strTitle);
		pBodySysNotice->set_content(strContent);
		pBodySysNotice->set_create_ts(iCreateTs);

		if (client.io(stRequest, stResponse, strErrMsg) < 0)
		{
			LOG4CPLUS_DEBUG(logger, "failed, strErrMsg=" << strErrMsg << std::endl;);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return -1;
		}

		if (stResponse.head().cmd() == hoosho::msg::UPDATE_SYS_NOTICE_RES && stResponse.head().seq() == iSeq)
		{
			if (stResponse.head().result() == hoosho::msg::E_OK)
			{
				LOG4CPLUS_DEBUG(logger, "succe!!\n"<<stResponse.Utf8DebugString() << std::endl);

				DoReply(CGI_RET_CODE_OK);
			}
			else if (stResponse.head().result() == hoosho::msg::E_INVALID_REQ_PARAM)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
			}
			else if (stResponse.head().result() == hoosho::msg::E_SERVER_INNER_ERROR)
			{
				LOG4CPLUS_DEBUG(logger, "error!! result=E_SERVER_INNER_ERROR" << std::endl);
				LOG4CPLUS_DEBUG(logger, stResponse.Utf8DebugString() << std::endl);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
			}
			else
			{
				LOG4CPLUS_DEBUG(logger, "error!! result =" << stResponse.head().result()<<std::endl);
				LOG4CPLUS_DEBUG(logger, stResponse.Utf8DebugString() << std::endl);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
			}
		}
		else
		{
			LOG4CPLUS_DEBUG(logger, "error!! what 7 package?" << std::endl);
			LOG4CPLUS_DEBUG(logger, stResponse.Utf8DebugString() << std::endl);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
		}
		return true;
	}
};

int main()
{
	CgiSysNoticeUpdate cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
