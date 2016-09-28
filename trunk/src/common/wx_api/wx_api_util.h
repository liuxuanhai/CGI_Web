#ifndef _HOOSHO_WX_API_UTIL_H_
#define _HOOSHO_WX_API_UTIL_H_

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include "util/logger.h"
#include <arpa/inet.h>


namespace common
{
	namespace wxapi
	{
		class WXUserInfo
		{	
		public:
				std::string _openid; 
				std::string _portrait_url;
				std::string _nick;
				std::string _country;
				std::string _province;
				std::string _city;
				uint32_t _sex;

				WXUserInfo():_openid(""), _portrait_url(""), _nick(""), _country(""), _province(""), _city(""), _sex(0)
				{
				}

				std::string to_string() const
				{
						std::ostringstream oss;
						oss.str("");
						oss<<"{openid:"<<_openid
								<<", portrait_url:"<<_portrait_url
								<<", nick:"<<_nick
								<<", country:"<<_country
								<<", province:"<<_province
								<<", city:"<<_city
								<<", sex:"<<_sex
								<<"}";
						return oss.str();
				}
		};

		class WXFansInfo: public WXUserInfo
		{	
		public:
				uint64_t _qwSubscribeTime;

				WXFansInfo():WXUserInfo(), _qwSubscribeTime(0)
				{
				}

				std::string to_string() const
				{
						std::ostringstream oss;
						oss.str("");
						oss<<"{wxUserInfo:"<<WXUserInfo::to_string()
								<<", subscribe_time:"<<_qwSubscribeTime
								<<"}";
						return oss.str();
				}
		};
	
		class WXAPIUtil
		{
		public:
			static bool CheckSignature(const std::string& strSignature, const std::string& strTimestamp,
                               const std::string& strNonce, const std::string& strToken);
			
            static int WXAPIMsgDecrypt(const std::string& strMsgSignature, const std::string& strTimestamp,
                                            const std::string& strNonce, const std::string& strPostData,
                                            const std::string& strToken, const std::string& strCryptkey,
                                            const std::string& strAppid, std::map<std::string, std::string>& mapMsg);

            static int WXAPIMsgEncrypt(const std::string& strReplyMsg, const std::string& strTimestamp,
                                            const std::string& strNonce, const std::string& strToken,
                                            const std::string& strCryptkey, const std::string& strAppid,
                                            std::string& strEncryptMsg);
		

			
			
		};
	}
}



#endif

