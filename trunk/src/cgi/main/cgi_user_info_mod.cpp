#include "main_cgi.h"

class CgiUserInfoMod: public MainCgi
{
     public:
        CgiUserInfoMod() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }
        
		bool InnerProcess()
		{
            string strNick = (string)GetInput().GetValue("nick");
            string strHeadPortraitUrl = (string)GetInput().GetValue("headportrait_url");
            string strContactorName = (string)GetInput().GetValue("contactor_name");
            string strContactorPhoneNO = (string)GetInput().GetValue("contactor_phone_no");
            string strContactorWX = (string)GetInput().GetValue("contactor_wx");
            string strContactorQQ = (string)GetInput().GetValue("contactor_qq");
            string strOrganizationName = (string)GetInput().GetValue("organization_name");

			bool bChanged = false;
			if(!strNick.empty() && strNick != m_user_info.m_nick)
			{
				bChanged = true;
				m_user_info.m_nick = strNick;
			}

			if(!strHeadPortraitUrl.empty() && strHeadPortraitUrl != m_user_info.m_headportait_url)
			{
				bChanged = true;
				m_user_info.m_headportait_url = strHeadPortraitUrl;
			}
			
			if(!strContactorName.empty() && strContactorName != m_user_info.m_contactor_name)
			{
				bChanged = true;
				m_user_info.m_contactor_name = strContactorName;
			}
			
			if(!strContactorPhoneNO.empty() && strContactorPhoneNO != m_user_info.m_contactor_phone_no)
			{
				bChanged = true;
				m_user_info.m_contactor_phone_no = strContactorPhoneNO;
			}

			if(!strContactorWX.empty() && strContactorWX != m_user_info.m_contactor_wx)
			{
				bChanged = true;
				m_user_info.m_contactor_wx = strContactorWX;
			}

			if(!strContactorQQ.empty() && strContactorQQ != m_user_info.m_contactor_qq)
			{
				bChanged = true;
				m_user_info.m_contactor_qq = strContactorQQ;
			}

			if(!strOrganizationName.empty() && strOrganizationName != m_user_info.m_organization_name)
			{
				bChanged = true;
				m_user_info.m_organization_name = strOrganizationName;
			}

			if(bChanged)
			{
				string strErrMsg = "";
		        if(m_user_info.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
		        {
		            DoReply(CGI_RET_CODE_SERVER_BUSY);
					LOG4CPLUS_ERROR(logger, "m_user_info.UpdateToDB failed, errmsg="<<strErrMsg);
					return true;
		        }
			}

		    lce::cgi::CAnyValue stAnyValue;
		    m_user_info.ToAnyValue(stAnyValue);
		    
		    GetAnyValue()["user_info"] = stAnyValue;
            DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiUserInfoMod cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


