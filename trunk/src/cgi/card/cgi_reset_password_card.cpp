#include "hoosho_cgi_card.h"
#include "wx_https_req.h"
#include <stdio.h>

class CgiResetPasswordCard: public HooshoCgiCard
{
public:
	CgiResetPasswordCard() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", false)
	{

	}

	bool InnerProcess()
	{

		string strToken = (string) GetInput().GetValue("token");
		string strPassword_1 = (string) GetInput().GetValue("password_1");
		string strPassword_2 = (string) GetInput().GetValue("password_2");
		string strUin = (string) GetInput().GetValue("uin");
		string strErrMsg = "";
		string strCachedName = "";
		string strVC = "";
		EMPTY_STR_RETURN(strToken);
		EMPTY_STR_RETURN(strPassword_1);
		EMPTY_STR_RETURN(strPassword_2);
		EMPTY_STR_RETURN(strUin);
		if (strPassword_1 != strPassword_2)
		{
			LOG4CPLUS_ERROR(logger, "fail, password_1 != password_2,password_1="<<strPassword_1<<", password_2="<<strPassword_2);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		strCachedName  = "ResetPassword" + strUin;
		if (!VerifyCodeQuery(strCachedName, strVC, strErrMsg))
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "query vc failed, errmsg="<<strErrMsg);
			return true;
		}
		EMPTY_STR_RETURN_ECHO(strVC, strCachedName);

		if (strVC != strToken)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "Cached vc!=token,token="<<strToken<<", cached vc="<<strVC);
			return true;
		}

		CardUserPassword stCardUserPassword(m_table_name_card_user_password);
		stCardUserPassword.m_uin = strUin;
		CHECK_DB(stCardUserPassword, strErrMsg);

		//save password
		stCardUserPassword.m_password = strPassword_1;
		UPDATE_DB(stCardUserPassword, strErrMsg);

		if (!MemcachedDelete(strCachedName, strErrMsg))
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "delete reset password token failed, errmsg="<<strErrMsg);
			return true;
		}

		DoReply(CGI_RET_CODE_OK);
		return true;

	}
};

int main()
{
	CgiResetPasswordCard cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

