#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <algorithm>
#include "tinyxml/tinyxml.h"
#include "wx_https_req.h"
#include "comm_def_card.h"
#include "util/lce_util.h"
#include "WXBizMsgCrypt.h"
#include "jsoncpp/json.h"
#include "libmemcached/memcached.h"
//#include <openssl/sha.h>
static log4cplus::Logger wx_logger = log4cplus::Logger::getInstance("wx_https");

int WXHttpsReq::WXUnifiedOrder(const std::string& strAppid //�����˺�ID
		, const std::string& strMchId //�̻���
		, const std::string& strNonceString //  ����ַ�
		, const std::string& strBody //��Ʒ����
		, const std::string& strOutTradeNo //�̻�������
		, const std::string& strTotalFee //�ܽ��, ��λ: ��(rmb)
		, const std::string& strSpbillCreateIp //�ն�IP , Native֧�������΢��֧��API�Ļ���IP
		, const std::string& strNotifyUrl // ֪ͨ��ַ, ��΢��֧���첽֪ͨ�ص���ַ
		, const std::string& strProductId //��ƷID, trade_type=NATIVE���˲���ش�����idΪ��ά���а����ƷID���̻����ж��塣
		, const std::string& strAPISecret //API ��Կ
		, std::string& strPrepayId, std::string& strErrMsg)
{
	std::map<std::string, std::string> mapParams;
	mapParams["appid"] = strAppid;
	mapParams["body"] = strBody;
	mapParams["mch_id"] = strMchId;
	mapParams["nonce_str"] = strNonceString;
	mapParams["notify_url"] = strNotifyUrl;
	mapParams["product_id"] = strProductId;
	mapParams["out_trade_no"] = strOutTradeNo;
	mapParams["spbill_create_ip"] = strSpbillCreateIp;
	mapParams["total_fee"] = strTotalFee;
	mapParams["trade_type"] = "NATIVE";

	std::string strReqBody = BuildParamsByDictSort(mapParams, strAPISecret);
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttps("api.mch.weixin.qq.com", 443, "pay/unifiedorder", strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttps failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	std::map<std::string, std::string> mapRsp;
	if (ParseWXRspXml2Map(strRspBody, mapRsp, strInnerErrMsg) < 0)
	{
		strErrMsg = "ParseWXRspXml2Map failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	if (mapRsp["return_code"] != "SUCCESS")
	{
		strErrMsg = "return_code != SUCCESS,  rsp_xml=" + strRspBody + ", req_body=" + strReqBody;
		return -1;
	}

	if (mapRsp["result_code"] != "SUCCESS")
	{
		strErrMsg = "result_code != SUCCESS,  rsp_xml=" + strRspBody + ", req_body=" + strReqBody;
		return -1;
	}

	strPrepayId = mapRsp["prepay_id"];
	return 0;
}

int WXHttpsReq::WXAPIUploadMedia(const string& strMedia, const std::string& strFilename, const std::string& strReqType, const std::string& strMyAccessToken, std::string& strMediaId, int& intCreateTime, std::string& strErrMsg)
{
	std::string strReqBody = strMedia;
	// https://api.weixin.qq.com/cgi-bin/media/upload?access_token=ACCESS_TOKEN&type=TYPE
	std::string strAPI = "cgi-bin/media/upload?access_token=";
	strAPI += strMyAccessToken;
	strAPI += "&type=";
	strAPI += strReqType;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsUpLoadFile("api.weixin.qq.com", 443, strAPI, strReqBody, strFilename, "media", strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsUpLoadFile failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{

		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}
	//{"type":"TYPE","media_id":"MEDIA_ID","created_at":123456789}
	if (!oJson.hasKey("type"))
	{
		strErrMsg = "wx return invalid, type not found!! resp_body=" + strRspBody;
		return -1;
	}
	string strRspType = oJson["type"].asString();
	if (strRspType != strReqType)
	{
		strErrMsg = "wx return strRspType=" + strRspType + " is not equal to strReqType=" + strReqType;
		return -1;
	}
	if (!oJson.hasKey("media_id"))
	{
		strErrMsg = "wx return invalid, MEDIA_ID not found!! resp_body=" + strRspBody;
		return -1;
	}
	strMediaId = oJson["media_id"].asString();
	if (!oJson.hasKey("created_at"))
	{
		strErrMsg = "wx return invalid, created_at not found!! resp_body=" + strRspBody;
		return -1;
	}
	intCreateTime = oJson["created_at"].asInt();
	LOG4CPLUS_DEBUG(wx_logger, "strMediaId="<<strMediaId<<"intCreateTime="<<intCreateTime);
	return 0;
}

int WXHttpsReq::WXAPIDownloadMedia(const string& strMediaId, const std::string& strMyAccessToken, std::string& strMediaData, std::string& strErrMsg)
{
	// https://api.weixin.qq.com/cgi-bin/media/get?access_token=ACCESS_TOKEN&media_id=MEDIA_ID
	std::string strAPI = "cgi-bin/media/get?access_token=";
	strAPI += strMyAccessToken;
	strAPI += "&media_id=" + strMediaId;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpDownLoadPic("api.weixin.qq.com", 80, strAPI, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpDownLoadPic failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	strMediaData = strRspBody;
	return 0;
}

int WXHttpsReq::CheckSign(const std::string& strNonce, const std::string& strTimeStamp, std::string& strSign)
{
	std::vector<std::string> vecKeys;
	vecKeys.push_back(strNonce);
	vecKeys.push_back(strTimeStamp);
	vecKeys.push_back(WX_HX_PLATFORM_DEV_MSG_CHECK_TOKEN);
	std::sort(vecKeys.begin(), vecKeys.end());

	std::string strKey = "";
	for (size_t i = 0; i != vecKeys.size(); ++i)
	{
		strKey = strKey + vecKeys[i];
	}
	unsigned char digest[20] =
	{ 0 };
	SHA1((const unsigned char *) strKey.c_str(), strKey.length(), digest);
	string strResult;
	char sHexStr[32];

	for (uint32_t i = 0; i < sizeof(digest); i++)
	{
		snprintf(sHexStr, sizeof(sHexStr), "%x%x", ((int) digest[i] & 0xf0) >> 4, ((int) digest[i] & 0x0f));
		strResult.append(sHexStr);
	}

	strSign = strResult;
	return 1;

}
std::string WXHttpsReq::BuildSignByDictSort(std::map<std::string, std::string>& mapParams, const std::string& strAPISecret)
{
	std::vector<std::string> vecKeys;
	for (std::map<std::string, std::string>::const_iterator iter = mapParams.begin(); iter != mapParams.end(); ++iter)
	{
		if (!iter->second.empty())
		{
			vecKeys.push_back(iter->first);
		}
	}

	std::sort(vecKeys.begin(), vecKeys.end());

	std::string strSortedParams;
	for (size_t i = 0; i != vecKeys.size(); ++i)
	{
		//for sign
		const std::string& strKey = vecKeys[i];
		const std::string& strValue = mapParams[strKey];
		if (i != 0)
		{
			strSortedParams += "&";
		}

		strSortedParams += (strKey + "=" + strValue);
	}

	strSortedParams += ("&key=" + strAPISecret);
	md5 md;
	strSortedParams = md.hash(strSortedParams);
	return lce::cgi::ToUpperCopy(strSortedParams);
}

std::string WXHttpsReq::BuildParamsByDictSort(std::map<std::string, std::string>& mapParams, const std::string& strAPISecret)
{
	std::vector<std::string> vecKeys;
	for (std::map<std::string, std::string>::const_iterator iter = mapParams.begin(); iter != mapParams.end(); ++iter)
	{
		if (!iter->second.empty())
		{
			vecKeys.push_back(iter->first);
		}
	}

	std::sort(vecKeys.begin(), vecKeys.end());

	//build sign param and final all Params
	std::string strFinalParams = "<xml>";
	std::string strSortedParams;
	for (size_t i = 0; i != vecKeys.size(); ++i)
	{
		//for sign
		const std::string& strKey = vecKeys[i];
		const std::string& strValue = mapParams[strKey];
		if (i != 0)
		{
			strSortedParams += "&";
		}
		strSortedParams += (strKey + "=" + strValue);

		//for final
		strFinalParams += ("<" + strKey + ">" + strValue + "</" + strKey + ">");
	}

	strSortedParams += ("&key=" + strAPISecret);
	md5 md;
	strSortedParams = md.hash(strSortedParams);
	string strSign = lce::cgi::ToUpperCopy(strSortedParams);

	//final
	strFinalParams += ("<sign>" + strSign + "</sign></xml>");

	return strFinalParams;
}

int WXHttpsReq::ParseWXRspXml2Map(const string& strRspXml, std::map<std::string, std::string>& mapRsp, std::string& strErrMsg)
{
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

	node = node->FirstChildElement();
	std::string strTagKey = node->Value();
	std::string strTagValue = node->GetText();
	mapRsp[strTagKey] = strTagValue;
	while ((node = node->NextSiblingElement()))
	{
		strTagKey = node->Value();
		strTagValue = node->GetText();
		mapRsp[strTagKey] = strTagValue;
	}

	return 0;
}

int WXHttpsReq::DoHttps(const string& strHost, uint16_t wPort, const string& strAPI, const string& strReqBody, string& strRspBody, string& strErrMsg)
{
	/*ȡ������IP��ַ */
	struct hostent *host;
	if ((host = gethostbyname(strHost.c_str())) == NULL)
	{
		strErrMsg = "Gethostname error, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	int iSockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (iSockFD < 0)
	{
		strErrMsg = "Socket Error:, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(wPort);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);
	if (connect(iSockFD, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) < 0)
	{

		strErrMsg = "Connect Error, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	/* SSL��ʼ�� */
	SSL_library_init();
	SSL_load_error_strings();
	SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL)
	{
		strErrMsg = "SSL_CTX_new failed!";
		return -1;
	}

	SSL *ssl = SSL_new(ctx);
	if (ssl == NULL)
	{
		strErrMsg = "SSL_new failed!";
		return -1;
	}

	/* ��socket��SSL���� */
	int iRet = SSL_set_fd(ssl, iSockFD);
	if (iRet == 0)
	{
		strErrMsg = "SSL_set_fd failed!";
		return -1;
	}

	RAND_poll();
	while (RAND_status() == 0)
	{
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}

	iRet = SSL_connect(ssl);
	if (iRet != 1)
	{
		strErrMsg = "SSL_connect failed!";
		return -1;
	}

	static char aczRequest[2048];
	bzero(aczRequest, 2048);
	snprintf(aczRequest, 2048, "POST /%s HTTP/1.1\r\nContent-Type: text/xml\r\nContent-Length: %zu\r\nHost: %s\r\n\r\n%s", strAPI.c_str(), strReqBody.size(), strHost.c_str(), strReqBody.c_str());

	//LOG4CPLUS_DEBUG(wx_logger, "https, reqeust_body="<<aczRequest);

	/*����https����request */
	int iSend = 0;
	int iTotalSend = 0;
	int iBytes = strlen(aczRequest);
	while (iTotalSend < iBytes)
	{
		iSend = SSL_write(ssl, aczRequest + iTotalSend, iBytes - iTotalSend);
		if (iSend < 0)
		{
			strErrMsg = "SSL_write failed !";
			return -1;
		}

		iTotalSend += iSend;
	}

	string strReadResult = "";
	static char Response[1024];
	bzero(Response, 1024);
	/* ���ӳɹ��ˣ�����https��Ӧ��response */
	while ((iBytes = SSL_read(ssl, Response, 1023)) > 0)
	{
		strReadResult += Response;
		if (strReadResult.substr(strReadResult.length() - 6) == "</xml>")
		{
			break;
		}
		bzero(Response, 1024);
	}

	string strSplit = "\r\n\r\n";
	vector<string> vecHeadBody;
	lce::cgi::Split(strReadResult, strSplit, vecHeadBody);
	if (vecHeadBody.size() != 2)
	{
		strErrMsg = "vecHeadBody.size() != 2, src_rsp=" + strReadResult;
		return -1;
	}

	strRspBody = vecHeadBody[1];

	/* ����ͨѶ */
	iRet = SSL_shutdown(ssl);
	//�Ȳ��ж��Ƿ�ɹ��ر��ˣ� �����΢�ź����ssl

	close(iSockFD);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	ERR_free_strings();
	return 0;
}

int WXHttpsReq::WXMsgDecrypt(const std::string &strMsgSignature, const std::string &strTimeStamp, const std::string &strNonce, const std::string &strPostData, const std::string strToken, const std::string strCryptkey, const std::string strAppid, std::map<std::string, std::string> &mapMsg)
{
	Tencent::WXBizMsgCrypt stWXBizMsgCrypt(strToken, strCryptkey, strAppid);

	string strXml = "";
	int iRet = stWXBizMsgCrypt.DecryptMsg(strMsgSignature, strTimeStamp, strNonce, strPostData, strXml);
	if (iRet < 0)
	{
		LOG4CPLUS_ERROR(wx_logger, "WXBizMsgCrypt.DecryptMsg failed!");
		return -1;
	}

	LOG4CPLUS_DEBUG(wx_logger, "DecryptMsg succ , xml="<<strXml);

	mapMsg.clear();
	string strErrMsg = "";
	iRet = ParseWXRspXml2Map(strXml, mapMsg, strErrMsg);
	if (iRet < 0)
	{
		LOG4CPLUS_ERROR(wx_logger, "parse xml to map failed, errmsg="<<strErrMsg);
		return -1;
	}

	return 0;
}

int WXHttpsReq::WXMsgEncrypt(const std::string &strReplyMsg, const std::string &strTimeStamp, const std::string &strNonce, const std::string strToken, const std::string strCryptkey, const std::string strAppid, std::string &strEncryptMsg)
{
	Tencent::WXBizMsgCrypt stWXBizMsgCrypt(strToken, strCryptkey, strAppid);

	int iRet = stWXBizMsgCrypt.EncryptMsg(strReplyMsg, strTimeStamp, strNonce, strEncryptMsg);
	if (iRet < 0)
	{
		LOG4CPLUS_ERROR(wx_logger, "WXBizMsgCrypt.EncryptMsg failed!");
		return -1;
	}

	return 0;
}

int WXHttpsReq::WXAPIGetComponentAccessToken(const std::string& strComponentAppid, const std::string& strComponentAppsecret, const std::string& strComponentVerifyTicket, std::string& strComponentAccessToken, std::string& strErrMsg)
{
	std::map<std::string, std::string> mapParams;
	mapParams["component_appid"] = strComponentAppid;
	mapParams["component_appsecret"] = strComponentAppsecret;
	mapParams["component_verify_ticket"] = strComponentVerifyTicket;

	std::string strReqBody = BuildParamsToJson(mapParams);
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "cgi-bin/component/api_component_token", strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		strErrMsg = "wx return errcode, resp_body=" + strRspBody;
		return -1;
	}

	if (!oJson.hasKey("component_access_token"))
	{
		strErrMsg = "wx return invalid, component_access_token not found!! resp_body=" + strRspBody;
		return -1;
	}

	strComponentAccessToken = oJson["component_access_token"].asString();
	return 0;
}
//https://api.weixin.qq.com/cgi-bin/token?grant_type=client_credential&appid=APPID&secret=APPSECRET
int WXHttpsReq::WXAPIGetMyAccessToken(const std::string& strMyAppid, const std::string& strMyAppsecret, const std::string& strCacheip, const int& iCacheport, std::string& strMyAccessToken, std::string& strErrMsg)
{

	if (!AccessTokenQuery(strCacheip, iCacheport, MEMCACHEDACCESSTOKEN, strMyAccessToken, strErrMsg))
	{
		return -1;
	}
	if (strMyAccessToken.empty())
	{

		std::string strAPI = "cgi-bin/token?grant_type=client_credential&appid=";
		strAPI += strMyAppid;
		strAPI += "&secret=";
		strAPI += strMyAppsecret;

		std::string strReqBody = "";
		std::string strRspBody = "";
		std::string strInnerErrMsg = "";
		int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
		if (iRet < 0)
		{
			strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
			return -1;
		}

		Json::Value oJson;
		Json::Reader reader;
		if (!reader.parse(strRspBody, oJson, false))
		{
			strErrMsg = "parse json error, resp_body=" + strRspBody;
			return -1;
		}

		if (oJson.hasKey("errcode"))
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return -1;
		}

		if (!oJson.hasKey("access_token"))
		{
			strErrMsg = "wx return invalid, component_access_token not found!! resp_body=" + strRspBody;
			return -1;
		}

		strMyAccessToken = oJson["access_token"].asString();
		if (!AccessTokenAdd(strCacheip, iCacheport, MEMCACHEDACCESSTOKEN, strMyAccessToken, strErrMsg))
		{
			return -1;
		}
	}

	return 0;
}

int WXHttpsReq::WXAPIGetOAuthAccessToken(const std::string& strAppid
					, const std::string& strAppsecret
					, const std::string& strCode
					, std::string& strAccessToken
					, std::string& strRefreshToken
					, std::string& strOpenid
					, std::string& strScope
					, std::string& strErrMsg)
{
	//https://api.weixin.qq.com/sns/oauth2/access_token?appid=APPID&secret=SECRET&code=CODE&grant_type=authorization_code
	std::string strAPI = "sns/oauth2/access_token?appid=";
    	strAPI += strAppid;
    	strAPI += "&secret=";
    	strAPI += strAppsecret;
	strAPI += "&code=";
	strAPI += strCode;
	strAPI += "&grant_type=authorization_code";
 

    	std::string strReqBody = "";
    	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
	if(iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
    Json::Value oJson;
    Json::Reader reader;
    if(!reader.parse(strRspBody, oJson, false))
    {
        strErrMsg = "parse json error, resp_body=" + strRspBody;
        return -1;
    }

    if(oJson.hasKey("errcode"))
    {
        strErrMsg = "wx return errcode, resp_body=" + strRspBody;
        return -1;
    }

    if(!oJson.hasKey("access_token"))
    {
        strErrMsg = "wx return invalid, component_access_token not found!! resp_body=" + strRspBody;
        return -1;
    }

	strErrMsg = strRspBody;
    	strAccessToken = oJson["access_token"].asString();
	strRefreshToken = oJson["refresh_token"].asString();
	strOpenid = oJson["openid"].asString();
	strScope = oJson["scope"].asString();

    return 0;

}		
int WXHttpsReq::WXAPIGetUserInfo(const std::string& strAccessToken
						, std::string& strOpenid
						, std::string& strLang
						, std::string& strInfo
						, std::string& strErrMsg)
{

	//https://api.weixin.qq.com/sns/userinfo?access_token=ACCESS_TOKEN&openid=OPENID&lang=zh_CN
    	std::string strAPI = "sns/userinfo?access_token=";
    	strAPI += strAccessToken;
	strAPI += "&openid=";
	strAPI += strOpenid;
	strAPI += "&lang=";
	strAPI += strLang;

    	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, "", strRspBody, strInnerErrMsg);
	if(iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	strInfo = strRspBody;

	return 0;
}


int WXHttpsReq::WXAPIUploadLogo(const string& strLogo, const std::string& strFilename, const std::string& strMyAccessToken, std::string& strImgurl, std::string& strErrMsg)
{
	std::string strReqBody = strLogo;
	//http://api.weixin.qq.com/cgi-bin/media/uploadimg?access_token=123123
	std::string strAPI = "cgi-bin/media/uploadimg?access_token=";
	strAPI += strMyAccessToken;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsUpLoadFile("api.weixin.qq.com", 443, strAPI, strReqBody, strFilename, "buffer", strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsUpLoadFile failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{

		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	if (!oJson.hasKey("url"))
	{
		strErrMsg = "wx return invalid, imgurl not found!! resp_body=" + strRspBody;
		return -1;
	}

	strImgurl = oJson["url"].asString();
	return 0;
}
int WXHttpsReq::WXAPICardCreate(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strCardid, std::string& strErrMsg)
{
	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	// https://api.weixin.qq.com/card/create?access_token=ACCESS_TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "card/create?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return="<<strRspBody);
	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	if (!oJson.hasKey("card_id"))
	{
		strErrMsg = "wx return invalid, card_id not found!! resp_body=" + strRspBody;
		return -1;
	}

	strCardid = oJson["card_id"].asString();
	return 0;

}
//https://api.weixin.qq.com/card/paycell/set?access_token=TOKEN
int WXHttpsReq::WXAPICardSetPaysell(const std::string& strMyAccessToken, const std::string& strCardid, const std::string& strIsopen, std::string& strReturn, std::string& strErrMsg)
{
	std::string strReqBody = "{\"card_id\":\"";
	strReqBody += strCardid;
	strReqBody += "\",\"is_open\":";
	if (strIsopen != "true" && strIsopen != "false")
	{
		strErrMsg = "Isopen error, strIsopen=" + strIsopen;
		return -1;
	}
	if (strIsopen == "true")
	{
		strReqBody += "true";
	}
	else
	{
		strReqBody += "false";
	}
	strReqBody += "}";

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	// https://api.weixin.qq.com/card/membercard/activateuserform/set?access_token=TOKEN
	LOG4CPLUS_DEBUG(wx_logger, "strReqBody="<<strReqBody);
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "card/paycell/set?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return="<<strRspBody);
	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	strReturn = strRspBody;
	return 0;

}
int WXHttpsReq::WXAPIEntityShop(const std::string strPostData, const std::string action, const std::string strMyAccessToken, std::string& strRspBody, bool& boolRet, std::string& strErrMsg)
{
	/*	std::string MyAccessToken = "";
	 int iRet = WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID
	 , WX_HX_PLATFORM_DEV_SECRET
	 , MyAccessToken
	 , strErrMsg);
	 if(iRet < 0)
	 {
	 strErrMsg = "WXAPIGetMyAccessToken failed , errmsg=" + strErrMsg;
	 return -1;
	 }
	 LOG4CPLUS_DEBUG(wx_logger, "MyAccessToken="<<MyAccessToken);*/

	std::string strAPI = "cgi-bin/poi/";
	strAPI += action;
	strAPI += "?access_token=";
	strAPI += strMyAccessToken;

	int iRet = WXHttpsReq::DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strPostData, strRspBody, strErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strErrMsg;
		return -1;
	}

	// check return json
	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse ret_oJson error, strRspBody=" + strRspBody;
		return -1;
	}

	if (!oJson.hasKey("errcode"))
	{
		strErrMsg = "entity_shop errcode doesn't exist";
		return -1;
	}

	// errcode is not 0，errmsg is the error msg return the whole json
	/*  if(ret_oJson["errcode"] != 0)
	 {
	 boolRet = false;
	 //LOG4CPLUS_DEBUG(logger,  "edit_entity_shop errcode != 0");
	 }*/

	if (oJson["errcode"].asInt() != 0)
	{
		strErrMsg = "wx return errcode, resp_body=" + strRspBody;
		boolRet = false;
		return oJson["errcode"].asInt();
	}
	boolRet = true;
	return 0;
}

int WXHttpsReq::WXAPICardUpdate(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strSendcheck, std::string& strErrMsg)
{
	//https://api.weixin.qq.com/card/update?access_token=TOKEN
	std::string strReqBody = strPostData;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";

	LOG4CPLUS_DEBUG(wx_logger, "strReqBody="<<strReqBody);
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "card/update?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return="<<strRspBody);
	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}
	if (!oJson.hasKey("send_check"))
	{
		strErrMsg = "wx return errcode, resp_body=" + strRspBody;
		return -1;
	}

	strSendcheck = oJson["send_check"].asString();
	return 0;

}
int WXHttpsReq::WXAPIGetStatData(const string& strBeginDate, const std::string& strEndDate, const std::string& DataName, const std::string& strMyAccessToken, std::string& strReturn, std::string& strErrMsg)
{
	std::map<std::string, std::string> mapParams;
	mapParams["begin_date"] = strBeginDate;
	mapParams["end_date"] = strEndDate;
	std::string strReqBody = BuildParamsToJson(mapParams);

	//https://api.weixin.qq.com/datacube/getarticlesummary?access_token=ACCESS_TOKEN
	std::string strAPI = "datacube/";
	strAPI += DataName;
	strAPI += "?access_token=";
	strAPI += strMyAccessToken;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";

	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	strReturn = strRspBody;
	return 0;
}

int WXHttpsReq::WXAPICardActivateFormCreate(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strReturn, std::string& strErrMsg)
{

	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	// https://api.weixin.qq.com/card/membercard/activateuserform/set?access_token=TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "card/membercard/activateuserform/set?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return="<<strRspBody);
	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	strReturn = strRspBody;
	return 0;

}
int WXHttpsReq::WXAPIGetSceneQrcode(const uint32_t& iflag,const std::string strSceneStr,const std::string& strMyAccessToken, std::string& strTicket, std::string& strShowCodeUrl, std::string& strErrMsg)
{
	std::string strReqBody = "";
	std::ostringstream osRequestBody;
	Json::Reader reader;

	if (iflag == 1)
	{
		osRequestBody << "{\"action_name\": \"QR_LIMIT_STR_SCENE\"";
		osRequestBody << ",\r\n\"action_info\": {\"scene\":{\"scene_str\":\"" << strSceneStr<< "\"}}}";
	}
	else
	{
		strErrMsg = "iflag error, errmsg=";
		strErrMsg += iflag;
		return -1;
	}
	strReqBody = osRequestBody.str();
	osRequestBody.clear();

	//https://api.weixin.qq.com/card/qrcode/create?access_token=TOKEN
	std::string strAPI = "card/qrcode/create?access_token=";
	strAPI += strMyAccessToken;

	LOG4CPLUS_DEBUG(wx_logger, "rsqbody = " << strReqBody );

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "httpsAPI in cardinfo OK");

	Json::Value oJson;

	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	if (!oJson.hasKey("ticket"))
	{
		strErrMsg = "wx return invalid, ticket not found!! resp_body=" + strRspBody;
		return -1;
	}
	if (!oJson.hasKey("show_qrcode_url"))
	{
		strErrMsg = "wx return invalid, show_qrcode_url not found!! resp_body=" + strRspBody;
		return -1;
	}

	strTicket = oJson["ticket"].asString();
	strShowCodeUrl = oJson["show_qrcode_url"].asString();
	return 0;
}
int WXHttpsReq::WXAPIGetCardCodeInfo(const std::string& strCardid, const std::string& strMyAccessToken, const int& iExpireSeconds, const int& iFlags, std::string& strTicket, std::string& strShowCodeUrl, std::string& strErrMsg)
{
	std::string strReqBody = "";
	std::ostringstream osRequestBody;
	Json::Reader reader;

	if (iFlags == 1)
	{

		osRequestBody << "{\"action_name\": \"QR_CARD\"";

		if (iExpireSeconds >= 60 && iExpireSeconds <= 1800)
		{
			osRequestBody << ",\r\n\"expire_seconds\":" << iExpireSeconds;

		}
		//osRequestBody << ",\r\n\"action_info\": {\"card\":{\"card_id\":\"" << oJsonCardid["card_id"][ii].asString() << "\",\r\n\"is_unique_code\": false,\r\n\"outer_id\":1}}}";
		osRequestBody << ",\r\n\"action_info\": {\"card\":{\"card_id\":\"" << strCardid << "\",\r\n\"is_unique_code\": false,\r\n\"outer_id\":1}}}";
	}
	/*
	 else if (iFlags == 2)
	 {

	 osRequestBody << "{\"action_name\":\"QR_MULTIPLE_CARD\"";
	 if (iExpireSeconds >= 60 && iExpireSeconds <= 1800)
	 {
	 osRequestBody << ",\"expire_seconds\":" << iExpireSeconds;
	 }
	 osRequestBody << ",\"action_info\":{\"multiple_card\":{\"card_list\":[";

	 for (size_t i = 0; i < oJsonCardid["card_id"].size(); i++)
	 {
	 if (i != 0)
	 {
	 osRequestBody << ",";
	 }
	 osRequestBody << "{\"card_id\":\"" << oJsonCardid["card_id"][i].asString() << "\"}";
	 }
	 osRequestBody << "]}}}";

	 }
	 */
	else
	{
		strErrMsg = "iFlag error, errmsg=";
		strErrMsg += iFlags;
		return -1;
	}
//	oJsonRoot["action_info"] = Json::Value(oJsonActionInfo);
//	strReqBody=writer.write(oJsonRoot);
	strReqBody = osRequestBody.str();
	osRequestBody.clear();

	//https://api.weixin.qq.com/card/qrcode/create?access_token=TOKEN
	std::string strAPI = "card/qrcode/create?access_token=";
	strAPI += strMyAccessToken;

	LOG4CPLUS_DEBUG(wx_logger, "rsqbody = " << strReqBody );

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "httpsAPI in cardinfo OK");

	Json::Value oJson;

	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	if (!oJson.hasKey("ticket"))
	{
		strErrMsg = "wx return invalid, ticket not found!! resp_body=" + strRspBody;
		return -1;
	}
	if (!oJson.hasKey("show_qrcode_url"))
	{
		strErrMsg = "wx return invalid, show_qrcode_url not found!! resp_body=" + strRspBody;
		return -1;
	}

	strTicket = oJson["ticket"].asString();
	strShowCodeUrl = oJson["show_qrcode_url"].asString();
	return 0;
}
int WXHttpsReq::WXAPICardDelete(const string& strCardid, const std::string& strMyAccessToken, std::string& strQuery, std::string& strErrMsg)
{
	std::map<std::string, std::string> mapParams;
	mapParams["card_id"] = strCardid;
	std::string strReqBody = BuildParamsToJson(mapParams);
	//https://api.weixin.qq.com/card/delete?access_token=TOKEN
	std::string strAPI = "card/delete?access_token=";
	strAPI += strMyAccessToken;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (!oJson.hasKey("errcode"))
	{

		strErrMsg = "wx return errcode, resp_body=" + strRspBody;
		return -1;

	}
	if (oJson["errcode"].asInt() != 0)
	{
		strErrMsg = "wx return errcode, resp_body=" + strRspBody;
		return oJson["errcode"].asInt();
	}

	strQuery = strRspBody;
	return 0;
}

