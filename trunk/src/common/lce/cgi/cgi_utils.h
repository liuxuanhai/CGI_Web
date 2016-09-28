#ifndef _LCE_CGI_UTILS_H_
#define _LCE_CGI_UTILS_H_

#include <string>
#include <vector>
#include <sys/time.h>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>

namespace lce
{
	namespace cgi
	{
		#if __BYTE_ORDER == __BIG_ENDIAN
		#define cgi_ntohll(x)       (x)
		#define cgi_htonll(x)       (x)
		#else 
		#if __BYTE_ORDER == __LITTLE_ENDIAN
		#define cgi_ntohll(x)     __bswap_64 (x)
		#define cgi_htonll(x)     __bswap_64 (x)
		#endif 
		#endif

		#ifndef htonll
		#define htonll(x) cgi_htonll((x))
		#endif
		#ifndef ntohll
		#define ntohll(x) cgi_ntohll((x))
		#endif

		#define TCE_IS_GBK_CHAR(c1, c2) \
			((unsigned char)c1 >= 0x81 \
			&& (unsigned char)c1 <= 0xfe \
			&& (unsigned char)c2 >= 0x40 \
			&& (unsigned char)c2 <= 0xfe \
			&& (unsigned char)c2 != 0x7f) 

		#define TCE_UTF8_LENGTH(c) \
			((unsigned char)c <= 0x7f ? 1 : \
			((unsigned char)c & 0xe0) == 0xc0 ? 2: \
			((unsigned char)c & 0xf0) == 0xe0 ? 3: \
			((unsigned char)c & 0xf8) == 0xf0 ? 4: 0)


		std::string FormUrlEncode(const std::string &sSrc);
		std::string FormUrlDecode(const std::string& sSrc);
		std::string TextEncodeStr(const std::string& sSrc, const bool bUtf8=true);
		std::string TextEncodeJSON(const std::string& sSrc, const bool bUtf8=true);
		std::string TextEncodeJS(const std::string& sSrc, const bool bUtf8=true);
		std::string TextEncodeXml( const std::string& sText, const bool bUtf8=true, bool bAttrib=false);
		inline char HexToChar(char first, char second)
		{
			int32_t digit;
			digit = (first >= 'A' ? ((first & 0xDF) - 'A') + 10 : (first - '0'));
			digit *= 16;
			digit += (second >= 'A' ? ((second & 0xDF) - 'A') + 10 : (second - '0'));
			return static_cast<char>(digit);
		}

		inline time_t GetTickCount()
		{
			timeval tv;
			gettimeofday(&tv, 0);
			return tv.tv_sec * 1000 + tv.tv_usec/1000; 
		}

		std::string extractBetween(const std::string& data, const std::string& separator1, const std::string& separator2);

		std::string getGMTDate(const time_t& cur);
		std::string& TrimString(std::string& sSource);

		template <class T> 
		std::string ToStr(const T &t)
		{
			std::stringstream stream;
			stream<<t;
			return stream.str();
		}

		inline std::string InetNtoA(const uint32_t dwIp)
		{
			struct in_addr in;
			in.s_addr = htonl(dwIp);
			return std::string(inet_ntoa(in));
		}

		int32_t socket_setNBlock(const int32_t iSocket);
		int32_t socket_setNCloseWait(const int32_t iSocket);

		inline void xsnprintf(char* szBuffer,const size_t size,const char *sFormat, ...)
		{
			va_list ap;
			va_start(ap, sFormat);
			vsnprintf(szBuffer,size,sFormat, ap);
			va_end(ap);
		}

		std::string HexShow(const unsigned char* pszData, const int32_t iLen, const int32_t iLineCount=0);
		inline std::string HexShow(const char* pszData, const int32_t iLen, const int32_t iLineCount=0)
		{
			return HexShow(reinterpret_cast<const unsigned char*>(pszData), iLen, iLineCount);
		}

		inline std::string HexShow(const std::string& sData, const int32_t iLineCount=0)
		{
			return HexShow(reinterpret_cast<const unsigned char*>(sData.data()), (int32_t)sData.size(), iLineCount);
		}


		bool Base16Decode(const char* hex, const size_t hexlen, char* bin, size_t& binlen);
		bool Base16Encode(const char* bin, const size_t binlen, char* hex, size_t& hexlen);
		inline std::string Base16Decode(const char* hex, const size_t hexlen)
		{
			std::string sResult;
			sResult.resize(hexlen/2);
			size_t nSize = hexlen/2;
			Base16Decode(hex, hexlen, (char*)sResult.data(), nSize);
			return sResult;
		}

		inline std::string Base16Encode(const char* bin, const size_t binlen)
		{
			std::string sResult;
			sResult.resize(binlen*2);
			size_t nSize = binlen*2;
			Base16Encode(bin, binlen, (char*)sResult.data(), nSize);
			return sResult;
		}

		//ver:
		//0:imweb
		//1:standard
		int base64Encode(const unsigned char *in_str, int length, char *out_str, int *ret_length, char ver=0);

		//ver:
		//0:imweb
		//1:standard
		int base64Decode(const unsigned char *in_str, int length, char *out_str, int *ret_length, char ver=0);

		inline std::string GetTimeStr(const time_t dwTime)
		{
				struct tm curr;
				curr = *localtime(&dwTime);
				char szDate[50];
				lce::cgi::xsnprintf(szDate, sizeof(szDate)-1, "%04d-%02d-%02d %02d:%02d:%02d", curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday, curr.tm_hour, curr.tm_min, curr.tm_sec);
				return std::string(szDate);
		}
	}
}

#endif

