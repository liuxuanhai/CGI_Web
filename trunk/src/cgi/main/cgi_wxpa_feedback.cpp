#include "main_cgi.h"

class CgiWxpaFeedback: public MainCgi
{
     public:
		CgiWxpaFeedback() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			if(m_user_info.m_user_type != USER_TYPE_DEMAND_SIDE)
			{
				DoReply(CGI_RET_CODE_NO_PREVILEDGES);
				LOG4CPLUS_ERROR(logger, "add wxpa feedback failed, not demand side user, uin="<<m_user_info.m_uin);
				return true;
			}


			std::string strWX = (string)GetInput().GetValue("wx");
			uint32_t dwReason = (uint32_t)GetInput().GetValue("reason");
			std::string strReasonExtra = (string)GetInput().GetValue("reason_extra");
			EMPTY_STR_RETURN(strWX);
			ZERO_INT_RETURN(dwReason);
	
			std::string strErrMsg = "";
			WXPublicAccountFeedBack stWXPublicAccountFeedBack(m_table_name_wx_public_account_feedback);
			stWXPublicAccountFeedBack.m_id = GenerateTransID(ID_TYPE_WX_PUBLIC_ACCNOUT_FEEDBACK);
			stWXPublicAccountFeedBack.m_to_wx = strWX;
			stWXPublicAccountFeedBack.m_from_uin =  m_user_info.m_uin;
			stWXPublicAccountFeedBack.m_reason = dwReason;
			stWXPublicAccountFeedBack.m_reason_extra = strReasonExtra;
			stWXPublicAccountFeedBack.m_ts = time(0);
			if(stWXPublicAccountFeedBack.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stWXPublicAccount.UpdateToDB failed, errmsg="<<strErrMsg);
				return true;
			}
			
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiWxpaFeedback cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




