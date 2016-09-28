#ifndef _HOOSHO_PA_SERVER_WX_API_MESSAGE_H_
#define _HOOSHO_PA_SERVER_WX_API_MESSAGE_H_

#include "wx_api.h"

namespace common
{
	namespace wxapi
	{
		class WXAPIMessage:public WXAPI
		{
		public:
			//发送模板消息
			int SendTemplateMessage(const std::string& strPAAccessToken, const std::string& strPostData, int& iErrcode);

			//群发消息接口
			int SendMessage(const std::string& strPAAccessToken, const std::string& strPostData, int& iErrcode);

			//预览消息接口
			int SendPreviewMessage(const std::string& strPAAccessToken, const std::string& strPostData, int& iErrcode);
		};
	}
}



#endif //SRC_WX_API_MESSAGE_H
