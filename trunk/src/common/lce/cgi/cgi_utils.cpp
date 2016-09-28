#include "cgi_utils.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <iconv.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

namespace lce
{
	namespace cgi
	{
		std::string CharToHex(char c)
		{
			std::string sResult;
			char first, second;

			first = (c & 0xF0) / 16;
			first += first > 9 ? 'A' - 10 : '0';
			second = c & 0x0F;
			second += second > 9 ? 'A' - 10 : '0';

			sResult.append(1, first);
			sResult.append(1, second);

			return sResult;
		}

		std::string FormUrlEncode(const std::string &sSrc)
		{
			std::string sResult;
			for(std::string::const_iterator iter = sSrc.begin(); iter != sSrc.end(); ++iter) {
				switch(*iter) {
			case ' ':
				sResult.append(1, '+');
				break;
				// alnum
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
			case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
			case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
			case 'V': case 'W': case 'X': case 'Y': case 'Z':
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
			case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
			case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
			case 'v': case 'w': case 'x': case 'y': case 'z':
			case '0': case '1': case '2': case '3': case '4': case '5': case '6':
			case '7': case '8': case '9':
				// mark
			case '-': case '_': case '.': case '!': case '~': case '*': case '\'': 
			case '(': case ')':
				sResult.append(1, *iter);
				break;
				// escape
			default:
				sResult.append(1, '%');
				sResult.append(CharToHex(*iter));
				break;
				}
			}

			return sResult;
		}

		std::string FormUrlDecode(const std::string& sSrc)
		{
			std::string sResult;
			const char* pszSrc = sSrc.c_str();
			char c;
			size_t iSize = sSrc.size();
			for (size_t i=0;i<iSize; ++i)
			{
				switch(*(pszSrc+i)) {
			case '+':
				sResult.append(1, ' ');
				break;
			case '%':
				// assume well-formed input
				++i;
				if( 'u' != *(pszSrc+i) )
				{
					c = *(pszSrc+i);
					++i;
					sResult.append(1, HexToChar(c, *(pszSrc+i)));
				}
				else
				{
					char u[2];
					u[1] = HexToChar(*(pszSrc+i+1), *(pszSrc+i+2));
					u[0] = HexToChar(*(pszSrc+i+3), *(pszSrc+i+4));
					char szOut[4];
					char* pIn = u;
					char* pOut = szOut;
					size_t in=2,out=4;
					iconv_t fdIconv = iconv_open("GBK","UNICODE"); 
					iconv(fdIconv,&pIn, &in,&pOut,&out);
					iconv_close(fdIconv);
					sResult.append(szOut,2);

					i +=4;
				}

				break;
			default:
				sResult.append(1, *(pszSrc+i));
				break;
				}
			}
			return sResult;
		}

		// locate data between separators, and return it
		std::string extractBetween(const std::string& data, 
									const std::string& separator1, 
									const std::string& separator2)
		{
			std::string result;
			size_t start, limit;

			start = data.find(separator1, 0);
			if(start != std::string::npos) {
				start += separator1.length();
				limit = data.find(separator2, start);
				if(limit != std::string::npos)
					result = data.substr(start, limit - start);
			}

			return result;
		}

		std::string getGMTDate(const time_t& cur)
		{
			static const char *days[] = {"Sun", "Mon", "Tue",  "Wed",  "Thu",  "Fri",  "Sat"};
			static const char *months[] = {"Jan",  "Feb",  "Mar",  "Apr",  "May",  "Jun",  "Jul",  "Aug", "Sep",  "Oct",  "Nov",  "Dec"};
			char buf[256];
			struct tm  *gt = gmtime(&cur);
			snprintf(buf, sizeof(buf),"%s, %02d %s %04d %02d:%02d:%02d GMT",
				days[gt->tm_wday],
				gt->tm_mday,
				months[gt->tm_mon],
				gt->tm_year + 1900,
				gt->tm_hour,
				gt->tm_min,
				gt->tm_sec);
			return buf;
		}


