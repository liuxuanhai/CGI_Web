#ifndef _EXECUTOR_THREAD_USER_H_
#define _EXECUTOR_THREAD_USER_H_

#include <string>
#include <map>
#include "thread/thread.h"
#include "util/logger.h"
#include "cgi/cgi.h"
#include "executor_thread_queue.h"

using namespace std;

class ExecutorThreadUser : public lce::thread::Thread
{
public:
    ExecutorThreadUser();
    ~ExecutorThreadUser();
    virtual void run();
	
	int init(ExecutorThreadQueue* queue);

	void process_user_login(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_get_phone_vc(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_set_phone(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_info_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_identity_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_identity_info_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_recv_info_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_recv_info_del(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_recv_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_recv_info_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	
public:
	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;

	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_db_table_user_info_name;		
	
	std::string m_pa_appid;
	std::string m_pa_appsecret;

	//sms param
	std::string m_sms_api_host;
	uint16_t m_sms_api_port;
	std::string m_sms_api_account;
	std::string m_sms_api_accountpwd;
	std::string m_sms_api_appid;
	std::string m_sms_api_version;
	std::string m_sms_templateid;	
	
private:
    DECL_LOGGER(logger);
};


#endif


