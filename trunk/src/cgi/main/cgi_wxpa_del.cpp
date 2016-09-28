#include "main_cgi.h"

class CgiWxpaDel: public MainCgi
{
     public:
		CgiWxpaDel() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			if(m_user_info.m_oper_id.empty())
			{
				DoReply(CGI_RET_CODE_NOT_OPER);
				LOG4CPLUS_ERROR(logger, "del wxpa failed, not oper");
				return true;
			}

			std::string strWX = (string)GetInput().GetValue("wx");
			EMPTY_STR_RETURN(strWX);
			
			//wxpa exists ?
			std::string strErrMsg = "";
			WXPublicAccount stWXPublicAccount(m_table_name_wx_public_account);
			stWXPublicAccount.m_wx = strWX;
			int iRet = stWXPublicAccount.SelectFromDB(strErrMsg);
			if(iRet != TableBase::TABLE_BASE_RET_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stWXPublicAccount.SelectFromDB failed , errmsg="<<strErrMsg);
				return true;
			}

			if(stWXPublicAccount.m_oper_id != m_user_info.m_oper_id)
			{
				DoReply(CGI_RET_CODE_NO_PREVILEDGES);
				LOG4CPLUS_ERROR(logger, "del wxpa failed, wx="<<strWX<<", owner_oper_id="<<stWXPublicAccount.m_oper_id
							<<", not req_oper_id="<<m_user_info.m_oper_id);
				return true;
			}
			
			// is in using ?
			OperOrderReverseWithWx stOperOrderReverseWithWx(m_table_name_oper_order_reverse);
			stOperOrderReverseWithWx.m_wx = strWX;
			iRet = stOperOrderReverseWithWx.SelectFromDB(strErrMsg);
			if(iRet != TableBase::TABLE_BASE_RET_OK && iRet != TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stOperOrderReverseWithWx.SelectFromDB failed , errmsg="<<strErrMsg);
				return true;
			}
			if(!stOperOrderReverseWithWx.m_oper_order_reverse_list.empty())
			{
				DoReply(CGI_RET_CODE_WXPA_USING);
				LOG4CPLUS_ERROR(logger, "del failed, ing oper order, wx="<<strWX);
				return true;
			}

			//del wxpa
			if(stWXPublicAccount.DeleteFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stWXPublicAccount.DeleteFromDB failed, errmsg="<<strErrMsg);
				return true;
			}

			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiWxpaDel cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




