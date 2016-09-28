#include "wx_api.h"
#include "jsoncpp/json.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <curl/curl.h>

namespace common
{
		namespace wxapi
		{
				IMPL_LOGGER(WXAPI,logger);

				int WXAPI::PAGetAccessToken(const std::string& strAppid, const std::string& strAppSecret, std::string& strPAAccessToken)
				{
						//https://api.weixin.qq.com/cgi-bin/token?grant_type=client_credential&appid=APPID&secret=APPSECRET
						std::string strAPI = "cgi-bin/token";
						strAPI += "?appid=" + strAppid;
						strAPI += "&secret=" + strAppSecret;
						strAPI += "&grant_type=client_credential";

						std::string strRspBody = "";
						int iRet = HttpsGet("api.weixin.qq.com", strAPI, strRspBody);
						if(iRet < 0)
						{
								LOG4CPLUS_ERROR(logger, "PAGetAccessToken failed, msg=HttpsGet failed");
								return -1;
						}

						Json::Value oJson;
						Json::Reader reader;
						if(!reader.parse(strRspBody, oJson, false))
						{
								LOG4CPLUS_ERROR(logger, "PAGetAccessToken failed, parse json error");
								return -1;
						}

						if(oJson.hasKey("errcode") && oJson["errcode"].asInt() != 0)
						{
								LOG4CPLUS_ERROR(logger, "PAGetAccessToken failed, wx return errcode="<<oJson["errcode"].asInt());
								return -1;
						}

						if(!oJson.hasKey("access_token"))
						{
								LOG4CPLUS_ERROR(logger, "PAGetAccessTokens failed, wx return invalid, access_token not found!!");
								return -1;
						}

						if(!oJson.hasKey("expires_in"))
						{
								LOG4CPLUS_ERROR(logger, "PAGetAccessToken failed, wx return invalid, expires_in not found!!");
								return -1;
						}

						strPAAccessToken = oJson["access_token"].asString();

						LOG4CPLUS_DEBUG(logger, "PAGetAccessToken succ, strAccessToken="<<strPAAccessToken);
						return 0;
				}


				int WXAPI::HttpsGet(const std::string& strHost, const std::string& strAPI, std::string& strRespBody)
				{
						//DNS
						in_addr_t stIP = DNSParse(strHost);
						if(0 == stIP)
						{
							LOG4CPLUS_ERROR(logger, "HttpsGet failed, DNSParse failed, strHost="<<strHost);
					        return -1;
						}
			
						//tcp connect
						int iSockFD = socket(AF_INET, SOCK_STREAM, 0);
						if (iSockFD < 0)
						{
							LOG4CPLUS_ERROR(logger, "HttpsGet failed, Socket Error:" << strerror(errno));
							return -1;
						}

						struct sockaddr_in server_addr;
						bzero(&server_addr, sizeof(server_addr));
						server_addr.sin_family = AF_INET;
						server_addr.sin_port = htons(443);
						server_addr.sin_addr.s_addr = stIP;
						if (connect(iSockFD, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) < 0)
						{
								LOG4CPLUS_ERROR(logger, "HttpsGet failed, Connect Error:" << strerror(errno));
								return -1;
						}

						//ssl connect
						SSL_library_init();
						//ERR_load_SSL_strings();
						//SSL_load_error_strings();
						SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
						if (ctx == NULL)
						{
								LOG4CPLUS_ERROR(logger, "HttpsGet failed, SSL_CTX_new failed");
								return -1;
						}

						SSL *ssl = SSL_new(ctx);
						if (ssl == NULL)
						{
								LOG4CPLUS_ERROR(logger, "HttpsGet failed, SSL_new failed");
								return -1;
						}

						int iRet = SSL_set_fd(ssl, iSockFD);
						if (iRet == 0)
						{
								LOG4CPLUS_ERROR(logger, "HttpsGet failed, SSL_set_fd failed");
								return -1;
						}

						RAND_poll();
						while (RAND_status() == 0)
						{
								unsigned short rand_ret = rand() % 65536;
								RAND_seed(&rand_ret, sizeof(rand_ret));
						}

						iRet = SSL_connect(ssl);
						if (iRet != 1)
						{
								LOG4CPLUS_ERROR(logger, "HttpsGet failed, SSL_connect failed");
								return -1;
						}


						//send
						bzero(_aczRequest, 2048);
						snprintf(_aczRequest, 2048, "GET /%s HTTP/1.1\r\nHost: %s\r\n"
										"Connection: close\r\n"
										"Accept: */*\r\n\r\n", strAPI.c_str(), strHost.c_str());

						int iSend = 0;
						int iTotalSend = 0;
						int iBytes = strlen(_aczRequest);
						while (iTotalSend < iBytes)
						{
								iSend = SSL_write(ssl, _aczRequest + iTotalSend, iBytes - iTotalSend);
								if (iSend < 0)
								{
										LOG4CPLUS_ERROR(logger, "HttpsGet failed, SSL_write failed");
										return -1;
								}

								iTotalSend += iSend;
						}

						LOG4CPLUS_DEBUG(logger, "HttpsGet, send succ: "<< _aczRequest);

						//recv
						std::string strReadResult = "";
						bzero(_aczResponse, 2048);
						while ((iBytes = SSL_read(ssl, _aczResponse, 2047)) > 0)
						{
								strReadResult.append(_aczResponse, iBytes);
								bzero(_aczResponse, 2048);
						}

						//parse http response body
						std::string strSplit = "\r\n\r\n";
						std::size_t ipos = strReadResult.find(strSplit);
						if(ipos == std::string::npos)
						{
								LOG4CPLUS_ERROR(logger, "HttpsGet failed, parse https resonse failed, response="+strReadResult;);
								return -1;
						}

						strRespBody = strReadResult.substr(ipos + strSplit.size());
						LOG4CPLUS_DEBUG(logger, "HttpsGet, recv succ: " + strRespBody);

						//close ssl and tcp
						iRet = SSL_shutdown(ssl);
						close(iSockFD);
						SSL_free(ssl);
						SSL_CTX_free(ctx);
						//ERR_free_strings();

						return 0;
				}