int WXHttpsReq::WXAPICardQuery(const string& strCardid, const std::string& strMyAccessToken, std::string& strQuery, std::string& strErrMsg)
{
	std::map<std::string, std::string> mapParams;
	mapParams["card_id"] = strCardid;
	std::string strReqBody = BuildParamsToJson(mapParams);
//	https://api.weixin.qq.com/card/get?access_token=TOKEN
	std::string strAPI = "card/get?access_token=";
	strAPI += strMyAccessToken;

	LOG4CPLUS_DEBUG(wx_logger, "parse ");
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	LOG4CPLUS_DEBUG(wx_logger, "parse ");
	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "parse ");

	if (!oJson.hasKey("errcode"))
	{

		strErrMsg = "wx return errcode, resp_body=" + strRspBody;
		return -1;

	}
	if (oJson["errcode"].asInt() != 0)
	{
		strErrMsg = "wx return errcode, resp_body=" + strRspBody;
		return oJson["errcode"].asInt();
	}

	LOG4CPLUS_DEBUG(wx_logger, "query " << strQuery);
	strQuery = oJson["card"].toJsonString();
	return 0;
}
int WXHttpsReq::WXAPICategoryQuery(const std::string& strMyAccessToken, std::string& strQuery, std::string& strErrMsg)
{

//	http://api.weixin.qq.com/cgi-bin/poi/getwxcategory?access_token=TOKEN
	std::string strAPI = "cgi-bin/poi/getwxcategory?access_token=";
	strAPI += strMyAccessToken;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPIGet("api.weixin.qq.com", 443, strAPI, strRspBody, strInnerErrMsg);
	//DoHttpDownLoadPic("mmbiz.qpic.cn", 80, strAPI, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	strQuery = strRspBody;
	return 0;
}
int WXHttpsReq::WXAPIMemberCardUserInfoQuery(const string& strCardid, const string& strCardcode, const std::string& strMyAccessToken, std::string& strQuery, std::string& strErrMsg)
{ //https://api.weixin.qq.com/card/membercard/userinfo/get?access_token=TOKEN
	std::map<std::string, std::string> mapParams;
	mapParams["card_id"] = strCardid;
	mapParams["code"] = strCardcode;
	std::string strReqBody = BuildParamsToJson(mapParams);
//	https://api.weixin.qq.com/card/get?access_token=TOKEN
	std::string strAPI = "card/membercard/userinfo/get?access_token=";
	strAPI += strMyAccessToken;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	strQuery = strRspBody;
	return 0;
}

