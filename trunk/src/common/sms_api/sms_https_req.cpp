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
#include "openssl/md5.h"
#include "cgi/cgi.h"

#include "sms_https_req.h"

namespace common
{
	namespace smsapi
	{
		#define REST_OK 1
		static const char basis_64[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		static int Base64encode(char *encoded, const char *string, int len)
		{
			int i;
			char *p;

			p = encoded;
			for (i = 0; i < len - 2; i += 3) {
			*p++ = basis_64[(string[i] >> 2) & 0x3F];
			*p++ = basis_64[((string[i] & 0x3) << 4) |
							((int) (string[i + 1] & 0xF0) >> 4)];
			*p++ = basis_64[((string[i + 1] & 0xF) << 2) |
							((int) (string[i + 2] & 0xC0) >> 6)];
			*p++ = basis_64[string[i + 2] & 0x3F];
			}
			if (i < len) {
			*p++ = basis_64[(string[i] >> 2) & 0x3F];
			if (i == (len - 1)) {
				*p++ = basis_64[((string[i] & 0x3) << 4)];
				*p++ = '=';
			}
			else {
				*p++ = basis_64[((string[i] & 0x3) << 4) |
								((int) (string[i + 1] & 0xF0) >> 4)];
				*p++ = basis_64[((string[i + 1] & 0xF) << 2)];
			}
			*p++ = '=';
			}

			*p++ = '\0';
			return p - encoded;
		}

		static void GetSecondTimeFmt_yyyyMMddHHmmss(char *_out_date)
		{
		   struct tm* systime;
		   time_t t;
		   t=time(NULL);
			//systime=gmtime(&t);//世界标准时间
			systime=localtime(&t);//localtime()函数在 中国地区获得的本地时间会比世界标准时间晚8个小时
			sprintf(_out_date,"%04d%02d%02d%02d%02d%02d"
								,systime->tm_year+1900
								,systime->tm_mon+1
								,systime->tm_mday
								,systime->tm_hour+8
								,systime->tm_min
								,systime->tm_sec);

		}

		static int Md5AndBase64(const char* account, const char* accountpwd,char* mdrstr, char* base64str)
		{
				int ret=1;
				if(!account||!accountpwd||!mdrstr||!mdrstr)
				{
					return 2;
				}
				if((strlen(account)<=0)||(strlen(accountpwd)<=0))
				{

					return 2;
				}
				char dateStr[15];
				char sigStr[100];
				char authStr[100];
				unsigned char md[MD5_DIGEST_LENGTH];
				char* p=(char*)mdrstr;

				GetSecondTimeFmt_yyyyMMddHHmmss(dateStr);
				sprintf(sigStr,"%s%s%s",account,accountpwd,dateStr);
				MD5((const unsigned char*)sigStr, strlen(sigStr), md);
				for(int i=0; i<16; i++){
					sprintf(p, "%02X", md[i]);
					p+=2;
				}
				sprintf(authStr,"%s:%s",account,dateStr);
				Base64encode(base64str, authStr,strlen(authStr));
				return ret;
		}

		SMSHttpsReq::SMSHttpsReq(string& sms_api_host
			 ,uint16_t& sms_api_port
			 ,string& sms_api_account
			 ,string& sms_api_accountpwd
			 ,string& sms_api_appid
			 ,string& sms_api_version
			 ,string& sms_templateid)
		{
			memset(m_server,0,sizeof(m_server));
			strncpy(m_server,sms_api_host.c_str(),sizeof(m_server));
			m_server[sizeof(m_server)-1]='\0';
			m_port=sms_api_port;
			strncpy(m_https,"https://",sizeof(m_https));
					m_https[sizeof(m_https)-1]='\0';
			strncpy(m_mainaccount,sms_api_account.c_str(),sizeof(m_mainaccount));
			m_mainaccount[sizeof(m_mainaccount)-1]='\0';

			strncpy(m_mainaccountpwd,sms_api_accountpwd.c_str(),sizeof(m_mainaccountpwd));
			m_mainaccountpwd[sizeof(m_mainaccountpwd)-1]='\0';

			strncpy(m_mainappid,sms_api_appid.c_str(),sizeof(m_mainappid));
			m_mainappid[sizeof(m_mainappid)-1]='\0';

			strncpy(m_templateid,sms_templateid.c_str(),sizeof(m_templateid));
			m_templateid[sizeof(m_templateid)-1]='\0';

			strncpy(m_RestVersion,sms_api_version.c_str(),sizeof(m_RestVersion));//2013-12-26
			m_RestVersion[sizeof(m_RestVersion)-1]='\0';
		}

		int SMSHttpsReq::DoHttps( const string& strReqBody
							, string& strRspBody
							, string& strRspTotal
							, string& strErrMsg)
		{
			struct hostent *host;
			if ((host = gethostbyname(m_server)) == NULL)
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
			server_addr.sin_port = htons(m_port);
			server_addr.sin_addr = *((struct in_addr *) host->h_addr);
			string ip = inet_ntoa (*(struct in_addr *)*host->h_addr_list);
			strErrMsg+=ip;
			strErrMsg+=":";
			char t[20];
			sprintf (t,"%d",m_port);


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


			char uriStr[1024]={0};

			char mdStr[33]={0};
			char auth64Str[100];
			char strDate[64]={0};

			if(Md5AndBase64(m_mainaccount,m_mainaccountpwd,mdStr, auth64Str)!=REST_OK)
			{

				strErrMsg = "Md5ANdBase64 failed";
				strErrMsg+= mdStr;
			}

			sprintf(uriStr,"%s%s:%d%s%s%s%s%s",m_https,m_server,m_port,m_RestVersion,"/Accounts/",m_mainaccount,"/SMS/TemplateSMS?sig=",mdStr);

			time_t lt = time(NULL);
			strcpy(strDate,asctime(localtime(&lt)));
			strDate[strlen(strDate)-1]=0;
			static char aczRequest[3072];
			bzero(aczRequest, 3072);
			snprintf(aczRequest, 3072,"POST %s HTTP/1.1\r\nDate:%s\r\nConnection:close\r\nAccept:application/xml\r\nAuthorization:%s\r\nHost:%s:8883\r\nContent-Type:application/xml;charset=utf-8\r\nContent-Length:%zu\r\n\r\n%s"
							, uriStr
							, strDate
							, auth64Str
							, m_server
							, strReqBody.size()
							, strReqBody.c_str());



			int iSend = 0;
			int iTotalSend = 0;
			int iBytes = strlen(aczRequest);
			while (iTotalSend < iBytes) {
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

			while ((iBytes = SSL_read(ssl, Response, 1023)) > 0) {
				strErrMsg +=Response ;
				strReadResult += Response;

				if(strReadResult.substr(strReadResult.length() - 11) == "</Response>")
				{
						break;
				}
				bzero(Response, 1024);
			}

			string strSplit = "statusCode";
			vector<string> vecHeadBody;
			lce::cgi::Split(strReadResult, strSplit, vecHeadBody);
			if(vecHeadBody.size() != 3)
			{
				strErrMsg = "SMS vecHeadBody.size() != 3, src_rsp=" + strReadResult;
				return -1;
			}


			strRspBody = vecHeadBody[1];
			strRspTotal= strReadResult;

			iRet = SSL_shutdown(ssl);


			close(iSockFD);
			SSL_free(ssl);
			SSL_CTX_free(ctx);
			ERR_free_strings();
			return 0;
		}

		std::string SMSHttpsReq::BuildParamsByDictSort(std::map<std::string, std::string>& mapParams)
		{
			string strdata1="<data>"+mapParams["data1"]+"</data>";
			string strdata2="<data>"+mapParams["data2"]+"</data>";


			string strFinalParams="";
			strFinalParams+="<?xml version='1.0' encoding='utf-8'?><TemplateSMS><to>";
			strFinalParams+=mapParams["to"].c_str();
			strFinalParams+="</to><templateId>";
			strFinalParams+=mapParams["templateId"].c_str();
			strFinalParams+="</templateId><appId>";
			strFinalParams+=mapParams["appId"].c_str();
			strFinalParams+="</appId>";
			strFinalParams+="<datas>";
			strFinalParams+=strdata1.c_str();
			strFinalParams+=strdata2.c_str();
			strFinalParams+="</datas>";
			strFinalParams+="</TemplateSMS>";

			return strFinalParams;
		}

		int SMSHttpsReq::SMSUnifiedPost(const std::string& strto,const std::string& strVC,std::string& strErrMsg)
		{
			std::map<std::string, std::string> mapParams;

			mapParams["to"] = strto;
			mapParams["appId"] = m_mainappid;
			mapParams["templateId"] = m_templateid;
			mapParams["data1"] = strVC;
			mapParams["data2"] = "10";


			std::string strReqBody = BuildParamsByDictSort(mapParams);
			std::string strRspBody = "";
			std::string strRspTotal="";
			std::string strInnerErrMsg = "";

			int iRet = DoHttps( strReqBody, strRspBody,strRspTotal, strInnerErrMsg);
			if(iRet < 0)
			{
				strErrMsg = "DoHttps failed, errmsg=" + strInnerErrMsg;
				return -1;
			}


			if(strRspBody!= ">000000</")
			{

				strErrMsg = "SMS statusCode != 0,  rsp_xml=" + strRspBody + ", req_body=" + strReqBody+"statusMsg=\r\n"+strRspTotal;
				return -1;
			}
			
			return 0;
		}
	}	
}

