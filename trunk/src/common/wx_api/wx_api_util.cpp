#include "wx_api_util.h"
#include "tinyxml/tinyxml.h"
#include "WXBizMsgCrypt.h"
#include <openssl/sha.h>

namespace common
{
	namespace wxapi
	{
		static log4cplus::Logger logger = log4cplus::Logger::getInstance("WXAPI_UTIL");
	
		static int ParseWXRspXml2Map(const std::string& strRspXml
								, std::map<std::string, std::string>& mapRsp
								, std::string& strErrMsg)
		{
			LOG4CPLUS_DEBUG(logger, "xml="<<strRspXml);
            //parse rsp to Map
            mapRsp.clear();

            TiXmlDocument *doc = new TiXmlDocument();
            if (NULL == doc->Parse((strRspXml + "\n").c_str()))
            {
                strErrMsg = "TiXmlDocument->Parse failed!, xml=" + strRspXml;
                return -1;
            }

            TiXmlElement *node = doc->RootElement();
            if (!node)
            {
                strErrMsg = "RootElement not exists1, xml=" + strRspXml;
                return -1;
            }
            std::string strTagKey;
            std::string strTagValue;
            std::ostringstream oss;
            node = node->FirstChildElement();
            while(node)
            {
                strTagKey = node->Value();

                oss.str("");
                oss<<node->GetText();
                strTagValue = oss.str();
                if(strTagValue == "")
                {
                    LOG4CPLUS_DEBUG(logger, "key="<<strTagKey<<", value NULL, skip");
                    node = node->NextSiblingElement();
                    continue;
                }

                LOG4CPLUS_DEBUG(logger, strTagKey<<"="<<strTagValue);
                mapRsp[strTagKey] = strTagValue;
                node = node->NextSiblingElement();
           }

            LOG4CPLUS_DEBUG(logger, "ParseWXRspXML2Map succ");
            return 0;
		}
	
		int WXAPIUtil::WXAPIMsgDecrypt(const std::string& strMsgSignature, const std::string& strTimestamp,
													const std::string& strNonce, const std::string& strPostData,
													const std::string& strToken, const std::string& strCryptkey,
													const std::string& strAppid, std::map<std::string, std::string>& mapMsg)
		{
			::Tencent::WXBizMsgCrypt stWXBizMsgCrypt(strToken, strCryptkey, strAppid);

            std::string strXml = "";
            int iRet = stWXBizMsgCrypt.DecryptMsg(strMsgSignature, strTimestamp, strNonce, strPostData, strXml);
            if (iRet < 0)
            {
                LOG4CPLUS_ERROR(logger, "WXBizMsgCrypt.DecryptMsg failed!");
                return -1;
            }

            LOG4CPLUS_DEBUG(logger, "DecryptMsg succ , xml="<<strXml);

            mapMsg.clear();
            std::string strErrMsg = "";
            iRet = ParseWXRspXml2Map(strXml, mapMsg, strErrMsg);
            if (iRet < 0)
            {
                LOG4CPLUS_ERROR(logger, "parse xml to map failed, errmsg="<<strErrMsg);
                return -1;
            }

            return 0;
		}
		
		int WXAPIUtil::WXAPIMsgEncrypt(const std::string& strReplyMsg, const std::string& strTimestamp,
										const std::string& strNonce, const std::string& strToken,
										const std::string& strCryptkey, const std::string& strAppid,
										std::string& strEncryptMsg)
		{
			::Tencent::WXBizMsgCrypt stWXBizMsgCrypt(strToken, strCryptkey, strAppid);

            int iRet = stWXBizMsgCrypt.EncryptMsg(strReplyMsg, strTimestamp, strNonce, strEncryptMsg);
            if (iRet < 0)
            {
                LOG4CPLUS_ERROR(logger, "WXBizMsgCrypt.EncryptMsg failed!");
                return -1;
            }

            return 0;
		}


		bool WXAPIUtil::CheckSignature(const std::string &strSignature, const std::string &strTimestamp,
                                                const std::string &strNonce, const std::string &strToken)
        {
            std::vector<std::string> vecKeys;
            vecKeys.push_back(strTimestamp);
            vecKeys.push_back(strNonce);
            vecKeys.push_back(strToken);
            std::sort(vecKeys.begin(), vecKeys.end());

            std::string strKeys = "";
            for(uint32_t i = 0; i < vecKeys.size(); i++)
            {
                strKeys += vecKeys[i];
            }

            unsigned char digest[20] = { 0 };
            SHA1((const unsigned char *) strKeys.c_str(), strKeys.length(), digest);
            std::string strResult;
            char sHexStr[32];

            for (uint32_t i = 0; i < sizeof(digest); i++)
            {
                snprintf(sHexStr, sizeof(sHexStr), "%x%x", ((int) digest[i] & 0xf0) >> 4, ((int) digest[i] & 0x0f));
                strResult.append(sHexStr);
            }

            return strResult == strSignature;
        }
	}
}








