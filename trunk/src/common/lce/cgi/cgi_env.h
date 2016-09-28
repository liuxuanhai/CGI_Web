#ifndef _LCE_CGI_ENV_H_
#define _LCE_CGI_ENV_H_

#include "cgi_utils.h"
#include <map>
#include <string>
#include <stdlib.h>

namespace lce
{
	namespace cgi
	{
		class CCgiEnv
		{
			struct SFile 
			{
				std::string sData;
				std::string sName;
				std::string sContentType;

			};
			
			struct SMIMEHeader
			{
				std::string sDisposition;
				std::string sName;
				std::string sFileName;
				std::string sContentType;
			};

			typedef std::map<std::string, std::string> MAP_COOKIE;
			typedef std::map<std::string, std::string> MAP_VALUE;
			typedef std::map<std::string, CCgiEnv::SFile> TCGI_MAP_FILE;

		public:
			const std::string& GetCookie(const std::string& sName) const 
			{
				MAP_COOKIE::const_iterator it = m_mapCookies.find(sName);
				if ( it != m_mapCookies.end() )
				{
					return it->second;
				}
				return m_sNull;
			}

			const std::string& GetValue(const std::string& sName) const 
			{
				MAP_VALUE::const_iterator it = m_mapValues.find(sName);
				if ( it != m_mapValues.end() )
				{
					return it->second;
				}
				return m_sNull;
			}

			bool HasParam(const std::string& sName) const 
			{
				MAP_VALUE::const_iterator it = m_mapValues.find(sName);
				if ( it != m_mapValues.end() )
				{
					return true;
				}
				return false;
			}

			const std::string& GetFileData(const std::string& sName) const 
			{
				TCGI_MAP_FILE::const_iterator it = m_mapFiles.find(sName);
				if ( it != m_mapFiles.end() )
				{
					return it->second.sData;
				}
				return m_sNull;		
			}

			const std::string& GetFileName(const std::string& sName) const 
			{
				TCGI_MAP_FILE::const_iterator it = m_mapFiles.find(sName);
				if ( it != m_mapFiles.end() )
				{
					return it->second.sName;
				}
				return m_sNull;	
			}

		public:
			CCgiEnv();
			~CCgiEnv();

			bool Init();

			const std::string& GetServerSoftware()	const {	return m_sServerSoftware;	}
			const std::string& GetServerName() const {	return m_sServerName;	}
			const std::string& GetGatewayInterface() const {	return m_sGatewayInterface;	}
			const std::string& GetServerProtocol() const {	return m_sServerProtocol;	}
			const std::string& GetRequestMethod() const {	return m_sRequestMethod;	}
			const std::string& GetPathInfo() const {	return m_sPathInfo;	}
			const std::string& GetPathTranslated() const {	return m_sPathTranslated;	}
			const std::string& GetScriptName() const {	return m_sScriptName;	}
			const std::string& GetQueryString() const {	return m_sQueryString;	}
			const std::string& GetRemoteHost() const {	return m_sRemoteHost;	}
			const std::string& GetRemoteAddr() const {	return m_sRemoteAddr;	}
			const std::string& GetAuthType() const {	return m_sAuthType;	}
			const std::string& GetRemoteUser() const {	return m_sRemoteUser;	}
			const std::string& GetRemoteIdent() const {	return m_sRemoteIdent;	}
			const std::string& GetContentType()	const {	return m_sContentType;	}
			const std::string& GetAccept()	const {	return m_sAccept;	}
			const std::string& GetUserAgent() const {	return m_sUserAgent;	}
			const std::string& GetPostData() const {	return m_sPostData;	}
			const std::string& GetRedirectRequest() const {	return m_sRedirectRequest;	}
			const std::string& GetRedirectURL() const {	return m_sRedirectURL;	}
			const std::string& GetRedirectStatus() const {	return m_sRedirectStatus;	}
			const std::string& GetReferrer() const {	return m_sReferrer;	}

			std::string GetValues(bool bFormated = false) const 
			{	
				std::string sData;
				for ( MAP_VALUE::const_iterator it=m_mapValues.begin(); it!=m_mapValues.end(); ++it )
				{
					if(bFormated)
						sData += it->first + "\t=\t" + it->second + "\n";
					else
						sData += it->first + "=" + it->second + ";";
				}

				return sData;
			}

