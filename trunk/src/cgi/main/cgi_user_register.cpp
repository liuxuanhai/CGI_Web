#include "main_cgi.h"

class CgiUserRegister: public MainCgi
{
     public:
        CgiUserRegister() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties",  CGI_NEED_LOGIN_NO)
        {

        }
        
		bool InnerProcess()
		{
		    string strUin = (string)GetInput().GetValue("uin");
            string strPassword = (string)GetInput().GetValue("password");
            string strVC = (string)GetInput().GetValue("vc");
            uint32_t dwUsertype = (uint32_t)GetInput().GetValue("user_type");
            
            if(strUin.empty() || strPassword.empty() || strVC.empty())
            {
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "empty uin="<<strUin<<", password="<<strPassword<<", vc="<<strVC);
				return true;
            }

            
            if(dwUsertype!=USER_TYPE_DEMAND_SIDE && dwUsertype!=USER_TYPE_SERVICE_SIDE)
            {
            	DoReply(CGI_RET_CODE_INVALID_PARAM);
            	LOG4CPLUS_ERROR(logger, "invalid usertype="<<dwUsertype);
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
			Password stPassword(m_table_name_password);
			stPassword.m_uin = strUin;
			if(stPassword.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				DoReply(CGI_RET_CODE_ALREADY_EXISTS);
				LOG4CPLUS_ERROR(logger, "db password already exists uin="<<strUin);
				return true;
			}

            //save password
            stPassword.m_uin = strUin;
            stPassword.m_password = strPassword;
            if(stPassword.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
            {
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stUserPassword.UpdateToDB failed, errmsg="<<strErrMsg);
				return true;
            }

			//create user_info
			UserInfo stUserInfo(m_table_name_user_info);
			stUserInfo.m_uin = strUin;
			stUserInfo.m_ts = time(0);
			stUserInfo.m_user_type=dwUsertype;

			if(stUserInfo.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stUserInfo.UpdateToDB failed, errmsg="<<strErrMsg);
				return true;
			}
            
            //succ
            DoReply(CGI_RET_CODE_OK);
            return true;
		}
};


int main()
{
    CgiUserRegister cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

