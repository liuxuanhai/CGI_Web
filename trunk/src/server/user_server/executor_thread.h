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
    void process_user_info_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_user_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_user_info_get_batch(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_user_fans_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_user_fans_num(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_user_follows_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_follow(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_power_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_zombies_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_zomanager_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

private:
	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;

	std::string db_ip;
	std::string db_user;
	std::string db_passwd;
	std::string db_name;
	std::string db_table_user_info_name_prefix;
	std::string db_table_user_password_name_prefix;
	std::string db_table_user_follow_name_prefix;
	std::string db_table_user_appid_power_name;
	std::string db_table_user_zombie_name;

private:
    DECL_LOGGER(logger);
};


#endif


