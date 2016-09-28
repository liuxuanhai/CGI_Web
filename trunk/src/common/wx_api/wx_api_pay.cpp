#include "wx_api_pay.h"
#include "jsoncpp/json.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util/md5.h"
#include "util/lce_util.h"
#include <tinyxml/tinyxml.h>
#include <tinyxml/tinystr.h>

namespace common
{
	namespace wxapi
	{	
		int WXAPIPAY::PaPayUnifiedOrder(const std::string& strPAAppid //公众号 Appid
				, const std::string& strMachId //商户号
				, const std::string& strMachSecretKey //商户API密钥
				, const std::string& strGoodsBody //商品描述
				, const std::string& strOuttradeNO //商户订单号, 商户系统内部的订单号,32个字符内、可包含字母
				, int iTotalFee  //订单总金额，单位为分
				, const std::string& strClientIP  //用户端ip
				, const std::string& strNotifyUrl //接收微信支付异步通知回调地址，通知url必须为直接可访问的url，不能携带参数
				, const std::string& strUserOpenid //用户相对于该公众号(strPAAppid)的openid
				, std::string& strPrepayId  //微信生成的预支付回话标识，用于后续接口调用中使用，该值有效期为2小时
				)
		{
			std::map<std::string, std::string> mapParams;
			mapParams["appid"] = strPAAppid;
			mapParams["mch_id"] = strMachId;
			mapParams["device_info"] = "WEB";
			mapParams["nonce_str"] = GenRandomString(32);
			mapParams["body"] = strGoodsBody;
			mapParams["detail"] = "";
			mapParams["attach"] = "";
			mapParams["out_trade_no"] = strOuttradeNO;
			mapParams["fee_type"] = "CNY";
			mapParams["total_fee"] = lce::util::StringOP::TypeToStr(iTotalFee);
			mapParams["spbill_create_ip"] = strClientIP;
			mapParams["time_start"] = GenTimeStringFromNow_yyyyMMddHHmmss(0);
			mapParams["time_expire"] = GenTimeStringFromNow_yyyyMMddHHmmss(600);
			mapParams["goods_tag"] = "";
			mapParams["notify_url"] = strNotifyUrl;
			mapParams["trade_type"] = "JSAPI";
			mapParams["product_id"] = "";
			mapParams["limit_pay"] = "no_credit";
			mapParams["openid"] = strUserOpenid;
			std::string strReqBody = Params2SignXML(mapParams, strMachSecretKey);
			if(strReqBody.empty())
			{
				LOG4CPLUS_ERROR(logger, "Params2SignXML return empty result, failed!!!");
				return -1;
			}
			
			LOG4CPLUS_DEBUG(logger, "Params2SignXML, xml="<<strReqBody);
			std::string strRspBody = "";
			if(PaPayUnifiedOrder(strReqBody, strRspBody) < 0)
			{
				LOG4CPLUS_ERROR(logger, "inner PaPayUnifiedOrder return error!");
				return -1;
			}
			if(strRspBody.empty())
			{
				LOG4CPLUS_ERROR(logger, "rsp bosy empty!");
				return -1;
			}
			LOG4CPLUS_DEBUG(logger, "wx return xml="<<strRspBody);


			mapParams.clear();
			m_doc->Clear();
			m_doc->Parse(strRspBody.c_str());
			TiXmlElement* pRootElem = m_doc->RootElement();
			TiXmlElement* pParamElem = pRootElem->FirstChildElement();
			while(pParamElem)
			{
				std::string strTagName = pParamElem->Value();
				std::string strTagValue = pParamElem->GetText();
				if(!strTagName.empty())
				{
					mapParams[strTagName] = strTagValue;
				}

				pParamElem = pParamElem->NextSiblingElement();
			}

			//RETURN CODE
			if(mapParams["return_code"] != "SUCCESS")
			{
				LOG4CPLUS_ERROR(logger, "wx return_code="<<mapParams["return_code"]<<", return_msg="<<mapParams["return_msg"]);
				return -1;
			}

			if(!ParamsSignCheck(mapParams, strMachSecretKey))
			{
				LOG4CPLUS_ERROR(logger, "ParamsSignCheck failed");
				return -1;
			}

			//RESULT CODE
			if(mapParams["result_code"] != "SUCCESS")
			{
				LOG4CPLUS_ERROR(logger, "wx result_code="<<mapParams["result_code"]
							<<", err_code="<<mapParams["err_code"]<<", err_code_des="<<mapParams["err_code_des"]);
				return -1;
			}

			if(mapParams["appid"] != strPAAppid)
			{
				LOG4CPLUS_ERROR(logger, "appid returned fake data, return appid="<<mapParams["appid"]<<", while our appid="<<strPAAppid);
				return -1;
			}

			if(mapParams["mch_id"] != strMachId)
			{
				LOG4CPLUS_ERROR(logger, "mch_id returned fake data, return mch_id="<<mapParams["mch_id"]<<", while our mch_id="<<strMachId);
				return -1;
			}

			//prepay_id
			strPrepayId = mapParams["prepay_id"];
			LOG4CPLUS_DEBUG(logger, "prepay_id="<<strPrepayId);

			if(strPrepayId.empty())
			{
				LOG4CPLUS_ERROR(logger, "prepay_id is empty, fuck!!!");
				return -1;
			}

			return 0;
		}


