#ifndef _WX_HTTPS_REQ_H_
#define _WX_HTTPS_REQ_H_

#include "cgi/cgi.h"

#define MEMCACHEDACCESSTOKEN "MyAccessToken38342499"
class WXHttpsReq
{
	public:
		/************************************ ΢��֧�����******************************************/
		static int WXUnifiedOrder(const std::string& strAppid //�����˺�ID
								, const std::string& strMchId //�̻���
								, const std::string& strNonceString //  ����ַ�
								, const std::string& strBody //��Ʒ����
								, const std::string& strOutTradeNo //�̻�������
								, const std::string& strTotalFee //�ܽ��, ��λ: ��(rmb)
								, const std::string& strSpbillCreateIp //�ն�IP , Native֧�������΢��֧��API�Ļ���IP
								, const std::string& strNotifyUrl // ֪ͨ��ַ, ��΢��֧���첽֪ͨ�ص���ַ
								, const std::string& strProductId //��ƷID, trade_type=NATIVE���˲���ش�����idΪ��ά���а����ƷID���̻����ж��塣
								, const std::string& strAPISecret //API ��Կ
								, std::string& strPrepayId
								, std::string& strErrMsg);
		static int DoHttpsOpenAPIGet(const string& strHost
									, uint16_t wPort
									, const string& strAPI
									, string& strRspBody
									, string& strErrMsg);
		static int DoHttps(const string& strHost
							, uint16_t wPort
							, const string& strAPI
							, const string& strReqBody
							, string& strRspBody
							, string& strErrMsg);

		static std::string BuildParamsByDictSort(std::map<std::string, std::string>& mapParams
													, const std::string& strAPISecret);
		static std::string BuildSignByDictSort(std::map<std::string, std::string>& mapParams
													, const std::string& strAPISecret);
	//S	static int WXAPIDownloadMedia(const string& strMediaId, const std::string& strMyAccessToken, std::string& strMediaData, std::string& strErrMsg);
		static int WXAPICategoryQuery(const std::string& strMyAccessToken
								, std::string& strQuery
								, std::string& strErrMsg);
		static int WXAPIMemberCardUserInfoQuery(const string& strCardid
													,const string& strCardcode
										,const std::string& strMyAccessToken
										, std::string& strQuery
										, std::string& strErrMsg);
		static int ParseWXRspXml2Map(const string& strRspXml
									, std::map<std::string, std::string>& mapRsp
									, std::string& strErrMsg);
		static int WXAPIEntityShop(const std::string strPostData
									,const std::string action
									,const std::string strMyAccessToken
									,std::string& strRspBody
									,bool& boolRet,std::string& strErrMsg);
		static int WXAPIGetSceneQrcode(const uint32_t& iflag
											,const std::string strSceneStr
											,const std::string& strMyAccessToken
											, std::string& strTicket
											, std::string& strShowCodeUrl
											, std::string& strErrMsg);
		static bool AccessTokenAdd(const std::string& strCacheip
											,const uint16_t& iCacheport
											,const std::string& strName
											,const std::string& strKey
											,std::string& strErrMsg);
		static bool  AccessTokenQuery(const std::string& strCacheip
										,const uint16_t& iCacheport
										,const std::string& strName
										,std::string& strKey
										,std::string& strErrMsg);
		static bool AccessTokenDelete(const std::string& strCacheip
										,const uint16_t& iCacheport
									//	,const string& strName
										, string& strErrMsg);
		static int WXAPICardCardinfoGet(const std::string &strMyAccessToken
										, const std::string& strPostData
										, std::string& strCardinfo
										, std::string& strErrMsg);

		static int WXAPICardDelete(const string& strCardid, const std::string& strMyAccessToken, std::string& strQuery, std::string& strErrMsg);

