#ifndef _LCE_CGI_TCP_CONNECTOR_H_
#define _LCE_CGI_TCP_CONNECTOR_H_

#include <string>
#include <stdint.h>
#include <string.h>

namespace lce
{
	namespace cgi
	{
		class CTCPConnector
		{
		public:
			CTCPConnector()
				:m_iFd(-1)
				,m_bConnect(false)
			{}
			~CTCPConnector(){
				Close();
			}

			bool Connect(const std::string& sSvrIp, 
				const uint16_t wSvrPort);
			int32_t Write(const char* pData, const int32_t iSize, const time_t dwTimeout=3000/*∫¡√Î*/);
			int32_t Write(const std::string& sData, const time_t dwTimeout=3000){
					return Write(sData.data(), (int32_t)sData.size(), dwTimeout);
				}
			int32_t Read(char* pBuf, const int32_t iBufSize, const time_t dwTimeout=3000/*∫¡√Î*/);
			int32_t Read(std::string& sBuf, const time_t dwTimeout=3000/*∫¡√Î*/);
			int32_t Read_n(char* pBuf, const int32_t iSize, const time_t dwTimeout=3000/*∫¡√Î*/);
			void Close();
			const char* GetErrMsg() const {	return m_szErrMsg;	}
		private:
			bool _imp_connect();
		private:
			CTCPConnector(const CTCPConnector& rhs);
			CTCPConnector& operator=(const CTCPConnector& rhs);
		private:
			char m_szErrMsg[1024];
			int32_t m_iFd;
			bool m_bConnect;
			std::string m_sSvrIp;
			uint16_t m_wSvrPort;
		};

	}
}




#endif

