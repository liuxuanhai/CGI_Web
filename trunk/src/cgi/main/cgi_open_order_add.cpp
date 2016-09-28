#include "main_cgi.h"

class CgiOpenOrderAdd: public MainCgi
{
     public:
		CgiOpenOrderAdd() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			if(m_user_info.m_user_type != USER_TYPE_DEMAND_SIDE)
			{
				DoReply(CGI_RET_CODE_NO_PREVILEDGES);
				LOG4CPLUS_ERROR(logger, "add open order failed, not demand side user, uin="<<m_user_info.m_uin);
				return true;
			}

			uint32_t dwNeedType =(uint32_t)GetInput().GetValue("need_type");
			std::string strExtraInfo =(string)GetInput().GetValue("extra_info");
			if(dwNeedType < OPEN_ORDER_TYPE_ACCOUNT || dwNeedType > OPEN_ORDER_TYPE_GAME)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "add open order failed, invali param dwNeedType="<<dwNeedType);
				return true;
			}
			EMPTY_STR_RETURN(strExtraInfo);

			string strErrMsg = "";
			//add open order
            OpenOrder stOpenOrder(m_table_name_open_order);
			stOpenOrder.m_id = GenerateTransID(ID_TYPE_ORDER_OPEN);
			stOpenOrder.m_need_uin = m_user_info.m_uin;
			stOpenOrder.m_need_type = dwNeedType;
			stOpenOrder.m_extra_info = strExtraInfo;
			stOpenOrder.m_status = OPEN_ORDER_STATUS_VERIFY_PENDING;
			stOpenOrder.m_create_ts = time(0);
			if(stOpenOrder.m_id.empty())
			{
				LOG4CPLUS_ERROR(logger, "generate open_order id failed, fuck, add open order failed");
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			
			if(stOpenOrder.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "stOpenOrder.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			GetAnyValue()["open_order_id"] = stOpenOrder.m_id;
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiOpenOrderAdd cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




