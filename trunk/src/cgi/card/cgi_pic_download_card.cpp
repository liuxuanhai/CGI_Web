#include "hoosho_cgi_card.h"

class CgiPicDownloadCard: public HooshoCgiCard
{
     public:
        CgiPicDownloadCard() : HooshoCgiCard(0, "config_card.ini", "logger.properties_card",USER_LOGIN_NOLOGIN)
        {

        }

    //image/png
        void Return404()
        { 
        	lce::cgi::CHttpHeader& stHttpRspHeader = GetHeader();
        	stHttpRspHeader.AddHeader("Status: 404");
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

            CardPicture stPicture(m_table_name_card_picture);
            stPicture.m_id = strPicId;
			string strErrMsg = "";
			if(stPicture.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
			{
			    Return404();
				LOG4CPLUS_ERROR(logger, "stPicture.SelectFromDB failed, errmsg="<<strErrMsg);
				return true;
			}
			if(stPicture.m_status!=PICTURE_STATUS_PUBLIC)
			{
				string strUin = m_user_info.m_uin;
				if(strUin.empty())
				{
					Return404();
					LOG4CPLUS_ERROR(logger, "stPicture no permittion");
					return true;
				}
				if(strUin != stPicture.m_uin)
				{
					Return404();
					LOG4CPLUS_ERROR(logger, "stPicture no permittion");
					return true;
				}

			}
			LOG4CPLUS_DEBUG(logger, "read pic.size="<<stPicture.m_data.size());
            Return200(stPicture.m_data);
			return true;

		}
};


int main()
{
    CgiPicDownloadCard cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

