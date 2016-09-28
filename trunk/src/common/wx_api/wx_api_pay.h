#ifndef _HOOSHO_PA_SERVER_WX_API_PAY_H_
#define _HOOSHO_PA_SERVER_WX_API_PAY_H_

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include "util/logger.h"
#include "wx_api.h"
#include "tinyxml/tinyxml.h"

namespace common
{
	namespace wxapi
	{
		class WXAPIPAY:public WXAPI
		{
			public:
				WXAPIPAY()
				{
					m_doc = new TiXmlDocument();
				}

				~WXAPIPAY()
				{
					if(m_doc)
					{
						delete m_doc;
						m_doc = NULL;
					}
				}

			public:
				/*******************  支付方式--公众号支付 *******************/
				int PaPayUnifiedOrder(const std::string& strPAAppid //公众号 Appid
									, const std::string& strMachId //商户号
									, const std::string& strMachSecretKey //商户API密钥
									, const std::string& strGoodsBody //商品描述
									, const std::string& strOuttradeNO //商户订单号, 商户系统内部的订单号,32个字符内、可包含字母
									, int iTotalFee  //订单总金额，单位为分
									, const std::string& strClientIP  //用户端ip
									, const std::string& strNotifyUrl //接收微信支付异步通知回调地址，通知url必须为直接可访问的url，不能携带参数
									, const std::string& strUserOpenid //用户相对于该公众号(strPAAppid)的openid
									, std::string& strPrepayId  //微信生成的预支付回话标识，用于后续接口调用中使用，该值有效期为2小时
									);
				
				int PaPayResultCallBackParse(const std::string& strCallbackXML
									, const std::string& strMachSecretKey //商户API密钥
									, std::string& strPaAppid //公众号 Appid
									, std::string& strMachId  //商户号
									, std::string& strUserOpenid //用户相对于该公众号(strPAAppid)的openid
									, int& iTotalFee  //订单总金额，单位为分
									, std::string& strOuttradeNO //商户订单号, 商户系统内部的订单号,32个字符内、可包含字母
									, std::string& strEndTime //支付完成时间
									);

				int PaPaySendRedPack(const std::string& strPAAppid //公众号 Appid
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
									);

				


			public:				
				int PaPayUnifiedOrder(const std::string& strReqBody, std::string& strRspBody);
				int PaPayTransferOrder(const std::string& strReqBody, std::string& strRspBody);
				int PaPayUnifiedOrderQuery(const std::string& strReqBody, std::string& strRspBody);
				int PaPaySendRedPack(const std::string& strReqBody, std::string& strRspBody);
				int PaPaySendRedPack(const std::string& strReqBody
											, std::string& strRspBody
											, const std::string& strRootcaPath
											, const std::string& strAPIClientCertPath
											, const std::string& strAPIClientKeyPath
											);

			public:
				static uint64_t GetTickCountUsec();
				static std::string GenTimeStringFromNow_yyyyMMddHHmmss(int expire_s);
				static std::string GenRandomString(int length);
				static std::string GenRedPackBillNoString(const std::string& mch_id);

				//请求参数做签名后，打包成xml，发给微信
				static std::string Params2Sign(const std::map<std::string, std::string>& mapParams, const std::string& strMachPaySecretKey);
				static std::string Params2SignXML(const std::map<std::string, std::string>& mapParams, const std::string& strMachPaySecretKey);
				
				//微信回包xml解析成map后， 做校验签名
				static bool ParamsSignCheck(const std::map<std::string, std::string>& mapParams, const std::string& strMachPaySecretKey);							
			
			private:
				TiXmlDocument* m_doc;
		};

	}
}


#endif


