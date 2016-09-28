#include "cgi_http_header.h"

namespace lce
{
	namespace cgi
	{
		CHttpHeader::CHttpHeader()
		:m_sContentType("text/html")
		,m_iStatusCode(200)
		{
			m_vecHeaders.clear();
			m_vecCookies.clear();
			m_sStatusMsg = "";
			m_sContentType = "text/html";
		}

		CHttpHeader::~CHttpHeader()
		{

		}


		void CHttpHeader::SetCookie(const std::string& sName, 
								   const std::string& sValue,
								   const std::string& sDomain, 
								   const uint32_t dwMaxAge, 
								   const std::string& sPath,
								   const bool bSecure,
								   const bool bHttpOnly)
		{
			SCookie stCookie;
			stCookie.bHttpOnly = bHttpOnly;
			stCookie.bSecure = bSecure;
			stCookie.dwMaxAge = dwMaxAge;
			stCookie.sDomain = sDomain;
			stCookie.sName = sName;
			stCookie.sValue = sValue;
			stCookie.sPath = sPath;
			m_vecCookies.push_back(stCookie);
		}
		
	}
}





