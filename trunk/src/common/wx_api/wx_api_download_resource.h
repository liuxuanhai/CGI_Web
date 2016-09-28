#ifndef _HOOSHO_PA_SERVER_WX_API_DOWNLOAD_RESOURCE_H_
#define _HOOSHO_PA_SERVER_WX_API_DOWNLOAD_RESOURCE_H_

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include "util/logger.h"
#include "wx_api.h"

namespace common
{
		namespace wxapi
		{

				class WXAPIDownloadResource:public WXAPI
				{
						public:
								int DownloadResource(const std::string& strAppid, const std::string& strAppscret, const std::string& strMediaId, std::string& strResource);
								int DownloadResource(const std::string& strPAAccessToken, const std::string& strMediaId, std::string& strResource);
				};
		}
}

#endif