		int WXAPIPAY::PaPaySendRedPack(const std::string& strPAAppid //公众号 Appid
									, const std::string& strMchId //商户号
									, const std::string& strMchSecretKey //商户API密钥
									, const std::string& strSenderName //商户名称
									, const std::string& strToOpenid //发送对象 openid
									, int iTotalAmount //红包金额
									, const std::string& strClientIP //客户端IP
									, int iTotalNum	//红包发送总人数
									, const std::string& strWishing //红包祝福语
									, const std::string& strActName //活动
									, const std::string& strRemark //备注
									, const std::string& strRootcaPath
									, const std::string& strAPIClientCertPath
									, const std::string& strAPIClientKeyPath
									, std::string& strMchBillNo
									, std::string& strErrMsg
									)
		{
			std::map<std::string, std::string> mapParams;
			mapParams["nonce_str"] = GenRandomString(32);
			mapParams["mch_billno"] = GenRedPackBillNoString(strMchId);
			mapParams["mch_id"] = strMchId;
			mapParams["wxappid"] = strPAAppid;
			mapParams["send_name"] = strSenderName;
			mapParams["re_openid"] = strToOpenid;
			mapParams["total_amount"] = lce::util::StringOP::TypeToStr(iTotalAmount);
			mapParams["total_num"] = lce::util::StringOP::TypeToStr(iTotalNum);
			mapParams["wishing"] = strWishing;
			mapParams["client_ip"] = strClientIP;
			mapParams["act_name"] = strActName;
			mapParams["remark"] = strRemark;			

			strMchBillNo = mapParams["mch_billno"];

			std::string strReqBody = Params2SignXML(mapParams, strMchSecretKey);
			if(strReqBody.empty())
			{
				LOG4CPLUS_ERROR(logger, "Params2SignXML return empty result, failed!!!");
				return -1;
			}
			
			LOG4CPLUS_DEBUG(logger, "Params2SignXML, xml="<<strReqBody);
			
			std::string strRspBody = "";
			if(PaPaySendRedPack(strReqBody, strRspBody, strRootcaPath, strAPIClientCertPath, strAPIClientKeyPath) < 0)
			{
				LOG4CPLUS_ERROR(logger, "inner PaPaySendRedPack return error!");
				strErrMsg = "PaPaySendRedPack Inner Error";
				return -1;
			}
			if(strRspBody.empty())
			{
				LOG4CPLUS_ERROR(logger, "rsp bosy empty!");
				return -1;
			}
			LOG4CPLUS_DEBUG(logger, "wx return xml="<<strRspBody);

			mapParams.clear();
			m_doc->Clear();
			m_doc->Parse(strRspBody.c_str());
			TiXmlElement* pRootElem = m_doc->RootElement();
			TiXmlElement* pParamElem = pRootElem->FirstChildElement();
			while(pParamElem)
			{
				std::string strTagName = pParamElem->Value();
				std::string strTagValue = pParamElem->GetText();
				if(!strTagName.empty())
				{
					mapParams[strTagName] = strTagValue;
				}

				pParamElem = pParamElem->NextSiblingElement();
			}

			//RETURN CODE
			if(mapParams["return_code"] != "SUCCESS")
			{
				LOG4CPLUS_ERROR(logger, "wx return_code="<<mapParams["return_code"]<<", return_msg="<<mapParams["return_msg"]);
				strErrMsg = mapParams["return_msg"];
				return -1;
			}

			if(!ParamsSignCheck(mapParams, strMchSecretKey))
			{
				LOG4CPLUS_ERROR(logger, "ParamsSignCheck failed");
				return -1;
			}

			//RESULT CODE
			if(mapParams["result_code"] != "SUCCESS")
			{
				LOG4CPLUS_ERROR(logger, "wx result_code="<<mapParams["result_code"]
							<<", err_code="<<mapParams["err_code"]<<", err_code_des="<<mapParams["err_code_des"]);
				strErrMsg = mapParams["err_code"];
				return -1;
			}

			if(mapParams["appid"] != strPAAppid)
			{
				LOG4CPLUS_ERROR(logger, "appid returned fake data, return appid="<<mapParams["appid"]<<", while our appid="<<strPAAppid);
				return -1;
			}

			if(mapParams["mch_id"] != strMchId)
			{
				LOG4CPLUS_ERROR(logger, "mch_id returned fake data, return mch_id="<<mapParams["mch_id"]<<", while our mch_id="<<strMchId);
				return -1;
			}

			if(mapParams["re_openid"] != strToOpenid)
			{
				LOG4CPLUS_ERROR(logger, "re_openid returned fake data, return re_openid="<<mapParams["re_openid"]<<", while our re_openid="<<strToOpenid);
				return -1;
			}

			if(mapParams["total_amount"] != lce::util::StringOP::TypeToStr(iTotalAmount))
			{
				LOG4CPLUS_ERROR(logger, "total_amount returned fake data, return total_amount="<<mapParams["total_amount"]<<", while our total_amount="<<iTotalAmount);
				return -1;
			}
			
			return 0;
		}

