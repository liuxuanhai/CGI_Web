/*
 * sms_https_req.h
 *
 *  Created on: 2015-8-19
 *      Author: root
 */

#ifndef SMS_HTTPS_REQ_H_
#define SMS_HTTPS_REQ_H_
#include "cgi/cgi.h"
class SMSHttpsReq
{
public:
	 SMSHttpsReq(string& sms_api_host
			 ,uint16_t& sms_api_port
			 ,string& sms_api_account
			 ,string& sms_api_accountpwd
			 ,string& sms_api_appid
			 ,string& sms_api_version
			 ,string& sms_templateid);
	 char m_server[64];
	 int  m_port;
	 char m_subaccount[33];
	 char m_subaccountpwd[33];
	 char m_voipid[33];
	 char m_voippwd[33];
	 char m_mainaccount[33];
	 char m_mainaccountpwd[33];
	 char m_RestVersion[20];//版本号有可能变化
	 char m_mainappid[40];
	 char m_templateid[10];
	 char m_https[10];//有可能走http协议
	 bool m_bUseSSL;//rest接口是否走https的ssl,默认https, 增加http支持和服务器地址容错
	 int  m_BodyType;//body 封装格式，0 xml,1 json
	int DoHttps( const string& strReqBody
						, string& strRspBody
						,string& strRspTotal
						, string& strErrMsg);
	std::string BuildParamsByDictSort(std::map<std::string, std::string>& mapParams);
	int SMSUnifiedPost(const std::string& strto,const std::string& strVC,std::string& strErrMsg);

};

#endif /* SMS_HTTPS_REQ_H_ */