		static int DoHttpsUpLoadFile(const string& strHost
							, uint16_t wPort
							, const std::string& strAPI
							, const std::string& strReqBody
							, const std::string& strFilename
							, const std::string& strBuffername
							, std::string& strRspBody
							, std::string& strErrMsg);
		static int WXAPIGetStatData(const string& strBeginDate
										,const std::string& strEndDate
										,const std::string& DataName
										,const std::string& strAccessToken
										, std::string& strReturn
										, std::string& strErrMsg);
		/************************************ ΢�ŵ���ƽ̨��������Ȩ���****************************/
		// ������Ϣ����ʵ�ԣ����һ�ȡ���ܺ������
		// @param sMsgSignature: ǩ����ӦURL�����msg_signature
		// @param strTimeStamp: ʱ�������ӦURL�����timestamp
		// @param strNonce: �����ӦURL�����nonce
		// @param strPostData: ���ģ���ӦPOST��������
		// @param mapMsg: ���ܺ�����ģ���return����0ʱ��Ч
		// @return: �ɹ�0��ʧ�ܷ��ض�Ӧ�Ĵ�����
		static int WXMsgDecrypt(const std::string &strMsgSignature
								, const std::string &strTimeStamp
								, const std::string &strNonce
								, const std::string &strPostData
								, const std::string strToken
								, const std::string strCryptkey
								, const std::string strAppid
								, std::map<std::string, std::string> &mapMsg);
	/*	static int WXMyMsgDecrypt(const std::string &strMsgSignature
										, const std::string &strTimeStamp
										, const std::string &strNonce
										, const std::string &strPostData
										, std::map<std::string, std::string> &mapMsg);*/
								
		//�����ںŻظ��û�����Ϣ���ܴ��
		// @param sReplyMsg:���ںŴ�ظ��û�����Ϣ��xml��ʽ���ַ�
		// @param sTimeStamp: ʱ����������Լ���ɣ�Ҳ������URL�����timestamp
		// @param sNonce: ��������Լ���ɣ�Ҳ������URL�����nonce
		// @param sEncryptMsg: ���ܺ�Ŀ���ֱ�ӻظ��û������ģ�����msg_signature, timestamp, nonce, encrypt��xml��ʽ���ַ�,��return����0ʱ��Ч
		// return���ɹ�0��ʧ�ܷ��ض�Ӧ�Ĵ�����
		static int WXMsgEncrypt(const std::string &strReplyMsg
								, const std::string &strTimeStamp
								, const std::string &strNonce
								, const std::string strToken
								, const std::string strCryptkey
								, const std::string strAppid
								, std::string &strEncryptMsg);
	/*	static int WXMyMsgEncrypt(const std::string &strReplyMsg
										, const std::string &strTimeStamp
										, const std::string &strNonce
										, std::string &strEncryptMsg);*/
		static int CheckSign(const std::string& strNonce
										, const std::string& strTimeStamp
										,std::string& strSign);

		/*��ȡ����ƽ̨access_token
		component_appid	����ƽ̨appid
		component_appsecret	����ƽ̨appsecret
		component_verify_ticket	΢�ź�̨���͵�ticket����ticket�ᶨʱ����
		*/

		static int WXAPIGetMyAccessToken(const std::string& strMyAppid
																, const std::string& strMyAppsecret
																, const string& strCacheip
																, const int& iCacheport
																, std::string& strMyAccessToken
																, std::string& strErrMsg);
																
		static int WXAPIGetOAuthAccessToken(const std::string& strAppid
						, const std::string& strAppsercret
						, const std::string& strCode
						, std::string& strAccessToken
						, std::string& strRefreshToken
						, std::string& strOpenid
						, std::string& strScope
						, std::string& strErrMsg);
		static int WXAPIGetUserInfo(const std::string& strAccessToken
						, std::string& strOpenid
						, std::string& strLang
						, std::string& strInfo
						, std::string& strErrMsg);

		
																
		static int WXAPIGetComponentAccessToken(const std::string& strComponentAppid
														, const std::string& strComponentAppsecret
														, const std::string& strComponentVerifyTicket
														, std::string& strComponentAccessToken
														, std::string& strErrMsg);

		/*��ȡԤ��Ȩ��, Ԥ��Ȩ�����ڹ��ں���Ȩʱ�ĵ���ƽ̨����ȫ��֤��
		component_appid	����ƽ̨appid
		component_accsess_token	����ƽ̨accesstoken
		*/
		static int WXAPIGetPreAuthCode(const std::string& strComponentAppid
														, const std::string& strComponentAccessToken
														, std::string& strPreAuthCode
														, std::string& strErrMsg);

