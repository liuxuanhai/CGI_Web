#ifndef _HOOSHO_CGI_WX_MSG_CALLBACK_H_
#define _HOOSHO_CGI_WX_MSG_CALLBACK_H_

#include "cgi_wx_callback.h"
#include "wx_api_util.h"
#include "WXBizMsgCrypt.h"

class CgiWXMsgCallback:public CgiWXCallback
{
public:
    CgiWXMsgCallback(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
            :CgiWXCallback(qwFlag, strCgiConfigFile, strLogConfigFile, false)
    {

    }

    virtual bool InnerProcess()
    {
        std::string strSignature;
        std::string strTimestamp;
        std::string strNonce;
        std::string strEchostr;
        std::string strMsgSignature;

        std::string strQuery = (std::string)GetInput().GetQueryString();
        std::vector<std::string> vecQuery;
        lce::cgi::Split(strQuery, "&", vecQuery);
        for(uint32_t i = 0; i < vecQuery.size(); i++)
        {
            std::vector<std::string> vecParams;
            lce::cgi::Split(vecQuery[i], "=", vecParams);
            if(vecParams.size() != 2)
            {
                continue;
            }
            if (vecParams[0] == "signature")
                strSignature = vecParams[1];
            else if (vecParams[0] == "nonce")
                strNonce = vecParams[1];
            else if (vecParams[0] == "timestamp")
                strTimestamp = vecParams[1];
            else if (vecParams[0] == "echostr")
                strEchostr = vecParams[1];
            else if (vecParams[0] == "msg_signature")
                strMsgSignature = vecParams[1];
        }

        LOG4CPLUS_DEBUG(logger, "signature="<<strSignature<<", nonce="<<strNonce
                    <<", timestamp="<<strTimestamp<<", echostr="<<strEchostr
                    <<", msg_signature="<<strMsgSignature);

        ::common::wxapi::WXAPIUtil stWXAPIUtil;

        //校验参数
        if(!stWXAPIUtil.CheckSignature(strSignature, strTimestamp, strNonce, m_mp_token))
        {
            LOG4CPLUS_ERROR(logger, "WX Callback CheckSignature failed");
            SetOutputJson("error");
            DoReply(CGI_RET_CODE_OK);
            return true;
        }

        std::string strPostData = (std::string) GetInput().GetPostData();
        //微信验证 “mp平台配置的服务器地址” 的有效性
        // http://mp.weixin.qq.com/wiki/8/f9a0b8382e0b77d87b3bcc1ce6fbc104.html#.E7.AC.AC.E4.BA.8C.E6.AD.A5.EF.BC.9A.E9.AA.8C.E8.AF.81.E6.9C.8D.E5.8A.A1.E5.99.A8.E5.9C.B0.E5.9D.80.E7.9A.84.E6.9C.89.E6.95.88.E6.80.A7
        if(strPostData == "" && strEchostr != "")
        {
            LOG4CPLUS_DEBUG(logger, "WX Callback Test succ, just return");
            SetOutputJson(strEchostr);
            DoReply(CGI_RET_CODE_OK);
            return true;
        }

        //微信推送消息
        //首先解密消息
        std::map<std::string, std::string> mapMsg;
        int iRet;
        iRet = stWXAPIUtil.WXAPIMsgDecrypt(strMsgSignature, strTimestamp, strNonce, strPostData, m_mp_token, m_encoding_aes_key, m_appid, mapMsg);
        if (iRet < 0)
        {
            LOG4CPLUS_ERROR(logger, "WXAPIMsgDecrypt failed");
            DoReply(CGI_RET_CODE_SERVER_BUSY);
            return true;
        }


        string strMsgType = mapMsg["MsgType"];
        if (strMsgType == "")
        {
            LOG4CPLUS_ERROR(logger, "invalid wx msg_type="<<strMsgType);
            DoReply(CGI_RET_CODE_SERVER_BUSY);
            return true;
        }

        for(std::map<std::string, std::string>::iterator iter = mapMsg.begin(); iter != mapMsg.end(); iter++)
        {
            LOG4CPLUS_DEBUG(logger, ""<<(iter->first)<<"="<<(iter->second));
        }

        //处理消息

        if(strMsgType == "event")
        {
            string strEvent = mapMsg["Event"];

            //发送模板消息后，微信推送结果
            if(strEvent == "TEMPLATESENDJOBFINISH")
            {
                LOG4CPLUS_DEBUG(logger, "msgid="<<mapMsg["MsgID"]<<" send to openid="<<mapMsg["FromUserName"]
                                        <<mapMsg["Status"]);

            }
        }


        //无需回复信息 或 未知消息类型,直接回复空串
        SetOutputJson("");
        DoReply(CGI_RET_CODE_OK);
        return true;
    }

};

int main(int argc, char** argv)
{
    CgiWXMsgCallback cgi(0, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif