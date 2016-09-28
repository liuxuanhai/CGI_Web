#include "bd_api_util.h"
#include "util/md5.h"

namespace common
{
	namespace bdapi
	{
		static log4cplus::Logger logger = log4cplus::Logger::getInstance("BDAPI_UTIL");

		std::string BuildSN(const std::string& strAPI, const std::string& strQueryString, const std::string &strSK)
		{
			std::string strTemp = "/" + strAPI + "?" + strQueryString + strSK;
			LOG4CPLUS_DEBUG(logger, "str to build sn = " << strTemp);
			strTemp = UrlEncode(strTemp);
			LOG4CPLUS_DEBUG(logger, "str after urlencode = " << strTemp);
			md5 stMd5;
			stMd5.update(strTemp);
			std::string strSN = stMd5.out();
			return strSN;
		}

		//过滤值为空的参数，并将参数按字典序升序拼接
		std::string ReqParamToQueryString(const std::map<std::string, std::string>& mapReqParam)
		{
			std::ostringstream ossResult;
			bool bFlag = true;
			std::map<std::string, std::string>::const_iterator iter;
			for(iter = mapReqParam.begin(); iter != mapReqParam.end(); iter++)
			{
				if((iter->second).empty())
				{
					LOG4CPLUS_ERROR(logger, "empty value of key = " << iter->first);
					continue;
				}
				if(bFlag)
				{
					bFlag = false;
				}
				else
				{
					ossResult << "&";
				}
				ossResult << (iter->first);
				ossResult << "=";
				ossResult << UrlEncode(iter->second);
			}
			return ossResult.str();
		}

		std::string UrlEncode(const std::string &strSrc)
		{
			/*
			http://lbsyun.baidu.com/index.php?title=lbscloud/api/appendix
			API请求中需要用到中文或一些特殊字符的参数，为了避免提交到后台乱码，需要对这几个参数值进行编码处理，
			 转换成UTF-8字符的二字符十六进制值，凡是不在下表中的字符都需要进行编码。

			字符集合		字符
			URL非保留字	a b c d e f g h i j k l m n o p q r s t u v w x y z
			 			A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
			 			0 1 2 3 4 5 6 7 8 9 - _ . ~
			URL保留字		! * ' ( ) ; : @ & = + $ , / ? % # [ ]

			如果中文参数中使用URL保留字字符的字面意思，例如，检索关键字包含一个问号，此问号也必须进行编码。
			 */

			std::string strResult;
			for(size_t i = 0; i < strSrc.size(); i++)
			{
				if(isalnum(strSrc[i]) || strSrc[i] == '-' || strSrc[i] == '_' || strSrc[i] == '.' || strSrc[i] == '~')
				{
					strResult.append(1, strSrc[i]);
				}
				else
				{
					strResult.append(1, '%');
					strResult.append(Char2Hex(strSrc[i]));
				}
			}
			return strResult;
		}

		std::string Char2Hex(const char c)
		{
			static const char sHex[] = "0123456789ABCDEF";
			std::string strHex;

			strHex.push_back(sHex[uint8_t(c) >> 4]);
			strHex.push_back(sHex[uint8_t(c) & 15]);

			return strHex;
		}
	}
}



