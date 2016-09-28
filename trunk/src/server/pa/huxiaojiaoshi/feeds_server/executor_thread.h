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

	int send_template_message_of_new_follow(const uint64_t& qwFollowId, const uint64_t& qwFeedId);
	int send_template_message_of_listen_follow(const uint64_t& qwListenId);

    int func_get_feed_detail(const std::vector<uint64_t>& vecFeedId, const std::string& strOpenid, std::vector<hoosho::j::commstruct::FeedInfo>& vecFeedInfo);
    int func_get_follow_detail(const std::vector<uint64_t>& vecFollowId, std::vector<hoosho::j::commstruct::FollowInfo>& vecFollowInfo);
    int func_add_history(const uint64_t qwFeedId, const std::string& strOpenid);
    int check_out_trade_no_unique(const std::string& strOutTradeNo);

    int func_inc_num_follow(const uint64_t& qwFeedId);
    int func_inc_num_listen(const uint64_t& qwFeedId, const uint64_t& qwFollowId);
    int func_inc_num_comment(const uint64_t& qwFollowId, const uint64_t& qwCommentType, std::string& strOpenid);

    void process_check_out_trade_no_unique(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

    void process_get_feed_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_get_feed_detail(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_add_feed(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

    void process_get_follow_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_get_follow_detail(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_add_follow(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

    void process_get_listen_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_get_listen_detail(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_add_listen(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_comment_follow(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

    void process_get_history_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_check_listen(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_get_comment_follow(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

	void process_set_out_trade_no(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_check_business_id_valid(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

private:
private:

    ExecutorThreadQueue* m_queue;
    lce::cgi::CMysql m_mysql_helper;
	lce::cgi::CMysql m_ano_mysql_helper;
    std::string m_table_name_feed;
    std::string m_table_name_follow;
    std::string m_table_name_listen;
    std::string m_table_name_history;
	std::string m_table_name_user;

	std::string m_pa_appid;

	std::string m_new_follow_template_id;
	std::string m_new_follow_color;
	std::string m_new_follow_url;
	std::string m_new_follow_first;
	std::string m_new_follow_keyword1;
	std::string m_new_follow_keyword2;
	std::string m_new_follow_keyword3;
	std::string m_new_follow_remark;

	std::string m_listen_follow_template_id;
	std::string m_listen_follow_color;
	std::string m_listen_follow_url;
	std::string m_listen_follow_first;
	std::string m_listen_follow_keyword1;
	std::string m_listen_follow_keyword2;
	std::string m_listen_follow_keyword3;
	std::string m_listen_follow_remark;

	uint32_t m_listen_price;
	uint32_t m_share_percent_default;
	uint32_t m_share_percent_good;
	uint32_t m_share_percent_bad;

	uint32_t m_auto_comment_time;

private:
    DECL_LOGGER(logger);
};


#endif