		/*ʹ����Ȩ�뻻ȡ���ںŵ���Ȩ��Ϣ, ����ȡauthorizer_access_token��authorizer_refresh_token��
		��Ȩ��Ļ�ȡ����Ҫ���û��ڵ���ƽ̨��Ȩҳ�������Ȩ���̺��ڻص�URI��ͨ��URL�����ṩ�����ƽ̨����
		component_appid	����ƽ̨appid
		component_accsess_token	����ƽ̨accesstoken
		auth_code ��Ȩcode,������Ȩ�ɹ�ʱ���ظ����ƽ̨
		*/
		static int WXAPIGetAuthInfo(const std::string& strComponentAppid
														, const std::string& strComponentAccessToken
														, const std::string& strAuthCode
														, std::string& strAuthAppid
														, std::string& strAuthAccessToken
														, std::string& strAuthRefreshToken
														, std::string& strErrMsg);
		static int WXAPIUploadMedia(const string& strMedia
											,const std::string& strFilename
											,const std::string& strReqType
											,const std::string& strMyAccessToken
											, std::string& strMediaId
											, int& intCreateTime
											, std::string& strErrMsg);

		static int WXAPIDownloadMedia(const string& strMedia
											, const std::string& strMyAccessToken
											, std::string& strMediaData
											, std::string& strErrMsg);
		
		static int WXAPISubmerchantCreate(const std::string& strMyAccessToken
										,const std::string& strPostData
										,std::string& strSubmerchantid
										,std::string& strErrMsg);

		static int WXAPIApplyprotocolGet(const std::string& strMyAccessToken
										,const std::string& strPostData
										,std::string& strApplyprotocol
										,std::string& strErrMsg);

		static int WXAPISubmerchantUpdate(const std::string& strMyAccessToken
										,const std::string& strPostData
										,std::string& strErrMsg);

		static int WXAPISubmerchantGet(const std::string& strMyAccessToken
										,const std::string& strPostData
										,std::string& strSubmerchantInfo
										,std::string& strErrMsg);

		static int WXAPISubmerchantBatchget(const std::string& strMyAccessToken
										,const std::string& strPostData
										,int& iNextbeginid
										,std::string& strInfolist
										,std::string& strErrMsg);



		/*��ȡ��ˢ�£���Ȩ���ںŵ�����
		��������Ȩ�����ƣ�auth_access_token��ʧЧʱ������ˢ�����ƣ�auth_refresh_token����ȡ�µ����ơ�
		component_appid	����ƽ̨appid
		component_accsess_token	����ƽ̨accesstoken
		auth_appid ��Ȩ��appid
		auth_refresh_token ��Ȩ����ˢ������
		*/
		static int WXAPIRefreshAuthInfo(const std::string& strComponentAppid
														, const std::string& strComponentAccessToken
														, const std::string& strAuthAppid
														, const std::string& strAuthRefreshToken
														, std::string& strNewAuthAccessToken
														, std::string& strNewAuthRefreshToken
														, std::string& strErrMsg);
		

		static int WXAPIUploadNews(const std::string& strMyAccessToken
										,const std::string& strPostData
										,std::string& strMediaid
										,std::string& strErrMsg);
		static int WXAPISendAllPreview(const std::string& strMyAccessToken
										,const std::string& strPostData
										,std::string& strMsgid
										,std::string& strErrMsg);
		static int WXAPISendAll(const std::string& strMyAccessToken
										,const std::string& strPostData
										,std::string& strMsgid
										,std::string& strErrMsg);

		static int WXAPIUploadVideo(const std::string& strMyAccessToken
										,const std::string& strPostData
										,std::string& strMediaid
										,std::string& strErrMsg);





		/*��ȡ��Ȩ�����˻���Ϣ
		���ڻ�ȡ��Ȩ���Ĺ��ںŻ���Ϣ������ͷ���ǳơ��ʺ����͡���֤���͡�΢�źš�ԭʼID�Ͷ�ά��ͼƬURL��
		component_appid	����ƽ̨appid
		component_accsess_token	����ƽ̨accesstoken
		auth_appid ��Ȩ��appid

		nick_name	��Ȩ���ǳ�
		head_img	��Ȩ��ͷ��
		service_type_info	��Ȩ�����ں����ͣ�0��?�ĺţ�1�������ʷ���ʺ����Ķ��ĺţ�2�������
		verify_type_info	��Ȩ����֤���ͣ�-1���δ��֤��0���΢����֤��1�������΢����֤��2�����Ѷ΢����֤��3�����������֤ͨ��δͨ�������֤��4�����������֤ͨ��δͨ�������֤����ͨ��������΢����֤��5�����������֤ͨ��δͨ�������֤����ͨ������Ѷ΢����֤
		user_name	��Ȩ�����ںŵ�ԭʼID
		alias	��Ȩ�����ں������õ�΢�źţ�����Ϊ��
		qrcode_url	��ά��ͼƬ��URL���������������Ҳ���б���
		*/
		//












