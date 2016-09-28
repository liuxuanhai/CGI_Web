#ifndef _LCE_CGI_HTTP_CONNECTOR_H_
#define _LCE_CGI_HTTP_CONNECTOR_H_

#include <string>
#include <stdlib.h>
#include <map>
#include "cgi_tcp_connector.h"

namespace lce
{
	namespace cgi
	{
		class CHTTPConnector
		{
		typedef bool (*DECODE_FUNC)(CHTTPConnector& oInstance, const char* pLine, const char* pLineEnd);
		
		public:
			enum HTTP_METHOD{
				METHOD_GET = 0,
				METHOD_POST = 1,
			};
		private:
			enum {
				MAX_URL_LEN			= 4096,
				MAX_COOKIE_LEN		= 4096,
				MAX_REFERER_LEN		= 1024,
				MAX_USERAGENT_LEN	= 1024,
				OTHER_HEAD_CNT		= 100,
				OTHER_HEAD_NAME_LEN	= 128,
				OTHER_HEAD_DATA_LEN	= 1024,
				MAX_ALLOW_CONTENT_LEN = 1024*1024,
				TAG_WIDTH = 10,
				TAG_LENGHT = 20,
			};
			struct SHeadBase
			{
				SHeadBase()
				{}
				char nMethod;
				int32_t nStatusCode;
				char szContentType[128];
				uint32_t nContentLength;
				union{
					struct{
						unsigned char bRequest:1;
						unsigned char bNoHeaderFlag:1;
						unsigned char bHttpV11:1;
						unsigned char bKeepAlive:1;
						unsigned char bHasContentType:1;
						unsigned char bHasContentLength:1;
						unsigned char bHasChunked:1;
						unsigned char bCookieLenOverflow:1;
					};
				};
			};

			struct SHeadOther{
				char szName[OTHER_HEAD_NAME_LEN];
				char szValue[OTHER_HEAD_DATA_LEN];
			};	

			struct SMimeValue
			{
				std::string sName;
				std::string sValue;
				std::string sFileName;
				std::string sContentType;
			};

			// struct SCookieInfo{
				// SCookieInfo(){}
				// std::string sValue;
				// std::string sDomain;
				// std::string sPath;
				// time_t dwCookieTime;
				// SCookieInfo& operator=(const SCookieInfo& rhs){
					// if (&rhs != this)
						// assign(rhs);

					// return *this;
				// }
				// SCookieInfo(const SCookieInfo& rhs)	{	assign(rhs);	}
			// private:
				// void assign(const SCookieInfo& rhs){
					// sValue = rhs.sValue;
					// sDomain = rhs.sDomain;
					// sPath = rhs.sPath;
					// dwCookieTime = rhs.dwCookieTime;
				// }
			// };
			// typedef std::map<std::string, SCookieInfo> MAP_COOKIE;
			typedef std::map<std::string, std::string> MAP_HEAD;
			typedef std::map<std::string, SMimeValue> MAP_VALUE;
		public:
			CHTTPConnector();
			~CHTTPConnector(){
				Close();
			}

			//设置cookie
			// bool SetCookie(const std::string& sName, const std::string& sValue, const std::string& sDomain="", const time_t dwCookieTime=0, const std::string& sPath="");
			
			//设置http协议头
			bool SetHead(const std::string& sName, const std::string& sValue);
			
			//设置参数值
			//bool SetValue(const std::string& sName, const std::string& sValue);

			//设置参数值
			//仅当需要传二进制文件时，传入FileName跟ContentType，使用表单传输
			bool SetValue(const std::string& sName, 
					const std::string& sValue,
					const std::string& sFileName = "",
					const std::string& sContentType = "");

			//设置Raw Body数据(仅在POST下使用，请不要与SetValue同时使用，使用SetBody会使SetValue设置的值失效)
			bool SetBody(const std::string& sBody);
			const std::string GetReqBody() const {return m_sReqBodyData;}
			
