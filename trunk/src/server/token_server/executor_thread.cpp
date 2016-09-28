#include "executor_thread.h"
#include "global_var.h"
#include "wx_api_oauth2.h"
#include "wx_api_ticket.h"
#include "wx_api_download_resource.h"

IMPL_LOGGER(ExecutorThread, logger)

ExecutorThread::ExecutorThread()
{
    m_queue = NULL;
}

ExecutorThread::~ExecutorThread()
{
}

int ExecutorThread::init(ExecutorThreadQueue * queue)
{
    m_queue = queue;
	
	return 0;
}

void ExecutorThread::run()
{
    LOG4CPLUS_TRACE(logger, "ExecutorThread "<<pthread_self()<<" running ...");

    while(true)
    {
        if(m_queue->request_empty())
        {
            usleep(10);
            continue;
        }

        ExecutorThreadRequestElement request = m_queue->get_request();
        m_queue->pop_request();
		
        LOG4CPLUS_TRACE(logger, "ExecutorThread get request, "<<request.ToString());
        ExecutorThreadResponseElement reply(request.m_request_type);

        switch(request.m_request_type)
        {
			case ExecutorThreadRequestType::T_PA_TOKEN_UPDATE:
				process_pa_token_update(request, reply);
				break;	
	
            default:
                LOG4CPLUS_ERROR(logger, "ExecutorThread get unknown type:"
					           <<request.m_request_type<<"("<<ExecutorThreadRequestType::type_string(request.m_request_type)<<")");
                break;
        }

        if(!request.m_need_reply)//no need reply
        {
			continue;
        }

        while(m_queue->reply_full())
		{
			usleep(50);
		}
		
        m_queue->push_reply(reply);
    }
}

void ExecutorThread::process_pa_token_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	reply.m_pa_appid = request.m_pa_appid;
	reply.m_pa_appsecret = request.m_pa_appsecret;

	std::string strPAAccessToken = "";
	::common::wxapi::WXAPI stWXAPI;
	int iRet = stWXAPI.PAGetAccessToken(request.m_pa_appid, request.m_pa_appsecret, strPAAccessToken);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		LOG4CPLUS_ERROR(logger, "fuck, PAGetAccessToken failed!");
		return;
	}
	reply.m_pa_accesstoken = strPAAccessToken;

	std::string strJSAPITicket = "";
	::common::wxapi::WXAPITicket stWXAPITicket;
	iRet = stWXAPITicket.GetJSAPITicket(strPAAccessToken, strJSAPITicket);
	if(iRet < 0)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_FAIL;
		LOG4CPLUS_ERROR(logger, "fuck, GetJSAPITicket failed!");
		return;
	}
	reply.m_pa_ticket_jsapi = strJSAPITicket;

	
	return;
}

