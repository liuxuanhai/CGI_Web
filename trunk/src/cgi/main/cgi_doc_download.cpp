#include "main_cgi.h"

class CgiDocDownload: public MainCgi
{
     public:
        CgiDocDownload() : MainCgi(0, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

    	//image/png
        void Return404()
        { 
        	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
        	stHttpRspHeader.AddHeader("Status: 404");
        	stHttpRspHeader.Output();
        }

        void Return403()
        { 
        	//no previledges
        	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
        	stHttpRspHeader.AddHeader("Status: 403");
        	stHttpRspHeader.Output();
        }

		void Return200(const string& strDocData, const string& strFileName)
		{
			GetHeader().AddHeader("Content-Disposition: attachment;filename=" + strFileName);
			GetHeader().SetContentType("application/octet-stream");
			GetHeader().Output();
			if(!std::cout.good())
			{
				std::cout.clear();
				std::cout.flush();
			}

			std::cout<<strDocData;

			return;
		}

		string GetDocFileName(const Document& doc)
		{
			string suffix = "";
			switch(doc.m_type)
			{
				case DOCUMENT_TYPE_TXT:  suffix = ".txt"; break;
				case DOCUMENT_TYPE_WORD:  suffix = ".doc"; break;
				case DOCUMENT_TYPE_PPT:  suffix = ".ppt"; break;
				case DOCUMENT_TYPE_EXCEL: suffix = ".xls";break;
				default: break;
			}

			return doc.m_id + suffix;
		}

		bool InnerProcess()
		{
			string strDocId = (string)GetInput().GetValue("doc_id");

			if(strDocId.empty())
			{
				Return404();
				LOG4CPLUS_ERROR(logger, "invalid pic id");
				return true;
			}

            Document stDocument(m_table_name_doc);
            stDocument.m_id = strDocId;

			string strErrMsg = "";
			if(stDocument.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
			    Return404();
				LOG4CPLUS_ERROR(logger, "stDocument.SelectFromDB failed, errmsg="<<strErrMsg);
				return true;
			}
	
			if(m_user_info.m_uin != stDocument.m_owner_uin)
			{
				Return403();
				LOG4CPLUS_ERROR(logger, "no previledges, stDocument.owner="<<stDocument.m_owner_uin
							<<", req_uin="<<m_user_info.m_uin
							<<", fuck out!!!");
				return true;
			}
				
			
			
			LOG4CPLUS_DEBUG(logger, "read doc.size="<<stDocument.m_data.size());
            Return200(stDocument.m_data, GetDocFileName(stDocument));
            
			return true;
		}
};


int main()
{
    CgiDocDownload cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

