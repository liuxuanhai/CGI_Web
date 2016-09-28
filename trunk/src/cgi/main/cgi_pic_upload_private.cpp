#include "main_cgi.h"

class CgiPicUploadPrivate: public MainCgi
{
     public:
        CgiPicUploadPrivate() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			string strPicData = (string)GetInput().GetFileData("pic_data");
			uint32_t dwSource = (uint32_t)GetInput().GetValue("source");
			
			if(strPicData.empty())
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid pic data");
				return true;
			}
			
			if(dwSource < PLATFORM_SOURCE_MAIN_WEBSITE || dwSource > PLATFORM_SOURCE_WX_CARD)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid source param="<<dwSource);
				return true;
			}
	
            CommonPicture stCommonPicture(m_table_name_pic);
            stCommonPicture.m_id = GenerateTransID(ID_TYPE_PIC);
            if("" == stCommonPicture.m_id)
            {
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "GenerateTransID failed!");
				return true;
            }

			stCommonPicture.m_type = PICTURE_PRIVATE_TYPE_YES;
			stCommonPicture.m_data = strPicData;
			stCommonPicture.m_source = dwSource;
			stCommonPicture.m_owner_uin = m_user_info.m_uin;

			LOG4CPLUS_DEBUG(logger, "pic.size="<<strPicData.size());
			
			string strErrMsg = "";
			if(stCommonPicture.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "stCommonPicture.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			GetAnyValue()["pic_id"] = stCommonPicture.m_id;
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiPicUploadPrivate cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}