		int WXAPIPAY::PaPayResultCallBackParse(const std::string& strCallbackXML
				, const std::string& strMachSecretKey //商户API密钥
				, std::string& strPaAppid //公众号 Appid
				, std::string& strMachId  //商户号
				, std::string& strUserOpenid //用户相对于该公众号(strPAAppid)的openid
				, int& iTotalFee  //订单总金额，单位为分
				, std::string& strOuttradeNO //商户订单号, 商户系统内部的订单号,32个字符内、可包含字母
				, std::string& strEndTime //支付完成时间
				)
		{
			std::map<std::string, std::string> mapParams;
			m_doc->Clear();
			m_doc->Parse(strCallbackXML.c_str());
			TiXmlElement* pRootElem = m_doc->RootElement();
			TiXmlElement* pParamElem = pRootElem->FirstChildElement();
			while(pParamElem)
			{   
				std::string strTagName = pParamElem->Value();
				std::string strTagValue = pParamElem->GetText();
				if(!strTagName.empty())
				{   
					mapParams[strTagName] = strTagValue;
				}   

				pParamElem = pParamElem->NextSiblingElement();
			}

			//RETURN CODE
			if(mapParams["return_code"] != "SUCCESS")
			{
				LOG4CPLUS_ERROR(logger, "wx return_code="<<mapParams["return_code"]<<", return_msg="<<mapParams["return_msg"]);
				return -1;
			}

			if(!ParamsSignCheck(mapParams, strMachSecretKey))
			{
				LOG4CPLUS_ERROR(logger, "ParamsSignCheck failed");
				return -1;
			}

			//RESULT CODE
			if(mapParams["result_code"] != "SUCCESS")
			{
				LOG4CPLUS_ERROR(logger, "wx result_code="<<mapParams["result_code"]
						<<", err_code="<<mapParams["err_code"]<<", err_code_des="<<mapParams["err_code_des"]);
				return -1;
			}

			strPaAppid = mapParams["appid"];
			strMachId = mapParams["mch_id"];
			strUserOpenid = mapParams["openid"];
			iTotalFee = atoi(mapParams["total_fee"].c_str());
			strOuttradeNO = mapParams["out_trade_no"];
			strEndTime = mapParams["time_end"];

			return 0;
		}