				int WXAPI::HttpsPost(const std::string& strHost, const std::string& strAPI, const std::string& strReqBody, std::string& strRespBody, const std::string& strContentType, uint16_t wPort)
				{
						//DNS
						in_addr_t stIP = DNSParse(strHost);
						if(0 == stIP)
						{
							LOG4CPLUS_ERROR(logger, "HttpsPost failed, DNSParse failed, strHost="<<strHost);
					        return -1;
						}

						//tcp connect
						int iSockFD = socket(AF_INET, SOCK_STREAM, 0);
						if (iSockFD < 0)
						{
							LOG4CPLUS_ERROR(logger, "HttpsPost failed, Socket Error:" << strerror(errno));
							return -1;
						}

						struct sockaddr_in server_addr;
						bzero(&server_addr, sizeof(server_addr));
						server_addr.sin_family = AF_INET;
						server_addr.sin_port = htons(wPort);
						server_addr.sin_addr.s_addr = stIP;
						if (connect(iSockFD, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) < 0)
						{
								LOG4CPLUS_ERROR(logger, "HttpsPost failed, Connect Error:" << strerror(errno));
								return -1;
						}

						//ssl connect
						SSL_library_init();
						//ERR_load_SSL_strings();
						//SSL_load_error_strings();
						SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
						if (ctx == NULL)
						{
								LOG4CPLUS_ERROR(logger, "HttpsPost failed, SSL_CTX_new failed");
								return -1;
						}

						SSL *ssl = SSL_new(ctx);
						if (ssl == NULL)
						{
								LOG4CPLUS_ERROR(logger, "HttpsPost failed, SSL_new failed");
								return -1;
						}

						int iRet = SSL_set_fd(ssl, iSockFD);
						if (iRet == 0)
						{
								LOG4CPLUS_ERROR(logger, "HttpsPost failed, SSL_set_fd failed");
								return -1;
						}

						RAND_poll();
						while (RAND_status() == 0)
						{
								unsigned short rand_ret = rand() % 65536;
								RAND_seed(&rand_ret, sizeof(rand_ret));
						}

						iRet = SSL_connect(ssl);
						if (iRet != 1)
						{
								LOG4CPLUS_ERROR(logger, "HttpsPost failed, SSL_connect failed");
								return -1;
						}


						//send
						bzero(_aczRequest, 2048);

//						snprintf(_aczRequest, 2048, "GET /%s HTTP/1.1\r\nHost: %s\r\n"
//										"Connection: close\r\n"
//										"Accept: */*\r\n\r\n", strAPI.c_str(), strHost.c_str());

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

						int iSend = 0;
						int iTotalSend = 0;
						int iBytes = strlen(_aczRequest);
						while (iTotalSend < iBytes)
						{
								iSend = SSL_write(ssl, _aczRequest + iTotalSend, iBytes - iTotalSend);
								if (iSend < 0)
								{
										LOG4CPLUS_ERROR(logger, "HttpsPost failed, SSL_write failed");
										return -1;
								}

								iTotalSend += iSend;
						}

						LOG4CPLUS_DEBUG(logger, "HttpsPost, send succ: "<< _aczRequest);

						//recv
						std::string strReadResult = "";
						bzero(_aczResponse, 2048);
						while ((iBytes = SSL_read(ssl, _aczResponse, 2047)) > 0)
						{
								strReadResult.append(_aczResponse, iBytes);
								bzero(_aczResponse, 2048);
						}

						//parse http response body
						std::string strSplit = "\r\n\r\n";
						std::size_t ipos = strReadResult.find(strSplit);
						if(ipos == std::string::npos)
						{
								LOG4CPLUS_ERROR(logger, "HttpsPost failed, parse https resonse failed, response="+strReadResult;);
								return -1;
						}

						strRespBody = strReadResult.substr(ipos + strSplit.size());
						LOG4CPLUS_DEBUG(logger, "HttpsPost, recv succ: " + strRespBody);

						//close ssl and tcp
						iRet = SSL_shutdown(ssl);
						close(iSockFD);
						SSL_free(ssl);
						SSL_CTX_free(ctx);
						//ERR_free_strings();


						return 0;
				}

