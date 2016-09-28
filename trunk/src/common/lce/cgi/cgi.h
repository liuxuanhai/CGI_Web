#ifndef _LCE_CGI_BASE_H_
#define _LCE_CGI_BASE_H_

#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cgi_any_value.h"
#include "cgi_config.h"
#include "cgi_env.h"
#include "cgi_http_connector.h"
#include "cgi_http_header.h"
#include "cgi_tcp_connector.h"
#include "cgi_input.h"
#include "cgi_mysql.h"
#include "cgi_package.h"
#include "cgi_string.h"
#include "cgi_utils.h"
#include "cgi_value.h"
#include "util/logger.h"

using namespace log4cplus;

namespace lce
{
    namespace cgi
    {
        class Cgi
        {
            public:
                Cgi(uint64_t qwFlag
                    , const std::string& strCgiConfigFile
                    , const std::string& strLogConfigFile);
                
                virtual ~Cgi(){}
                bool Run();
                void SetJsonpCallBack(const std::string& strCallBack, const uint8_t type=0) {   m_strCallBack = strCallBack; m_callBackType = type; }
                const float GetRunTime() const { return ((m_tEndTime.tv_sec - m_tBeginTime.tv_sec)*1000.0+(m_tEndTime.tv_usec - m_tBeginTime.tv_usec)/1000.0); }

            public:
                enum CgiBaseFlag
                {
                    FLAG_POST_ONLY = 1
                };

                enum ReturnErrCode
                {
                    EC_OK = 0,   
                    EC_POST_ONLY = 1,   
                    EC_SERVER_BUSY = 2,
                    EC_PARAM_ERR = 3,
                    EC_BASE_KEY_ERR = 4,  
                };

            protected:
                virtual bool Process()=0;
                virtual bool DerivedInit() {    return true;    }
                virtual bool DoReply(const int iErrCode, const std::string strErr="", const bool bClean=true, const bool bNoCache=true,  const bool bEncodeJS=true);
                
                cgi::CCgiInput& GetInput() {   return m_CgiInput;  }
                cgi::CConfig& GetConfig() const {    return m_sta_Config;    }
                cgi::CHttpHeader& GetHeader()  {    return m_tHeader;   }
                const string& GetCgiName() const {    return m_strCgiName;    }
                const std::string& GetUserIP() const {    return m_strUserIP; }
                const std::string& GetServerIP() const {    return m_strLocalIP;    }
                const std::string& GetConfigPath() const { return m_strPath; }

                cgi::CAnyValueRoot& GetAnyValue() {    return m_AnyValue;  }
                std::string& GetSuccStr() { return m_strSuccInfo;   }
                void PrintTime(const std::string& info);
                
                void SetOutputJson(const std::string& strJson) { m_strJson = strJson; }

            private:
                static cgi::CConfig m_sta_Config; //配置信息对象
                static bool m_sta_bIsLoadConfig; //配置信息是否初始化过的标志

                cgi::CCgiInput m_CgiInput; //用于获取前台输入参数
                cgi::CHttpHeader m_tHeader; //返回的HTTP头部
                cgi::CAnyValueRoot m_AnyValue; //返回给前台的数据

                std::string m_strPath;//配置文件路径
                std::string m_strCgiConfigName;//配置文件名
                std::string m_strLogConfigName;
                std::string m_strCallBack;//JosnP CallBack函数名
                uint8_t m_callBackType; // 0 - Jsonp, 1 - iframe Jsonp
                std::string m_strSuccInfo;//NLog
                std::string m_strLocalIP; //WebServerIP
                std::string m_strUserIP; //ClientIP

                std::string m_strCgiName;//Cgi名称
                std::string m_strJson; //自定义的json输出
                
                bool m_bPostOnly; //只允许post方式访问

                timeval m_tBeginTime; //CGI执行开始时间
                timeval m_tEndTime;   //CGI执行结束时间

            private:
                std::string ReturnErrMsg(const int iErrCode);
                
                bool CanZip();
                bool Compress(string& strCompr, const string& strUnCompr);
                bool BaseInit(); 
                bool AddSuccess();
            private:
                DECL_LOGGER(logger);
        };    

    }
}

#endif
