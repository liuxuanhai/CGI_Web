#include "main_cgi.h"
#include "image_helper.h"

class CgiPicUpload: public MainCgi
{
     public:
        CgiPicUpload() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			string strPicData = (string)GetInput().GetFileData("pic_data");
			uint32_t dwSource = (uint32_t)GetInput().GetValue("source");
			uint32_t dwX = (uint32_t)GetInput().GetValue("x");
			uint32_t dwY = (uint32_t)GetInput().GetValue("y");
			uint32_t dwW = (uint32_t)GetInput().GetValue("w");
			uint32_t dwH = (uint32_t)GetInput().GetValue("h");
			

			LOG4CPLUS_DEBUG(logger, "source pic.size="<<strPicData.size());
			LOG4CPLUS_DEBUG(logger, "req_param=(x,y,w,h)=("<<dwX<<", "<<dwY<<", "<<dwW<<", "<<dwH<<")");
			
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

			if(dwW != 0 && dwH != 0)
			{
				//前端带上来长度和宽度,意味着需要裁剪, 使用ImageHelper对前端的裁剪需求进行裁剪		
				//image helper
				ImageHelper ih; 
				int iRet = ih.build((char*)strPicData.data(), strPicData.size());
				if(iRet != 0)
				{
						DoReply(CGI_RET_CODE_INVALID_PARAM);
						LOG4CPLUS_ERROR(logger, "invalid pic data, ImageHelper::build return "<<iRet);
						return true;
				}

				LOG4CPLUS_DEBUG(logger, "source{pic.size="<<strPicData.size()
									<<", width="<<ih.get_width()
									<<", height="<<ih.get_height()
									<<", type="<<ImageHelper::type_string((ImageHelper::ImageType)ih.get_format())
									<<"}");

				strPicData = "";
				iRet = ih.crop(dwX, dwY, dwW, dwH, ImageHelper::T_JPEG, strPicData);
				if(iRet != 0)
				{
						DoReply(CGI_RET_CODE_SERVER_BUSY);
						LOG4CPLUS_ERROR(logger, "ImageHelper::corp return "<<iRet);
						return true;
				}
			}
	
            CommonPicture stCommonPicture(m_table_name_pic);
            stCommonPicture.m_id = GenerateTransID(ID_TYPE_PIC);
            if("" == stCommonPicture.m_id)
            {
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "GenerateTransID failed!");
				return true;
            }

			stCommonPicture.m_type = PICTURE_PRIVATE_TYPE_NO;
			stCommonPicture.m_data = strPicData;
			stCommonPicture.m_source = dwSource;
			stCommonPicture.m_owner_uin = "";

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
    CgiPicUpload cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}
