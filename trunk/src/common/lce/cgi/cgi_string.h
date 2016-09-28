#ifndef _LCE_CGI_STRING_H_
#define _LCE_CGI_STRING_H_

#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <cctype>
#include <ctime>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iconv.h>

namespace lce
{
	namespace cgi 
	{
		template <typename T>
		std::string IntToStr(const T &n)
		{
			std::stringstream stream;
			stream << n;
			return stream.str();
		}

		void ToUpper(std::string &str);
		void ToLower(std::string &str);
		std::string ToUpperCopy(const std::string &str);
		std::string ToLowerCopy(const std::string &str);

		std::string GetTime(time_t seconds=0);   //返回格式为 HH:MM:SS的时间值,默认取当前时间
		std::string GetDate(time_t seconds=0);   //返回格式为 YYYY-MM-DD的日期值 
		std::string GetDateTime(time_t seconds=0); //返回格式为 YYYY-MM-DD HH:MM:SS 的日期时间值
		int GetSeconds(const std::string &strDateTime,time_t &t); //输入格式为 YYYY-MM-DD H:M:S的日期值，返回其对应的秒数

		std::string Trim(const std::string &str);
		void ReplaceStr(std::string &strInput,const std::string &strSearch,const std::string &strNew);

		template <typename T>
		void SplitInt(const std::string& original,const std::string& splitor, std::vector<T>& result)
		{
		    if(original.empty() || splitor.empty())
		    {   
				return;
		    }   

		    result.clear();
		    std::string::size_type start = 0;
		    while(start < original.size())    
		    {
				std::string::size_type pos = original.find(splitor, start);
				if(pos == 0)
				{
					start = pos + splitor.size(); 
					continue;
				}

				if(pos == std::string::npos)
				{   
					T tmp = (T)atoll(original.substr(start).c_str());
					result.push_back(tmp);
					break;
				}

				if(!original.substr(start, pos - start).empty())
				{
					T tmp = (T)atoll(original.substr(start, pos - start).c_str());
					result.push_back(tmp);
				}
				start = pos + splitor.size(); 
		    }

		}

		void Split(const std::string& original,const std::string& splitor, std::vector<std::string>& result);



		/****
		 * split ||a|b| result:
		 * vec[0]=
		 * vec[1]=a
		 * vec[2]=b
		 *
		 * split |a|b| result:
		 * vec[0]=a
		 * vec[1]=b
		 *
		 * split ||a||b| result:
		 * vec[0]=
		 * vec[1]=a
		 * vec[2]=
		 * vec[3]=b
		 *
		 * split |||b| result:
		 * vec[0]=
		 * vec[1]=
		 * vec[2]=b
		 */
		void SplitWithEmpty(const std::string& original,const std::string& splitor, std::vector<std::string>& result);

		//example: 127.0.0.1:20000
		//IP 127.0.0.1  port 20000
		std::pair<std::string,uint16_t> SplitAddr(const std::string &strAddr);

		std::string Base16Encode(const std::string strBin);
		std::string Base16Decode(const std::string strHex);
		std::string Base64Encode(const std::string str, char ver=0);
		std::string Base64Decode(const std::string str, char ver=0);

		std::string EncodeXml(const std::string& sText, const bool bAttrib=false);
		std::string EncodeJSON(const std::string &sSrc, const bool bUtf8 = true);
		std::string EncodeJS(const std::string& sSrc, const bool bUtf8=true);

		std::string Char2Hex(char c);
		char Hex2Char(char first, char second);
		std::string UrlEncode(const std::string &sSrc);
		std::string UrlDecode(const std::string &sSrc);

		std::string ShowHex(const char *sStr,const int iLen,const int iLineCount = 16);
	}
}

#endif
