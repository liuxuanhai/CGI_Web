#ifndef _HOOSHO_PA_SERVER_WX_API_TICKET_H_
#define _HOOSHO_PA_SERVER_WX_API_TICKET_H_

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include "util/logger.h"
#include "wx_api.h"

namespace common
{
		namespace wxapi
		{

				class WXAPITicket:public WXAPI
				{
						public:
								int GetJSAPITicket(const std::string& strAppid, const std::string& strAppscret, std::string& strJSAPITicket);
								int GetJSAPITicket(const std::string& strPAAccessToken, std::string& strJSAPITicket);
				};
		}
}

#endif













