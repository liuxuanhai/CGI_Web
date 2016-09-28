#include "hoosho_cgi_card.h"
#include "sms_https_req.h"
class CgiUserRegisterCard: public HooshoCgiCard
{
     public:
        CgiUserRegisterCard() : HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", NO_LOGIN)
        {

        }

        bool ProcessSendMail()
        {
            string strUin = (string)GetInput().GetValue("uin");
            EMPTY_STR_RETURN(strUin);
            string strErrMsg = "";
            CardUserPassword stCardUserPassword(m_table_name_card_user_password);
            stCardUserPassword.m_uin = strUin;

            CHECK_DB_NOT_EXIST(stCardUserPassword,strErrMsg,strUin);

            //get old
            string strVC = "";
            if(!VerifyCodeQuery(strUin, strVC, strErrMsg))
            {
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "query vc failed, errmsg="<<strErrMsg);
				return true;
            }

            //add new
            if(strVC.empty())
            {
                uint32_t dwNewVc = GetRandNum(10000, 99999);
                strVC = int_2_str(dwNewVc);
                LOG4CPLUS_DEBUG(logger, "vc not exists, so build one new vc="<<strVC);

                if(!VerifyCodeAdd(strUin, strVC, strErrMsg))
                {
                    DoReply(CGI_RET_CODE_SERVER_BUSY);
    				LOG4CPLUS_ERROR(logger, "add vc failed, errmsg="<<strErrMsg);
    				return true;
                }
            }

            //send mail
            if(SendRegisterMail(strUin, strVC, strErrMsg) < 0)
            {
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "SendRegisterMail failed, errmsg="<<strErrMsg);
				return true;
            }
            
            //succ
            DoReply(CGI_RET_CODE_OK);
            return true;
        }
        bool ProcessSendSMS()
        {
        	string strUin = (string)GetInput().GetValue("uin");
        	if(strUin.empty())
            {
                 DoReply(CGI_RET_CODE_INVALID_PARAM);
                 LOG4CPLUS_ERROR(logger, "invalid uin="<<strUin);
                 return true;
            }
        	string strErrMsg = "";
        	//check duplicate
        	CardUserPassword stCardUserPassword(m_table_name_card_user_password);
        	stCardUserPassword.m_uin = strUin;
        	CHECK_DB_NOT_EXIST(stCardUserPassword,strErrMsg,strUin);

            //get old
            string strVC = "";
            if(!VerifyCodeQuery(strUin, strVC, strErrMsg))
            {
                DoReply(CGI_RET_CODE_SERVER_BUSY);
       			LOG4CPLUS_ERROR(logger, "query vc failed, errmsg="<<strErrMsg);
       			return true;
            }

            //add new
            if(strVC.empty())
            {
            	uint32_t dwNewVc = GetRandNum(10000, 99999);
                strVC = int_2_str(dwNewVc);
                LOG4CPLUS_DEBUG(logger, "vc not exists, so build one new vc="<<strVC);

                if(!VerifyCodeAdd(strUin, strVC, strErrMsg))
                {
                   DoReply(CGI_RET_CODE_SERVER_BUSY);
                   LOG4CPLUS_ERROR(logger, "add vc failed, errmsg="<<strErrMsg);
                   return true;
                }
            }



             SMSHttpsReq smsSend(m_sms_api_host,m_sms_api_port,m_sms_api_account,m_sms_api_accountpwd,m_sms_api_appid,m_sms_api_version,m_sms_templateid);
             //send SMS


             if(smsSend.SMSUnifiedPost(strUin, strVC, strErrMsg)< 0)
             {
                  DoReply(CGI_RET_CODE_SERVER_BUSY);
                  LOG4CPLUS_ERROR(logger, "SendRegisterSMS failed, errmsg="<<strErrMsg);
                  return true;
             }

            //succ
            DoReply(CGI_RET_CODE_OK);
            return true;
        }
        bool ProcessDoRegister()
        {
            string strUin = (string)GetInput().GetValue("uin");
            string strPassword = (string)GetInput().GetValue("password");
            string strVC = (string)GetInput().GetValue("vc");
            uint32_t dwUsertype = (uint32_t)GetInput().GetValue("usertype");
            if(dwUsertype!=USER_NORMAL )
            {
            	 DoReply(CGI_RET_CODE_INVALID_PARAM);
            	LOG4CPLUS_ERROR(logger, "invalid usertype="<<dwUsertype);
            	return true;
            }
            if(strUin.empty() || strPassword.empty() || strVC.empty())
            {
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "empty uin="<<strUin<<", password="<<strPassword<<", vc="<<strVC);
				return true;
            }

            string strErrMsg = "";
            
            //check vc
            if(!VerifyCodeCheck(strUin, strVC, strErrMsg))
            {
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "check vc failed, errmsg="<<strErrMsg);
				return true;
            }

			//check duplicate
			CardUserPassword stCardUserPassword(m_table_name_card_user_password);
			stCardUserPassword.m_uin = strUin;
			CHECK_DB_NOT_EXIST(stCardUserPassword,strErrMsg,strUin);

            //save password
            stCardUserPassword.m_uin = strUin;
            stCardUserPassword.m_password = strPassword;
            UPDATE_DB(stCardUserPassword,strErrMsg);

			//create user_info
			CardUserInfo stCardUserInfo(m_table_name_card_user_info);
			stCardUserInfo.m_uin = strUin;
			stCardUserInfo.m_createtimestamp = time(0);
			stCardUserInfo.m_usertype=dwUsertype;
			stCardUserInfo.m_status = 0;
			stCardUserInfo.m_lasttimestamp =time(0);
			UPDATE_DB(stCardUserInfo,strErrMsg);
            //succ
            DoReply(CGI_RET_CODE_OK);
            return true;
        }
        
		bool InnerProcess()
		{
		    string strErrMsg = "";
			uint32_t dwReqType = (uint32_t)GetInput().GetValue("req_type"); // 1:send vc mail; 2: do register

			if(dwReqType != 1 && dwReqType != 2)
			{
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid req_type="<<dwReqType);
				return true;
			}

			if(1 == dwReqType)
			{
			    return ProcessSendMail();
			}
			else
			{
                return ProcessDoRegister();
			}
			
			return true;
		}
};


int main()
{
    CgiUserRegisterCard cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