			std::string GetCookies(bool bFormated = false) const 
			{	
				std::string sData;
				for ( MAP_COOKIE::const_iterator it=m_mapCookies.begin(); it!=m_mapCookies.end(); ++it )
				{
					if(bFormated)
						sData += it->first + "\t=\t" + it->second + "\n";
					else
						sData += it->first + "=" + it->second + ";";
				}

				return sData;
			}

			std::string GetFiles() const 
			{	
				std::string sData;
				for ( TCGI_MAP_FILE::const_iterator it=m_mapFiles.begin(); it!=m_mapFiles.end(); ++it )
				{
					sData += it->first + "\t=\t(filesize)" + cgi::ToStr(it->second.sData.size()) + "\n";
				}

				return sData;
			}

			std::string GetEnvs()
			{
				std::string sData;

				sData += "SERVER_SOFTWARE:" + m_sServerSoftware + "\n";
				sData += "SERVER_NAME:" + m_sServerName + "\n";
				sData += "GATEWAY_INTERFACE:" + m_sGatewayInterface + "\n";
				sData += "SERVER_PROTOCOL:" + m_sServerProtocol + "\n";
				sData += "REQUEST_METHOD:" + m_sRequestMethod + "\n";
				sData += "PATH_INFO:" + m_sPathInfo + "\n";
				sData += "PATH_TRANSLATED:" + m_sPathTranslated + "\n";
				sData += "SCRIPT_NAME:" + m_sScriptName + "\n";
				sData += "QUERY_STRING:" + m_sQueryString + "\n";
				sData += "REMOTE_HOST:" + m_sRemoteHost + "\n";
				sData += "REMOTE_ADDR:" + m_sRemoteAddr + "\n";
				sData += "AUTH_TYPE:" + m_sAuthType + "\n";
				sData += "REMOTE_USER:" + m_sRemoteUser + "\n";
				sData += "REMOTE_IDENT:" + m_sRemoteIdent + "\n";
				sData += "CONTENT_TYPE:" + m_sContentType + "\n";
				sData += "HTTP_ACCEPT:" + m_sAccept + "\n";
				sData += "HTTP_USER_AGENT:" + m_sUserAgent + "\n";
				sData += "REDIRECT_REQUEST:" + m_sRedirectRequest + "\n";
				sData += "REDIRECT_URL:" + m_sRedirectURL + "\n";
				sData += "REDIRECT_STATUS:" + m_sRedirectStatus + "\n";
				sData += "HTTP_REFERER:" + m_sReferrer + "\n";

				return sData;
			}

		private:
			inline std::string get_env(const char* env){
				char *var = getenv(env);
				return (var == 0) ? std::string("") : var;
			}


			bool LoadCgiEnv();
			bool LoadPostData();
			bool LoadDebugData();
			bool parseCookies();
			bool parseValues();
			void parseMIME(const std::string& data);
			CCgiEnv::SMIMEHeader parseMIMEHeader(const std::string& data);

			
		private:
			uint16_t m_wPort;			// httpd server port 
			uint32_t m_dwContentLen;		// post method ,data length. 

			std::string m_sServerSoftware;	// httpd server software name.
			std::string m_sServerName;		// httpd server host name
			std::string m_sGatewayInterface;	//cgi version 
			std::string m_sServerProtocol;	// http protocol .
			std::string m_sRequestMethod;	// client request GET or POST.
			std::string m_sPathInfo;		// client request PATH .
			std::string m_sPathTranslated;	// client request PATH.
			std::string m_sScriptName;		// script  name.
			std::string m_sQueryString;		// GET method get data.
			std::string m_sRemoteHost;		// client ip or host name.
			std::string m_sRemoteAddr;		// client ip address.
			std::string m_sAuthType;		// server authentication type. 
			std::string m_sRemoteUser;		// client login user.
			std::string m_sRemoteIdent;		// get login name from server.
			std::string m_sContentType;		// POST content type.
			std::string m_sAccept;			// client browser accept type.
			std::string m_sUserAgent;		// client browser  type.
			std::string m_sPostData;			// POST method data.
			std::string m_sRedirectRequest; // redirect request.
			std::string m_sRedirectURL;		// redirect url.
			std::string m_sRedirectStatus;	// redirect status
			std::string m_sReferrer;		// referrer
			std::string m_sCookie;			// cookie data
			MAP_COOKIE m_mapCookies;	//cookie values
			MAP_VALUE m_mapValues;		//param values
			TCGI_MAP_FILE m_mapFiles;			//files 

			static std::string m_sNull;
		};
	}
}


#endif