int WXHttpsReq::WXAPICardCodeQuery(const bool &iflag,const string& strCardCode,const std::string& strMyAccessToken, std::string& strCardid, std::string& strErrMsg)
{
//	std::map<std::string, std::string> mapParams;
//    mapParams["code"] = strCardCode;
//    mapParams["check_consume"] = false;
//    std::string strReqBody = BuildParamsToJson(mapParams);
	Json::Value oJson1;
	oJson1["code"] = strCardCode;
	oJson1["check_consume"] = iflag;
	string strReqBody = oJson1.toJsonString();
	LOG4CPLUS_DEBUG(wx_logger, "strReqBody ="<<strReqBody);

	std::string strAPI = "card/code/get?access_token=";
	strAPI += strMyAccessToken;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "WXAPICardCodeQuery failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	Json::Value oJson;
	Json::Reader reader;
	std::string strParseData = "";
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}
	strParseData = oJson["card"].toJsonString();
	if (!reader.parse(strParseData, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (!oJson.hasKey("card_id"))
	{
		strErrMsg = "parse card_id error , resp_body=" + strRspBody;
		return -1;
	}

	strCardid = oJson["card_id"].asString();
	return 0;
}

int WXHttpsReq::WXAPICardDataQuery(const string& strQueryInfo, const std::string& strMyAccessToken, std::string& strQueryResult, std::string& strErrMsg)
{

	// get MyAccessToken
	/*  std::string strMyAccessToken = "";
	 int iRet = WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID
	 , WX_HX_PLATFORM_DEV_SECRET
	 , strMyAccessToken
	 , strErrMsg);
	 if(iRet < 0)
	 {
	 strErrMsg = "WXAPIGetMyAccessToken failed , errmsg=" + strErrMsg;
	 return -1;
	 }
	 */
	// https://api.weixin.qq.com/datacube/getcardcardinfo?access_token=ACCESS_TOKEN
	std::string strAPI = "datacube/getcardcardinfo?access_token=";
	strAPI += strMyAccessToken;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strQueryInfo, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "WXAPICardDataQuery failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}
	strQueryResult = strRspBody;
	return 0;
}

int WXHttpsReq::WXAPICardConsume(const string& strCardCode, const string& strMyAccessToken, std::string& strQuery, std::string& strErrMsg)
{
	std::map<std::string, std::string> mapParams;
	mapParams["code"] = strCardCode;
	std::string strReqBody = BuildParamsToJson(mapParams);

	// get MyAccessToken
	/*  std::string strMyAccessToken = "";
	 int iRet = WXAPIGetMyAccessToken(WX_HX_PLATFORM_DEV_APPID
	 , WX_HX_PLATFORM_DEV_SECRET
	 , strMyAccessToken
	 , strErrMsg);
	 if(iRet < 0)
	 {
	 strErrMsg = "WXAPIGetMyAccessToken failed , errmsg=" + strErrMsg;
	 return -1;
	 }*/

	//https://api.weixin.qq.com/card/code/consume?access_token=TOKEN
	std::string strAPI = "card/code/consume?access_token=";
	strAPI += strMyAccessToken;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "WXAPICardConsume failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{

		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}
	else
	{
		strErrMsg = "parse errcode error , resp_body=" + strRspBody;
		return -1;
	}

//	strQuery = strRspBody;
	strQuery = oJson["card"].toJsonString();
	return 0;

}