		int WXAPIPAY::PaPayUnifiedOrder(const std::string& strReqBody, std::string& strRspBody)
		{
			//https://api.mch.weixin.qq.com/pay/unifiedorder
			std::string strAPI = "pay/unifiedorder";
			
			int iRet = HttpsPost("api.mch.weixin.qq.com", strAPI, strReqBody, strRspBody, "text/xml", 443);						
			if(iRet < 0)
			{
					LOG4CPLUS_ERROR(logger, "PaPayUnifiedOrder failed, msg=HttpsPost failed");
					return -1;
			}

			return 0;
		}

		int WXAPIPAY::PaPayTransferOrder(const std::string& strReqBody, std::string& strRspBody)
		{
			//https://api.mch.weixin.qq.com/mmpaymkttransfers/promotion/transfers
			std::string strAPI = "mmpaymkttransfers/promotion/transfers";
			
			int iRet = HttpsPost("api.mch.weixin.qq.com", strAPI, strReqBody, strRspBody, "text/xml", 443);						
			if(iRet < 0)
			{
					LOG4CPLUS_ERROR(logger, "PaPayTransferOrder failed, msg=HttpsPost failed");
					return -1;
			}

			return 0;			
		}

		int WXAPIPAY::PaPayUnifiedOrderQuery(const std::string& strReqBody, std::string& strRspBody)
		{
			//https://api.mch.weixin.qq.com/pay/orderquery
			std::string strAPI = "pay/orderquery";
			
			int iRet = HttpsPost("api.mch.weixin.qq.com", strAPI, strReqBody, strRspBody, "text/xml", 443);						
			if(iRet < 0)
			{
					LOG4CPLUS_ERROR(logger, "PaPayUnifiedOrderQuery failed, msg=HttpsPost failed");
					return -1;
			}

			return 0;			
		}

		int WXAPIPAY::PaPaySendRedPack(const std::string& strReqBody, std::string& strRspBody)
		{			
			//https://api.mch.weixin.qq.com/mmpaymkttransfers/sendredpack
			std::string strAPI = "mmpaymkttransfers/sendredpack";
			
			int iRet = HttpsPost("api.mch.weixin.qq.com", strAPI, strReqBody, strRspBody, "text/xml", 443);						
			if(iRet < 0)
			{
					LOG4CPLUS_ERROR(logger, "PaPaySendRedPack failed, msg=HttpsPost failed");
					return -1;
			}

			return 0;	
		}

		int WXAPIPAY::PaPaySendRedPack(const std::string& strReqBody
											, std::string& strRspBody
											, const std::string& strRootcaPath
											, const std::string& strAPIClientCertPath
											, const std::string& strAPIClientKeyPath
											)
		{
			//https://api.mch.weixin.qq.com/mmpaymkttransfers/sendredpack
			std::string strUrl = "https://api.mch.weixin.qq.com/mmpaymkttransfers/sendredpack";

			int iRet = CurlSSLPost(strUrl, strReqBody, strRspBody, strRootcaPath, strAPIClientCertPath, strAPIClientKeyPath);
			if(iRet < 0)
			{
				LOG4CPLUS_ERROR(logger, "PaPaySendRedPack failed, msg=CurlSSLPost failed");
				return -1;
			}			

			return 0;			
		}

		

		uint64_t WXAPIPAY::GetTickCountUsec()
		{
			timeval tv;
			gettimeofday(&tv, 0);
			return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
		}

		std::string WXAPIPAY::GenTimeStringFromNow_yyyyMMddHHmmss(int expire_s)
		{
			std::string strTime;
			struct tm* local;
			time_t t = time(NULL) + expire_s;
			local = localtime(&t);
			char tmp[16];
			sprintf(tmp, "%04d%02d%02d%02d%02d%02d"
					, local->tm_year + 1900
					, local->tm_mon + 1
					, local->tm_mday
					, local->tm_hour
					, local->tm_min
					, local->tm_sec);
			strTime = tmp;
			return strTime;
		}

