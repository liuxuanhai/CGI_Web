#ifndef _HOOSHO_PA_SERVER_WX_API_OAUTH2_H_
#define _HOOSHO_PA_SERVER_WX_API_OAUTH2_H_

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include "util/logger.h"
#include "wx_api.h"
#include "wx_api_util.h"

namespace common
{
		namespace wxapi
		{

				class WXAPIOAuth2:public WXAPI
				{
						public:
								int OAuth2GetAccessToken(const std::string& strAppid
												, const std::string& strAppscret
												, const std::string& strPreAuthCode
												, std::string& strAccessToken
												, std::string& strRefreshToken
												, std::string& strOpenid
												, std::string& strScope);
								
								int OAuth2GetUserInfo(const std::string& strOpenid
												, const std::string& strUserInfoScoreAccessToken
												, WXUserInfo& stWXUserInfo);

								int OAuth2GetUserInfo(const std::string& strAppid
												, const std::string& strAppscret
												, const std::string& strPreAuthCode
												, WXUserInfo& stWXUserInfo);
				};

		}
}


#endif













