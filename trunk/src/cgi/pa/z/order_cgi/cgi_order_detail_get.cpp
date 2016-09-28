#include "cgi_order_base.h"
#include "msg.pb.h"

class CgiOrderDetailGet:public CgiOrderBase
{
public:
	CgiOrderDetailGet():CgiOrderBase(0, "config.ini", "logger.properties")
	{

	}

	bool InnerProcess()
	{
		string strCode = GetInput().GetValue("code");
		string strOrderidList = GetInput().GetValue("orderid_list");

		EMPTY_STR_RETURN(strCode);
		EMPTY_STR_RETURN(strOrderidList);

		vector<string> vecStrOrderidList;
		vector<uint64_t> vecOrderidList;
		lce::util::StringOP::Split(strOrderidList, "|", vecStrOrderidList);

		for(size_t i = 0; i < vecStrOrderidList.size(); i++)
		{
			vecOrderidList.push_back(strtoul(vecStrOrderidList[i].c_str(), NULL, 10));
		}

		::hoosho::msg::Msg stReqMsg;
		::hoosho::msg::Msg stResMsg;

		::hoosho::msg::MsgHead* pReqMsgHead = stReqMsg.mutable_head();
		pReqMsgHead->set_cmd(::hoosho::msg::Z_PROJECT_REQ);
		pReqMsgHead->set_seq(time(NULL));

		::hoosho::msg::z::MsgReq* pZMsgReq = stReqMsg.mutable_z_msg_req();
		pZMsgReq->set_sub_cmd(::hoosho::msg::z::GET_ORDER_DETAIL_REQ);
		pZMsgReq->set_code(strCode);

		::hoosho::msg::z::GetOrderDetailReq* pGetOrderDetailReq = pZMsgReq->mutable_get_order_detail_req();
		for(size_t i = 0; i < vecOrderidList.size(); i++)
		{
			pGetOrderDetailReq->add_orderid_list(vecOrderidList[i]);
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
		LOG4CPLUS_DEBUG(logger, stResMsg.Utf8DebugString());

		const ::hoosho::msg::z::MsgRes stZMsgRes = stResMsg.z_msg_res();

		for(int i = 0; i < stZMsgRes.order_info_list_size(); i++)
		{
			lce::cgi::CAnyValue any;
			OrderInfoPB2Any(stZMsgRes.order_info_list(i), any);
			GetAnyValue()["order_list"].push_back(any);
		}
		for(int i = 0; i < stZMsgRes.user_info_list_size(); i++)
		{
			lce::cgi::CAnyValue any;
			LOG4CPLUS_DEBUG(logger, stZMsgRes.user_info_list(i).Utf8DebugString());
			UserInfoPB2Any(stZMsgRes.user_info_list(i), any);
			GetAnyValue()["user_list"].push_back(any);
		}

		const ::hoosho::msg::z::GetOrderDetailRes stGetOrderDetailRes = stZMsgRes.get_order_detail_res();
		for(int i = 0; i < stGetOrderDetailRes.user_extra_info_list_size(); i++)
		{
			lce::cgi::CAnyValue item, stAnyValue;
			const ::hoosho::msg::z::UserExtraInfo& stUserExtraInfo = stGetOrderDetailRes.user_extra_info_list(i);
			UserExtraInfoPB2Any(stUserExtraInfo, stAnyValue);
			item[stUserExtraInfo.openid()] = stAnyValue;					
			GetAnyValue()["user_extra_info_list"].push_back(item);
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}

};

int main()
{
	CgiOrderDetailGet cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
