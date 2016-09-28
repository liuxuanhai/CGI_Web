#include <iostream>
#include <string>
#include <vector>
#include <zlib.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "cgi.h"

namespace lce
{
	namespace cgi
	{
		IMPL_LOGGER(Cgi, logger);

		cgi::CConfig Cgi::m_sta_Config;
		bool Cgi::m_sta_bIsLoadConfig = false;

		Cgi::Cgi(uint64_t qwFlag
                    , const std::string& strCgiConfigFile
                    , const std::string& strLogConfigFile)
		{
		    m_bPostOnly = ((qwFlag & FLAG_POST_ONLY) > 0);
		    m_sta_bIsLoadConfig = false;
		    m_strCgiConfigName = strCgiConfigFile;
		    m_strLogConfigName = strLogConfigFile;
		}

		bool Cgi::BaseInit()
		{
			if (!m_CgiInput.Init()) 
			{   
				LOG4CPLUS_ERROR(logger, m_strCgiName <<":m_CgiInput init Failed!");
				return false; 
			}

			m_strCgiName = GetInput().GetScriptName();
			m_strPath = GetInput().GetPathTranslated();

			if(!m_strPath.empty())
			{
				size_t pos = m_strPath.find_last_of('/');
				m_strPath.erase(pos);
				m_strPath += '/';
			}

			//log4cplus
			PropertyConfigurator::doConfigure((m_strPath + m_strLogConfigName).c_str());

			if(!m_sta_bIsLoadConfig)
			{
				std::string strConfigPath = m_strPath + m_strCgiConfigName;
				if( !m_sta_Config.LoadConfig(strConfigPath) )
				{   
					LOG4CPLUS_ERROR(logger, m_strCgiName <<":LoadConfig Failed!msg=" << m_sta_Config.GetErrMsg());
					return false; 
				}   
				m_sta_bIsLoadConfig = true;
			}   
			
			m_AnyValue.clear();
			m_tHeader.Reset();
			
			m_strUserIP = std::string(GetInput().GetClientIPStr());
			//m_strLocalIP = cgi::GetLocalIP();
			

			return true;
		}

		bool Cgi::CanZip()
		{
			char *szEncoding = getenv("HTTP_ACCEPT_ENCODING");
			bool bCanZip = (szEncoding != NULL && strstr(szEncoding, "gzip") != NULL);
			return bCanZip;
		}

		bool Cgi::Compress(string& strCompr, const string& strUnCompr)
		{
			uInt uSize = m_sta_Config.GetValue("cgi", "gz_size", 1024);

			if(strUnCompr.size() < uSize)
			{
				return false;
			}

			Bytef out[10240];
			z_stream zstrm;
			zstrm.next_in = (Bytef*)strUnCompr.c_str();
			zstrm.avail_in = strUnCompr.size();
			zstrm.next_out = out;
			zstrm.avail_out = sizeof(out);
			zstrm.zalloc = Z_NULL;
			zstrm.zfree = Z_NULL;
			zstrm.opaque = Z_NULL;

			int iRet = deflateInit2(&zstrm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY);
			if(Z_OK != iRet)
			{
				LOG4CPLUS_ERROR(logger, m_strCgiName <<":Compress failed,  deflateInit2 return "<<iRet);
				return false;
			}

			strCompr.clear();
			while(Z_OK == iRet)
			{
				iRet = deflate(&zstrm, Z_FINISH);
				if(Z_OK == iRet || Z_STREAM_END == iRet)
				{
					uInt have = sizeof(out) - zstrm.avail_out;
					if(have) strCompr.append((const char*)out, have);
					zstrm.next_out = out;
					zstrm.avail_out = sizeof(out);
				}
				else
				{
					deflateEnd(&zstrm);
					LOG4CPLUS_ERROR(logger, m_strCgiName <<":Compress failed,  deflate return "<<iRet);
					return false;
				}
			}
			deflateEnd(&zstrm);

			return true;
		}

		bool Cgi::DoReply(const int iErrCode,const std::string strErr, const bool bClean, const bool bNoCache, const bool bEncodeJS)
		{
			if(bNoCache)
			{
				m_tHeader.AddHeader("Cache-Control: max-age=0, must-revalidate");
			}

			std::string strOutput;

			if( m_strJson.empty())
			{
				if(0 != iErrCode)
				{
					if(bClean) m_AnyValue.clear();

					if (strErr.empty())
					{
						m_AnyValue["em"] = ReturnErrMsg(iErrCode);	
					}
					else
					{	
						m_AnyValue["em"] = strErr;
					}
				}

				m_AnyValue["ec"] = iErrCode;
				m_AnyValue.encodeJSON(true, bEncodeJS);
				strOutput = m_AnyValue.data();
			}
			else
			{
				strOutput = m_strJson;
			}

			std::ostringstream  osData;
			if(!m_strCallBack.empty())
			{

				switch(m_callBackType)
				{
					case 0:
						osData << m_strCallBack << "(" << strOutput <<")";
						break;
					case 1:
						osData << "<script>parent."<< m_strCallBack << "(" << strOutput <<")</script>";
						break;
				}
			}
			else
			{
				osData << strOutput;
			}

			std::string strCompr;
			if( CanZip() && Compress(strCompr, osData.str().data()))
			{
				m_tHeader.AddHeader("Content-Encoding: gzip");
		        m_tHeader.Output();
		        if(!std::cout.good())
		        {
		            std::cout.clear();
		            std::cout.flush();
		        }
				std::cout << strCompr;
			}
			else
			{
				m_tHeader.Output();
			    if(!std::cout.good())
			    {
			        std::cout.clear();
			        std::cout.flush();
			    }
				std::cout << osData.str().data();
			}

			return true;
		}

		bool Cgi::Run()
		{
			//timeval m_tBeginTime,m_tEndTime;
			gettimeofday(&m_tBeginTime,NULL);

			if(!BaseInit())
			{
				return false;
			}

			if (!DerivedInit())
			{
				return false;
			}
			
			if(m_bPostOnly && !GetInput().IsPostMethod())
			{
				DoReply(EC_POST_ONLY);
				return false;
			}

			if(!Process())
			{
				gettimeofday(&m_tEndTime,NULL);
				return false;
			}
			
			gettimeofday(&m_tEndTime,NULL);
			return true;
		}

		std::string Cgi::ReturnErrMsg(const int iErrCode)
		{
			  enum ReturnErrCode
                {
                    EC_OK = 0,   
                    EC_POST_ONLY = 1,   
                    EC_SERVER_BUSY = 2,
                    EC_PARAM_ERR = 3,
                    EC_BASE_KEY_ERR = 4,  
                };
		    static char sErrMsg[][64] = 
		    {
		        "ok",
		        "post only",
		        "server busy",
		        "param err",
		        "basekey err"
		    };

		    if((size_t)iErrCode <= sizeof(sErrMsg)/sizeof(sErrMsg[0]))
		    {
		        return std::string(sErrMsg[iErrCode]);
		    }
		    
		    return std::string("");
		}

	}
}
