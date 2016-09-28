#include "notice_server_cgi.h"
#include "proto_io_tcp_client.h"

class CgiNoticeRecordUpdate: public NoticeServerCgi
{
public:
	CgiNoticeRecordUpdate() :
			NoticeServerCgi(0, "config.ini", "logger.properties")
	{

	}
	bool InnerProcess()
	{
		uint64_t iPaAppidMd5 = strtoul(((string)GetInput().GetValue("pa_appid_md5")).c_str(),NULL,10);
		uint64_t iOpenidMd5 = strtoul(((string)GetInput().GetValue("openid_md5")).c_str(),NULL,10);
		uint32_t iType = (uint32_t) GetInput().GetValue("type");
		uint64_t iCreateTs = strtoul(((string)GetInput().GetValue("create_ts")).c_str(),NULL,10);
		string strExtraData0 = (string) GetInput().GetValue("extra_data_0");
		string strExtraData1 = (string) GetInput().GetValue("extra_data_1");
		string strExtraData2 = (string) GetInput().GetValue("extra_data_2");

		ZERO_INT_RETURN(iPaAppidMd5);
		ZERO_INT_RETURN(iOpenidMd5);
		ZERO_INT_RETURN(iType);
		ZERO_INT_RETURN(iCreateTs);
		if (iType < 1 || iType > 3)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid iType="<<iType);
			return true;
		}
		::common::protoio::ProtoIOTcpClient client(m_notice_server_ip, m_notice_server_port);
		std::string strErrMsg = "";
		::hoosho::msg::Msg stRequest;
		::hoosho::msg::Msg stResponse;

		::hoosho::msg::MsgHead* header = stRequest.mutable_head();
		uint32_t iSeq = 1;
		header->set_seq(iSeq);
		header->set_cmd(hoosho::msg::UPDATE_NOTICE_RECORD_REQ);
		::hoosho::noticenotify::UpdateNoticeRecordReq* req = stRequest.mutable_notice_record_update_req();
		hoosho::commstruct::NoticeRecord * pBodyNoticeRecord = req->mutable_notice_record();

		pBodyNoticeRecord->set_pa_appid_md5(iPaAppidMd5);
		pBodyNoticeRecord->set_openid_md5(iOpenidMd5);
		pBodyNoticeRecord->set_type(iType);
		pBodyNoticeRecord->set_create_ts(iCreateTs);
		pBodyNoticeRecord->set_extra_data_0(strExtraData0);
		pBodyNoticeRecord->set_extra_data_1(strExtraData1);
		pBodyNoticeRecord->set_extra_data_2(strExtraData2);

		if (client.io(stRequest, stResponse, strErrMsg) < 0)
		{
			LOG4CPLUS_DEBUG(logger, "failed, strErrMsg=" << strErrMsg << std::endl;);

			return -1;
		}
		std::ostringstream oss;
		if (stResponse.head().cmd() == hoosho::msg::UPDATE_NOTICE_RECORD_RES && stResponse.head().seq() == iSeq)
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
				LOG4CPLUS_DEBUG(logger, "error!! result=1" << std::endl);
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
	CgiNoticeRecordUpdate cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
