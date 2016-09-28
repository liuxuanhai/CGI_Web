#include "hoosho_cgi_card.h"

class CgiUserLoginCard: public HooshoCgiCard
{
     public:
        CgiUserLoginCard() : HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", NO_LOGIN)
        {

        }

		bool InnerProcess()
		{
		    string strErrMsg = "";
			string strUin = (string)GetInput().GetValue("uin");
            string strPassword = (string)GetInput().GetValue("password");

			if(strUin.empty() || strPassword.empty())
			{
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "empty uin="<<strUin<<", or password="<<strPassword);
				return true;
			}

            //get password
			CardUserPassword stCardUserPassword(m_table_name_card_user_password);
			stCardUserPassword.m_uin = strUin;
			if(stCardUserPassword.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
			{
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stCardUserPassword.SelectFromDB failed , errmsg="<<strErrMsg);
				return true;
			}

            //check password
			if(stCardUserPassword.m_password != strPassword)
			{
                DoReply(CGI_RET_CODE_INVALID_PASSWORD);
				LOG4CPLUS_ERROR(logger, "check password failed, real_password="<<stCardUserPassword.m_password+", req_password="<<strPassword);
				return true;
			}
			// get info
			CardUserInfo stCardUserInfo(m_table_name_card_user_info);
			stCardUserInfo.m_uin = strUin;
			if(stCardUserInfo.SelectFromDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_FATAL(logger, "init CardUserInfo failed, uin="<<strUin<<", errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			LOG4CPLUS_DEBUG(logger, "one req, CardUserInfo="<<m_user_info.ToString());

		    if(MakeAddReplyCookie(1, strUin, strErrMsg) < 0)
		    {

                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "makeaddreply cookie failed, errmsg="<<strPassword);
				return true;
		    }
            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiUserLoginCard cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}





