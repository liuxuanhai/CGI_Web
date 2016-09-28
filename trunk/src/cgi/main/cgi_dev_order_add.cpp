#include "main_cgi.h"

class CgiDevOrderAdd: public MainCgi
{
     public:
		CgiDevOrderAdd() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			if(m_user_info.m_user_type != USER_TYPE_DEMAND_SIDE)
			{
				DoReply(CGI_RET_CODE_NO_PREVILEDGES);
				LOG4CPLUS_ERROR(logger, "add dev order failed, not demand side user, uin="<<m_user_info.m_uin);
				return true;
			}

			std::string strDevID =(string)GetInput().GetValue("dev_id");
			std::string strIntroduction =(string)GetInput().GetValue("introduction");
			std::string strDocUrl = (string)GetInput().GetValue("doc_url");
			std::string strLikePreviewUrl = (string)GetInput().GetValue("like_preview_url");
			std::string strLikeWebsiteUrl = (string)GetInput().GetValue("like_website_url");
			std::string strExpectInfo = (string)GetInput().GetValue("expect_info");
			EMPTY_STR_RETURN(strDevID);
			EMPTY_STR_RETURN(strIntroduction);
			EMPTY_STR_RETURN(strExpectInfo);

			string strErrMsg = "";
			//check dev 
			DeveloperInfo stDeveloperInfo(m_table_name_dev_info);
			stDeveloperInfo.m_id = strDevID;
			int iRet = stDeveloperInfo.SelectFromDB(strErrMsg);
			if(iRet == TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "stDeveloperInfo.SelectFromDB return not exists, errmsg="<<strErrMsg);
				return true;
			}
			else if(iRet != TableBase::TABLE_BASE_RET_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stDeveloperInfo.SelectFromDB failed, errmsg="<<strErrMsg);
				return true;
			}
			else if(stDeveloperInfo.m_status != DEVELOPER_STATUS_VERIFY_OK)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "dev is not verify passed state, dev_id="<<strDevID);
				return true;
			}
			
			
            DevOrder stDevOrder(m_table_name_dev_order);
			stDevOrder.m_id = GenerateTransID(ID_TYPE_ORDER_DEV);
			stDevOrder.m_need_uin = m_user_info.m_uin;
			stDevOrder.m_dev_id = strDevID;
			stDevOrder.m_introduction = strIntroduction;
			stDevOrder.m_doc_url= strDocUrl;
			stDevOrder.m_like_preview_url = strLikePreviewUrl;
			stDevOrder.m_like_website_url = strLikeWebsiteUrl;
			stDevOrder.m_expect_info = strExpectInfo;
			stDevOrder.m_status = DEV_ORDER_STATUS_VERIFY_PENDING;
			stDevOrder.m_create_ts = time(0);
			stDevOrder.m_dev_start_ts = 0;
			stDevOrder.m_dev_end_ts = 0;
			stDevOrder.m_cancel_ts = 0;
			if(stDevOrder.m_id.empty())
			{
				LOG4CPLUS_ERROR(logger, "generate dev_order id failed, fuck, add dev order failed");
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			
			if(stDevOrder.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "stDevOrder.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			GetAnyValue()["dev_order_id"] = stDevOrder.m_id;
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiDevOrderAdd cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




