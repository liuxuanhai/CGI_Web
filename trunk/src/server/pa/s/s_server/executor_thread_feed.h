#ifndef _EXECUTOR_THREAD_FEED_H_
#define _EXECUTOR_THREAD_FEED_H_

#include <string>
#include <map>
#include "thread/thread.h"
#include "util/logger.h"
#include "cgi/cgi.h"
#include "executor_thread_queue.h"
#include "dao.h"

using namespace std;

class ExecutorThreadFeed : public lce::thread::Thread
{
public:
    ExecutorThreadFeed();
    ~ExecutorThreadFeed();
    virtual void run();
	
	int init(ExecutorThreadQueue* queue);

	void process_get_good_type(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_good_info(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_add_ticket(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_user_feed_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_feed_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_user_show_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_show_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_show_detail(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_add_show(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_user_collect_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_add_collect(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_feed_join_user_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_user_feed_contend_id_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_banner_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_del_collect(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_check_user_collect(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_user_luck_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_feed_detail(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_user_ticket_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

public:
	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;

	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_db_table_user_info_name;

	std::string m_pa_appid;

	uint32_t m_user_upload_ticket_limit = 10;
	
private:
    DECL_LOGGER(logger);
};


#endif


