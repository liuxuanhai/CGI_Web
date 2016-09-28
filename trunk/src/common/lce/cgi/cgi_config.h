#ifndef _LCE_CGI_CONFIG_H_
#define _LCE_CGI_CONFIG_H_

#include <string>
#include <map>
#include <vector>
#include "cgi_value.h"

namespace lce
{
	namespace cgi
	{
		class CConfig
		{
			typedef std::map<std::string, std::string> CONFIG_VALUE;
			typedef std::map<std::string, CONFIG_VALUE > MAPCONFIG;
		public:
			CConfig(void);
			~CConfig(void);

			bool LoadConfig(const std::string& sCfgFileName);
			
			inline CCgiValue GetValue(const std::string& sApp, const std::string& sName, const CCgiValue& vDefValue="")
			{
				MAPCONFIG::iterator it = m_mapConfig.find(sApp);
				if (it != m_mapConfig.end())
				{
					CONFIG_VALUE::iterator subIt = it->second.find(sName);
					if (subIt != it->second.end())
					{
						return CCgiValue(subIt->second);
					}
				}
				return vDefValue;
			}

			inline bool Has(const std::string& sApp, const std::string& sName)
			{
				MAPCONFIG::iterator it = m_mapConfig.find(sApp);
				if (it != m_mapConfig.end())
				{
					CONFIG_VALUE::iterator subIt = it->second.find(sName);
					if (subIt != it->second.end())
					{
						return true;
					}
				}
				return false;
			}

			std::string& GetValue(const std::string& sApp, const std::string& sName, std::string& sValue, const std::string& sDefault = "");
			int32_t GetValue(const std::string& sApp, const std::string& sName,int32_t& iValue, const int32_t iDefault = 0);
			uint32_t GetValue(const std::string& sApp, const std::string& sName,uint32_t& uiValue, const uint32_t uiDefault = 0);
			long GetValue(const std::string& sApp, const std::string& sName,long& lValue, const long lDefault = 0);
			uint16_t GetValue(const std::string& sApp, const std::string& sName,uint16_t& wValue, const uint16_t wDefault=0);
			unsigned long GetValue(const std::string& sApp, const std::string& sName,unsigned long& dwValue, const unsigned long dwDefault=0);
			bool GetValue(const std::string& sApp, const std::string& sName,bool& bValue, const bool bDefault = false );

			bool GetIPAndPort(const std::string& sApp, const std::string& sName, std::string &sIP, uint16_t& wPort, const std::string &sDefaultIP, const uint16_t wDefaultPort);
			bool GetIPAndPortList(const std::string& sApp, const std::string& sName, std::vector<std::pair<std::string, uint16_t> >& vecSvrs, const std::string &sDefaultIP, const uint16_t wDefaultPort);

			const char* GetErrMsg() const {	return m_szErrMsg;	}
			
		private:
			CConfig(const CConfig& rhs);
			CConfig& operator=(const CConfig& rhs);
		private:
			std::string m_sCfgFileName;
			char m_szErrMsg[1024];
			MAPCONFIG m_mapConfig;
		};
	}
}

#endif

