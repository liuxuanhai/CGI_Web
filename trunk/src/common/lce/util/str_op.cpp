#include "util/str_op.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <iconv.h>
 
namespace lce
{
	namespace util
	{
		#ifndef IS_GBK_CHAR
		#define IS_GBK_CHAR(c1, c2) \
			((unsigned char)c1 >= 0x81 \
			&& (unsigned char)c1 <= 0xfe \
			&& (unsigned char)c2 >= 0x40 \
			&& (unsigned char)c2 <= 0xfe \
			&& (unsigned char)c2 != 0x7f)
		#endif

		#ifndef IS_UTF8_CHAR
		#define IS_UTF8_CHAR(c) \
			((unsigned char)c <= 0x7f ? 1 : \
			((unsigned char)c & 0xe0) == 0xc0 ? 2: \
			((unsigned char)c & 0xf0) == 0xe0 ? 3: \
			((unsigned char)c & 0xf8) == 0xf0 ? 4: 0)
		#endif

		void StringOP::ToUpper(std::string &str)
		{
			for (size_t i = 0; i < str.size(); ++i)
			{
				if( (unsigned char)str[i] >= 0x81 && (unsigned char)str[i]<= 0xfe  )
				{
					if ( i+1 != str.size() )
					{
						if ( IS_GBK_CHAR(str[i], str[i+1]) )
						{
							i = i+1;
							continue;
						}
					}
				}

				str[i] = toupper(str[i]);
			}
		}
		void StringOP::ToLower(std::string &str)
		{
			for (size_t i = 0; i < str.size(); ++i)
			{
				if( (unsigned char)str[i] >= 0x81 && (unsigned char)str[i]<= 0xfe  )
				{
					if ( i+1 != str.size() )
					{
						if ( IS_GBK_CHAR(str[i], str[i+1]) )
						{
							i = i+1;
							continue;
						}
					}
				}
				str[i] = tolower(str[i]);
			}
		}

		std::string StringOP::ToUpperCopy(const std::string &str)
		{
			std::string sTmp(str);

			for (size_t i = 0; i < sTmp.size(); ++i)
			{
				if( (unsigned char)str[i] >= 0x81 && (unsigned char)str[i]<= 0xfe  )
				{
					if ( i+1 != str.size() )
					{
						if ( IS_GBK_CHAR(str[i], str[i+1]) )
						{
							i = i+1;
							continue;
						}
					}
				}
				sTmp[i] = toupper(sTmp[i]);
			}

			return sTmp;
		}
		std::string StringOP::ToLowerCopy(const std::string &str)
		{
			std::string sTmp(str);

			for (size_t i = 0; i < sTmp.size(); ++i)
			{
				if( (unsigned char)str[i] >= 0x81 && (unsigned char)str[i]<= 0xfe  )
				{
					if ( i+1 != str.size() )
					{
						if ( IS_GBK_CHAR(str[i], str[i+1]) )
						{
							i = i+1;
							continue;
						}
					}
				}
				sTmp[i] = tolower(sTmp[i]);
			}

			return sTmp;
		}

		std::string StringOP::Trim(const std::string &str)
		{
			if (str.size()==0)
			{
				return str;
			}
				

			std::string sDest = "";

			size_t i = 0;
			for(; i < str.size() && 
				(str[i] == ' '  ||
				str[i] == '\r'  || 
				str[i] == '\n'  || 
				str[i] == '\t'); i++);

			size_t j = str.size() - 1;
			if(j > 0)
			{
				for(; j > 0 && 
					(str[j] == ' '   ||
					str[j] == '\r'  || 
					str[j] == '\n'  || 
					str[j] == '\t')
					; j--);
			}

			if(j >= i)
			{
				sDest = str.substr(i, j-i+1);
			}
			
			return sDest;
		}

		void StringOP::Split(const std::string& original,const std::string& splitor, std::vector<std::string>& result)
		{
			if(original.empty() || splitor.empty())
			{   
				return;
			}   

			result.clear();
			std::string::size_type start = 0;
			while(start < original.size())    
			{
				std::string::size_type pos = 0;

				pos = original.find(splitor, start);
				if(pos == 0)
				{
					start = pos + splitor.size(); 
					continue;
				}

				if(pos == std::string::npos)
				{   
					result.push_back(original.substr(start));
					break;
				}

				if(!original.substr(start, pos - start).empty())
				{
					result.push_back(original.substr(start, pos - start));
				}
				start = pos + splitor.size(); 
			}

		}
		
		void StringOP::SplitWithEmpty(const std::string& original,const std::string& splitor, std::vector<std::string>& result)
		{
			if(original.empty() || splitor.empty())
			{   
				return;
			}   

			result.clear();
			std::string::size_type start = 0;
			while(start < original.size())    
			{
				std::string::size_type pos = 0;

				pos = original.find(splitor, start);
				if(pos == 0)
				{
					start = pos + splitor.size(); 
					continue;
				}

				if(pos == std::string::npos)
				{   
					result.push_back(original.substr(start));
					break;
				}

				result.push_back(original.substr(start, pos - start));
				start = pos + splitor.size(); 
			}

		}
		

