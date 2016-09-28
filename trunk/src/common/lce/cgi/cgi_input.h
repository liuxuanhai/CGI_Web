#ifndef _LCE_CGI_INPUT_H_
#define _LCE_CGI_INPUT_H_

#include "cgi_env.h"
#include "cgi_value.h"
#include <string.h>
#include <arpa/inet.h>

namespace lce
{
	namespace cgi
	{

		class CCgiInput
		{
		public:
			CCgiInput(){}
			~CCgiInput(){}

			bool Init();

			inline CCgiValue GetValue(const std::string& sName) const 
			{
				return CCgiValue(m_oEnv.GetValue(sName));
			}
			
			inline bool HasParam(const std::string& sName) const 
			{
				return m_oEnv.HasParam(sName);
			}
			
			inline CCgiValue GetCookie(const std::string& sName) const 
			{
				return CCgiValue(m_oEnv.GetCookie(sName));
			}

			inline const std::string& GetFileName(const std::string& sName) const 
			{
				return m_oEnv.GetFileName(sName);
			}

			inline const std::string& GetFileData(const std::string& sName) const 
			{
				return m_oEnv.GetFileData(sName);
			}
			inline size_t GetFileSize(const std::string& sName)
			{
				return m_oEnv.GetFileData(sName).size();
			}

			inline const char* GetServerSoftware()	const {	return m_oEnv.GetServerSoftware().c_str();	}
			inline const char* GetServerName() const {	return m_oEnv.GetServerName().c_str();	}
			inline const char* GetGatewayInterface() const {	return m_oEnv.GetGatewayInterface().c_str();	}
			inline const char* GetServerProtocol() const {	return m_oEnv.GetServerProtocol().c_str();	}
			inline const char* GetRequestMethod() const {	return m_oEnv.GetRequestMethod().c_str();	}
			inline bool IsPostMethod() const {	
				return strncasecmp(m_oEnv.GetRequestMethod().c_str(), "post",sizeof("post")) == 0 ? true : false;	
			}
			inline const char* GetPathInfo() const {	return m_oEnv.GetPathInfo().c_str();	}
			inline const char* GetPathTranslated() const {	return m_oEnv.GetPathTranslated().c_str();	}
			inline const char* GetScriptName() const {	return m_oEnv.GetScriptName().c_str();	}
			inline const char* GetQueryString() const {	return m_oEnv.GetQueryString().c_str();	}
			inline const char* GetRemoteHost() const {	return m_oEnv.GetRemoteHost().c_str();	}
			inline const char* GetRemoteAddr() const {	return m_oEnv.GetRemoteAddr().c_str();	}
			inline const char* GetClientIPStr() const {	return GetRemoteAddr();	}
			inline uint32_t GetClientIP() const {	return ntohl(inet_addr(GetRemoteAddr()));}
			inline const char* GetAuthType() const {	return m_oEnv.GetAuthType().c_str();	}
			inline const char* GetRemoteUser() const {	return m_oEnv.GetRemoteUser().c_str();	}
			inline const char* GetRemoteIdent() const {	return m_oEnv.GetRemoteIdent().c_str();	}
			inline const char* GetContentType()	const {	return m_oEnv.GetContentType().c_str();	}
			inline const char* GetAccept()	const {	return m_oEnv.GetAccept().c_str();	}
			inline const char* GetUserAgent() const {	return m_oEnv.GetUserAgent().c_str();	}
			inline const char* GetPostData() const {	return m_oEnv.GetPostData().c_str();	}
			inline const size_t GetPostDataSize() const {	return m_oEnv.GetPostData().size();	}
			inline const char* GetRedirectRequest() const {	return m_oEnv.GetRedirectRequest().c_str();	}
			inline const char* GetRedirectURL() const {	return m_oEnv.GetRedirectURL().c_str();	}
			inline const char* GetRedirectStatus() const {	return m_oEnv.GetRedirectStatus().c_str();	}
			inline const char* GetReferrer() const {	return m_oEnv.GetReferrer().c_str();	}

			std::string GetValuesAll() const 
			{
		        return m_oEnv.GetValues();
			}

			std::string GetCookiesAll() const 
			{
				return m_oEnv.GetCookies();
			}

			std::string GetInputAll()
			 {	
				std::string sData;
				sData = "value list:\n";
				sData += m_oEnv.GetValues();
				sData += "\ncookie list:\n";
				sData += m_oEnv.GetCookies();
				sData += "\nfile list:\n";
				sData += m_oEnv.GetFiles();
				sData += "\nenv list:\n";
				sData += m_oEnv.GetEnvs();
				sData += "\n";

				return sData;
			}

		private:
			CCgiInput(const CCgiInput& rhs);
			CCgiInput operator=(const CCgiInput& rhs);

		private:
			lce::cgi::CCgiEnv m_oEnv;
		};

	}

}

#endif


