#ifndef _HOOSHO_PA_SERVER_WX_API_MENU_H_
#define _HOOSHO_PA_SERVER_WX_API_MENU_H_

#include "wx_api.h"

namespace common
{
    namespace wxapi
    {
        class WXAPIMenu:public WXAPI
        {
            public:
                int CreateMenu(const std::string& strPAAccessToken, const std::string& strMenuContent, int& iErrcode);

                int GetMenu(const std::string& strPAAccessToken, std::string& strMenuContent, int& iErrcode);

                int DeleteMenu(const std::string& strPAAccessToken, int& iErrcode);

        };

    }
}



#endif //_HOOSHO_PA_SERVER_WX_API_MENU_H_
