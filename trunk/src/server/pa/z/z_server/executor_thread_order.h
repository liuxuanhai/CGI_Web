#ifndef _Z_SERVER_EXECUTOR_THREAD_ORDER_H_
#define _Z_SERVER_EXECUTOR_THREAD_ORDER_H_

#include <string>
#include <map>
#include "thread/thread.h"
#include "util/logger.h"
#include "cgi/cgi.h"
#include "executor_thread_queue.h"
#include "dao.h"

using namespace std;

class ExecutorThreadOrder : public lce::thread::Thread
{
public:
    ExecutorThreadOrder();
    ~ExecutorThreadOrder();
    virtual void run();
	
	int init(ExecutorThreadQueue* queue);

	void process_cash_flow_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_outcome_commit(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_outcome_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	
    void process_order_create(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_payed(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_fetch(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_fetch_canceled(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_fetch_accepted(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_search(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_cancel(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_finish(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_comment(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_info_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_list_user_created(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_list_user_fetched(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_reward(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

	void process_order_favor_check(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_favor_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_favor_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_follow_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_comment_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_comment_detail(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_order_follow_del(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

	void func_get_order_info(ExecutorThreadResponseElement &reply, const std::set<uint64_t>& orderIdSet);
	void func_get_user_info_by_order_info(ExecutorThreadResponseElement& reply, bool bGetUserStar = false);
	void func_get_user_info_by_favor_info(ExecutorThreadResponseElement& reply);
	void func_get_user_info_by_comment_info(ExecutorThreadResponseElement& reply);

	//有人抢单
	int send_template_message_of_order_fetch(const DaoOrderInfo& stDaoOrderInfo);
	//确认抢单者
	int send_template_message_of_order_fetch_accept(const DaoOrderInfo& stDaoOrderInfo);
	//订单完成
	int send_template_message_of_order_finish(const DaoOrderInfo& stDaoOrderInfo);
	int send_message_of_order_finish(const DaoOrderInfo& stDaoOrderInfo);

public:
	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;


	std::string m_pa_appid;
	std::string m_pa_pay_mchid;
	std::string m_pa_pay_api_key;
	std::string m_pa_pay_ntf_url;
	std::string m_pa_pay_reward_desc;

	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_db_table_user_info_name;
	std::string m_db_table_order_info_name;
	std::string m_db_table_order_media_info_name;
	std::string m_db_table_order_fetch_state_name;
	std::string m_db_table_order_reward_name;
	std::string m_db_table_user_trust_name;
	std::string m_db_table_wx_outtrade_name;
	std::string m_db_table_user_cash_name;
	std::string m_db_table_cash_flow_name;
	std::string m_db_table_out_come_name;
	std::string m_db_table_user_star_name;
	std::string m_db_table_order_favor_name;
	std::string m_db_table_order_follow_name;
	std::string m_db_table_comment_index_on_orderid_name;
	std::string m_db_table_reply_index_on_origin_comment_name;

	std::string m_order_fetch_template_id;
	std::string m_order_fetch_color;
	std::string m_order_fetch_url;
	std::string m_order_fetch_first;
	std::string m_order_fetch_keyword1;
	std::string m_order_fetch_keyword2;
	std::string m_order_fetch_keyword3;
	std::string m_order_fetch_remark;
	uint32_t m_order_fetch_waiting_user_size_first;
	uint32_t m_order_fetch_waiting_user_size_second;
	uint32_t m_order_fetch_waiting_user_size_third;

	std::string m_order_fetch_accept_template_id;
	std::string m_order_fetch_accept_color;
	std::string m_order_fetch_accept_url;
	std::string m_order_fetch_accept_first;
	std::string m_order_fetch_accept_keyword1;
	std::string m_order_fetch_accept_keyword2;
	std::string m_order_fetch_accept_keyword3;
	std::string m_order_fetch_accept_remark;
	std::string m_order_fetch_accept_order_desc_of_audio;

	std::string m_order_finish_template_id;
	std::string m_order_finish_color;
	std::string m_order_finish_url;
	std::string m_order_finish_first;
	std::string m_order_finish_keyword1;
	std::string m_order_finish_keyword2;
	std::string m_order_finish_keyword3;
	std::string m_order_finish_remark;

	std::string m_bd_api_ak;
	std::string m_bd_api_sk;
	uint64_t m_bd_api_geotable_id;
	uint32_t m_bd_api_geosearch_radius;
private:
    DECL_LOGGER(logger);
};


#endif


