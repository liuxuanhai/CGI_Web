#include "wx_api_menu.h"
#include "jsoncpp/json.h"

namespace common
{
    namespace wxapi
    {
        int WXAPIMenu::CreateMenu(const std::string &strPAAccessToken, const std::string &strMenuContent, int& iErrcode)
        {
            //https://api.weixin.qq.com/cgi-bin/menu/create?access_token=ACCESS_TOKEN
            std::string strAPI = "cgi-bin/menu/create";
            strAPI += "?access_token=" + strPAAccessToken;

            std::string strRspBody = "";
            int iRet = HttpsPost("api.weixin.qq.com", strAPI, strMenuContent, strRspBody);
            if(iRet < 0)
            {
                LOG4CPLUS_ERROR(logger, "CreateMenu failed, msg=HttpsPost failed");
                return -1;
            }
            if(strRspBody.empty())
            {
                LOG4CPLUS_ERROR(logger, "CreateMenu failed, msg=strRspBody empty");
                return -1;
            }
            Json::Value oJson;
            Json::Reader reader;
            if(!reader.parse(strRspBody, oJson, false))
            {
                LOG4CPLUS_ERROR(logger, "CreateMenu failed, msg=parse json error");
                return -1;
            }
            if(oJson.hasKey("errcode") && oJson["errcode"].asInt() != 0)
            {
                iErrcode = oJson["errcode"].asInt();
                LOG4CPLUS_ERROR(logger, "CreateMenu failed, msg=wx return errcode="<<oJson["errcode"].asInt()<<" ,errmsg="<<errcode2str(iErrcode));
                return -1;
            }

            LOG4CPLUS_DEBUG(logger, "CreateMenu succ");
            iErrcode = 0;
            return 0;
        }

        int WXAPIMenu::GetMenu(const std::string &strPAAccessToken, std::string &strMenuContent, int& iErrcode)
        {
            //https://api.weixin.qq.com/cgi-bin/menu/get?access_token=ACCESS_TOKEN
            std::string strAPI = "cgi-bin/menu/get";
            strAPI += "?access_token=" + strPAAccessToken;

            std::string strRspBody = "";
            int iRet = HttpsGet("api.weixin.qq.com", strAPI, strRspBody);
            if(iRet < 0)
            {
                LOG4CPLUS_ERROR(logger, "GetMenu failed, msg=HttpsGet failed");
                return -1;
            }
            if(strRspBody.empty())
            {
                LOG4CPLUS_ERROR(logger, "GetMenu failed, msg=strRspBody empty");
                return -1;
            }
            Json::Value oJson;
            Json::Reader reader;
            if(!reader.parse(strRspBody, oJson, false))
            {
                LOG4CPLUS_ERROR(logger, "GetMenu failed, msg=parse json error");
                return -1;
            }
            if(oJson.hasKey("errcode") && oJson["errcode"].asInt() != 0)
            {
                iErrcode = oJson["errcode"].asInt();
                LOG4CPLUS_ERROR(logger, "CreateMenu failed, msg=wx return errcode="<<oJson["errcode"].asInt()<<" ,errmsg="<<errcode2str(iErrcode));
                return -1;
            }

            strMenuContent = strRspBody;
            LOG4CPLUS_DEBUG(logger, "GetMenu succ, menu content="<<strMenuContent);
            iErrcode = 0;
            return 0;
        }

        int WXAPIMenu::DeleteMenu(const std::string &strPAAccessToken, int& iErrcode)
        {
            //https://api.weixin.qq.com/cgi-bin/menu/delete?access_token=ACCESS_TOKEN
            std::string strAPI = "cgi-bin/menu/delete";
            strAPI += "?access_token=" + strPAAccessToken;

            std::string strRspBody = "";
            int iRet = HttpsGet("api.weixin.qq.com", strAPI, strRspBody);
            if(iRet < 0)
            {
                LOG4CPLUS_ERROR(logger, "DeleteMenu failed, msg=HttpsGet failed");
                return -1;
            }
            if(strRspBody.empty())
            {
                LOG4CPLUS_ERROR(logger, "DeleteMenu failed, msg=strRspBody empty");
                return -1;
            }
            Json::Value oJson;
            Json::Reader reader;
            if(!reader.parse(strRspBody, oJson, false))
            {
                LOG4CPLUS_ERROR(logger, "DeleteMenu failed, msg=parse json error");
                return -1;
            }
            if(oJson.hasKey("errcode") && oJson["errcode"].asInt() != 0)
            {
                iErrcode = oJson["errcode"].asInt();
                LOG4CPLUS_ERROR(logger, "CreateMenu failed, msg=wx return errcode="<<oJson["errcode"].asInt()<<" ,errmsg="<<errcode2str(iErrcode));
                return -1;
            }

            LOG4CPLUS_DEBUG(logger, "DeleteMenu succ");
            iErrcode = 0;
            return 0;
        }
    }
}
