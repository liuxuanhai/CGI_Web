#ifndef _SERVER_EXECUTOR_THREAD_H_
#define _SERVER_EXECUTOR_THREAD_H_

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

	void process_msg_query(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	void process_msg_add(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	void process_msg_delete(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	void process_query_newmsg_status(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	void process_query_session_list(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	void process_delete_session(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	void process_red_point_query(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	void process_red_point_update(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	int process_red_point_update(uint64_t st_openid_md5, uint64_t st_pa_appid_md5, uint64_t st_redpoint_type, uint64_t st_redpoint_opt);
	void process_notice_record_add(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	void process_notice_record_query(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	void process_notice_record_delete(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);
	void process_system_msg_query(ExecutorThreadRequestElement& request, ::hoosho::msg::Msg& stRespMsg);

private:

	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;
private:
    DECL_LOGGER(logger);
};


#endif


