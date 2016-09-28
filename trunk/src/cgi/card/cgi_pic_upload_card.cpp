#include "hoosho_cgi_card.h"

class CgiPicUploadCard: public HooshoCgiCard
{
     public:
        CgiPicUploadCard() : HooshoCgiCard(FLAG_POST_ONLY, "config.ini", "logger.properties", USER_LOGIN)
        {

        }

		bool InnerProcess()
		{
			string strPicData = (string)GetInput().GetFileData("pic_data");

			if(strPicData.empty())
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid pic data");
				return true;
			}

            CardPicture stCardPicture(m_table_name_card_picture);
            stCardPicture.m_id = GenerateTransID(ID_TYPE_PICTURE);
            if("" == stCardPicture.m_id)
            {
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "GenerateTransID failed!");
				return true;
            }

			stCardPicture.m_data = strPicData;
			LOG4CPLUS_DEBUG(logger, "pic.size="<<strPicData.size());
			stCardPicture.m_status =PICTURE_STATUS_PUBLIC;
			stCardPicture.m_uin ="";
			string strErrMsg = "";
			if(stCardPicture.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "stCardPicture.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			GetAnyValue()["pic_id"] = stCardPicture.m_id;
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiPicUploadCard cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}
