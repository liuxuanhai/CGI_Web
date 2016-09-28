#include "cgi_tcp_connector.h"
#include "cgi_utils.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace lce
{
	namespace cgi
	{
		bool CTCPConnector::Connect(const std::string& sSvrIp, 
									const uint16_t wSvrPort)
		{
			m_sSvrIp = sSvrIp;
			m_wSvrPort = wSvrPort;
			
			return _imp_connect();
		}

		bool CTCPConnector::_imp_connect()
		{
			if ( m_iFd != -1 )
			{
				Close();
			}

			m_iFd = socket(PF_INET, SOCK_STREAM, 0);
			if ( -1 == m_iFd ) {
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect: socket error(%d):%s.",errno, strerror(errno));
				return -1;
			}

			struct sockaddr_in sockSvrAddr;
			memset(&sockSvrAddr, 0, sizeof(sockaddr_in));
			sockSvrAddr.sin_family = AF_INET;
			sockSvrAddr.sin_addr.s_addr = inet_addr(m_sSvrIp.c_str());
			sockSvrAddr.sin_port = htons(m_wSvrPort);

			if(::connect(m_iFd, (struct sockaddr *)&sockSvrAddr, sizeof(struct sockaddr)) == -1)
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect:connect error(%d):%s.",errno, strerror(errno));
				Close();
				return false;
			}

			m_bConnect = true;

			return true;
		}

		int32_t CTCPConnector::Write(const char* pData, const int32_t iSize, const time_t dwTimeout)
		{
			if (!m_bConnect)
			{
				_imp_connect();
			}

			struct timeval tv;
			tv.tv_sec = dwTimeout/1000;
			tv.tv_usec = (dwTimeout%1000)*1000;
			setsockopt(m_iFd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

			int32_t iSendLen = ::send(m_iFd, pData, iSize, 0);
			if ( iSendLen<0)
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Write: send error:%s.",strerror(errno));
				Close();
				return -1;
			}

			return iSendLen;	
		}

		int32_t CTCPConnector::Read_n(char* pBuf, const int32_t iSize, const time_t dwTimeout/*∫¡√Î*/)
		{
			if (!m_bConnect)
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"no create Connect.");
				return -1;
			}

			struct timeval tv;
			tv.tv_sec = dwTimeout/1000;
			tv.tv_usec = (dwTimeout%1000)*1000;
			setsockopt(m_iFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

			time_t dwStartTime = cgi::GetTickCount();
			int32_t iReadSize = 0;
			while (iReadSize < iSize )
			{
				int32_t n = ::recv(m_iFd, pBuf+iReadSize, iSize-iReadSize, 0);
				if (n < 0)
				{
					if ( errno != EINTR && errno != EAGAIN )
					{
						snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read: recv error(%d):%s",errno, strerror(errno));
						return -1;
					}
				}
				else
				{
					if (0 == n)
					{
						this->Close();
						return iReadSize;
					}
					iReadSize += n;
				}

				if (cgi::GetTickCount() > dwStartTime + dwTimeout )
				{
					break;
				}
			}

			return iReadSize;
		}


		int32_t CTCPConnector::Read(char* pBuf, const int32_t iBufSize, const time_t dwTimeout/*∫¡√Î*/)
		{
			if (!m_bConnect)
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect fail or no create connect.");
				return -1;
			}

			int32_t iRe = -1;
			struct timeval tv;
			tv.tv_sec = dwTimeout/1000;
			tv.tv_usec = (dwTimeout%1000)*1000;
			setsockopt(m_iFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

			int32_t n = ::recv(m_iFd, pBuf, iBufSize, 0);
			if (n < 0)
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read: recv error(%d):%s",errno, strerror(errno));
				return -1;
			}
			else
			{
				if (0 == n)
				{
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read: recv error(%d):%s",errno, strerror(errno));
					this->Close();
				}
				iRe = n;
			}

			return iRe;
		}

		int32_t CTCPConnector::Read(std::string& sBuf, const time_t dwTimeout/*∫¡√Î*/)
		{
			if (!m_bConnect)
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Connect fail or no create connect.");
				return -1;
			}

			int32_t iBufSize = 65535;
			sBuf.resize(iBufSize);

			int32_t iRe = 0;
			struct timeval tv;
			tv.tv_sec = dwTimeout/1000;
			tv.tv_usec = (dwTimeout%1000)*1000;
			setsockopt(m_iFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

			int32_t n = ::recv(m_iFd, (char*)sBuf.data(), iBufSize, 0);
			if (n < 0)
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"Read: recv error(%d):%s",errno, strerror(errno));
				return -1;
			}
			else
			{
				if (0 == n)
				{
					this->Close();
					sBuf.clear();
				}
				iRe = n;
				sBuf.resize(iRe);
			}

			return iRe;
		}

		void CTCPConnector::Close()
		{
			::close(m_iFd);
			m_iFd = -1;
			m_bConnect = false;
		}

	}
}

