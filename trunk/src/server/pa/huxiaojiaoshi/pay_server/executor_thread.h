#ifndef _SERVER_EXECUTOR_THREAD_H_
#define _SERVER_EXECUTOR_THREAD_H_

#include <string>
#include <map>
#include "thread/thread.h"
#include "util/logger.h"
#include "executor_thread_queue.h"
#include "cgi/cgi_mysql.h"
#include <tinyxml/tinyxml.h>
#include <tinyxml/tinystr.h>
#include "pay_common_util.h"


using namespace std;

class ExecutorThread : public lce::thread::Thread
{
public:
    ExecutorThread();
    ~ExecutorThread();
    virtual void run();
	
	int init(ExecutorThreadQueue* queue);    	
	int getUnifiedOrderBusinessInfo(const std::string& openid, const std::string& out_trade_no, ExecutorThreadResponseElement& reply);
	bool getUserCashInfo(const std::string& openid, UserCash& tUserCash);	
	int checkOutTradeNoFromWX(const std::string& out_trade_no, const std::string& openid);
	void process_unified_order(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_transfer_order(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_user_cash(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_query_cash_flow(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_unified_order_callback(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_unified_order_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_listen_income(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_sendredpack(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_outcome_req_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_outcome_req_commit(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_outcome_req_check(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_outcome_req_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

private:
	TiXmlDocument* m_xml_document;
	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;
	uint32_t listen_price;
	float share_percent_default;
	float share_percent_good;
	float share_percent_bad;	
	std::string pa_appid;
	std::string answer_body;
	std::string listen_body;
	
private:
    DECL_LOGGER(logger);
};


#endif