		static int WXAPIGetBaseInfo(const std::string& strComponentAppid
														, const std::string& strComponentAccessToken
														, const std::string& strAuthAppid
														, std::string& strNickName
														, std::string& strHeadImgUrl
														, int& iServiceType
														, int& iVerifyType
														, std::string& strUserName
														, std::string& strAlias
														, std::string& strQRCodeUrl
														, std::string& strErrMsg);	

		/*���ÿͷ��ӿڷ����ı���Ϣ���û�
			strAccountAccessToken ���?�ںŵ�accessToken
			strToOpenid �û�openid
			strTextContent �����ı�����
		*/
		static int WXAPISendKFMsg(const std::string& strAccountAccessToken
														, const std::string& strToOpenid
														, std::string& strTextContent
														, std::string& strErrMsg);
		/* 微信卡券*/
		static int WXAPIUploadLogo(const std::string& strLogo
															, const std::string& strFilename
															, const std::string& strMyAccessToken

															, std::string& strImgurl
															, std::string& strErrMsg);         // LOGO上传
		static int WXAPILandingPageCreate(const std::string& strMyAccessToken
										     	,const std::string& strPostData
										     	,std::string& strPageUrl
										     	,int& strPageId
										     	,std::string& retMsg
										     	,std::string& strErrMsg);
		static int WXAPICardCreate(const std::string& strMyAccessToken
				                         ,const std::string& strPostData
										 ,std::string& strCardid
										 ,std::string& strErrMsg);
		static int WXAPICardUpdate(const std::string& strMyAccessToken
										     ,const std::string& strPostData
											 ,std::string& strReturn
											 ,std::string& strErrMsg);
		static int WXAPICardQuery(const string& strCardid
										,const std::string& strMyAccessToken
										, std::string& strQuery
										, std::string& strErrMsg);
		static int WXAPICardCodeQuery(const bool& iflag
									,const string& strCardCode
										,const std::string& strMyAccessToken
										, std::string& strCardid
										, std::string& strErrMsg);
		static int WXAPICardDataQuery(const string& strQueryInfo
										,const std::string& strMyAccessToken
										, std::string& strQueryResult
										, std::string& strErrMsg);
		static int WXAPICardConsume(const string& strCardCode
										,const string& strMyAccessToken
										, std::string& strQuery
										, std::string& strErrMsg);		
		static int WXAPIGetCardCodeInfo(const string& strCardid
										,const std::string& strMyAccessToken
										,const int& iExpireSeconds
										,const int& iFlags
										, std::string& strTicket
										, std::string& strShowCodeUrl
										, std::string& strErrMsg);
		static int WXAPIMemberCardActivated(const string& strPostData
										,const std::string& strMyAccessToken
										, std::string& strReturn
										, std::string& strErrMsg);

		static int WXAPICardActivateFormCreate(const std::string& strMyAccessToken
												     ,const std::string& strPostData
													 ,std::string& strReturn
													 ,std::string& strErrMsg);
		static int WXAPICardSetPaysell(const std::string& strMyAccessToken
												     ,const std::string& strCardid
												     ,const std::string& strIsopen
													 ,std::string& strReturn
													 ,std::string& strErrMsg);
		static int DoHttpsOpenAPI(const string& strHost
							, uint16_t wPort
							, const string& strAPI
							, const string& strReqBody
							, string& strRspBody
							, string& strErrMsg);
							
		static std::string BuildParamsToJson(const std::map<std::string, std::string>& mapParams);

		static int  WXPictureDownLoad(const std::string& strUrl,std::string& strPic,std::string& strErrMsg);
		static int DoHttpDownLoadPic(const string& strHost, uint16_t wPort,const string& strAPI,  string& strRspBody,
				string& strErrMsg);
};
		

#endif

