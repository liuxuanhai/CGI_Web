#include "bd_api.h"
#include "jsoncpp/json.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

namespace common
{
	namespace bdapi
	{
		IMPL_LOGGER(BDAPI, logger);

		int BDAPI::HttpGet(const std::string& strHost, uint16_t wPort, const std::string& strAPI, std::string& strRespBody)
		{
			strRespBody = "";

			in_addr_t stIP = DNSParse(strHost);
			if(0 == stIP)
			{
				LOG4CPLUS_ERROR(logger, "HttpGet failed, DNSParse failed, strHost="<<strHost);
				return -1;
			}

			int iSockFD = socket(AF_INET, SOCK_STREAM, 0);
			if (iSockFD < 0)
			{
			    LOG4CPLUS_ERROR(logger, "HttpGet failed, create socket failed, msg="<<strerror(errno));
			    return -1;
			}
			
			struct sockaddr_in server_addr;
			bzero(&server_addr, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(wPort);
			server_addr.sin_addr.s_addr = stIP;
			if (connect(iSockFD, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) < 0)
			{
			    LOG4CPLUS_ERROR(logger, "HttpGet failed, connect failed, msg="<<strerror(errno));
			    return -1;
			}
			
			bzero(_aczRequest, 2048);
			snprintf(_aczRequest, 2048, "GET /%s HTTP/1.1\r\n"
					"Host: %s\r\n"
			        "Connection: close\r\n"
			        "Accept: */*\r\n\r\n"
			        , strAPI.c_str()
			        , strHost.c_str());
			
			int iSend = 0;
			int iTotalSend = 0;
			int iBytes = strlen(_aczRequest);
			while (iTotalSend < iBytes)
			{
			    iSend = write(iSockFD, _aczRequest + iTotalSend, iBytes - iTotalSend);
			    if (iSend < 0)
			    {
			        LOG4CPLUS_ERROR(logger, "HttpGet failed, write failed, msg="<<strerror(errno));
			        return -1;
			    }
			
			    iTotalSend += iSend;
			}

			std::string strReadResult = "";
			bzero(_aczResponse, 2048);
			while ((iBytes = read(iSockFD, _aczResponse, 2047)) > 0)
			{
			    strReadResult.append(_aczResponse, iBytes);
			    bzero(_aczResponse, 2048);
			}
			
			LOG4CPLUS_DEBUG(logger, "HttpGet, recv size="<<strReadResult.size());
			std::string strSplit = "\r\n\r\n";
			std::size_t ipos = strReadResult.find(strSplit);
			if(ipos != std::string::npos)
			{
			   	strRespBody = strReadResult.substr(ipos + strSplit.size());
			}
			
			LOG4CPLUS_DEBUG(logger, "HttpGet, recv body size="<<strRespBody.size());
			
			close(iSockFD);
			return 0;
		}

		int BDAPI::HttpPost(const std::string& strHost, uint16_t wPort, const std::string& strAPI,
							const std::string& strReqBody, std::string& strRespBody, const std::string& strContentType)
		{
			strRespBody = "";
		
			in_addr_t stIP = DNSParse(strHost);
			if(0 == stIP)
			{
				LOG4CPLUS_ERROR(logger, "HttpPost failed, DNSParse failed, strHost="<<strHost);
				return -1;
			}
		
			int iSockFD = socket(AF_INET, SOCK_STREAM, 0);
			if (iSockFD < 0)
			{
			    LOG4CPLUS_ERROR(logger, "HttpPost failed, create socket failed, msg="<<strerror(errno));
			    return -1;
			}
			
			struct sockaddr_in server_addr;
			bzero(&server_addr, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(wPort);
			server_addr.sin_addr.s_addr = stIP;
			if (connect(iSockFD, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) < 0)
			{
			    LOG4CPLUS_ERROR(logger, "HttpPost failed, connect failed, msg="<<strerror(errno));
			    return -1;
			}
			
			bzero(_aczRequest, 2048);
			snprintf(_aczRequest, 2048, "POST /%s HTTP/1.1\r\n"
				   						"Content-Type: %s\r\n"
										"Content-Length: %zu\r\n"
										"Connection: close\r\n"
										"Host: %s\r\n\r\n%s",
								 strAPI.c_str(),
								 strContentType.c_str(),
								 strReqBody.size(),
								 strHost.c_str(),
								 strReqBody.c_str());
			//LOG4CPLUS_DEBUG(logger, "req = " << _aczRequest);

			int iSend = 0;
			int iTotalSend = 0;
			int iBytes = strlen(_aczRequest);
			while (iTotalSend < iBytes)
			{
			    iSend = write(iSockFD, _aczRequest + iTotalSend, iBytes - iTotalSend);
			    if (iSend < 0)
			    {
			        LOG4CPLUS_ERROR(logger, "HttpPost failed, write failed, msg="<<strerror(errno));
			        return -1;
			    }
			
			    iTotalSend += iSend;
			}
			
			std::string strReadResult = "";
			bzero(_aczResponse, 2048);
			while ((iBytes = read(iSockFD, _aczResponse, 2047)) > 0)
			{
			    strReadResult.append(_aczResponse, iBytes);
			    bzero(_aczResponse, 2048);
			}
			
			LOG4CPLUS_DEBUG(logger, "HttpPost, recv size="<<strReadResult.size());
			std::string strSplit = "\r\n\r\n";
			std::size_t ipos = strReadResult.find(strSplit);
			 if(ipos != std::string::npos)
			 {
				strRespBody = strReadResult.substr(ipos + strSplit.size());
			 }
			
			 LOG4CPLUS_DEBUG(logger, "HttpPost, recv body size="<<strRespBody.size());
			
			close(iSockFD);
			
			return 0;
		}

		in_addr_t BDAPI::DNSParse(const std::string& strHost)
		{
			struct hostent stHostent;
			struct hostent *pHostent = NULL;
			int iHErrNO = 0;
			int iRet = ::gethostbyname_r(strHost.c_str(), &stHostent, _aczHostentTempBuffer, 8192, &pHostent, &iHErrNO);
			if (iRet != 0)
			{
				LOG4CPLUS_ERROR(logger, "DNSParse failed, gethostbyname_r return" << iRet << ", iHErrNO=" << iHErrNO);
				return 0;
			}

			if (!pHostent)
			{
				LOG4CPLUS_ERROR(logger,
								"DNSParse failed, gethostbyname_r return 0, but pHostent is NULL, maybe invalid host=" <<
								strHost);
				return 0;
			}

			struct in_addr *pInAddr = (struct in_addr *) pHostent->h_addr;
			if (!pInAddr)
			{
				LOG4CPLUS_ERROR(logger, "DNSParse failed, gethostbyname_r return 0, but pInAddr is NULL, maybe invalid host=" <<
										strHost);
				return 0;
			}

			in_addr_t stIP = pInAddr->s_addr;

			LOG4CPLUS_DEBUG(logger, "DNSParse succ, strHost=" << strHost << ", ip=" << inet_ntoa(*pInAddr));
			return stIP;
		}
	}
}