int WXHttpsReq::WXAPILandingPageCreate(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strPageUrl, int& strPageId, std::string& retMsg, std::string& strErrMsg)
{

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";

	// https://api.weixin.qq.com/card/landingpage/create?access_token=$TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "card/landingpage/create?access_token=" + strMyAccessToken, strPostData, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	LOG4CPLUS_DEBUG(wx_logger, "return strRspBody="<<strRspBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			LOG4CPLUS_DEBUG(wx_logger, "WXAPILandingPageCreate errcode != 0");
			if (strPageUrl != "")
			{
				strPageUrl = "";
			};
			if (strPageId != 0)
			{
				strPageId = 0;
			};
			retMsg = strRspBody;
			return oJson["errcode"].asInt();
		}

	}
	else
	{
		strErrMsg = "parse errcode error , resp_body=" + strRspBody;
		return -1;
	}

	if (!oJson.hasKey("url"))
	{
		strErrMsg = "wx return invalid, url not found!! resp_body=" + strRspBody;
		return -1;
	}

	if (!oJson.hasKey("page_id"))
	{
		strErrMsg = "wx return invalid, page_id not found!! resp_body=" + strRspBody;
		return -1;
	}

	if (retMsg != "")
		retMsg = "";
	strPageUrl = oJson["url"].asString();
	strPageId = oJson["page_id"].asInt();
	LOG4CPLUS_DEBUG(wx_logger, "WXAPILandingPageCreate return");
	return 0;

}