		int32_t socket_setNBlock(const int32_t iSocket)
		{
		#ifdef WIN32
			uint32_t uiTmp = 1;
			if(ioctlsocket(iSocket, FIONBIO, &uiTmp) == SOCKET_ERROR)
				return -1;
			else
				return 0;
		#else
			int32_t opts;
			opts=fcntl(iSocket,F_GETFL);
			if(opts<0)
			{
				printf("[error]**** fcntl(sock,GETFL), error=%s ****\n", strerror(errno));
				return -1;
			}
			opts = opts|O_NONBLOCK;
			if(fcntl(iSocket,F_SETFL,opts)<0)
			{
				printf("[error]**** fcntl(sock,SETFL,opts), error=%s ****\n", strerror(errno));
				return -1;
			}       
		#endif
			return 0;
		}

		int32_t socket_setNCloseWait(const int32_t iSocket)
		{
			//for CLOSE_WAIT
			linger sLinger;
			sLinger.l_onoff = 1;  // (在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
			sLinger.l_linger = 0; // (容许逗留的时间为0秒)
			if(setsockopt(iSocket, SOL_SOCKET,	SO_LINGER,	(char*)&sLinger, sizeof(linger)) != 0)
			{
				//		printf("[error]**** err=%s, %s,%d ****\n", strerror(errno), __FILE__, __LINE__);
				return -1;
			}
			// 如果服务器终止后,服务器可以第二次快速启动而不用等待一段时间 
			int32_t nREUSEADDR = 1;
			if(setsockopt(iSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nREUSEADDR, sizeof(int32_t)) != 0)
			{
				//		printf("[error]**** err=%s, %s,%d ****\n", strerror(errno), __FILE__, __LINE__);
				return -1;
			}
			return 0;
		}

		//去掉字符串2头的空格
		std::string& TrimString(std::string& sSource)
		{
			if (sSource.size()==0)
				return sSource;		

			std::string sDest = "";

			size_t i = 0;
			for(; i < sSource.size() && 
				(sSource[i] == ' '  ||
				sSource[i] == '\r'  || 
				sSource[i] == '\n'  || 
				sSource[i] == '\t'); i++);

			size_t j = sSource.size() - 1;
			if(j > 0)
			{
				for(; j > 0 && 
					(sSource[j] == ' '   ||
					sSource[j] == '\r'  || 
					sSource[j] == '\n'  || 
					sSource[j] == '\t')
					; j--);
			}

			if(j >= i)
			{
				sDest = sSource.substr(i, j-i+1);
				sSource = sDest;
			}
			else
				sSource = "";
			return sSource;
		}

		std::string TextEncodeStr(const std::string& sSrc, const bool bUtf8)
		{
			std::string sDesc;	
			for (std::string::size_type i=0; i<sSrc.size(); i++)
			{
				if ( bUtf8 )
				{
					int32_t iUtf8Size =  TCE_UTF8_LENGTH(sSrc[i]);
					if ( iUtf8Size == 1 )
					{
						switch (sSrc[i])
						{
							case '\"': sDesc += "&#34;"; break;
							case '\'': sDesc += "&#39;"; break;
							case '\n': sDesc += "&#10;"; break;
							case '\\': sDesc += "&#92;"; break;
							default:
							{
								if (sSrc[i] < 0x20 || sSrc[i]=='<' || sSrc[i]=='>')
								{
									char tmp[10];
									snprintf(tmp, 9, "&#%d;", (int32_t)sSrc[i]);
									sDesc += tmp;
								}
								else
								{
									sDesc += sSrc[i];
								}
							}
							break;
						}
					}
					else
					{
						if(iUtf8Size > 1)
						{
							if ( iUtf8Size+i<=sSrc.size() )
							{
								sDesc.append(sSrc.data()+i, iUtf8Size);
								i += iUtf8Size-1;
							}
							else
							{
								//static const unsigned short __wrong_padding_utf16 = 0xff1f;  
								//sDesc.append((char*)&__wrong_padding_utf16, 2);
								//sDesc += tce_single_utf16to8(__wrong_padding_utf16);
								i += iUtf8Size-1;
							}
						}
					}
				}
				else
				{
					if (sSrc[i] == '\"')
					{
						sDesc += "&#34;";
					}
					 else if (sSrc[i] == '\'')
					{
						sDesc += "&#39;";
					}
					else if (sSrc[i] == '\\')
					{
						sDesc += "&#92;";
					}
					else if (sSrc[i] == '\n')
					{
						sDesc += "&#10;";
					}
					else if( (unsigned char)sSrc[i] >= 0x81 && (unsigned char)sSrc[i]<= 0xfe  )
					{
						if ( i+1 != sSrc.size() )
						{
							if ( TCE_IS_GBK_CHAR(sSrc[i], sSrc[i+1]) )
							{
								sDesc += sSrc[i] ;
								sDesc += sSrc[++i] ;
							}
						}
						else
						{
							sDesc += sSrc[i] ;
							sDesc += " ";
						}
					}
					else
					{
						sDesc += sSrc[i] ;
					}
				}
			}
			return sDesc;
		}