		std::string WXAPIPAY::GenRandomString(int length)
		{		
			std::string randstr;
			char tmp[length + 1];
			int i = 0;
			for(; i<length; i++)
			{
				srand((GetTickCountUsec() >> i) ^ 77);
				int type = rand() % 3;				

				switch(type)
				{
					case 0:
						tmp[i] = '0' + rand() % 10;								
						break;
					case 1:
						tmp[i] = 'A' + rand() % 26;																
						break;
					case 2:
						tmp[i] = 'a' + rand() % 26;							
						break;
					default:								
						break;
				}				
			}

			tmp[i] = '\0';
			return randstr = tmp;
		}

		std::string WXAPIPAY::GenRedPackBillNoString(const std::string& mch_id)
		{
			std::string strMchidLocalTime = mch_id + GenTimeStringFromNow_yyyyMMddHHmmss(0);	

			int length = 4;
			char suffix[length + 1];
			
			int i = 0;
			for(; i < length; i++)
			{
				srand((GetTickCountUsec() >> i) ^ 77);
				suffix[i] = '0' + rand() % 10;
			}
			suffix[i] = '\0';

			return strMchidLocalTime + suffix;
		}

		bool WXAPIPAY::ParamsSignCheck(const std::map<std::string, std::string>& mapParams, const std::string& strMachPaySecretKey)
		{
			std::map<std::string, std::string>::const_iterator iter = mapParams.find("sign");
			if(iter == mapParams.end() || iter->second.empty())
			{
				LOG4CPLUS_ERROR(logger, "ParamsSignCheck failed, [sign] not found or ''");	
				return false;
			}
			
			std::string strWXSign = iter->second;
			std::string strOurSign = Params2Sign(mapParams, strMachPaySecretKey);
			if(strWXSign != strOurSign)
			{
				LOG4CPLUS_ERROR(logger, "ParamsSignCheck failed, not equal, strWXSign="<<strWXSign<<", strOurSign="<<strOurSign);
				return false;
			}

			return true;
		}

		std::string WXAPIPAY::Params2SignXML(const std::map<std::string, std::string>& mapParams
								, const std::string& strMachPaySecretKey)
		{
			std::string strSign = Params2Sign(mapParams, strMachPaySecretKey);
			if(strSign.empty())
			{
				return "";
			}

			//Build XML
			std::ostringstream oss;
			oss.str("");
			oss<<"<xml>";
			oss<<"<sign><![CDATA["<<strSign<<"]]></sign>";
			for(std::map<std::string, std::string>::const_iterator iter = mapParams.begin(); iter != mapParams.end(); ++iter)
			{
				const std::string strKey = iter->first;
				const std::string strValue = iter->second;
				if(strKey != "sign")
				{
					oss<<"<"<<strKey<<">"; 
					oss<<"<![CDATA["<<strValue<<"]]>";
					oss<<"</"<<strKey<<">";
				}
			}

			oss<<"</xml>";
			return oss.str();
		}
		
		std::string WXAPIPAY::Params2Sign(const std::map<std::string, std::string>& mapParams, const std::string& strMachPaySecretKey)
		{
			//Count Sign, sorted string by asii dict order
			std::vector<std::string> vecKeys;
			for(std::map<std::string, std::string>::const_iterator iter = mapParams.begin()
					; iter != mapParams.end()
					; ++iter)
			{
				if(iter->first != "sign")
				{
					vecKeys.push_back(iter->first);
				}
			}

			if(vecKeys.empty())
			{
				return "";
			}

			std::ostringstream oss;
			oss.str("");
			std::sort(vecKeys.begin(), vecKeys.end());
			for(size_t i=0; i!=vecKeys.size(); ++i)
			{
				const std::string& strKey = vecKeys[i];
				const std::string& strValue = (mapParams.find(strKey))->second;

				if(!strValue.empty())
				{
					oss<<"&"<<strKey<<"="<<strValue;	
				}
			}

			oss<<"&key="<<strMachPaySecretKey;
			
			md5 stMd5;
			std::string strSign = stMd5.hash(oss.str().substr(1));  // remove first &
			::transform(strSign.begin(), strSign.end(), strSign.begin(), ::toupper);
			
			return strSign;
		}				
	}
}
