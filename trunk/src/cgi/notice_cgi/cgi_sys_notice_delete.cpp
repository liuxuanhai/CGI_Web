#include "notice_server_cgi.h"
#include "proto_io_tcp_client.h"

class CgiSysNoticeDelete: public NoticeServerCgi
{
public:
	CgiSysNoticeDelete() :
			NoticeServerCgi(0, "config.ini", "logger.properties")
	{

	}
	bool InnerProcess()
	{
		uint64_t iPaAppidMd5 = strtoul(((string)GetInput().GetValue("pa_appid_md5")).c_str(),NULL,10);
		uint64_t iCreateTs = strtoul(((string)GetInput().GetValue("create_ts")).c_str(),NULL,10);

		ZERO_INT_RETURN(iPaAppidMd5);
		ZERO_INT_RETURN(iCreateTs);

		::common::protoio::ProtoIOTcpClient client(m_notice_server_ip, m_notice_server_port);
		std::string strErrMsg = "";
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::Msg stResponse;
		::hoosho::msg::MsgHead* header = stRequest.mutable_head();

		uint32_t iSeq = 1;
		header->set_seq(iSeq);
		header->set_cmd(hoosho::msg::DELETE_SYS_NOTICE_REQ);
		::hoosho::sysnotice::DeleteSysNoticeReq* req = stRequest.mutable_sys_notice_del_req();

		req->set_pa_appid_md5(iPaAppidMd5);
		req->set_create_ts(iCreateTs);


		if (client.io(stRequest, stResponse, strErrMsg) < 0)
		{
			LOG4CPLUS_DEBUG(logger, "failed, strErrMsg=" << strErrMsg << std::endl;);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return -1;
		}

		if (stResponse.head().cmd() == hoosho::msg::DELETE_SYS_NOTICE_RES && stResponse.head().seq() == iSeq)
		{
			if (stResponse.head().result() == hoosho::msg::E_OK)
			{
				LOG4CPLUS_DEBUG(logger, "succe!!\n"<<stResponse.Utf8DebugString() << std::endl);

				DoReply(CGI_RET_CODE_OK);
			}
			else if (stResponse.head().result() == hoosho::msg::E_SERVER_INNER_ERROR)
			{
				LOG4CPLUS_DEBUG(logger, "error!! result=E_SERVER_INNER_ERROR" << std::endl);
				LOG4CPLUS_DEBUG(logger, stResponse.Utf8DebugString() << std::endl);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
			}
			else if (stResponse.head().result() == hoosho::msg::E_INVALID_REQ_PARAM)
			{
				LOG4CPLUS_DEBUG(logger, "error!! result=E_INVALID_REQ_PARAM" << std::endl);
				LOG4CPLUS_DEBUG(logger, stResponse.Utf8DebugString() << std::endl);
				DoReply(CGI_RET_CODE_INVALID_PARAM);
			}
			else
			{
				LOG4CPLUS_DEBUG(logger, "error!! result no 1 or 0" << std::endl);
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
	CgiSysNoticeDelete cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
