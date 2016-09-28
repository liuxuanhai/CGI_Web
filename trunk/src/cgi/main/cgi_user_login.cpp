#include "main_cgi.h"

class CgiUserLogin: public MainCgi
{
     public:
        CgiUserLogin() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_NO)
        {

        }
        
		bool InnerProcess()
		{
		    string strErrMsg = "";
			string strUin = (string)GetInput().GetValue("uin");
            string strPassword = (string)GetInput().GetValue("password");
            uint32_t dwUserType = (uint32_t)GetInput().GetValue("user_type");
            
			if(strUin.empty() || strPassword.empty())
			{
                DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "empty uin="<<strUin<<", or password="<<strPassword);
				return true;
			}

            //get password
			Password stPassword(m_table_name_password);
			stPassword.m_uin = strUin;
			int iRet = stPassword.SelectFromDB(strErrMsg);
			if(iRet == TableBase::TABLE_BASE_RET_NOT_EXIST)
			{
				DoReply(CGI_RET_CODE_NOT_REGISTER_YET);
				LOG4CPLUS_ERROR(logger, "not exists user, stPassword.SelectFromDB failed, errmsg="<<strErrMsg);
				return true;
			}
			
			if(iRet != TableBase::TABLE_BASE_RET_OK)
			{
                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "stPassword.SelectFromDB failed , errmsg="<<strErrMsg);
				return true;
			}

            //check password
			if(stPassword.m_password != strPassword)
			{
                DoReply(CGI_RET_CODE_INVALID_PASSWORD);
				LOG4CPLUS_ERROR(logger, "check password failed, real_password="<<stPassword.m_password+", req_password="<<strPassword);
				return true;
			}

			//check role 
			UserInfo stUserInfo(m_table_name_user_info);
			stUserInfo.m_uin = strUin;
			if(stUserInfo.SelectFromDB(strErrMsg) == TableBase::TABLE_BASE_RET_OK)
			{
				//用户可能只注册， 但没有完善userinfo， 所以这里做个弱判断就好了
				if(dwUserType != stUserInfo.m_user_type)
				{
					DoReply(CGI_RET_CODE_LOGIN_ROLE_ERR);
					LOG4CPLUS_ERROR(logger, "login role err, req_usertype="<<dwUserType<<", real_usertype="<<stUserInfo.m_user_type);
					return true;
				}
			}
		
			
		    //check ok,  generate session
		    lce::cgi::CHttpHeader& stHttpRspHeader =  GetHeader();
		    if(MakeAddReplyCookie(strUin, stHttpRspHeader, strErrMsg) < 0)
		    {

                DoReply(CGI_RET_CODE_SERVER_BUSY);
				LOG4CPLUS_ERROR(logger, "makeaddreply cookie failed, errmsg="<<strPassword);
				return true;
		    }

			LOG4CPLUS_DEBUG(logger, "one user login succ, uin="<<strUin);

            DoReply(CGI_RET_CODE_OK);
            return true;
	    }
};


int main()
{
    CgiUserLogin cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


