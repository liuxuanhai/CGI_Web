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

    void process_check_out_trade_no_unique(ExecutorThreadResponseElement& reply);

    void process_get_feed_list_resp(ExecutorThreadResponseElement& reply);
    void process_get_feed_detail_resp(ExecutorThreadResponseElement& reply);
    void process_add_feed_resp(ExecutorThreadResponseElement& reply);

    void process_get_follow_list_resp(ExecutorThreadResponseElement& reply);
    void process_get_follow_detail_resp(ExecutorThreadResponseElement& reply);
    void process_add_follow_resp(ExecutorThreadResponseElement& reply);

    void process_get_listen_list_resp(ExecutorThreadResponseElement& reply);
    void process_get_listen_detail_resp(ExecutorThreadResponseElement& reply);
    void process_add_listen_resp(ExecutorThreadResponseElement& reply);
    void process_comment_follow_resp(ExecutorThreadResponseElement& reply);

    void process_get_history_list_resp(ExecutorThreadResponseElement& reply);
    void process_check_listen_resp(ExecutorThreadResponseElement& reply);
    void process_get_comment_follow_resp(ExecutorThreadResponseElement& reply);

    void process_set_out_trade_no_resp(ExecutorThreadResponseElement& reply);
    void process_check_business_id_resp(ExecutorThreadResponseElement& reply);

private:
    int m_size;
    int m_queue_capacity;
    ExecutorThreadQueue* m_queue_arr;

    DECL_LOGGER(logger);
};

#endif