		std::string TextEncodeJSON(const std::string& sSrc, const bool bUtf8)
		{
			std::string sDesc;	
			for (std::string::size_type i=0; i<sSrc.size(); i++)
			{
				if ( bUtf8 )
				{
					int32_t iUtf8Size =  TCE_UTF8_LENGTH(sSrc[i]);
					if ( iUtf8Size == 1 )
					{
						switch (sSrc[i])
						{
							case '\"': sDesc += "\\\""; break;
							case '\\': sDesc += "\\\\"; break;
							case '\n': sDesc += "\\n"; break;
							case '\r': sDesc += "\\r"; break;
							case '\b': sDesc += "\\b"; break;
							case '\f': sDesc += "\\f"; break;
							case '\t': sDesc += "\\t"; break;
							case '/': sDesc += "\\/"; break;
							default:
							{
								if (sSrc[i] < 0x20 || sSrc[i]=='<' || sSrc[i]=='>')
								{
									char tmp[10];
									snprintf(tmp, 9, "\\u%04x", (int32_t)sSrc[i]);
									sDesc += tmp;
								}
								else
								{
									sDesc += sSrc[i];
								}
							}
							break;
						}
					}
					else
					{
						if(iUtf8Size > 1)
						{
							if ( iUtf8Size+i<=sSrc.size() )
							{
								sDesc.append(sSrc.data()+i, iUtf8Size);
								i += iUtf8Size-1;
							}
							else
							{
								//static const unsigned short __wrong_padding_utf16 = 0xff1f;  
								//sDesc.append((char*)&__wrong_padding_utf16, 2);
								//sDesc += tce_single_utf16to8(__wrong_padding_utf16);
								i += iUtf8Size-1;
							}
						}
					}
				}
				else
				{
					if (sSrc[i] == '\"')
					{
						sDesc += "\\\"";
					}
					else if (sSrc[i] == '\r')
					{
						sDesc += "\\r";
					}
					else if (sSrc[i] == '\n')
					{
						sDesc += "\\n";
					}
					else if (sSrc[i] == '\\')
					{
						sDesc += "\\\\";
					}
					else if (sSrc[i] == '/')
					{
						sDesc += "\\/";
					}
					else if (sSrc[i] == '\b')
					{
						sDesc += "\\b";
					}
					else if (sSrc[i] == '\f')
					{
						sDesc += "\\f";
					}
					else if (sSrc[i] == '\t')
					{
						sDesc += "\\t";
					}
					else if( (unsigned char)sSrc[i] >= 0x81 && (unsigned char)sSrc[i]<= 0xfe  )
					{
						if ( i+1 != sSrc.size() )
						{
							if ( TCE_IS_GBK_CHAR(sSrc[i], sSrc[i+1]) )
							{
								sDesc += sSrc[i] ;
								sDesc += sSrc[++i] ;
							}
						}
						else
						{
							sDesc += sSrc[i] ;
							sDesc += " ";
						}
					}
					else
					{
						sDesc += sSrc[i] ;
					}
				}
			}
			return sDesc;
		}


