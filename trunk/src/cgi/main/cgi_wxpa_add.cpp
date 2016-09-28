#include "main_cgi.h"

class CgiWxpaAdd: public MainCgi
{
     public:
		CgiWxpaAdd() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			if(m_user_info.m_oper_id.empty())
			{
				DoReply(CGI_RET_CODE_NOT_OPER);
				LOG4CPLUS_ERROR(logger, "add wxpa failed, not oper");
				return true;
			}

			OperInfo stOperInfo(m_table_name_oper_info);
			stOperInfo.m_id = m_user_info.m_oper_id;
			std::string strErrMsg = "";
			if(stOperInfo.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stOperInfo.SelectFromDB failed , oper_id="<<stOperInfo.m_id);
				return true;
			}

			if(stOperInfo.m_status != OPERATOR_STATUS_VERIFY_OK)
			{
				DoReply(CGI_RET_CODE_LIMITED_OPER);
				LOG4CPLUS_ERROR(logger, "limited oper, uin="<<m_user_info.m_uin<<", oper_id="<<stOperInfo.m_id<<", status="<<stOperInfo.m_status);
				return true;
			}
		
			std::string strName = (string)GetInput().GetValue("name");
			std::string strWX = (string)GetInput().GetValue("wx");
			std::string strHeadPortraitUrl =(string)GetInput().GetValue("head_portrait_url");
			std::string strQRcodeUrl = (string)GetInput().GetValue("qrcode_url");
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

			EMPTY_STR_RETURN(strName);
			EMPTY_STR_RETURN(strWX);
			EMPTY_STR_RETURN(strHeadPortraitUrl);
			EMPTY_STR_RETURN(strQRcodeUrl);
			EMPTY_STR_RETURN(strIntroduction);
			ZERO_INT_RETURN(qwTag)
			if(dwIdentified)
			{
				EMPTY_STR_RETURN(strIdentifiedInfo);
			}

			//check exists wxpa already
			WXPublicAccount stWXPublicAccount(m_table_name_wx_public_account);
			stWXPublicAccount.m_wx = strWX;
			int iRet = stWXPublicAccount.SelectFromDB(strErrMsg);
			if(iRet != TableBase::TABLE_BASE_RET_OK && iRet!= TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stWXPublicAccount.SelectFromDB failed , errmsg="<<strErrMsg);
				return true;
			}
			if(iRet == TableBase::TABLE_BASE_RET_OK)
			{
				DoReply(CGI_RET_CODE_ALREADY_EXISTS);
				LOG4CPLUS_ERROR(logger, "add wxpa failed, already exists wx="<<strWX);
				return true;
			}

			//add wxpa to db
			stWXPublicAccount.m_wx = strWX;
			stWXPublicAccount.m_oper_id = m_user_info.m_oper_id;
			stWXPublicAccount.m_name = strName;
			stWXPublicAccount.m_headportrait_url = strHeadPortraitUrl;
			stWXPublicAccount.m_qrcode_url = strQRcodeUrl;
			stWXPublicAccount.m_introduction = strIntroduction;
			stWXPublicAccount.m_tag = qwTag;
			stWXPublicAccount.m_fans_num = dwFansNum;
			stWXPublicAccount.m_read_num = dwReadNum;
			stWXPublicAccount.m_identified = dwIdentified;
			stWXPublicAccount.m_identified_info = strIdentifiedInfo;
			stWXPublicAccount.m_price_multi_pic_text_first_soft = dwMFSoft;
			stWXPublicAccount.m_price_multi_pic_text_first_hard = dwMFHard;
			stWXPublicAccount.m_price_multi_pic_text_second_soft = dwMSSoft;
			stWXPublicAccount.m_price_multi_pic_text_second_hard = dwMSHard;
			stWXPublicAccount.m_price_multi_pic_text_third_soft = dwMTSoft;
			stWXPublicAccount.m_price_multi_pic_text_third_hard = dwMTHard;
			stWXPublicAccount.m_price_single_pic_text_soft = dwSSoft;
			stWXPublicAccount.m_price_single_pic_text_hard = dwSHard;
			stWXPublicAccount.m_status = WX_PUBLIC_ACCOUNT_VERIFY_PEDING;
			if(stWXPublicAccount.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
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
	CgiWxpaAdd cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




