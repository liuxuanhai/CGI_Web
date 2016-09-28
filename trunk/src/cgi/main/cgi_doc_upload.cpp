#include "main_cgi.h"

class CgiPicUploadPrivate: public MainCgi
{
     public:
        CgiPicUploadPrivate() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			string strDocData = (string)GetInput().GetFileData("doc_data");
			uint32_t dwType = (uint32_t)GetInput().GetValue("type");
			uint32_t dwSource = (uint32_t)GetInput().GetValue("source");
			
			if(strDocData.empty())
			{
				DoReply(CGI_RET_CODE_EMPTY_CONTENT);
				LOG4CPLUS_ERROR(logger, "invalid doc data");
				return true;
			}
			
			if(dwType < DOCUMENT_TYPE_TXT || dwType > DOCUMENT_TYPE_EXCEL)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid type param="<<dwType);
				return true;
			}
			
			if(dwSource < PLATFORM_SOURCE_MAIN_WEBSITE || dwSource > PLATFORM_SOURCE_WX_CARD)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid source param="<<dwSource);
				return true;
			}
	
            Document stDocument(m_table_name_doc);
            stDocument.m_id = GenerateTransID(ID_TYPE_DOC);
            if("" == stDocument.m_id)
            {
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "GenerateTransID failed!");
				return true;
            }

			stDocument.m_type = dwType;
			stDocument.m_data = strDocData;
			stDocument.m_source = dwSource;
			stDocument.m_owner_uin = m_user_info.m_uin;

			LOG4CPLUS_DEBUG(logger, "doc.size="<<strDocData.size());
			
			string strErrMsg = "";
			if(stDocument.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "stCommonPicture.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			GetAnyValue()["doc_id"] = stDocument.m_id;
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

