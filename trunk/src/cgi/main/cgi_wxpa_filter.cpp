#include "main_cgi.h"

class CgiWxpaFilter: public MainCgi
{
     public:
		CgiWxpaFilter() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_NO)
        {

        }

		bool InnerProcess()
		{	
			//param check
			uint64_t qwTag = (uint64_t)GetInput().GetValue("tag");
			uint32_t dwWXIdentified = (uint32_t)GetInput().GetValue("wx_identified");
			uint32_t dwOffset = (uint32_t)GetInput().GetValue("offset");
			uint32_t dwLen = (uint32_t)GetInput().GetValue("len");

			ZERO_INT_RETURN(qwTag);
			ZERO_INT_RETURN(dwLen);

			uint32_t dwTotalNum = 0;
			std::string strErrMsg = "";
			WXPublicAccountWithTagAndCertifiedStatus stWXPublicAccountWithTagAndCetifiedStatus(m_table_name_wx_public_account);
			stWXPublicAccountWithTagAndCetifiedStatus.m_tag = qwTag;
			stWXPublicAccountWithTagAndCetifiedStatus.m_cetified_status = dwWXIdentified;
			stWXPublicAccountWithTagAndCetifiedStatus.m_limit_from = dwOffset;
			stWXPublicAccountWithTagAndCetifiedStatus.m_limit_len = dwLen;
			int iRet = stWXPublicAccountWithTagAndCetifiedStatus.SelectFromDB(dwTotalNum, strErrMsg);
			if(iRet != TableBase::TABLE_BASE_RET_OK && iRet != TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				LOG4CPLUS_ERROR(logger, "stWXPublicAccountWithTagAndCetifiedStatus.SelectFromDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			GetAnyValue()["total_num"] = dwTotalNum;
			GetAnyValue()["offset"] = dwOffset;
			GetAnyValue()["len"] = dwLen;

			for(size_t i=0; i<stWXPublicAccountWithTagAndCetifiedStatus.m_wx_public_account_list.size(); ++i)
			{
				lce::cgi::CAnyValue stAnyValue;
				stWXPublicAccountWithTagAndCetifiedStatus.m_wx_public_account_list[i].ToAnyValue(stAnyValue);
				GetAnyValue()["wxpa_list"].push_back(stAnyValue);	
			}
			
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiWxpaFilter cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




