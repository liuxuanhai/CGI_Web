#include "main_cgi.h"
class CgiSendVC: public MainCgi
{
     public:
        CgiSendVC() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_NO)
        {

        }
        
		bool InnerProcess()
		{
		    string strUin = (string)GetInput().GetValue("uin");
            if(strUin.empty())
            {
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid uin="<<strUin);
				return true;
            }
			
            string strErrMsg = "";
            Password stPassword(m_table_name_password);
            stPassword.m_uin = strUin;
			int iRet = stPassword.SelectFromDB(strErrMsg);
			if(iRet < 0)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
                LOG4CPLUS_ERROR(logger, "stPassword.SelectFromDB failed, errmsg="<<strErrMsg);
                return true;
			}
			
            if(iRet != TableBase::TABLE_BASE_RET_NOT_EXIST)
            {
                 DoReply(CGI_RET_CODE_ALREADY_EXISTS);
                 LOG4CPLUS_ERROR(logger, "db user_password already exists, uin="<<strUin);
                 return true;
             }

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
};


int main()
{
    CgiSendVC cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