			//发生请求数据
			bool Execute(const std::string& sIP, const int16_t wPort, const std::string& sURI, const HTTP_METHOD nHttpMethod=METHOD_GET, const time_t uiTimeout=1000);
			const std::string& GetBody() const {	return m_sBodyData;	}
			const std::string& GetData() const {	return m_sData;	}
			const int32_t GetStatusCode() const {	return m_stBase.nStatusCode;	}
			void Reset() {	this->Clear();	}
			void Clear();
			void Close()	{	m_oHttp.Close();	}
			const char* GetErrMsg() const {	return m_szErrMsg;	}
		private:
			bool AnalyzeResponse(const char* pszData, const int32_t dwSize);
			bool EncodeRequest(std::string& sRequestData, const std::string& sUrl, const HTTP_METHOD nHttpMethod);
			std::string EncodeForm();
			char* GetHttpMethod(const HTTP_METHOD nHttpMethod){
				static char arrStr[2][100]={"GET","POST"};
				if ( nHttpMethod >= 0 && nHttpMethod < 2 )
				{
					return arrStr[nHttpMethod];
				}

				return arrStr[METHOD_POST];
			}
				
			static inline bool decode_connection(CHTTPConnector& oInstance, const char* pLine, const char* pLineEnd)
			{
				if( 0 == strncasecmp(pLine, "Connection:", 11))
				{
					pLine += 11;
					while(*pLine==' '||*pLine=='\t'||*pLine=='\r') pLine++;

					if(!strncasecmp(pLine, "close", 5)) 
					{
						oInstance.m_stBase.bKeepAlive = false;
					} 
					else if( 0 == strncasecmp(pLine, "keep-alive", 10))
					{
						oInstance.m_stBase.bKeepAlive = true;
					}
				}

				return true;
			}

			static inline bool decode_content_type(CHTTPConnector& oInstance, const char* pLine, const char* pLineEnd)
			{
				if( 0 == strncasecmp(pLine, "Content-type:", 13) ) 
				{
					pLine += 13;
					while(*pLine==' '||*pLine=='\t'||*pLine=='\r') pLine++;

					oInstance.m_stBase.bHasContentType = true;
					int32_t nValueLen = pLineEnd-pLine;
					if ( nValueLen > 128 )
					{
						memcpy(oInstance.m_stBase.szContentType, pLine, 128);
						oInstance.m_stBase.szContentType[127] = 0;
					}
					else if ( nValueLen >= 0)
					{
						memcpy(oInstance.m_stBase.szContentType, pLine, nValueLen);
						oInstance.m_stBase.szContentType[nValueLen] = 0;
					}
				}

				return true;
			}
			static inline bool decode_content_length(CHTTPConnector& oInstance, const char* pLine, const char* pLineEnd)
			{
				if( 0 == strncasecmp(pLine, "Content-length:", 15) ) 
				{
					if(oInstance.m_stBase.nMethod != METHOD_POST) {
						oInstance.m_stBase.bKeepAlive = false;
					}
					oInstance.m_stBase.bHasContentLength = true;
					oInstance.m_stBase.bHasChunked = false;
					
					oInstance.m_stBase.nContentLength = atoi(pLine+15);
					if(oInstance.m_stBase.nContentLength < 0 || oInstance.m_stBase.nContentLength > MAX_ALLOW_CONTENT_LEN ) 
					{
						oInstance.m_stBase.nStatusCode = 413;
						return false;
					}
				}

				return true;
			}
			
			static inline bool decode_transfer_encoding(CHTTPConnector& oInstance, const char* pLine, const char* pLineEnd)
			{
				if( 0 == strncasecmp(pLine, "Transfer-Encoding:", 18))
				{
					pLine += 18;
					while(*pLine==' '||*pLine=='\t'||*pLine=='\r') pLine++;

					if( 0 == strncasecmp(pLine, "chunked", 7)) 
					{
						oInstance.m_stBase.bHasContentLength = false;
						oInstance.m_stBase.bHasChunked = true;
					} 
				}

				return true;
			}
			
		private:
			CHTTPConnector(const CHTTPConnector& rhs);
			CHTTPConnector& operator=(const CHTTPConnector& rhs);
		private:
			static char m_arrFirstCharIndex[255];
			static bool m_bInitStaticData;
			static DECODE_FUNC m_arrTagFuncs[TAG_WIDTH*TAG_LENGHT];

			SHeadBase m_stBase;
			SHeadOther m_arrOtherHeads[OTHER_HEAD_CNT];
		
		
		
			char m_szErrMsg[1024];
			std::string m_sSvrIp;
			uint16_t m_wSvrPort;

			static const std::string m_strBoundary;
			bool m_bUseForm;

			MAP_HEAD m_mapHeads;
	//		MAP_COOKIE m_mapCookies;
			MAP_VALUE m_mapValues;
			cgi::CTCPConnector m_oHttp;
			std::string m_sReqBodyData;
			std::string m_sBodyData;
			std::string m_sData;
		};
	}
}

#endif