		std::string TextEncodeJS(const std::string& sSrc, const bool bUtf8)
		{
			std::string sDesc;	
			for (std::string::size_type i=0; i<sSrc.size(); i++)
			{
				if ( bUtf8 )
				{
					int32_t iUtf8Size =  TCE_UTF8_LENGTH(sSrc[i]);
					if ( iUtf8Size == 1 )
					{
						if (sSrc[i] == '\"')
						{
							sDesc += "&quot;";
						}
						else if (sSrc[i] == '<')
						{
							sDesc += "&lt;";
						}
						else if (sSrc[i] == '>')
						{
							sDesc += "&gt;";
						}
						else if (sSrc[i] == '&')
						{
							sDesc += "&amp;";
						}
						else if (sSrc[i] == '\'')
						{
							sDesc += "&#39;";
						}
						else if (sSrc[i] == '\n')
						{
							sDesc += "<br>";
						}
						else if (sSrc[i] == '\r')
						{
							if (i+1>=sSrc.size() || sSrc[i+1] != '\n')
							{
								sDesc += "<br>";
							}
						}
						else if (sSrc[i] == '\\')
						{
							sDesc += "&#92;";
						}
						else if (sSrc[i] == ' ')
						{
							sDesc += "&nbsp;";
							//sDesc += "&#160;";
						}
						else
						{
							sDesc += sSrc[i];
						}
					}
					else
					{
						if(iUtf8Size > 1)
						{
							if ( iUtf8Size+i<=sSrc.size() )
							{
								if(3 == iUtf8Size && 0xe2 == (uint8_t)sSrc[i] && 0x80 == (uint8_t)sSrc[i+1] && (0xa8 == (uint8_t)sSrc[i+2] || 0xa9 == (uint8_t)sSrc[i+2]))
								{
									// 0xe280a8  Line separator
									// 0xe280a9  Paragraph separator
									sDesc += "<br>";
								}
								else if(2 == iUtf8Size && 0xc2 == (uint8_t)sSrc[i] && 0xa0 == (uint8_t)sSrc[i+1])
								{ // 0xc2a0    Non-breaking space
									sDesc += "&nbsp;";
								}
								else
								{
									sDesc.append(sSrc.data()+i, iUtf8Size);
								}
								i += iUtf8Size-1;
							}
							else
							{
								//static const unsigned short __wrong_padding_utf16 = 0xff1f;  
								//						sDesc.append((char*)&__wrong_padding_utf16, 2);
								//sDesc += tcgi_single_utf16to8(__wrong_padding_utf16);
								i += iUtf8Size-1;
							}
						}
					}
				}
				else
				{
					if (sSrc[i] == '\"')
					{
						sDesc += "&quot;";
					}
					else if (sSrc[i] == '<')
					{
						sDesc += "&lt;";
					}
					else if (sSrc[i] == '>')
					{
						sDesc += "&gt;";
					}
					else if (sSrc[i] == '&')
					{
						sDesc += "&amp;";
					}
					else if (sSrc[i] == '\'')
					{
						sDesc += "&#39;";
					}
					else if (sSrc[i] == '\n')
					{
						sDesc += "<br>";
					}
					else if (sSrc[i] == '\r')
					{
						if (i+1>=sSrc.size() || sSrc[i+1] != '\n')
						{
							sDesc += "<br>";
						}
					}
					else if (sSrc[i] == '\\')
					{
						sDesc += "&#92;";
					}
					else if (sSrc[i] == ' ')
					{
						sDesc += "&nbsp;";
					}
					else if( (unsigned char)sSrc[i] >= 0x81 && (unsigned char)sSrc[i]<= 0xfe  )
					{
						if ( i+1 != sSrc.size() )
						{
							if ( TCE_IS_GBK_CHAR(sSrc[i], sSrc[i+1]) )
							{
								sDesc += sSrc[i] ;
								sDesc += sSrc[++i] ;
							}
						}
						else
						{
							sDesc += sSrc[i] ;
							sDesc += " ";
						}
					}
					else
					{
						sDesc += sSrc[i] ;
					}
				}
			}
			return sDesc;
		}


