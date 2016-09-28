#include "pa_info_manager.h"
#include "global_var.h"
#include "util/lce_util.h"
#include "wx_api_ticket.h"
#include "executor_thread_processor.h"
#include <openssl/sha.h>

IMPL_LOGGER(PAInfo, logger);
IMPL_LOGGER(PAInfoManager, logger);


std::string PAInfo::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss<<"{appid: "<<strAppid
		<<", appsecret: "<<strAppSecret
		<<", baseaccesstoken: "<<strBaseAccessToken
		<<", jsapiticket: "<<strJSAPITicket
		<<", lastupdatets: "<<qwLastUpdateTs
		<<"}";

	return oss.str();
}

void PAInfo::update_token(const std::string& strAccessToekn, const std::string& strTicketJSAPI)
{
	strBaseAccessToken = strAccessToekn;
	strJSAPITicket = strTicketJSAPI;
	qwLastUpdateTs = time(0);

	LOG4CPLUS_DEBUG(logger, "NOTICE:!!!!!!!!!! update token to pa_info="<<ToString());
}


int PAInfoManager::init()
{
	mapPAInfo.clear();
	
	const lce::app::Config& stConfig = g_server->config(); 
	int iNum = stConfig.get_int_param("PA_INFO", "num");
	for(int i=0; i<iNum; ++i)
	{
		std::string strIdx = lce::util::StringOP::TypeToStr(i);
		std::string strAppid = lce::util::StringOP::Trim(stConfig.get_string_param("PA_INFO", "appid_"+strIdx, "xxx"));
		std::string strAppSecret = lce::util::StringOP::Trim(stConfig.get_string_param("PA_INFO", "appsecret_"+strIdx, "yyy"));
		if(strAppid.empty() || strAppSecret.empty())
		{
			LOG4CPLUS_DEBUG(logger, "found empty painfo, appid="<<strAppid<<", appsecret="<<strAppSecret);
			return -1;
		}

		::common::wxapi::WXAPI stWXAPI;
		std::string strBaseAccessToken = "";
		int iRet = stWXAPI.PAGetAccessToken(strAppid, strAppSecret, strBaseAccessToken);
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "fuck, PAGetAccessToken failed!strAppid="<<strAppid);
			return -1;
		}

		std::string strJSAPITicket = "";
		::common::wxapi::WXAPITicket stWXAPITicket;
		iRet = stWXAPITicket.GetJSAPITicket(strBaseAccessToken, strJSAPITicket);
		if(iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "fuck, GetJSAPITicket failed!strAppid="<<strAppid);
			return -1;
		}
		
		PAInfo stPaInfo(strAppid, strAppSecret);
		stPaInfo.update_token(strBaseAccessToken, strJSAPITicket);
		
		mapPAInfo[strAppid] = stPaInfo;
	}

	if(mapPAInfo.empty())
	{
		LOG4CPLUS_ERROR(logger, "no painfo found, init failed");
		return -1;
	}

	assert(g_timer_container->schedule(*this, 60, 60, NULL) >= 0);
	
	return 0;
}

void PAInfoManager::handle_timeout(void * param)
{
	for(PAInfoMap::iterator iter = mapPAInfo.begin(); iter!=mapPAInfo.end(); ++iter)
	{
		const PAInfo& stPAInfo = iter->second;
		
		LOG4CPLUS_DEBUG(logger, "ckeck if need update token for pa_info="<<stPAInfo.ToString());	
		bool bNeedUpdate = false;
		if(stPAInfo.strBaseAccessToken.empty() || stPAInfo.strJSAPITicket.empty() || 0 == stPAInfo.qwLastUpdateTs)
		{
			LOG4CPLUS_DEBUG(logger, "need update !!! reason: data is null");
			bNeedUpdate = true;
		}
				
		int iTokenExpiredSeconds = g_server->config().get_int_param("PA_TOKEN", "expired", 0);
		if(stPAInfo.qwLastUpdateTs + iTokenExpiredSeconds <= (uint64_t)time(0))
		{
			LOG4CPLUS_DEBUG(logger, "need update !!! reason: data is expired for iTokenExpiredSeconds="<<iTokenExpiredSeconds);
			bNeedUpdate = true;
		}

		if(!bNeedUpdate)
		{
			LOG4CPLUS_DEBUG(logger, "no need update, ignore ^ _ ^");
			continue;
		}

		ExecutorThreadRequestElement element(ExecutorThreadRequestType::T_PA_TOKEN_UPDATE);
		element.need_reply();
		element.m_pa_appid = stPAInfo.strAppid;
		element.m_pa_appsecret = stPAInfo.strAppSecret;
		g_executor_thread_processor->send_request(element);	
	}
	
	return;
}

void PAInfoManager::update_pa_info(const std::string& strAppid, const std::string& strAccessToekn, const std::string& strTicketJSAPI)
{
	PAInfoMap::iterator iter = mapPAInfo.find(strAppid);
	if(iter == mapPAInfo.end())
	{
		LOG4CPLUS_ERROR(logger, "update_pa_info failed, no PAInfo Founnd for strAppid="<<strAppid);
		return;
	}

	PAInfo& stPaInfo = iter->second;
	stPaInfo.update_token(strAccessToekn, strTicketJSAPI);
	return;
}


