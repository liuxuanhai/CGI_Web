#ifndef _WX_API_PARSE_IPADDR_H_
#define _WX_API_PARSE_IPADDR_H_

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include "util/logger.h"
#include "wx_api.h"

namespace common
{
	namespace wxapi
	{
		class WXAPIParseIPAddr:public WXAPI
		{
			public:
				int ParseIPAddr(const std::string& strIP, std::string& strIPCity);
		};
	}
}

#endif

