#ifndef _NOTICE_SERVER_EXECUTOR_THREAD_H_
#define _NOTICE_SERVER_EXECUTOR_THREAD_H_

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
    void process_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

private:

	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;
private:
    DECL_LOGGER(logger);
};


#endif


