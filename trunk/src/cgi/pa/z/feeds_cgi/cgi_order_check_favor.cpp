#include "cgi_feeds_base.h"
#include "msg.pb.h"

class CgiFeedsCheckFavor:public CgiFeedsBase
{
public:
	CgiFeedsCheckFavor():CgiFeedsBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");
		string strOrderidList = GetInput().GetValue("orderid_list");

		std::vector<std::string> vecStrOrderid;
		std::vector<uint64_t> vecOrderid;
		lce::cgi::Split(strOrderidList, "|", vecStrOrderid);
		for(size_t i = 0; i < vecStrOrderid.size(); i++)
		{
			vecOrderid.push_back(strtoul(vecStrOrderid[i].c_str(), NULL, 10));
		}

		EMPTY_STR_RETURN(strCode);
		ZERO_INT_RETURN(vecOrderid.size());

		::hoosho::msg::Msg stReqMsg;
		::hoosho::msg::Msg stResMsg;

		::hoosho::msg::MsgHead* pReqMsgHead = stReqMsg.mutable_head();
		pReqMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pReqMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stReqMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::CHECK_ORDER_FAVOR_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::CheckOrderFavorReq *pCheckOrderFavorReq = pZMsgReq->mutable_check_order_favor_req();
		for(size_t i = 0; i < vecOrderid.size(); i++)
		{
			pCheckOrderFavorReq->add_orderid_list(vecOrderid[i]);
		}


		string strErrMsg;
		int iRet;

		::common::protoio::ProtoIOTcpClient ioclient(m_z_server_ip, m_z_server_port);
		iRet = ioclient.io(stReqMsg, stResMsg, strErrMsg);

		if(iRet != 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "ProtoIOTcpClient IO failed, errmsg = " << strErrMsg);
			return true;
		}

		const ::hoosho::msg::MsgHead& msgHead = stResMsg.head();
		if(msgHead.cmd() != ::hoosho::msg::Z_PROJECT_RES)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "response.cmd = " << msgHead.cmd() << " unknow");
			return true;
		}
		SERVER_NOT_OK_RETURN(msgHead.result());

		const ::hoosho::msg::z::MsgRes stZMsgRes = stResMsg.z_msg_res();
		const ::hoosho::msg::z::CheckOrderFavorRes stCheckOrderFavorRes = stZMsgRes.check_order_favor_res();
		for(int i = 0; i < stCheckOrderFavorRes.favor_list_size(); i++)
		{
			GetAnyValue()["check_order_favor"][int_2_str(stCheckOrderFavorRes.favor_list(i).orderid())] = stCheckOrderFavorRes.favor_list(i).create_ts();
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiFeedsCheckFavor cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
