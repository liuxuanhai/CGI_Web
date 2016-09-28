#include "notice_server_cgi.h"
#include "proto_io_tcp_client.h"

class CgiSysNoticeGetForPa: public NoticeServerCgi
{
public:
	CgiSysNoticeGetForPa() :
			NoticeServerCgi(0, "config.ini", "logger.properties")
	{

	}
	bool InnerProcess()
	{
		uint64_t iPaAppidMd5 = strtoul(((string)GetInput().GetValue("pa_appid_md5")).c_str(),NULL,10);
		uint32_t iPagesize = (uint32_t) GetInput().GetValue("pagesize");
		uint64_t iLimitTs = strtoul(((string)GetInput().GetValue("limit_ts")).c_str(),NULL,10);

		ZERO_INT_RETURN(iPaAppidMd5);


		::common::protoio::ProtoIOTcpClient client(m_notice_server_ip, m_notice_server_port);
		std::string strErrMsg = "";
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::Msg stResponse;

		::hoosho::msg::MsgHead* header = stRequest.mutable_head();
		uint32_t iSeq = 1;
		header->set_seq(iSeq);
		header->set_cmd(hoosho::msg::QUERY_SYS_NOTICE_ALL_REQ);
		::hoosho::sysnotice::QuerySysNoticeAllReq* req = stRequest.mutable_sys_notice_query_all_req();
		req->set_pa_appid_md5(iPaAppidMd5);
		req->set_pagesize(iPagesize);
		req->set_limit_ts(iLimitTs);
		if (client.io(stRequest, stResponse, strErrMsg) < 0)
		{
			LOG4CPLUS_DEBUG(logger, "failed, strErrMsg=" << strErrMsg << std::endl;);

			return -1;
		}
		std::ostringstream oss;
		if (stResponse.head().cmd() == hoosho::msg::QUERY_SYS_NOTICE_ALL_RES && stResponse.head().seq() == iSeq)
		{
			if (stResponse.head().result() == hoosho::msg::E_OK)
			{
				LOG4CPLUS_DEBUG(logger, "succe!!\n"<<stResponse.Utf8DebugString() << std::endl);
				const hoosho::sysnotice::QuerySysNoticeAllRes& res = stResponse.sys_notice_query_all_res();
				int iSize = res.sys_notice_list_size();
				for (int i = 0; i < iSize; i++)
				{
					lce::cgi::CAnyValue stSysNotice;

					stSysNotice["pa_appid_md5"] = int_2_str(res.sys_notice_list(i).pa_appid_md5());
					stSysNotice["create_ts"] = res.sys_notice_list(i).create_ts();
					stSysNotice["title"] = res.sys_notice_list(i).title();
					stSysNotice["content"] = res.sys_notice_list(i).content();

					GetAnyValue()["sys_notice_list"].push_back(stSysNotice);
				}
				DoReply(CGI_RET_CODE_OK);
			}
			else if (stResponse.head().result() == hoosho::msg::E_SERVER_INNER_ERROR)
			{
				LOG4CPLUS_DEBUG(logger, "error!! result= 1" << std::endl);
				LOG4CPLUS_DEBUG(logger, stResponse.Utf8DebugString() << std::endl);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
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
	CgiSysNoticeGetForPa cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
