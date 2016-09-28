#ifndef _LCE_CGI_HTTP_HEADER_H_
#define _LCE_CGI_HTTP_HEADER_H_

#include "cgi_utils.h"
#include <string>
#include <vector>
#include <stdio.h>

namespace lce
{
	namespace cgi
	{
		class CHttpHeader
		{
			struct SCookie 
			{
				std::string 	sName;
				std::string 	sValue;
				std::string 	sDomain;
				uint32_t 	dwMaxAge;
				std::string 	sPath;
				bool 			bSecure;
				bool			bHttpOnly;
			};
			typedef std::vector<SCookie> VEC_COOKIE;
			typedef std::vector<std::string> VEC_HEADER;
		public:
			CHttpHeader();
			~CHttpHeader();

			void SetCookie(const std::string& sName, 
				const std::string& sValue,
				const std::string& sDomain="", 
				const uint32_t sMaxAge=0, 
				const std::string& sPath="",
				const bool bSecure=false,
				const bool bHttpOnly=true);
			void AddHeader(const std::string& sHead)	{	m_vecHeaders.push_back(sHead); }
			void SetRedirect(const std::string& sUrl) {	m_vecHeaders.push_back("Location: "+sUrl);	}
			void SetStatus(const int32_t iStatus, const std::string& sMsg="") {	m_iStatusCode = iStatus;	m_sStatusMsg = sMsg;	}
			void SetContentType(const std::string& sType){	m_sContentType = sType;	}
			void Output()
			{
				if ( m_iStatusCode != 200 )
				{
					printf("HTTP/1.1 %d %s\r\n", m_iStatusCode, m_sStatusMsg.c_str());
				}
				for ( VEC_HEADER::const_iterator it=m_vecHeaders.begin(); it!=m_vecHeaders.end(); ++it )
				{
					printf("%s\r\n", (*it).c_str());
				}
				for ( VEC_COOKIE::const_iterator it=m_vecCookies.begin(); it!=m_vecCookies.end(); ++it )
				{
					printf("Set-Cookie:%s=%s", it->sName.c_str(), it->sValue.c_str());
					if ( !it->sDomain.empty() )
						printf("; Domain=%s", it->sDomain.c_str());
					if ( 0 != it->dwMaxAge )
						printf("; Expires=%s", getGMTDate(it->dwMaxAge+time(NULL)).c_str());
					if ( !it->sPath.empty() )
						printf("; Path=%s", it->sPath.c_str());
					if ( it->bSecure )
						printf("; Secure");
					if ( it->bHttpOnly )
						printf("; HTTPOnly");
					printf("\r\n");
				}

				printf("Content-Type: %s\r\n", m_sContentType.c_str());
				printf("\r\n");
			}

			void Reset(){	
				m_vecHeaders.clear();
				m_vecCookies.clear();
				m_sStatusMsg = "";
				m_sContentType = "text/html";
				m_iStatusCode = 200;
			}
		protected:
		private:
			//head
			std::string m_sContentType;
			int32_t m_iStatusCode;
			std::string m_sStatusMsg;
			VEC_COOKIE m_vecCookies;
			VEC_HEADER m_vecHeaders;
		};

		}

}

#endif