int WXHttpsReq::WXAPIMemberCardActivated(const string& strPostData, const std::string& strMyAccessToken, std::string& strReturn, std::string& strErrMsg)
{

	std::string strReqBody = strPostData;
	;

//	https://api.weixin.qq.com/card/membercard/activate?access_token=TOKEN
	std::string strAPI = "card/membercard/activate?access_token=";
	strAPI += strMyAccessToken;

	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, strAPI, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{

		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	strReturn = strRspBody;
	return 0;
}
int WXHttpsReq::WXAPIGetPreAuthCode(const std::string& strComponentAppid, const std::string& strComponentAccessToken, std::string& strPreAuthCode, std::string& strErrMsg)
{
	std::map<std::string, std::string> mapParams;
	mapParams["component_appid"] = strComponentAppid;

	std::string strReqBody = BuildParamsToJson(mapParams);
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "cgi-bin/component/api_create_preauthcode?component_access_token=" + strComponentAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{

		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	if (!oJson.hasKey("pre_auth_code"))
	{
		strErrMsg = "wx return invalid, pre_auth_code not found!! resp_body=" + strRspBody;
		return -1;
	}

	strPreAuthCode = oJson["pre_auth_code"].asString();
	return 0;
}

int WXHttpsReq::WXAPIGetAuthInfo(const std::string& strComponentAppid, const std::string& strComponentAccessToken, const std::string& strAuthCode, std::string& strAuthAppid, std::string& strAuthAccessToken, std::string& strAuthRefreshToken, std::string& strErrMsg)
{
	std::map<std::string, std::string> mapParams;
	mapParams["component_appid"] = strComponentAppid;
	mapParams["authorization_code"] = strAuthCode;

	std::string strReqBody = BuildParamsToJson(mapParams);
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "cgi-bin/component/api_query_auth?component_access_token=" + strComponentAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{

		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	if (!oJson.hasKey("authorization_info"))
	{
		strErrMsg = "wx return invalid, authorization_info not found!! resp_body=" + strRspBody;
		return -1;
	}

	const Json::Value& oAuthInfo = oJson["authorization_info"];
	if (!oAuthInfo.hasKey("authorizer_appid") || !oAuthInfo.hasKey("authorizer_access_token") || !oAuthInfo.hasKey("authorizer_refresh_token"))
	{
		strErrMsg = "wx return invalid, authorization_xxx(appid, accesstoken, refreshtoken) not found!! resp_body=" + strRspBody;
		return -1;
	}

	strAuthAppid = oAuthInfo["authorizer_appid"].asString();
	strAuthAccessToken = oAuthInfo["authorizer_access_token"].asString();
	strAuthRefreshToken = oAuthInfo["authorizer_refresh_token"].asString();
	return 0;
}

int WXHttpsReq::WXAPIRefreshAuthInfo(const std::string& strComponentAppid, const std::string& strComponentAccessToken, const std::string& strAuthAppid, const std::string& strAuthRefreshToken, std::string& strNewAuthAccessToken, std::string& strNewAuthRefreshToken, std::string& strErrMsg)
{
	std::map<std::string, std::string> mapParams;
	mapParams["component_appid"] = strComponentAppid;
	mapParams["authorizer_appid"] = strAuthAppid;
	mapParams["authorizer_refresh_token"] = strAuthRefreshToken;

	std::string strReqBody = BuildParamsToJson(mapParams);
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "cgi-bin/component/api_authorizer_token?component_access_token=" + strComponentAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{

		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	if (!oJson.hasKey("authorizer_access_token"))
	{
		strErrMsg = "wx return invalid, authorizer_access_token not found!! resp_body=" + strRspBody;
		return -1;
	}

	if (!oJson.hasKey("authorizer_refresh_token"))
	{
		strErrMsg = "wx return invalid, authorizer_refresh_token not found!! resp_body=" + strRspBody;
		return -1;
	}

	strNewAuthAccessToken = oJson["authorizer_access_token"].asString();
	strNewAuthRefreshToken = oJson["authorizer_refresh_token"].asString();
	return 0;
}

int WXHttpsReq::WXAPIGetBaseInfo(const std::string& strComponentAppid, const std::string& strComponentAccessToken, const std::string& strAuthAppid, std::string& strNickName, std::string& strHeadImgUrl, int& iServiceType, int& iVerifyType, std::string& strUserName, std::string& strAlias, std::string& strQRCodeUrl, std::string& strErrMsg)
{
	std::map<std::string, std::string> mapParams;
	mapParams["component_appid"] = strComponentAppid;
	mapParams["authorizer_appid"] = strAuthAppid;

	std::string strReqBody = BuildParamsToJson(mapParams);
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "cgi-bin/component/api_get_authorizer_info?component_access_token=" + strComponentAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	LOG4CPLUS_DEBUG(wx_logger, "WXAPIGetBaseInfo, wx_rsp="<<strRspBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}

	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	if (!oJson.hasKey("authorizer_info"))
	{
		strErrMsg = "wx return invalid, authorizer_info not found!! resp_body=" + strRspBody;
		return -1;
	}

	const Json::Value& oAutherInfo = oJson["authorizer_info"];
	if (oAutherInfo.hasKey("nick_name"))
	{
		strNickName = oAutherInfo["nick_name"].asString();
	}
	if (oAutherInfo.hasKey("head_img"))
	{
		strHeadImgUrl = oAutherInfo["head_img"].asString();
	}
	if (oAutherInfo.hasKey("user_name"))
	{
		strUserName = oAutherInfo["user_name"].asString();
	}
	if (oAutherInfo.hasKey("alias"))
	{
		strAlias = oAutherInfo["alias"].asString();
	}
	if (oAutherInfo.hasKey("qrcode_url"))
	{
		strQRCodeUrl = oAutherInfo["qrcode_url"].asString();
	}
	if (oAutherInfo.hasKey("service_type_info") && oAutherInfo["service_type_info"].isObject() && oAutherInfo["service_type_info"].hasKey("id"))
	{
		iServiceType = oAutherInfo["service_type_info"]["id"].asInt();
	}
	if (oAutherInfo.hasKey("verify_type_info") && oAutherInfo["verify_type_info"].isObject() && oAutherInfo["verify_type_info"].hasKey("id"))
	{
		iVerifyType = oAutherInfo["verify_type_info"]["id"].asInt();
	}

	return 0;
}

int WXHttpsReq::WXAPISendKFMsg(const std::string& strAccountAccessToken, const std::string& strToOpenid, std::string& strTextContent, std::string& strErrMsg)
{
	std::ostringstream oss;
	oss.str("");
	oss << "{" << "\"touser\":\"" << strToOpenid << "\"" << ",\"msgtype\":\"text\"" << ",\"text\":{\"content\":\"" << strTextContent << "\"}" << "}";
	std::string strReqBody = oss.str();
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "cgi-bin/message/custom/send?access_token=" + strAccountAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}

	LOG4CPLUS_DEBUG(wx_logger, "WXAPIGetBaseInfo, wx_rsp="<<strRspBody);

	/*
	 Json::Value oJson;
	 Json::Reader reader;
	 if(!reader.parse(strRspBody, oJson, false))
	 {
	 strErrMsg = "parse json error, resp_body=" + strRspBody;
	 return -1;
	 }

	 if(oJson.hasKey("errcode"))
	 {
	 strErrMsg = "wx return errcode, resp_body=" + strRspBody;
	 return -1;
	 }

	 if(!oJson.hasKey("authorizer_info"))
	 {
	 strErrMsg = "wx return invalid, authorizer_info not found!! resp_body=" + strRspBody;
	 return -1;
	 }*/

	return 0;
}
int WXHttpsReq::DoHttpsUpLoadFile(const string& strHost, uint16_t wPort, const string& strAPI, const string& strReqBody, const string& strFilename, const string& strBuffername, string& strRspBody, string& strErrMsg)
{
	/*ȡ������IP��ַ */
	struct hostent *host;
	if ((host = gethostbyname(strHost.c_str())) == NULL)
	{
		strErrMsg = "Gethostname error, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	int iSockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (iSockFD < 0)
	{
		strErrMsg = "Socket Error:, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(wPort);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);
	if (connect(iSockFD, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) < 0)
	{

		strErrMsg = "Connect Error, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	SSL_library_init();
	SSL_load_error_strings();
	SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL)
	{
		strErrMsg = "SSL_CTX_new failed!";
		return -1;
	}

	SSL *ssl = SSL_new(ctx);
	if (ssl == NULL)
	{
		strErrMsg = "SSL_new failed!";
		return -1;
	}

	int iRet = SSL_set_fd(ssl, iSockFD);
	if (iRet == 0)
	{
		strErrMsg = "SSL_set_fd failed!";
		return -1;
	}

	RAND_poll();
	while (RAND_status() == 0)
	{
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}

	iRet = SSL_connect(ssl);
	if (iRet != 1)
	{
		strErrMsg = "SSL_connect failed!";
		return -1;
	}
	std::ostringstream osRequestForm;
	std::ostringstream osRequest;

	std::string strRands = "hx";
	unsigned short uirandnum = rand() % 65536;
	RAND_seed(&uirandnum, sizeof(uirandnum));
	uirandnum = rand() % 65536;
	strRands += uirandnum;
	osRequestForm << "------------------------------" << strRands << "\r\nContent-Disposition: form-data; name=\"" << strBuffername.c_str() << "\"; filename=\"" << strFilename.c_str() << "\"\r\nContent-Type: application/octet-stream\r\n\r\n" << strReqBody << "\r\n------------------------------" << strRands << "--\r\n";
	//snprintf(acxRequestForm[])

	osRequest << "POST /" << strAPI.c_str() << " HTTP/1.1\r\nContent-Length: " << osRequestForm.str().size() << "\r\nExcept: 100-continue\r\nHost: " << strHost.c_str() << "\r\nAccept:*/*\r\nContent-Type: multipart/form-data; boundary=----------------------------" << strRands << "\r\n\r\n" << osRequestForm.str();

	/*����https����request */
	int iSend = 0;
	int iTotalSend = 0;
	int iBytes = osRequest.str().size();
	char aczRequest[osRequest.str().size() + 1];
	osRequest.str().copy(aczRequest, iBytes, 0);
	osRequest.clear();
	osRequestForm.clear();

	while (iTotalSend < iBytes)
	{
		iSend = SSL_write(ssl, aczRequest + iTotalSend, iBytes - iTotalSend);

		if (iSend < 0)
		{
			strErrMsg = "SSL_write failed !";
			return -1;
		}

		iTotalSend += iSend;
	}
	string strReadResult = "";
	static char Response[2048];
	bzero(Response, 2048);

	while ((iBytes = SSL_read(ssl, Response, 2047)) > 0)
	{

		strReadResult += Response;

		int iLeftCount = 0;
		int iRightCount = 0;
		for (size_t i = 0; i < strReadResult.length(); ++i)
		{
			char c = strReadResult[i];
			if ('{' == c)
			{
				iLeftCount++;
			}
			else if ('}' == c)
			{
				iRightCount++;
			}
		}
		if (iLeftCount == iRightCount)
		{
			break;
		}

		bzero(Response, 2048);
	}
	LOG4CPLUS_DEBUG(wx_logger, "\r\nhttps, response=\r\n"<<strReadResult);

	string strSplit = "\r\n\r\n";
	vector<string> vecHeadBody;
	lce::cgi::Split(strReadResult, strSplit, vecHeadBody);
	if (vecHeadBody.size() != 2)
	{
		strErrMsg = "vecHeadBody.size() != 2, src_rsp=" + strReadResult;
		return -1;
	}

	strRspBody = vecHeadBody[1];

	iRet = SSL_shutdown(ssl);

	close(iSockFD);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	ERR_free_strings();
	return 0;
}
int WXHttpsReq::DoHttpsOpenAPI(const string& strHost, uint16_t wPort, const string& strAPI, const string& strReqBody, string& strRspBody, string& strErrMsg)
{
	/*ȡ������IP��ַ */
	struct hostent *host;
	if ((host = gethostbyname(strHost.c_str())) == NULL)
	{
		strErrMsg = "Gethostname error, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	int iSockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (iSockFD < 0)
	{
		strErrMsg = "Socket Error:, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(wPort);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);
	if (connect(iSockFD, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) < 0)
	{

		strErrMsg = "Connect Error, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	/* SSL��ʼ�� */
	SSL_library_init();
	SSL_load_error_strings();
	SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL)
	{
		strErrMsg = "SSL_CTX_new failed!";
		return -1;
	}

	SSL *ssl = SSL_new(ctx);
	if (ssl == NULL)
	{
		strErrMsg = "SSL_new failed!";
		return -1;
	}

	/* ��socket��SSL���� */
	int iRet = SSL_set_fd(ssl, iSockFD);
	if (iRet == 0)
	{
		strErrMsg = "SSL_set_fd failed!";
		return -1;
	}

	RAND_poll();
	while (RAND_status() == 0)
	{
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}

	iRet = SSL_connect(ssl);
	if (iRet != 1)
	{
		strErrMsg = "SSL_connect failed!";
		return -1;
	}

	static char aczRequest[2048];
	bzero(aczRequest, 2048);
	snprintf(aczRequest, 2048, "POST /%s HTTP/1.1\r\nContent-Type: text/html\r\nContent-Length: %zu\r\nHost: %s\r\n\r\n%s", strAPI.c_str(), strReqBody.size(), strHost.c_str(), strReqBody.c_str());
	LOG4CPLUS_DEBUG(wx_logger, "https, write\n="<<aczRequest);
	/*����https����request */
	int iSend = 0;
	int iTotalSend = 0;
	int iBytes = strlen(aczRequest);
	while (iTotalSend < iBytes)
	{
		iSend = SSL_write(ssl, aczRequest + iTotalSend, iBytes - iTotalSend);
		if (iSend < 0)
		{
			strErrMsg = "SSL_write failed !";
			return -1;
		}

		iTotalSend += iSend;
	}

	string strReadResult = "";
	static char Response[2048];
	bzero(Response, 2048);
	/* ���ӳɹ��ˣ�����https��Ӧ��response */
	while ((iBytes = SSL_read(ssl, Response, 2047)) > 0)
	{
		strReadResult += Response;

		int iLeftCount = 0;
		int iRightCount = 0;
		for (size_t i = 0; i < strReadResult.length(); ++i)
		{
			char c = strReadResult[i];
			if ('{' == c)
			{
				iLeftCount++;
			}
			else if ('}' == c)
			{
				iRightCount++;
			}
		}
		if (iLeftCount == iRightCount)
		{
			break;
		}

		bzero(Response, 2048);
	}
	LOG4CPLUS_DEBUG(wx_logger, "https, response_body="<<strReadResult);
	string strSplit = "\r\n\r\n";
	vector<string> vecHeadBody;
	lce::cgi::Split(strReadResult, strSplit, vecHeadBody);
	if (vecHeadBody.size() != 2)
	{
		strErrMsg = "vecHeadBody.size() != 2, src_rsp=" + strReadResult;
		return -1;
	}

	strRspBody = vecHeadBody[1];

	/* ����ͨѶ */
	iRet = SSL_shutdown(ssl);
	//�Ȳ��ж��Ƿ�ɹ��ر��ˣ� �����΢�ź����ssl

	close(iSockFD);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	ERR_free_strings();
	return 0;
}

std::string WXHttpsReq::BuildParamsToJson(const std::map<std::string, std::string>& mapParams)
{
	std::string strJson = "{";
	for (std::map<std::string, std::string>::const_iterator iter = mapParams.begin(); iter != mapParams.end(); ++iter)
	{
		std::string strKey = iter->first;
		std::string strValue = iter->second;

		if (iter != mapParams.begin())
		{
			strJson += ",";
		}

		strJson += ("\"" + strKey + "\":\"" + strValue + "\"");
	}

	strJson += "}";

	return strJson;
}
int WXHttpsReq::WXAPIUploadNews(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strMediaid, std::string& strErrMsg)
{
	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	//https://api.weixin.qq.com/cgi-bin/media/uploadnews?access_token=ACCESS_TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "cgi-bin/media/uploadnews?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return=" << strRspBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}
	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}
	if (!oJson.hasKey("media_id"))
	{
		strErrMsg = "wx return invalid, media_id not found!! resp_body=" + strRspBody;
		return -1;
	}

	strMediaid = oJson["media_id"].asString();
	return 0;
}

int WXHttpsReq::WXAPISendAllPreview(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strMsgid, std::string& strErrMsg)
{
	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	//https://api.weixin.qq.com/cgi-bin/message/mass/preview?access_token=ACCESS_TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "cgi-bin/message/mass/preview?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return=" << strRspBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}
	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}
	if (!oJson.hasKey("msg_id"))
	{
		strErrMsg = "wx return invalid, msg_id not found!! resp_body=" + strRspBody;
		return -1;
	}
	strMsgid = oJson["msg_id"].asString();
	return 0;

}
int WXHttpsReq::WXAPISendAll(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strMsgid, std::string& strErrMsg)
{
	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	//https://api.weixin.qq.com/cgi-bin/message/mass/sendall?access_token=ACCESS_TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "cgi-bin/message/mass/sendall?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return=" << strRspBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}
	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}
	if (!oJson.hasKey("msg_id"))
	{
		strErrMsg = "wx return invalid, msg_id not found!! resp_body=" + strRspBody;
		return -1;
	}
	strMsgid = oJson["msg_id"].asString();
	return 0;

}
int WXHttpsReq::WXAPIUploadVideo(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strMediaid, std::string& strErrMsg)
{
	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	// https://api.weixin.qq.com/cgi-bin/media/uploadvideo?access_token=ACCESS_TOKEN POST
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "cgi-bin/media/uploadvideo?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return=" << strRspBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}
	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}
	if (!oJson.hasKey("media_id"))
	{
		strErrMsg = "wx return invalid, msg_id not found!! resp_body=" + strRspBody;
		return -1;
	}
	strMediaid = oJson["media_id"].asString();
	return 0;

}

int WXHttpsReq::WXAPISubmerchantCreate(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strSubmerchantid, std::string& strErrMsg)
{

	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	//https://api.weixin.qq.com/card/submerchant/submit?access_token=TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "card/submerchant/submit?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return=" << strRspBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}
	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	std::string strParseData = oJson["info"].toJsonString();
	if (!reader.parse(strParseData, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strParseData;
		return -1;
	}
	if (!oJson.hasKey("merchant_id"))
	{
		strErrMsg = "wx return invalid, merchant_id not found!! resp_body=" + strRspBody;
		return -1;
	}

	std::ostringstream oss;
	oss.str("");
	oss << oJson["merchant_id"].asInt();
	strSubmerchantid = oss.str();

	return 0;
}
int WXHttpsReq::WXAPISubmerchantUpdate(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strErrMsg)
{
	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";

	//https://api.weixin.qq.com/card/submerchant/update?access_token=TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "card/submerchant/update?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return=" << strRspBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}
	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	std::string strParseData = oJson["info"].toJsonString();
	if (!reader.parse(strParseData, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strParseData;
		return -1;
	}
	if (!oJson.hasKey("merchant_id"))
	{
		strErrMsg = "wx return invalid, merchant_id not found!! resp_body=" + strRspBody;
		return -1;
	}

	return 0;
}

int WXHttpsReq::WXAPISubmerchantGet(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strSubmerchantinfo, std::string& strErrMsg)
{

	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	//https://api.weixin.qq.com/card/submerchant/get?access_token=TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "card/submerchant/get?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return=" << strRspBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}
	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}

	if (oJson.hasKey("info"))
	{
		strSubmerchantinfo = oJson["info"].toJsonString();
	}
	else
	{
		strErrMsg = "wx return lack info, resp_body=" + strRspBody;
		return -1;
	}

	return 0;
}

int WXHttpsReq::WXAPISubmerchantBatchget(const std::string& strMyAccessToken, const std::string& strPostData, int& iNextbeginid, std::string& strInfolist, std::string& strErrMsg)
{

	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	//https://api.weixin.qq.com/card/submerchant/batchget?access_token=TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "card/submerchant/batchget?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);
	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "return=" << strRspBody);

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body=" + strRspBody;
		return -1;
	}
	if (oJson.hasKey("errcode"))
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			return oJson["errcode"].asInt();
		}
	}
	if (oJson.hasKey("next_begin_id"))
	{
		iNextbeginid = oJson["next_begin_id"].asInt();
	}

	if (oJson.hasKey("info_list"))
	{
		strInfolist = oJson["info_list"].toJsonString();
	}
	else
	{
		strErrMsg = "wx return lack info_list, resp_body=" + strRspBody;
		return -1;
	}

	return 0;
}

