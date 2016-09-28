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
	void process_user_login(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_user_info_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_user_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

private:
	std::string get_pa_base_accesstoken(const std::string& strAPPID, const std::string& strTokenServerIP, int iTokenServerPort);

private:
	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;

	std::string db_ip;
	std::string db_user;
	std::string db_passwd;
	std::string db_name;
	std::string db_table_user_info_name;

	std::string pa_appid;
	std::string pa_appsecret;

private:
    DECL_LOGGER(logger);
};


#endif


