#include "wx_api_message.h"
#include "jsoncpp/json.h"

namespace common {
    namespace wxapi {
        int WXAPIMessage::SendTemplateMessage(const std::string &strPAAccessToken, const std::string &strPostData,
                                              int &iErrcode)
        {
            //https://api.weixin.qq.com/cgi-bin/message/template/send?access_token=ACCESS_TOKEN
            std::string strAPI = "cgi-bin/message/template/send";
            strAPI += "?access_token=" + strPAAccessToken;

            std::string strRspBody = "";
            int iRet = HttpsPost("api.weixin.qq.com", strAPI, strPostData, strRspBody);
            if (iRet < 0)
            {
                LOG4CPLUS_ERROR(logger, "SendTemplateMessage failed, msg=HttpsPost failed");
                return -1;
            }
            if (strRspBody.empty())
            {
                LOG4CPLUS_ERROR(logger, "SendTemplateMessage failed, msg=strRspBody empty");
                return -1;
            }
            Json::Value oJson;
            Json::Reader reader;
            if (!reader.parse(strRspBody, oJson, false))
            {
                LOG4CPLUS_ERROR(logger, "SendTemplateMessage failed, msg=parse json error");
                return -1;
            }
            if (oJson.hasKey("errcode") && oJson["errcode"].asInt() != 0)
            {
                iErrcode = oJson["errcode"].asInt();
                LOG4CPLUS_ERROR(logger, "SendTemplateMessage failed, msg=wx return errcode=" << oJson["errcode"].asInt() <<
                                        " ,errmsg=" << errcode2str(iErrcode));
                return -1;
            }

            LOG4CPLUS_DEBUG(logger, "SendTemplateMessage succ");
            iErrcode = 0;
            return 0;
        }

        int WXAPIMessage::SendMessage(const std::string &strPAAccessToken, const std::string &strPostData, int &iErrcode)
        {
            //https://api.weixin.qq.com/cgi-bin/message/mass/send?access_token=ACCESS_TOKEN
            std::string strAPI = "cgi-bin/message/mass/send";
            strAPI += "?access_token=" + strPAAccessToken;

            std::string strRspBody = "";
            int iRet = HttpsPost("api.weixin.qq.com", strAPI, strPostData, strRspBody);
            if (iRet < 0)
            {
                LOG4CPLUS_ERROR(logger, "SendMessage failed, msg=HttpsPost failed");
                return -1;
            }
            if (strRspBody.empty())
            {
                LOG4CPLUS_ERROR(logger, "SendMessage failed, msg=strRspBody empty");
                return -1;
            }
            Json::Value oJson;
            Json::Reader reader;
            if (!reader.parse(strRspBody, oJson, false))
            {
                LOG4CPLUS_ERROR(logger, "SendMessage failed, msg=parse json error");
                return -1;
            }
            if (oJson.hasKey("errcode") && oJson["errcode"].asInt() != 0)
            {
                iErrcode = oJson["errcode"].asInt();
                LOG4CPLUS_ERROR(logger, "SendMessage failed, msg=wx return errcode=" << oJson["errcode"].asInt() <<
                                        " ,errmsg=" << errcode2str(iErrcode));
                return -1;
            }

            LOG4CPLUS_DEBUG(logger, "SendMessage succ");
            iErrcode = 0;
            return 0;
        }

        int WXAPIMessage::SendPreviewMessage(const std::string &strPAAccessToken, const std::string &strPostData, int &iErrcode)
        {
            //https://api.weixin.qq.com/cgi-bin/message/mass/preview?access_token=ACCESS_TOKEN
            std::string strAPI = "cgi-bin/message/mass/preview";
            strAPI += "?access_token=" + strPAAccessToken;

            std::string strRspBody = "";
            int iRet = HttpsPost("api.weixin.qq.com", strAPI, strPostData, strRspBody);
            if (iRet < 0)
            {
                LOG4CPLUS_ERROR(logger, "SendPreviewMessage failed, msg=HttpsPost failed");
                return -1;
            }
            if (strRspBody.empty())
            {
                LOG4CPLUS_ERROR(logger, "SendPreviewMessage failed, msg=strRspBody empty");
                return -1;
            }
            Json::Value oJson;
            Json::Reader reader;
            if (!reader.parse(strRspBody, oJson, false))
            {
                LOG4CPLUS_ERROR(logger, "SendPreviewMessage failed, msg=parse json error");
                return -1;
            }
            if (oJson.hasKey("errcode") && oJson["errcode"].asInt() != 0)
            {
                iErrcode = oJson["errcode"].asInt();
                LOG4CPLUS_ERROR(logger, "SendPreviewMessage failed, msg=wx return errcode=" << oJson["errcode"].asInt() <<
                                        " ,errmsg=" << errcode2str(iErrcode));
                return -1;
            }

            LOG4CPLUS_DEBUG(logger, "SendPrevieweMessage succ");
            iErrcode = 0;
            return 0;
        }
    }
}