		std::string StringOP::GetTime(time_t seconds)
		{
			time_t tt = seconds;
			if(tt == 0)
			{
				tt = time(NULL);
			}

			struct tm *ptm = localtime(&tt);
			char sTime[256];
			strftime(sTime,256,"%T",ptm);

			return std::string(sTime);

		}

		std::string StringOP::GetDate(time_t seconds)
		{
			time_t tt = seconds;
			if(tt == 0)
			{
				tt = time(NULL);
			}

			struct tm *ptm = localtime(&tt);
			char sTime[256];
			strftime(sTime,256,"%F",ptm);

			return std::string(sTime);
		}
		
		std::string StringOP::GetDateTime(time_t seconds)
		{
			time_t tt = seconds;
			if(tt == 0)
			{
				tt = time(NULL);
			}

			struct tm *ptm = localtime(&tt);
			char sTime[256];
			strftime(sTime,256,"%F %T",ptm);

			return std::string(sTime);
		}

		int StringOP::GetSeconds(const std::string &strDateTime,time_t &t)
		{
			std::vector<std::string> vStr;
			std::vector<int> vDate;
			std::vector<int> vTime;
		    Split(strDateTime," ",vStr);

		    if(vStr.size() !=2 )
		    {
		    	return -1;
		    }
		    else
		    {
		    	SplitInt<int>(vStr[0],"-",vDate);
		    	SplitInt<int>(vStr[1],":",vTime);
		    }

		    if (vDate.size() != 3 || vTime.size() != 3)
		    {
		    	return -2;
		    }
		    else
		    {
		    	struct tm oTime;

		    	oTime.tm_year = vDate[0] - 1900;
		    	oTime.tm_mon = vDate[1] - 1;
		    	oTime.tm_mday = vDate[2];

		    	oTime.tm_hour = vTime[0];
		    	oTime.tm_min = vTime[1];
		    	oTime.tm_sec = vTime[2];
		    	t = mktime(&oTime);
		    }

		    return 0;

		}

		std::string StringOP::Base16Encode(const std::string strBin)
		{
			const char sHex[] = "0123456789abcdef";
			std::string strHex;
			strHex.reserve(strBin.size()*2);

			for (size_t i = 0; i < strBin.size(); ++i)
			{
				strHex.push_back(sHex[uint8_t(strBin[i]) >> 4]);
				strHex.push_back(sHex[uint8_t(strBin[i]) & 15]);
			}

			return strHex;
		}

		std::string StringOP::Base16Decode(const std::string strHex)
		{
			unsigned char c, s;
			std::string strResult;
			for (size_t i = 0; i < strHex.size(); i=i+2)
			{
				s = 0x20 | (strHex[i]);
				if( s >= '0' && s <='9')
				{
					c = s - '0';
				}
				else if(s >= 'a' && s <= 'f')
				{
					c = s - 'a' + 10;
				}
				else
				{
					break;
				}

				c <<=4;
				s = 0x20 | (strHex[i+1]);
				if ( s >= '0' && s <= '9')
				{
					c += s - '0';
				}
				else if ( s >= 'a' && s <= 'f')
				{
					c += s - 'a' + 10;
				}
				else
				{
					break;
				}

				strResult.push_back(c);
			}
			
			return strResult;
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

		std::string StringOP::Base64Encode(const std::string str, char ver)
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

			std::string strResult;
			size_t i = 0;
			for ( i = 0; i+3 <= str.size(); i=i+3 )
			{
				strResult.push_back(base64_table[uint8_t(str[i+0]) >> 2]);
				strResult.push_back(base64_table[((uint8_t(str[i+0]) & 0x03) << 4) + (uint8_t(str[i+1]) >> 4)]);
				strResult.push_back(base64_table[((uint8_t(str[i+1]) & 0x0f) << 2) + (uint8_t(str[i+2]) >> 6)]);
				strResult.push_back(base64_table[uint8_t(str[i+2]) & 0x3f]);
			}

			if (i < str.size())
			{
				strResult.push_back(base64_table[uint8_t(str[i+0]) >> 2]);
				if(str.size() - i > 1)
				{
					strResult.push_back(base64_table[((uint8_t(str[i+0]) & 0x03) << 4) + (uint8_t(str[i+1]) >> 4)]);
					strResult.push_back(base64_table[(uint8_t(str[i+1]) & 0x0f) << 2]);
					strResult.push_back(base64_pad);
				}
				else
				{
					strResult.push_back(base64_table[(uint8_t(str[i+0]) & 0x03) << 4]);
					strResult.push_back(base64_pad);
					strResult.push_back(base64_pad);
				}
			}

		    return strResult;
		}
		std::string StringOP::Base64Decode(const std::string str, char ver)
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

