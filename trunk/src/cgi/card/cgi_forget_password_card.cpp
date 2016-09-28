#include "hoosho_cgi_card.h"
#include "sms_https_req.h"
class CgiForgetPasswordCard: public HooshoCgiCard
{
public:
	CgiForgetPasswordCard() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", false)
	{

	}

	int ResetIDBuild(const string strUin, string &strID, string &strErrMsg)
	{

		string strResetPasswordID = "ResetPassword" + strUin;
		string strVC = "";
		if (!VerifyCodeQuery(strResetPasswordID, strVC, strErrMsg))
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "query vc failed, errmsg="<<strErrMsg);
			return true;
		}
		//add new
		if (strVC.empty())
		{
			uint32_t dwNewVc = GetRandNum(1000000, 9999999);
			strVC = int_2_str(dwNewVc);
			LOG4CPLUS_DEBUG(logger, "vc not exists, so build one new vc="<<strVC);

			if (!VerifyCodeAdd(strResetPasswordID, strVC, strErrMsg))
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "add vc failed, errmsg="<<strErrMsg);
				return true;
			}
		}
		strID = strVC;
		return 0;
	}


	bool ProcessSendMail()
	{
		string strUin = (string) GetInput().GetValue("uin");
		string strErrMsg = "";
		string strMail = strUin;
		string strID = "";
		string ret = "";
		EMPTY_STR_RETURN(strUin);

		CardUserInfo stCardUserInfo(m_table_name_card_user_info);
		stCardUserInfo.m_uin = strUin;
		CHECK_DB(stCardUserInfo, strErrMsg);

		if (ResetIDBuild(strUin, strID, strErrMsg) != 0)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "fail to call ResetIDBuild, errmsg="<<strErrMsg);
			return true;
		}
		if (SendRegisterMail(strMail, strID, strErrMsg) < 0)
		{
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			LOG4CPLUS_ERROR(logger, "SendRegisterMail failed, errmsg="<<strErrMsg);
			return true;
		}

		DoReply(CGI_RET_CODE_OK);
		return true;
	}



	bool InnerProcess()
	{
		string strErrMsg = "";
		uint32_t dwReqType = (uint32_t) GetInput().GetValue("req_type"); // 1:send vc sms; 2:send link -> email

		if (dwReqType != 1)
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid req_type="<<dwReqType);
			return true;
		}


		if (1 == dwReqType)
		{
			return ProcessSendMail();
		}
		else
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "invalid req_type="<<dwReqType<<",fuck big problem");
			return true;
		}

		return true;
	}
};

int main()
{
	CgiForgetPasswordCard cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

