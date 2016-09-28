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
	ExecutorThreadQueue* get_queue(int n);
    int send_request(ExecutorThreadRequestElement& request);
    int poll();

    void process(ExecutorThreadResponseElement& reply);
	void process_user_info_get_resp(ExecutorThreadResponseElement& reply);
	void process_user_info_update_resp(ExecutorThreadResponseElement& reply);
	void process_user_info_get_batch_resp(ExecutorThreadResponseElement& reply);
	void process_user_fans_list_resp(ExecutorThreadResponseElement& reply);
	void process_user_fans_num_resp(ExecutorThreadResponseElement& reply);
	void process_user_follows_list_resp(ExecutorThreadResponseElement& reply);
	void process_follow_resp(ExecutorThreadResponseElement& reply);
	void process_user_power_get_resp(ExecutorThreadResponseElement& reply);
	void process_user_zombie_get_resp(ExecutorThreadResponseElement& reply);
	void process_user_zomanager_get_resp(ExecutorThreadResponseElement& reply);

	
private:
    int m_size;
    int m_queue_capacity;
    ExecutorThreadQueue* m_queue_arr;

    DECL_LOGGER(logger);
};

#endif