int WXHttpsReq::WXAPIApplyprotocolGet(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strApplyprotocol, std::string& strErrMsg)
{
	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	//https://api.weixin.qq.com/card/getapplyprotocol?access_token=TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "card/getapplyprotocol?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);

	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg = " + strInnerErrMsg;
		return -1;
	}
	LOG4CPLUS_DEBUG(wx_logger, "DoHttpsOpenAPI end.");

	Json::Value oJson;
	Json::Reader reader;
	LOG4CPLUS_DEBUG(wx_logger, "parse json start..");
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body = " + strRspBody;
		LOG4CPLUS_ERROR(wx_logger, "parse json error");
		return -1;
	}
	if (oJson.hasKey("errcode") && oJson["errcode"].asInt() != 0)
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			LOG4CPLUS_ERROR(wx_logger, "wx return errcode");
			return oJson["errcode"].asInt();
		}
	}
	LOG4CPLUS_DEBUG(wx_logger, "parse json end..");

	strApplyprotocol = oJson["category"].toJsonString();

	LOG4CPLUS_DEBUG(wx_logger, "WXAPIApplyprotocolGet end");
	return 0;
}
bool WXHttpsReq::AccessTokenAdd(const std::string& strCacheip, const uint16_t& iCacheport, const std::string& strName, const std::string& strKey, std::string& strErrMsg)
{
	if (strName.empty() || strKey.empty())
	{
		strErrMsg = "empty uin or key!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, strCacheip.c_str(), iCacheport, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//set
	rc = memcached_set(memc, strName.c_str(), strName.size(), strKey.c_str(), strKey.size(), 3600, 0);
	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_set failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	memcached_free(memc);
	return true;

}
bool WXHttpsReq::AccessTokenQuery(const std::string& strCacheip, const uint16_t& iCacheport, const std::string& strName, std::string& strKey, std::string& strErrMsg)
{
	if (strName.empty())
	{
		strErrMsg = "empty uin!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, strCacheip.c_str(), iCacheport, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//get
	uint32_t dwFlags = 0;
	size_t szValueLength;
	char* cpValue = memcached_get(memc, strName.c_str(), strName.size(), &szValueLength, &dwFlags, &rc);
	if (MEMCACHED_NOTFOUND == rc)
	{
		strKey = "";
		memcached_free(memc);
		return true;
	}

	if (rc != MEMCACHED_SUCCESS)
	{
		strErrMsg = "memcached_get failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
	}

	//check
	strKey = string(cpValue, szValueLength);
	memcached_free(memc);
	return true;

}

bool WXHttpsReq::AccessTokenDelete(const std::string& strCacheip, const uint16_t& iCacheport, std::string& strErrMsg)
{
	string strName = MEMCACHEDACCESSTOKEN;
	if (strName.empty())
	{
		strErrMsg = "empty key!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, strCacheip.c_str(), iCacheport, &rc);
	rc = memcached_server_push(memc, server);
	if (MEMCACHED_SUCCESS != rc)
	{
		strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		memcached_server_list_free(server);
		return false;
	}
	memcached_server_list_free(server);

	//delete
	memcached_delete(memc, strName.c_str(), strName.size(), 0);

	memcached_free(memc);
	return true;
}
int WXHttpsReq::DoHttpDownLoadPic(const string& strHost, uint16_t wPort, const string& strAPI, string& strRspBody, string& strErrMsg)
{
	/*ȡ������IP��ַ */
	struct hostent *host;
	if ((host = gethostbyname(strHost.c_str())) == NULL)
	{
		strErrMsg = "Gethostname error, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	int iSockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (iSockFD < 0)
	{
		strErrMsg = "Socket Error:, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(wPort);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);
	if (connect(iSockFD, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) < 0)
	{

		strErrMsg = "Connect Error, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	static char aczRequest[2048];
	bzero(aczRequest, 2048);
	snprintf(aczRequest, 2048, "GET /%s HTTP/1.1\r\nHost: %s\r\n"
			"Connection: close\r\n"
			"Accept: */*\r\n\r\n", strAPI.c_str(), strHost.c_str());
	/*����https����request */
	int iSend = 0;
	int iTotalSend = 0;
	int iBytes = strlen(aczRequest);
	while (iTotalSend < iBytes)
	{
		iSend = write(iSockFD, aczRequest + iTotalSend, iBytes - iTotalSend);
		if (iSend < 0)
		{
			strErrMsg = "SSL_write failed !";
			return -1;
		}

		iTotalSend += iSend;
	}

	string strReadResult = "";
	static char Response[2048];
	bzero(Response, 2048);

	while ((iBytes = read(iSockFD, Response, 2047)) > 0)
	{
		strReadResult.append(Response, iBytes);
		bzero(Response, 2048);
	}
	LOG4CPLUS_DEBUG(wx_logger, "https, bodysize="<<strReadResult.size());

	string strSplit = "\r\n\r\n";

	std::size_t ipos = strReadResult.find(strSplit);
	strRspBody = strReadResult.substr(ipos + strSplit.size());

	//LOG4CPLUS_DEBUG(wx_logger, "https, reqeust_body=\r\n"<<strRspBody);
	/* ����ͨѶ */

	//�Ȳ��ж��Ƿ�ɹ��ر��ˣ� �����΢�ź����ssl
	close(iSockFD);

	return 0;
}
int WXHttpsReq::DoHttpsOpenAPIGet(const string& strHost, uint16_t wPort, const string& strAPI, string& strRspBody, string& strErrMsg)
{
	/*ȡ������IP��ַ */
	struct hostent *host;
	if ((host = gethostbyname(strHost.c_str())) == NULL)
	{
		strErrMsg = "Gethostname error, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	int iSockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (iSockFD < 0)
	{
		strErrMsg = "Socket Error:, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(wPort);
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);
	if (connect(iSockFD, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) < 0)
	{

		strErrMsg = "Connect Error, ";
		strErrMsg += strerror(errno);
		return -1;
	}

	/* SSL��ʼ�� */
	SSL_library_init();
	SSL_load_error_strings();
	SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL)
	{
		strErrMsg = "SSL_CTX_new failed!";
		return -1;
	}

	SSL *ssl = SSL_new(ctx);
	if (ssl == NULL)
	{
		strErrMsg = "SSL_new failed!";
		return -1;
	}

	/* ��socket��SSL���� */
	int iRet = SSL_set_fd(ssl, iSockFD);
	if (iRet == 0)
	{
		strErrMsg = "SSL_set_fd failed!";
		return -1;
	}

	RAND_poll();
	while (RAND_status() == 0)
	{
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}

	iRet = SSL_connect(ssl);
	if (iRet != 1)
	{
		strErrMsg = "SSL_connect failed!";
		return -1;
	}
	static char aczRequest[2048];
	bzero(aczRequest, 2048);
	snprintf(aczRequest, 2048, "GET /%s HTTP/1.1\r\nHost: %s\r\n"
			"Connection: close\r\n"
			"Accept: */*\r\n\r\n", strAPI.c_str(), strHost.c_str());
	/*����https����request */
	LOG4CPLUS_DEBUG(wx_logger, "https, write=\n"<<aczRequest);
	int iSend = 0;
	int iTotalSend = 0;
	int iBytes = strlen(aczRequest);
	while (iTotalSend < iBytes)
	{
		iSend = SSL_write(ssl, aczRequest + iTotalSend, iBytes - iTotalSend);
		if (iSend < 0)
		{
			strErrMsg = "SSL_write failed !";
			return -1;
		}

		iTotalSend += iSend;
	}

	string strReadResult = "";
	static char Response[2048];
	bzero(Response, 2048);

	while ((iBytes = SSL_read(ssl, Response, 2047)) > 0)
	{
		strReadResult.append(Response, iBytes);
		bzero(Response, 2048);
	}
	LOG4CPLUS_DEBUG(wx_logger, "https, body=\n"<<strReadResult);

	string strSplit = "\r\n\r\n";

	std::size_t ipos = strReadResult.find(strSplit);
	strRspBody = strReadResult.substr(ipos + strSplit.size());

	iRet = SSL_shutdown(ssl);
	//�Ȳ��ж��Ƿ�ɹ��ر��ˣ� �����΢�ź����ssl

	close(iSockFD);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	ERR_free_strings();

	return 0;
}
int WXHttpsReq::WXPictureDownLoad(const std::string& strUrl, std::string& strPic, std::string& strErrMsg)
{
	//http://mmbiz.qpic.cn/mmbiz/rrogclysZkcvkZHbDI9EP4IM3IrUDTEylWBt7kPHtxGZibd6txMWwqYhcBnFG60785ic2BMsLsCJAf4vxl9SRaZQ/0
	//https://mp.weixin.qq.com/cgi-bin/showqrcode?ticket=gQFU7zoAAAAAAAAAASxodHRwOi8vd2VpeGluLnFxLmNvbS9xL1dFTTdBVXpscndIV1BiNno4MmlVAAIELH9tVgMEAKd2AA%3D%3D
	std::string strTemp = "http://mmbiz.qpic.cn/";
	std::size_t iAPI = strUrl.find(strTemp);
	int flag = 0;
	if (iAPI == std::string::npos)
	{
		flag = 1;
		strTemp = "https://mp.weixin.qq.com/";
		if (((iAPI = strUrl.find(strTemp)) == std::string::npos))
		{
			strErrMsg = "strUrl=" + strUrl + ", not found targetStr=" + strTemp;
			return -1;
		}
	}

	std::string strAPI = strUrl.substr(iAPI + strTemp.size());
	LOG4CPLUS_DEBUG(wx_logger, "API="<<strAPI);
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	int iRet;
	if (flag == 0)
		iRet = DoHttpDownLoadPic("mmbiz.qpic.cn", 80, strAPI, strRspBody, strInnerErrMsg);
	else if (flag == 1)
		iRet = DoHttpDownLoadPic("mp.weixin.qq.com", 80, strAPI, strRspBody, strInnerErrMsg);
	if (iRet != 0)
	{
		strErrMsg = "WXAPICardConsume failed, errmsg=" + strInnerErrMsg;
		return -1;
	}
	strPic = strRspBody;
	return 0;
}

int WXHttpsReq::WXAPICardCardinfoGet(const std::string& strMyAccessToken, const std::string& strPostData, std::string& strCardinfo, std::string& strErrMsg)
{
	std::string strReqBody = strPostData;
	std::string strRspBody = "";
	std::string strInnerErrMsg = "";
	//https://api.weixin.qq.com/datacube/getcardcardinfo?access_token=ACCESS_TOKEN
	int iRet = DoHttpsOpenAPI("api.weixin.qq.com", 443, "datacube/getcardcardinfo?access_token=" + strMyAccessToken, strReqBody, strRspBody, strInnerErrMsg);

	if (iRet < 0)
	{
		strErrMsg = "DoHttpsOpenAPI failed, errmsg = " + strInnerErrMsg;
		return -1;
	}

	Json::Value oJson;
	Json::Reader reader;
	if (!reader.parse(strRspBody, oJson, false))
	{
		strErrMsg = "parse json error, resp_body = " + strRspBody;
		LOG4CPLUS_ERROR(wx_logger, "parse json error");
		return -1;
	}
	if (oJson.hasKey("errcode") && oJson["errcode"].asInt() != 0)
	{
		if (oJson["errcode"].asInt() != 0)
		{
			strErrMsg = "wx return errcode, resp_body=" + strRspBody;
			LOG4CPLUS_ERROR(wx_logger, "wx return errcode");
			return oJson["errcode"].asInt();
		}
	}

	strCardinfo = strRspBody;
	LOG4CPLUS_DEBUG(wx_logger, "WXAPICardCardinfoGet end");
	return 0;
}