		std::string TextEncodeXml( const std::string& sText, const bool bUtf8, bool bAttrib)
		{
			// Convert text as seen outside XML document to XML friendly
			// replacing special characters with ampersand escape codes
			// E.g. convert "6>7" to "6&gt;7"
			//
			// &lt;   less than
			// &amp;  ampersand
			// &gt;   greater than
			//
			// and for attributes:
			//
			// &apos; apostrophe or single quote
			// &quot; double quote
			//
			static const char* szaReplace[] = { "&lt;","&amp;","&gt;","&apos;","&quot;" };
			const char* pFind = bAttrib?"<&>\'\"":"<&>";
			std::string strText;
			const char* pSource = sText.c_str();
			int32_t nDestSize = (int32_t)strlen(pSource);
			nDestSize += nDestSize / 10 + 7;
			strText.reserve( nDestSize );
			char cSource = *pSource;
			const char* pFound;
			while ( cSource )
			{
				if ( (pFound=strchr(pFind,cSource)) != NULL )
				{
					pFound = szaReplace[pFound-pFind];
					strText.append( pFound );
				}
				else
				{
					strText += cSource;
				}
				++pSource;
				cSource = *pSource;
			}
			return strText;
		}

		std::string HexShow(const unsigned char* sStr, const int32_t iLen, const int32_t iLineCount)
		{
			register int32_t iCount;
			std::string sResult;
			char szTmp[10];
			for (iCount = 0; iCount < iLen; iCount++) {
				//if (iFlag && sStr[iCount] > 0x1f) 
				//{
				//	snprintf(szTmp, sizeof(szTmp), "%2c ", sStr[iCount]);
				//	sResult += szTmp;
				//}
				//else 
				{
					xsnprintf(szTmp, sizeof(szTmp), "%.2x", (unsigned char)sStr[iCount]);
					sResult += szTmp;
				}
				if (iLineCount > 0 && (iCount+1) % iLineCount == 0 )
					sResult += "\n";
			}

			return sResult;
		}

		bool Base16Decode(const char* hex, const size_t hexlen, char* bin, size_t& binlen)
		{
			char c, s;
			size_t i = 0;

			while(i < binlen && *hex) {
				s = 0x20 | (*hex++);
				if(s >= '0' && s <= '9')
					c = s - '0';
				else if(s >= 'a' && s <= 'f')
					c = s - 'a' + 10;
				else
					break;

				c <<= 4;
				s = 0x20 | (*hex++);
				if(s >= '0' && s <= '9')
					c += s - '0';
				else if(s >= 'a' && s <= 'f')
					c += s - 'a' + 10;
				else
					break;
				bin[i++] = c;
			}
			if(i<binlen) bin[i] = '\0';
			binlen = i;
			return true;
		}

		bool Base16Encode(const char* bin, const size_t binlen, char* hex, size_t& hexlen)
		{
			if ( hexlen < binlen*2 )	return false;
			
			for(size_t i=0; i<binlen; i++) {
				*hex++ = "0123456789abcdef"[((unsigned char *)bin)[i] >> 4];
				*hex++ = "0123456789abcdef"[((unsigned char *)bin)[i] & 15];
			}
			if ( hexlen > binlen*2 ) *hex = '\0';
			hexlen = binlen*2;
			return true;
		}

		static const char base64_table1[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
		    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '-', '\0'
		};

		static const char base64_table2[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
		    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
		    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
		};

		static const char base64_pad1 = '@';
		static const char base64_pad2 = '=';

		static const short base64_reverse_table1[256] = {
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 63, -1, -1,
		    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
		    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 62,
		    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
		};
		static const short base64_reverse_table2[256] = {
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
		    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
		    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
		    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
		};

