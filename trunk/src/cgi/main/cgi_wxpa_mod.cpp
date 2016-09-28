#include "main_cgi.h"

class CgiWxpaMod: public MainCgi
{
     public:
		CgiWxpaMod() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			if(m_user_info.m_oper_id.empty())
			{
				DoReply(CGI_RET_CODE_NOT_OPER);
				LOG4CPLUS_ERROR(logger, "mod wxpa failed, not oper");
				return true;
			}

			std::string strWX = (string)GetInput().GetValue("wx");
			EMPTY_STR_RETURN(strWX);
			
			//field can be modified
			std::string strIntroduction = (string)GetInput().GetValue("introduction");
			uint64_t qwTag = (uint64_t)GetInput().GetValue("tag");
			uint32_t dwFansNum = (uint32_t)GetInput().GetValue("fans_num");
			uint32_t dwReadNum = (uint32_t)GetInput().GetValue("read_num");
			uint32_t dwIdentified = (uint32_t)GetInput().GetValue("identified");
			std::string strIdentifiedInfo = (string)GetInput().GetValue("identified_info");
			uint32_t dwMFSoft = (uint32_t)GetInput().GetValue("price_multi_pic_text_first_soft");
			uint32_t dwMFHard = (uint32_t)GetInput().GetValue("price_multi_pic_text_first_hard");
			uint32_t dwMSSoft = (uint32_t)GetInput().GetValue("price_multi_pic_text_second_soft");
			uint32_t dwMSHard = (uint32_t)GetInput().GetValue("price_multi_pic_text_second_hard");
			uint32_t dwMTSoft = (uint32_t)GetInput().GetValue("price_multi_pic_text_third_soft");
			uint32_t dwMTHard = (uint32_t)GetInput().GetValue("price_multi_pic_text_third_hard");
			uint32_t dwSSoft = (uint32_t)GetInput().GetValue("price_single_pic_text_soft");
			uint32_t dwSHard = (uint32_t)GetInput().GetValue("price_single_pic_text_hard");

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
				LOG4CPLUS_ERROR(logger, "mod wxpa failed, wx="<<strWX<<", owner_oper_id="<<stWXPublicAccount.m_oper_id
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
				LOG4CPLUS_ERROR(logger, "mod failed, ing oper order, wx="<<strWX);
				return true;
			}

			//check update
			bool bUpdate = false;
			if(!strIntroduction.empty() && stWXPublicAccount.m_introduction != strIntroduction)
			{
				bUpdate = true;
				stWXPublicAccount.m_introduction = strIntroduction;
			}

			if(qwTag != 0 && qwTag != stWXPublicAccount.m_tag)
			{
				bUpdate = true;
				stWXPublicAccount.m_tag = qwTag;
			}

			if(dwFansNum != 0 && dwFansNum != stWXPublicAccount.m_fans_num)
			{
				bUpdate = true;
				stWXPublicAccount.m_fans_num = dwFansNum;
			}

			if(dwReadNum != 0 && dwReadNum != stWXPublicAccount.m_read_num)
			{
				bUpdate = true;
				stWXPublicAccount.m_read_num = dwReadNum;
			}

			if(dwIdentified != stWXPublicAccount.m_identified)
			{
				if(dwIdentified)
				{
					EMPTY_STR_RETURN(strIdentifiedInfo);
					bUpdate = true;
					stWXPublicAccount.m_identified = dwIdentified;
					stWXPublicAccount.m_identified_info = strIdentifiedInfo;
				}
				else
				{
					stWXPublicAccount.m_identified = dwIdentified;
					stWXPublicAccount.m_identified_info = "";
				}
			}

			if(dwMFSoft != 0 && dwMFSoft != stWXPublicAccount.m_price_multi_pic_text_first_soft)
			{
				bUpdate = true;
				stWXPublicAccount.m_price_multi_pic_text_first_soft = dwMFSoft;
			}

			if(dwMFHard != 0 && dwMFHard != stWXPublicAccount.m_price_multi_pic_text_first_hard)
			{
				bUpdate = true;
				stWXPublicAccount.m_price_multi_pic_text_first_hard = dwMFHard;
			}

			if(dwMSSoft != 0 && dwMSSoft != stWXPublicAccount.m_price_multi_pic_text_second_soft)
			{
				bUpdate = true;
				stWXPublicAccount.m_price_multi_pic_text_second_soft = dwMSSoft;
			}

			if(dwMSHard != 0 && dwMSHard != stWXPublicAccount.m_price_multi_pic_text_second_hard)
			{
				bUpdate = true;
				stWXPublicAccount.m_price_multi_pic_text_second_hard = dwMSHard;
			}

			if(dwMTSoft != 0 && dwMTSoft != stWXPublicAccount.m_price_multi_pic_text_third_soft)
			{
				bUpdate = true;
				stWXPublicAccount.m_price_multi_pic_text_third_soft = dwMTSoft;
			}

			if(dwMTHard != 0 && dwMTHard != stWXPublicAccount.m_price_multi_pic_text_third_hard)
			{
				bUpdate = true;
				stWXPublicAccount.m_price_multi_pic_text_third_hard = dwMTHard;
			}

			if(dwSSoft != 0 && dwSSoft != stWXPublicAccount.m_price_single_pic_text_soft)
			{
				bUpdate = true;
				stWXPublicAccount.m_price_single_pic_text_soft = dwSSoft;
			}

			if(dwSHard != 0 && dwSHard != stWXPublicAccount.m_price_single_pic_text_hard)
			{
				bUpdate = true;
				stWXPublicAccount.m_price_single_pic_text_hard = dwSHard;
			}

			if(bUpdate)
			{
				//once update, goto verify pending state
				stWXPublicAccount.m_status = WX_PUBLIC_ACCOUNT_VERIFY_PEDING;
				if(stWXPublicAccount.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
				{
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					LOG4CPLUS_ERROR(logger, "stWXPublicAccount.UpdateToDB failed, errmsg="<<strErrMsg);
					return true;
				}
			}
			
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiWxpaMod cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




