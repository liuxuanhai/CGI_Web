#include "cgi_http_connector.h"
#include "cgi_utils.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace lce
{
	namespace cgi
	{
		char CHTTPConnector::m_arrFirstCharIndex[255]={0};
		bool CHTTPConnector::m_bInitStaticData = false;
		const std::string CHTTPConnector::m_strBoundary = "--WebKitFormBoundaryjHKq7EtACCgzARiH";
		CHTTPConnector::DECODE_FUNC CHTTPConnector::m_arrTagFuncs[TAG_WIDTH*TAG_LENGHT] = {0};

		CHTTPConnector::CHTTPConnector()
		{
			if ( !m_bInitStaticData ){
				m_bInitStaticData = true;
				m_arrFirstCharIndex[(int32_t)'c'] = 1;
				m_arrFirstCharIndex[(int32_t)'C'] = 1;
				m_arrFirstCharIndex[(int32_t)'h'] = 2;
				m_arrFirstCharIndex[(int32_t)'H'] = 2;
				m_arrFirstCharIndex[(int32_t)'i'] = 3;
				m_arrFirstCharIndex[(int32_t)'I'] = 3;
				m_arrFirstCharIndex[(int32_t)'k'] = 4;
				m_arrFirstCharIndex[(int32_t)'K'] = 4;
				m_arrFirstCharIndex[(int32_t)'r'] = 5;
				m_arrFirstCharIndex[(int32_t)'R'] = 5;
				m_arrFirstCharIndex[(int32_t)'a'] = 6;
				m_arrFirstCharIndex[(int32_t)'A'] = 6;
				m_arrFirstCharIndex[(int32_t)'e'] = 7;
				m_arrFirstCharIndex[(int32_t)'E'] = 7;
				m_arrFirstCharIndex[(int32_t)'t'] = 8;
				m_arrFirstCharIndex[(int32_t)'T'] = 8;
				m_arrFirstCharIndex[(int32_t)'q'] = 9;
				m_arrFirstCharIndex[(int32_t)'Q'] = 9;
				m_arrFirstCharIndex[(int32_t)'u'] = 10;
				m_arrFirstCharIndex[(int32_t)'U'] = 10;

				m_arrTagFuncs[10*TAG_WIDTH+1] = &decode_connection;
				m_arrTagFuncs[12*TAG_WIDTH+1] = &decode_content_type;
				m_arrTagFuncs[14*TAG_WIDTH+1] = &decode_content_length;
				m_arrTagFuncs[17*TAG_WIDTH+8] = &decode_transfer_encoding;
			}	
			
			m_mapHeads["Connection"] = "Keep-Alive";
			m_mapHeads["Accept"] = "*/*";	
			m_bUseForm = false;
		}



		// bool CHTTPConnector::SetCookie(const std::string& sName, const std::string& sValue, const std::string& sDomain, const time_t dwCookieTime, const std::string& sPath)
		// {
			// SCookieInfo stCookieInfo;
			// stCookieInfo.dwCookieTime = dwCookieTime;
			// stCookieInfo.sDomain = sDomain;
			// stCookieInfo.sPath = sPath;
			// stCookieInfo.sValue = sValue;
			// m_mapCookies[sName] = stCookieInfo;

			// return true;
		// }

		bool CHTTPConnector::SetHead(const std::string& sName, const std::string& sValue)
		{
			m_mapHeads[sName] = sValue;

			return true;
		}

		/*
		bool CHTTPConnector::SetValue(const std::string& sName, const std::string& sValue)
		{
			m_mapValues[sName] = sValue;

			return true;
		}
		*/

		//bool CHTTPConnector::SetFile(const std::string& sName, const std::string& sValue)
		bool CHTTPConnector::SetValue(
				const std::string& sName, 
				const std::string& sValue,
				const std::string& sFileName,
				const std::string& sContentType)
		{
			m_mapValues[sName].sName = sName;
			m_mapValues[sName].sValue = sValue;
			m_mapValues[sName].sFileName = sFileName;
			if(!sContentType.empty())
			{
				m_mapValues[sName].sContentType = sContentType;
				m_bUseForm = true;
			}


			return true;
		}

		bool CHTTPConnector::SetBody(const std::string& sBody)
		{
			m_sReqBodyData = sBody;

			return true;
		}

		void CHTTPConnector::Clear()
		{
			m_mapHeads.clear();
		//	m_mapCookies.clear();
			m_mapValues.clear();
			m_mapValues.clear();
			m_sReqBodyData.clear();
			m_mapHeads["Connection"] = "Keep-Alive";
			m_mapHeads["Accept"] = "*/*";	

		}

		std::string CHTTPConnector::EncodeForm()
		{
			std::string strResult;

			for ( MAP_VALUE::const_iterator it = m_mapValues.begin(); it != m_mapValues.end(); ++it)
			{
				SMimeValue stMValue = it->second;

				strResult += "--" + m_strBoundary + "\r\n";

				strResult += "Content-Disposition: form-data; ";
				strResult += "name=\"" + it->first + "\"";
				if(!stMValue.sFileName.empty())
					strResult += "; filename=\"" + stMValue.sFileName + "\"";

				if(!stMValue.sContentType.empty())
					strResult += "\r\nContent-Type: " + stMValue.sContentType;

				strResult += "\r\n\r\n" + stMValue.sValue;
				strResult += "\r\n";
			}

			strResult += "--" + m_strBoundary + "--\r\n";
			return strResult;
		}

		bool CHTTPConnector::EncodeRequest(std::string& sRequestData, const std::string& sURI, const HTTP_METHOD nHttpMethod)
		{
			std::string sValues;
			if ( nHttpMethod == METHOD_POST && m_bUseForm )
			{
				m_mapHeads["Content-Type"] = "multipart/form-data; boundary=" + m_strBoundary;
				sValues += EncodeForm();
			}
			else
			{
				for ( MAP_VALUE::const_iterator it=m_mapValues.begin(); it!=m_mapValues.end(); ++it )
				{
					sValues += (it==m_mapValues.begin() ? "" : "&") + cgi::FormUrlEncode(it->second.sName) + "=" + cgi::FormUrlEncode(it->second.sValue);
				}
			}
			
			sRequestData = GetHttpMethod(nHttpMethod) + std::string(" ") + sURI;
			if ( m_mapValues.size() > 0 && nHttpMethod == METHOD_GET )
			{
				if ( std::string::npos != sURI.find('?') )
					sRequestData += "&";
				else
					sRequestData += "?";
				sRequestData += sValues;
			}
			sRequestData += " HTTP/1.1\r\n";;
			
			for ( MAP_HEAD::const_iterator it=m_mapHeads.begin(); it!=m_mapHeads.end(); ++it )
			{
				sRequestData += it->first + ": " + it->second + "\r\n";
			}
			
			//set cookie
			// for (MAP_COOKIE::const_iterator it=m_mapCookies.begin(); it!=m_mapCookies.end(); ++it)
			// {
				// sRequestData += "Set-Cookie:"  + it->first + "=" + it->second.sValue;
				// if ( !it->second.sDomain.empty() )
					// sRequestData += "; Domain=" + it->second.sDomain;
				// if ( it->second.dwCookieTime != 0 )
					// sRequestData += "; Expires=" + getGMTDate(it->second.dwCookieTime+time(NULL));
				// if ( !it->second.sPath.empty() )
					// sRequestData += "; Path=" + it->second.sPath;
				// sRequestData += "\r\n";
			// }
			
			if ( nHttpMethod == METHOD_POST )
			{
				if ( m_sReqBodyData.empty() )
				{
					sRequestData += "Content-Length: " + cgi::ToStr(sValues.size()) + "\r\n";
				}
				else
				{
					sRequestData += "Content-Length: " + cgi::ToStr(m_sReqBodyData.size()) + "\r\n";
				}
			}
			sRequestData += "\r\n";
			
			
			if ( nHttpMethod == METHOD_POST )
			{
				if ( m_sReqBodyData.empty() )
				{
					sRequestData += sValues;
				}
				else
				{
					sRequestData += m_sReqBodyData;
				}
			}

			m_sReqBodyData = sRequestData;

			return true;
		}

		bool CHTTPConnector::Execute(const std::string& sIP, const int16_t wPort, const std::string& sUrl, const HTTP_METHOD nHttpMethod, const time_t uiTimeout)
		{
			time_t nStartTime = cgi::GetTickCount();
			if ( !m_oHttp.Connect(sIP, wPort) )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "http-connect error:%s", m_oHttp.GetErrMsg());
				return false;
			}

			MAP_HEAD::const_iterator itHost = m_mapHeads.find("Host");
			if ( m_mapHeads.end() == itHost )
			{
				m_mapHeads["Host"] = sIP;
			}
			
			std::string sSend;
			if ( !EncodeRequest(sSend, sUrl, nHttpMethod) )
			{
				return false;
			}
			

			if ( m_oHttp.Write(sSend, uiTimeout) < 0 )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "http-write error:%s", m_oHttp.GetErrMsg());
				return false;
			}
			
			time_t dwCurTick = cgi::GetTickCount();
			if ( dwCurTick > uiTimeout + nStartTime )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "send timeout");
				return false;
			}
			
			m_sData.clear();
			m_sBodyData.clear();
			std::string sReadData;
			bool bReadHeadOk = false;
			std::string::size_type nHeadSize = std::string::npos;
			std::string::size_type nReadChunkPos = std::string::npos;
			std::string::size_type nCurChunkSize = -1;
			std::string::size_type nCurChunkNeedReadSize = 0;
			while(true)
			{
				std::string sTmp;
				if ( m_oHttp.Read(sTmp, uiTimeout-(dwCurTick-nStartTime)) < 0 )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "recv error:%s", m_oHttp.GetErrMsg());
					return false;
				}
				
				
				if ( sTmp.size() == 0 )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "recv data no finish, close by the other side.");
					return false;
				}
				
				m_sData += sTmp;
				
				
				if ( !bReadHeadOk )
				{
					if ( m_sData.size() > 10 )
					{
						if( *(uint32_t *)m_sData.data() == *(uint32_t *)"HTTP" 
							&& m_sData[4] == '/' && isdigit(m_sData[5]) 
							&& m_sData[6] == '.' && isdigit(m_sData[7])
							&& m_sData[8] == ' ') 
						{
							std::string::size_type nHeadEnd = m_sData.find("\r\n\r\n");
							if ( nHeadEnd != std::string::npos )
							{
								if ( !AnalyzeResponse(m_sData.data(), m_sData.size()) )
								{
									return false;
								}
								bReadHeadOk = true;
								nHeadSize = nHeadEnd + 4;
							}
							else
							{
								if ( m_sData.size() > 1024*1024 )
								{
									snprintf(m_szErrMsg, sizeof(m_szErrMsg), "can't find http head end.");
									return false;
								}
							}
						}
						else
						{
							snprintf(m_szErrMsg, sizeof(m_szErrMsg), "http head error:format error.");
							return false;
						}
					}
				}
				
				if ( bReadHeadOk )
				{
					if ( m_stBase.bHasContentLength )
					{
						if ( m_stBase.nContentLength <= m_sData.size() - nHeadSize  )
						{
							m_sBodyData = m_sData.substr(nHeadSize);
							return true;
						}
					}
					else if ( m_stBase.bHasChunked )
					{
						if ( std::string::npos == nReadChunkPos )
						{
							nReadChunkPos = nHeadSize;
						}
						
						while(nReadChunkPos < m_sData.size() )
						{
							if ( nCurChunkNeedReadSize <= 0 )
							{
								std::string::size_type nEnd= m_sData.find("\r\n", nReadChunkPos);
								if ( std::string::npos != nEnd )
								{
									char* pTmpEnd = NULL;
									nCurChunkSize = nCurChunkNeedReadSize = strtoul(m_sData.data()+nReadChunkPos, &pTmpEnd, 16);
									nReadChunkPos = nEnd + 2;
									
									if ( 0 == nCurChunkSize )
									{
										//recv finish
										return true;
									}
								}
								else
								{
									break;
								}
							}
							
							if ( m_sData.size() >= nReadChunkPos + nCurChunkNeedReadSize + 2 )
							{
								m_sBodyData.append(m_sData, nReadChunkPos, nCurChunkNeedReadSize);
								nReadChunkPos += nCurChunkNeedReadSize;
								nCurChunkNeedReadSize = 0;
								if ( m_sData[nReadChunkPos] != '\r' 
									|| m_sData[nReadChunkPos+1] != '\n' )
								{
									snprintf(m_szErrMsg, sizeof(m_szErrMsg), "chunk format error: can't find CRLF.");
									return false;
								}
								nReadChunkPos += 2;
							}
							
							if ( nCurChunkNeedReadSize > 0 )
							{
								break;
							}
						}
					}
					else
					{
						//no body
						return true;
					}
				}
			
			}
			
			
			return false;
		}

		bool CHTTPConnector::AnalyzeResponse(const char* pszData, const int32_t dwSize)
		{
			m_stBase.bRequest = false;
			const char* pLine = pszData;
			if(*(uint32_t *)pLine != *(uint32_t *)"HTTP" ||
				pLine[4] != '/' || !isdigit(pLine[5]) ||
				pLine[6] != '.' || !isdigit(pLine[7]) || pLine[8] != ' ') 
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "AnalyzeResponse error: version error, %s.", std::string(pLine, 20).c_str());
				return false;
			}

			//http version
			if(pLine[5]>'1' || (pLine[5]=='1' && pLine[7]>='1'))
			{	
				m_stBase.bHttpV11 = 1;
				m_stBase.bKeepAlive = true;
			}
			else
				m_stBase.bKeepAlive = false;


			pLine += 9;
			//http code
			if ( (pLine[0] != '1' && pLine[0] != '2' && pLine[0] != '3' && pLine[0] != '4' && pLine[0] != '5') || !isdigit(pLine[1]) || !isdigit(pLine[2]) || isdigit(pLine[3]) )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "AnalyzeResponse error: httpcode error, %s.", std::string(pLine, 5).c_str());
				return false;
			}
			m_stBase.nStatusCode = atoi(pLine);

			const char* pLineEnd = NULL;
			const char* pTmpLineEnd = NULL;
			pLineEnd = strchr(pLine, '\n');
			if ( NULL == pLineEnd )
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "AnalyzeResponse error: no complete line, %s.", pszData);
				return false;
			}
			pLine = pLineEnd + 1;

			const char* pTagEnd = NULL;
			int32_t iCurOtherHeadIndex = 0;
			int32_t n=0;
			while ( true )
			{
				pLineEnd = strchr(pLine, '\n');
				if ( NULL == pLineEnd )
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "AnalyzeResponse parse line error:can't find '\\n',line=%s.", pLine);
					return false;
				}
				if ( pLine[0] == '\r' && pLine[1] == '\n' )
				{
					//head end
					break;
				}

				const char* pValueStart = NULL;
				bool bOtherHead = true;
				/* parse tags */
				if( (pTagEnd = strchr(pLine, ':')) != NULL && pTagEnd < pLineEnd ) 
				{
					if ( pLineEnd[-1]=='\r') 
						pTmpLineEnd = pLineEnd-1; 
					else
						pTmpLineEnd = pLineEnd;

					n = pTagEnd - pLine;
					DECODE_FUNC pFun = m_arrTagFuncs[n*TAG_WIDTH + m_arrFirstCharIndex[*(unsigned char *)pLine]];
					if ( NULL != pFun )
					{
						bOtherHead = !(*pFun)(*this, pLine, pTmpLineEnd);
					}

					if ( bOtherHead )
					{
						//copy name
						if ( n < OTHER_HEAD_NAME_LEN )
						{
							memcpy(m_arrOtherHeads[iCurOtherHeadIndex].szName, pLine, n);
							m_arrOtherHeads[iCurOtherHeadIndex].szName[n] = 0;
						}
						else
						{
							memcpy(m_arrOtherHeads[iCurOtherHeadIndex].szName, pTagEnd+1, OTHER_HEAD_NAME_LEN);
							m_arrOtherHeads[iCurOtherHeadIndex].szName[OTHER_HEAD_NAME_LEN-1] = 0;
						}

						//copy value
						pValueStart = pTagEnd+1;
						while(*pValueStart==' '||*pValueStart=='\t'||*pValueStart=='\r') pValueStart++;
						int32_t nCnt = pTmpLineEnd - pValueStart;
						if ( nCnt > OTHER_HEAD_DATA_LEN )
						{
							memcpy(m_arrOtherHeads[iCurOtherHeadIndex].szValue, pValueStart, OTHER_HEAD_DATA_LEN);
							m_arrOtherHeads[iCurOtherHeadIndex].szValue[OTHER_HEAD_DATA_LEN-1] = 0;
						}
						else if ( nCnt >= 0 )
						{
							memcpy(m_arrOtherHeads[iCurOtherHeadIndex].szValue, pValueStart, nCnt);
							m_arrOtherHeads[iCurOtherHeadIndex].szValue[nCnt] = 0;
						}
						++iCurOtherHeadIndex;
					}
				}
				else
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "AnalyzeResponse parse tags error:%s.", pLine);
					m_stBase.nStatusCode = 400;
					return false;
				}

				pLine = pLineEnd + 1;
			}
			return true;
		}

	}
}


