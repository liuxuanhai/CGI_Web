#ifndef _TEST_SERVER_EXECUTOR_THREAD_PROCESSOR_H_
#define _TEST_SERVER_EXECUTOR_THREAD_PROCESSOR_H_

#include "executor_thread_queue.h"
#include "util/logger.h"

class ExecutorThreadProcessor
{
public:
    ExecutorThreadProcessor()
    {
    }
    ~ExecutorThreadProcessor()
    {
    }

    int init(int size,int queue_capacity);
    int get_size();
	ExecutorThreadQueue* get_user_queue(int n);
	ExecutorThreadQueue* get_order_queue(int n);
    int send_request(ExecutorThreadRequestElement& request);
    int poll();

    void process(ExecutorThreadResponseElement& reply);
private:
    int m_size;
    int m_queue_capacity;
    ExecutorThreadQueue* m_queue_arr_user;
	ExecutorThreadQueue* m_queue_arr_order;

    DECL_LOGGER(logger);
};

#endif