			unsigned char sBuf[str.size()];
			memset(sBuf,0,sizeof(sBuf));

			size_t i = 0,j = 0;
			int ch = 0;
			for ( size_t n = 0; n < str.size(); ++n)
			{
				ch = str[n];
				if (ch == base64_pad )
				{
					break;
				}
				/* When Base64 gets POSTed, all pluses are interpreted as spaces.
		           This line changes them back.  It's not exactly the Base64 spec,
		           but it is completely compatible with it (the spec says that
		           spaces are invalid).  This will also save many people considerable
		           headache.  - Turadg Aleahmad <turadg@wise.berkeley.edu>
		         */
				if (ch == ' ')
				{
					ch = '+';
				}

				ch = base64_reverse_table[ch];
		        if(ch < 0)
		        {
		        	continue;
		        }

		        switch (i % 4)
		        {
		            case 0:
		                sBuf[j] = ch << 2;
		                break;
		            case 1:
		                sBuf[j++] |= ch >> 4;
		                sBuf[j] = (ch & 0x0f) << 4;
		                break;
		            case 2:
		                sBuf[j++] |= ch >> 2;
		                sBuf[j] = (ch & 0x03) << 6;
		                break;
		            case 3:
		                sBuf[j++] |= ch;
		                break;
		        }
		        i++;
			}

			size_t k = j;
		    /* mop things up if we ended on a boundary */
		    if(ch == base64_pad)
		    {
		        switch (i % 4)
		        {
		            case 0:
		            case 1:
		                break;
		            case 2:
		                k++;
		            case 3:
		                sBuf[k++] = 0;
		        }
		    }

		    return std::string((char*)sBuf, j);
		}


		std::string StringOP::EncodeXml(const std::string &sText, bool bAttrib)
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

		std::string StringOP::EncodeJSON(const std::string &sSrc, const bool bUtf8 )
		{
			std::string sDesc;	
			for (std::string::size_type i=0; i<sSrc.size(); i++)
			{
				if ( bUtf8 )
				{
					int32_t iUtf8Size =  IS_UTF8_CHAR(sSrc[i]);
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
							if ( IS_GBK_CHAR(sSrc[i], sSrc[i+1]) )
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

		std::string StringOP::EncodeJS(const std::string& sSrc, const bool bUtf8)
		{
			std::string sDesc;	
			for (std::string::size_type i=0; i<sSrc.size(); i++)
			{
				if ( bUtf8 )
				{
					int32_t iUtf8Size =  IS_UTF8_CHAR(sSrc[i]);
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
							//sDesc += "&nbsp;";
							sDesc += "&#160;";
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
								sDesc.append(sSrc.data()+i, iUtf8Size);
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
							if ( IS_GBK_CHAR(sSrc[i], sSrc[i+1]) )
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

		std::string StringOP::UrlEncode(const std::string &sSrc)
		{
			std::string sResult;
			for(std::string::const_iterator iter = sSrc.begin(); iter != sSrc.end(); ++iter) 
			{
				switch(*iter) 
				{
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
						sResult.append(Char2Hex(*iter));
						break;
				}
			}

			return sResult;
		}

		std::string StringOP::UrlDecode(const std::string& sSrc)
		{
			std::string sResult;
			const char* pszSrc = sSrc.c_str();
			char c;
			size_t iSize = sSrc.size();
			for (size_t i=0;i<iSize; ++i)
			{
				switch(*(pszSrc+i)) 
				{
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
							sResult.append(1, Hex2Char(c, *(pszSrc+i)));
						}
						else
						{
							char u[2];
							u[1] = Hex2Char(*(pszSrc+i+1), *(pszSrc+i+2));
							u[0] = Hex2Char(*(pszSrc+i+3), *(pszSrc+i+4));

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

		std::string StringOP::Char2Hex(const char c)
		{
			static const char sHex[] = "0123456789ABCDEF";
			std::string strHex;

			strHex.push_back(sHex[uint8_t(c) >> 4]);
			strHex.push_back(sHex[uint8_t(c) & 15]);

			return strHex;
		}

		char StringOP::Hex2Char(char first, char second)
		{
			int32_t digit;
			digit = (first >= 'A' ? ((first & 0xDF) - 'A') + 10 : (first - '0'));
			digit *= 16;
			digit += (second >= 'A' ? ((second & 0xDF) - 'A') + 10 : (second - '0'));
			return static_cast<char>(digit);
		}

		

		std::string StringOP::ShowHex(const char *sStr,const int iLen,const int iLineCount)
		{
			std::string sResult;
			char szTmp[10];
			for (int i = 0; i < iLen; i++) 
			{
				snprintf(szTmp, sizeof(szTmp), "%.2x", (unsigned char)sStr[i]);
				sResult += szTmp;

				if ((i%2) == 1)
				{
					sResult +=" ";
				}
				if (iLineCount > 0 && (i+1) % iLineCount == 0 )
				{
					sResult += "\n";
				}
			}

			return sResult;
		}
	}
}