#include "main_cgi.h"

class CgiPicDownloadPrivate: public MainCgi
{
     public:
        CgiPicDownloadPrivate() : MainCgi(0, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
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

		void Return200(const string& strPicData)
		{
			GetHeader().AddHeader("Cache-Control: max-age=0, must-revalidate");
			GetHeader().SetContentType("image/png");

			GetHeader().Output();
			if(!std::cout.good())
			{
				std::cout.clear();
				std::cout.flush();
			}

			std::cout<<strPicData;

			return;
		}


		bool InnerProcess()
		{
			string strPicId = (string)GetInput().GetValue("pic_id");

			if(strPicId.empty())
			{
				Return404();
				LOG4CPLUS_ERROR(logger, "invalid pic id");
				return true;
			}

            CommonPicture stCommonPicture(m_table_name_pic);
            stCommonPicture.m_id = strPicId;

			string strErrMsg = "";
			if(stCommonPicture.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
			    Return404();
				LOG4CPLUS_ERROR(logger, "stCommonPicture.SelectFromDB failed, errmsg="<<strErrMsg);
				return true;
			}
			
			if(stCommonPicture.m_type == PICTURE_PRIVATE_TYPE_YES)
			{
				if(m_user_info.m_uin != stCommonPicture.m_owner_uin)
				{
					Return403();
					LOG4CPLUS_ERROR(logger, "no previledges, stCommonPicture.owner="<<stCommonPicture.m_owner_uin
								<<", req_uin="<<m_user_info.m_uin
								<<", fuck out!!!");
					return true;
				}
				
			}
			
			LOG4CPLUS_DEBUG(logger, "read pic.size="<<stCommonPicture.m_data.size());
            Return200(stCommonPicture.m_data);
            
			return true;
		}
};


int main()
{
    CgiPicDownloadPrivate cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

