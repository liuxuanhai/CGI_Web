#ifndef _FEEDS_SERVER_EXECUTOR_THREAD_PROCESSOR_H_
#define _FEEDS_SERVER_EXECUTOR_THREAD_PROCESSOR_H_

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
	void process_query_feed_detail_resp(ExecutorThreadResponseElement& reply);
	void process_query_feed_resp(ExecutorThreadResponseElement& reply);
	void process_add_feed_resp(ExecutorThreadResponseElement& reply);
	void process_delete_feed_resp(ExecutorThreadResponseElement& reply);

	void process_add_report_feed_resp(ExecutorThreadResponseElement& reply);
	void process_query_user_forbid_resp(ExecutorThreadResponseElement& reply);
	
	
	void process_query_follow_detail_resp(ExecutorThreadResponseElement& reply);
	void process_query_follow_list_resp(ExecutorThreadResponseElement& reply);
	void process_query_follow_comment_detail_resp(ExecutorThreadResponseElement& reply);
	void process_query_follow_comment_resp(ExecutorThreadResponseElement& reply);
	void process_add_follow_resp(ExecutorThreadResponseElement& reply);
	void process_delete_follow_resp(ExecutorThreadResponseElement& reply);


	void process_query_feed_list_favorite_resp(ExecutorThreadResponseElement& reply);
	void process_query_favorite_resp(ExecutorThreadResponseElement& reply);
	void process_add_favorite_resp(ExecutorThreadResponseElement& reply);
	void process_delete_favorite_resp(ExecutorThreadResponseElement& reply);	
	
	void process_query_feed_list_collect_resp(ExecutorThreadResponseElement& reply);
	void process_query_collect_resp(ExecutorThreadResponseElement& reply);
	void process_add_collect_resp(ExecutorThreadResponseElement& reply);
	void process_delete_collect_resp(ExecutorThreadResponseElement& reply);
			
private:
    int m_size;
    int m_queue_capacity;
    ExecutorThreadQueue* m_queue_arr;

    DECL_LOGGER(logger);
};

#endif

