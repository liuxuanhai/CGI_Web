#ifndef _HOOSHO_PA_SERVER_WX_API_FANS_H_
#define _HOOSHO_PA_SERVER_WX_API_FANS_H_

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
				class WXAPIFans: public WXAPI
				{
				public:
					int GetFansInfo(const std::string& strAppid
									, const std::string& strAppscret
									, const std::string& strOpenid
									, WXFansInfo& stWXFansInfo);
					
					int GetFansInfo(const std::string& strAccessToken
									, const std::string& strOpenid
									, WXFansInfo& stWXFansInfo);
				};

		}
}


#endif













