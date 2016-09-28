#ifndef _LCE_UTIL_STR_OP_H_
#define  _LCE_UTIL_STR_OP_H_

#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>


namespace lce
{
	namespace util
	{
		class StringOP
		{
		public:
			template <typename T>
			static std::string TypeToStr(const T &n)
			{
				std::ostringstream stream;
				stream << n;
				return stream.str();
			}

			static void ToUpper(std::string &str);
			static void ToLower(std::string &str);
			static std::string ToUpperCopy(const std::string &str);
			static std::string ToLowerCopy(const std::string &str);
			static std::string Trim(const std::string &str);
			static void Split(const std::string& original,const std::string& splitor, std::vector<std::string>& result);
			template <typename T>
			static void SplitInt(const std::string& original,const std::string& splitor, std::vector<T>& result)
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
						long long ll = atoll(original.substr(start).c_str());
						result.push_back((T)ll);
						break;
					}

					if(!original.substr(start, pos - start).empty())
					{
						long long ll = atoll(original.substr(start, pos - start).c_str());
						result.push_back((T)ll);
					}
					start = pos + splitor.size(); 
			    }

			}

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
			static void SplitWithEmpty(const std::string& original,const std::string& splitor, std::vector<std::string>& result);

			
			static std::string GetTime(time_t seconds=0);	//返回格式为 HH:MM:SS的时间值,默认取当前时间
			static std::string GetDate(time_t seconds=0);	//返回格式为 YYYY-MM-DD的日期值 
			static std::string GetDateTime(time_t seconds=0); //返回格式为 YYYY-MM-DD HH:MM:SS 的日期时间值
			static int GetSeconds(const std::string &strDateTime,time_t &t); //输入格式为 YYYY-MM-DD H:M:S的日期值，返回其对应的秒数

			static std::string Base16Encode(const std::string strBin);
			static std::string Base16Decode(const std::string strHex);
			static std::string Base64Encode(const std::string str, char ver=0);
			static std::string Base64Decode(const std::string str, char ver=0);

			static std::string EncodeXml(const std::string& sText, const bool bAttrib=false);
			static std::string EncodeJSON(const std::string &sSrc, const bool bUtf8 = true);
			static std::string EncodeJS(const std::string& sSrc, const bool bUtf8=true);
			
			static std::string UrlEncode(const std::string &sSrc);
			static std::string UrlDecode(const std::string &sSrc);

			
			static std::string Char2Hex(char c);
			static char Hex2Char(char first, char second);
			static std::string ShowHex(const char *sStr,const int iLen,const int iLineCount = 16);
		};
	}
}


#endif

