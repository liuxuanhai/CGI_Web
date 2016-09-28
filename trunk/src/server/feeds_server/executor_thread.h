#ifndef _FEEDS_SERVER_EXECUTOR_THREAD_H_
#define _FEEDS_SERVER_EXECUTOR_THREAD_H_

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
	void process_feed_detail_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);	
    void process_feed_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_feed_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_feed_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

	void process_report_feed_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_forbid_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	
	
	void process_follow_detail_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_follow_list_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_follow_comment_detail_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_follow_comment_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_follow_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_follow_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);


	void process_feed_list_favorite_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_favorite_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_favorite_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_favorite_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);


	void process_feed_list_collect_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_collect_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_collect_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_collect_delete(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	

	
private:
private:

	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;
	std::string m_table_name_prefix_feed;
	std::string m_table_name_prefix_feed_index_on_appid;
	std::string m_table_name_prefix_feed_report;
	std::string m_table_name_prefix_user_forbid;
	std::string m_table_name_prefix_feed_favorite;
	std::string m_table_name_prefix_feed_collect;
	std::string m_table_name_prefix_feed_follow;
	std::string m_table_name_prefix_comment_index_on_feedid;
	std::string m_table_name_prefix_reply_index_on_origin_comment;
	
	
private:
    DECL_LOGGER(logger);
};


#endif