				int WXAPI::CurlSSLPost(const std::string& strUrl
											, const std::string& strReqBody
											, std::string& strRspBody
											, const std::string& strRootcaPath
											, const std::string& strAPIClientCertPath
											, const std::string& strAPIClientKeyPath
											)
				{
					CURL *pCurl;
					CURLcode eRetCode;

					eRetCode = curl_global_init(CURL_GLOBAL_SSL);
					if(CURLE_OK != eRetCode)
					{
						LOG4CPLUS_ERROR(logger, "curl_global_init failed, RetCode: "<<eRetCode<<", ErrMsg: "<<curl_easy_strerror(eRetCode));
						return -1;
					}

					pCurl = curl_easy_init();

					curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 5);	
					curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());
					curl_easy_setopt(pCurl, CURLOPT_HEADER, true);	
					curl_easy_setopt(pCurl, CURLOPT_POST, true);
					curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strReqBody.c_str());
					curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2);	
					curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, true);

					curl_easy_setopt(pCurl, CURLOPT_CAINFO, strRootcaPath.c_str());	
					curl_easy_setopt(pCurl, CURLOPT_SSLCERTTYPE, "PEM");

					curl_easy_setopt(pCurl, CURLOPT_SSLCERT, strAPIClientCertPath.c_str());	
					curl_easy_setopt(pCurl, CURLOPT_SSLKEYTYPE, "PEM");

					curl_easy_setopt(pCurl, CURLOPT_SSLKEY, strAPIClientKeyPath.c_str());

					std::stringstream ssBody;
					curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_data);	
					curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &ssBody);
					
					eRetCode = curl_easy_perform(pCurl);	
					if(CURLE_OK != eRetCode)
					{
						LOG4CPLUS_ERROR(logger, "curl_easy_perform failed. RetCode:" << eRetCode << " ErrMsg:" << curl_easy_strerror(eRetCode));
						return -1;
					}
					
					std::string strReadResult = "";
					strReadResult = ssBody.str();

					//parse http response body
					std::string strSplit = "\r\n\r\n";
					std::size_t ipos = strReadResult.find(strSplit);
					if(ipos == std::string::npos)
					{
						LOG4CPLUS_ERROR(logger, "CurlSSLPost failed, parse https resonse failed, response="+strReadResult;);
						return -1;
					}

					strRspBody = strReadResult.substr(ipos + strSplit.size());
					LOG4CPLUS_DEBUG(logger, "CurlSSLPost, recv succ: " + strRspBody);

					curl_easy_cleanup(pCurl);	
					curl_global_cleanup();

					return 0;			
				}

				int WXAPI::HttpGet(const std::string& strHost, uint16_t wPort, const std::string& strAPI, std::string& strRespBody)
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
				    snprintf(_aczRequest, 2048, "GET /%s HTTP/1.0\r\n"
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


				in_addr_t WXAPI::DNSParse(const std::string& strHost)
				{
					struct hostent stHostent;
					struct hostent* pHostent = NULL;
					int iHErrNO = 0;
					int iRet = ::gethostbyname_r(strHost.c_str(), &stHostent, _aczHostentTempBuffer, 8192, &pHostent, &iHErrNO);
					if (iRet != 0)
					{
						LOG4CPLUS_ERROR(logger, "DNSParse failed, gethostbyname_r return"<<iRet<<", iHErrNO="<<iHErrNO);
				        return 0;
			    	}

			    	if(!pHostent)
			    	{
						LOG4CPLUS_ERROR(logger, "DNSParse failed, gethostbyname_r return 0, but pHostent is NULL, maybe invalid host="<<strHost);
				        return 0;
			    	}

					struct in_addr* pInAddr = (struct in_addr *) pHostent->h_addr;
					if(!pInAddr)
					{
						LOG4CPLUS_ERROR(logger, "DNSParse failed, gethostbyname_r return 0, but pInAddr is NULL, maybe invalid host="<<strHost);
				        return 0;
					}

					in_addr_t stIP = pInAddr->s_addr;

					LOG4CPLUS_DEBUG(logger, "DNSParse succ, strHost="<<strHost<<", ip="<<inet_ntoa(*pInAddr));	
					return stIP;
				}

				size_t WXAPI::write_data(void *ptr, size_t sSize, size_t sNmemb, void *stream)
				{
					std::string strBuf = std::string(static_cast<char *>(ptr), sSize * sNmemb);	
					std::stringstream *ssResponse = static_cast<std::stringstream *>(stream);			
					*ssResponse << strBuf;	
					return sSize * sNmemb;
				}
		}
}




