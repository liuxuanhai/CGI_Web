#ifndef _TEST_SERVER_EXECUTOR_THREAD_H_
#define _TEST_SERVER_EXECUTOR_THREAD_H_

#include <string>
#include <map>
#include "thread/thread.h"
#include "util/logger.h"
#include "executor_thread_queue.h"
#include "cgi/cgi_mysql.h"

using namespace std;

class ExecutorThread : public lce::thread::Thread
{
public:
    ExecutorThread();
    ~ExecutorThread();
    virtual void run();
	
	int init(ExecutorThreadQueue* queue);       
    void process_preauth(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_pa_token(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_resource_download(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_pa_list_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_pa_tab_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);	
	void process_pa_expert_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

private:
	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;

	std::string db_ip;
	std::string db_user;
	std::string db_passwd;
	std::string db_name;
	std::string db_table_pa_info_name;
	std::string db_table_pa_tab_name;
	std::string db_table_pa_expert_prefix;
private:
    DECL_LOGGER(logger);
};


#endif


