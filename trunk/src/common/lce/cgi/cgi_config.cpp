#include "cgi_config.h"
#include <errno.h>
#include <fstream>
#include "cgi_utils.h"
#include <stdio.h>


namespace lce
{
	namespace cgi
	{
		CConfig::CConfig(void)
		{
		}

		CConfig::~CConfig(void)
		{
		}

		bool CConfig::LoadConfig(const std::string& sCfgFileName)
		{
			std::ifstream ifsConfig;
			ifsConfig.open(sCfgFileName.c_str());
			if (ifsConfig.fail())
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"errno=%d",errno);
				return false;
			}

			std::string sApp;
			std::string sName;
			std::string sValue;
			std::string sLine;
			while (getline(ifsConfig,sLine))
			{
				if (sLine.empty())
				{
					continue;
				}

				size_t i = 0;
				for(i = 0; i < sLine.size(); i++)
				{
					if(sLine[i] != ' ' || sLine[i] != '\t') 
					{
						break;
					}
				}

				switch(sLine[i]) {
			case '#':
			case ';':
				break;
			case '[':
				{
					size_t j = sLine.find(']', i);
					if(std::string::npos != j)
					{
						sApp = sLine.substr(i+1, j-i-1);
						cgi::TrimString(sApp);
						if (sApp.empty())
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
			default:
				size_t j = sLine.find('=', i);
				if(j > i)
				{
					sName = sLine.substr(i, j-i);

					cgi::TrimString(sName);
					cgi::TrimString(sApp);

					if(!sName.empty())
					{
						sValue = sLine.substr(j+1);
						m_mapConfig[sApp][sName] = cgi::TrimString(sValue);
					}
				}
				break;
				}
			}

			return true;
		}

		std::string& CConfig::GetValue(const std::string& sApp, const std::string& sName, std::string& sValue, const std::string& sDefault)
		{
			sValue = sDefault;
			MAPCONFIG::iterator it = m_mapConfig.find(sApp);
			if (it != m_mapConfig.end())
			{
				CONFIG_VALUE::iterator subIt = it->second.find(sName);
				if (subIt != it->second.end())
				{
					sValue = subIt->second;
				}
			}

			return sValue;
		}

		int32_t CConfig::GetValue(const std::string& sApp, const std::string& sName,int32_t& iValue, const int32_t iDefault)
		{
			iValue = iDefault;
			MAPCONFIG::iterator it = m_mapConfig.find(sApp);
			if (it != m_mapConfig.end())
			{
				CONFIG_VALUE::iterator subIt = it->second.find(sName);
				if (subIt != it->second.end())
				{
					iValue = atoi(subIt->second.c_str());
				}
			}
			return iValue;
		}

		long CConfig::GetValue(const std::string& sApp, const std::string& sName,long& lValue, const long lDefault)
		{
			lValue = lDefault;
			MAPCONFIG::iterator it = m_mapConfig.find(sApp);
			if (it != m_mapConfig.end())
			{
				CONFIG_VALUE::iterator subIt = it->second.find(sName);
				if (subIt != it->second.end())
				{
					lValue =  atol(subIt->second.c_str());
				}
			}
			return lValue;
		}

		uint16_t CConfig::GetValue(const std::string& sApp, const std::string& sName, uint16_t& wValue,const uint16_t wDefault)
		{
			wValue = wDefault;
			MAPCONFIG::iterator it = m_mapConfig.find(sApp);
			if (it != m_mapConfig.end())
			{
				CONFIG_VALUE::iterator subIt = it->second.find(sName);
				if (subIt != it->second.end())
				{
					wValue =  static_cast<uint16_t>(atol(subIt->second.c_str()));
				}
			}
			return wValue;
		}

		unsigned long CConfig::GetValue(const std::string& sApp, const std::string& sName,unsigned long& dwValue,const unsigned long dwDefault)
		{
			dwValue = dwDefault;
			MAPCONFIG::iterator it = m_mapConfig.find(sApp);
			if (it != m_mapConfig.end())
			{
				CONFIG_VALUE::iterator subIt = it->second.find(sName);
				if (subIt != it->second.end())
				{
					dwValue = static_cast<unsigned long>(atoll(subIt->second.c_str()));
				}
			}
			return dwValue;
		}

		bool CConfig::GetValue(const std::string& sApp, const std::string& sName,bool& bValue, const bool bDefault)
		{
			bValue = bDefault;
			MAPCONFIG::iterator it = m_mapConfig.find(sApp);
			if (it != m_mapConfig.end())
			{
				CONFIG_VALUE::iterator subIt = it->second.find(sName);
				if (subIt != it->second.end())
				{
					bValue =  atol(subIt->second.c_str()) == 0 ? false : true;
				}
			}
			return bValue;
		}

		uint32_t CConfig::GetValue(const std::string& sApp, const std::string& sName,uint32_t& uiValue, const uint32_t uiDefault)
		{
			uiValue = uiDefault;
			MAPCONFIG::iterator it = m_mapConfig.find(sApp);
			if (it != m_mapConfig.end())
			{
				CONFIG_VALUE::iterator subIt = it->second.find(sName);
				if (subIt != it->second.end())
				{
					uiValue = static_cast<uint32_t>(atoll(subIt->second.c_str()));
				}
			}
			return uiValue;
		}

		bool CConfig::GetIPAndPort(const std::string& sApp, 
			const std::string& sName, 
			std::string &sIP, 
			uint16_t& wPort, 
			const std::string &sDefaultIP, 
			const uint16_t wDefaultPort)
		{
			sIP = "";
			wPort = 0;

			std::string sIPAndPort("");
			GetValue(sApp, sName, sIPAndPort, "");
			if(sIPAndPort.size() >= 10)
			{
				std::string::size_type ID = sIPAndPort.find(":");
				if(ID != std::string::npos)
				{
					sIP = sIPAndPort.substr(0, ID);
					wPort = (uint16_t)atoi(sIPAndPort.substr(ID+1).c_str());
				}
			}

			if(sIP.size() >= 7 && wPort > 0)
			{
				return true;
			}
			else
			{
				sIP = sDefaultIP;
				wPort = wDefaultPort;
				return false;
			}
		}

		bool CConfig::GetIPAndPortList(const std::string& sApp, 
			const std::string& sName, 
			std::vector<std::pair<std::string, uint16_t> >& vecSvrs, 
			const std::string &sDefaultIP, 
			const uint16_t wDefaultPort)
		{
			vecSvrs.clear();

			std::string sServerList("");
			GetValue(sApp, sName, sServerList, "");
			if(sServerList.size() >= 10  )
			{
				std::string::size_type id0 = 0, id1 = 0;
				while(1)
				{
					bool bEnd = false;

					std::string sServer;
					if((id1 = sServerList.find("|", id0)) != std::string::npos)
					{
						sServer = sServerList.substr(id0, id1-id0);
						id0 = id1 + 1;
					}
					else
					{
						sServer = sServerList.substr(id0);
						bEnd = true;
					}


					std::string::size_type ID = sServer.find(":");
					if(ID != std::string::npos)
					{
						std::string sIP = sServer.substr(0, ID);
						uint16_t wPort = (uint16_t)atoi(sServer.substr(ID+1).c_str());

						vecSvrs.push_back(std::make_pair(sIP, wPort));
					}

					if(bEnd)
					{
						break;
					}
				}
			}

			if(vecSvrs.size() > 0)
			{
				return true;
			}
			else
			{
				vecSvrs.push_back(std::make_pair(sDefaultIP, wDefaultPort));
				return false;
			}
		}


	}
}