		int base64Encode(const unsigned char *in_str, int length, char *out_str, int *ret_length, char ver)
		{
			char *base64_table, base64_pad;
			if (ver == 0)
			{
				base64_pad = base64_pad1;
				base64_table = (char *)base64_table1;
			}
			else
			{
				base64_pad = base64_pad2;
				base64_table = (char *)base64_table2;
			}
		    const unsigned char *current = in_str;
		    unsigned char *p;

		    if((length + 2) < 0 || ((length + 2) / 3) >= (1 << (sizeof(int) * 8 - 2)))
		        return -1;
		    if(*ret_length < (((length + 2) / 3) * 4))
		        return -1;

		    p = (unsigned char *) out_str;

		    while(length > 2)
		    {   /* keep going until we have less than 24 bits */
		        *p++ = base64_table[current[0] >> 2];
		        *p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
		        *p++ = base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
		        *p++ = base64_table[current[2] & 0x3f];

		        current += 3;
		        length -= 3;    /* we just handle 3 octets of data */
		    }

		    /* now deal with the tail end of things */
		    if(length != 0)
		    {
		        *p++ = base64_table[current[0] >> 2];
		        if(length > 1)
		        {
		            *p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
		            *p++ = base64_table[(current[1] & 0x0f) << 2];
		            *p++ = base64_pad;
		        }
		        else
		        {
		            *p++ = base64_table[(current[0] & 0x03) << 4];
		            *p++ = base64_pad;
		            *p++ = base64_pad;
		        }
		    }
		    if(ret_length != NULL)
		    {
		        *ret_length = (int) (p - (unsigned char *) out_str);
		    }
		    *p = '\0';
		    return 0;
		}

		int base64Decode(const unsigned char *in_str, int length, char *out_str, int *ret_length, char ver)
		{
			short *base64_reverse_table;
			char base64_pad;
			if (ver == 0)
			{
				base64_pad = base64_pad1;
				base64_reverse_table = (short*)base64_reverse_table1;
			}
			else
			{
				base64_pad = base64_pad2;
				base64_reverse_table = (short*)base64_reverse_table2;
			}
		    const unsigned char *current = in_str;
		    int ch, i = 0, j = 0, k;

		    /* this sucks for threaded environments */
		    unsigned char *result = (unsigned char *) out_str;

		    if(*ret_length < length + 1)
		        return -1;

		    /* run through the whole string, converting as we go */
		    while((ch = *current++) != '\0' && length-- > 0)
		    {
		        if(ch == base64_pad)
		            break;

		        /* When Base64 gets POSTed, all pluses are interpreted as spaces.
		           This line changes them back.  It's not exactly the Base64 spec,
		           but it is completely compatible with it (the spec says that
		           spaces are invalid).  This will also save many people considerable
		           headache.  - Turadg Aleahmad <turadg@wise.berkeley.edu>
		         */

		        if(ch == ' ')
		            ch = '+';

		        ch = base64_reverse_table[ch];
		        if(ch < 0)
		            continue;

		        switch (i % 4)
		        {
		            case 0:
		                result[j] = ch << 2;
		                break;
		            case 1:
		                result[j++] |= ch >> 4;
		                result[j] = (ch & 0x0f) << 4;
		                break;
		            case 2:
		                result[j++] |= ch >> 2;
		                result[j] = (ch & 0x03) << 6;
		                break;
		            case 3:
		                result[j++] |= ch;
		                break;
		        }
		        i++;
		    }

		    k = j;
		    /* mop things up if we ended on a boundary */
		    if(ch == base64_pad)
		    {
		        switch (i % 4)
		        {
		            case 0:
		            case 1:
		                return -1;
		            case 2:
		                k++;
		            case 3:
		                result[k++] = 0;
		        }
		    }
		    if(ret_length)
		    {
		        *ret_length = j;
		    }
		    result[j] = '\0';
		    return 0;
		}
	}
}



