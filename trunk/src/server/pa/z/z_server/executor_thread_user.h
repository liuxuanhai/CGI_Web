#ifndef _Z_SERVER_EXECUTOR_THREAD_H_
#define _Z_SERVER_EXECUTOR_THREAD_H_

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
	void process_user_get_phone_vc(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_update_phone(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_user_info_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_user_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
    void process_user_get_cash(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_recommend_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_trust(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_trust_sb_check(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_my_trust_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_trust_me_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_get_ranking_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_activity_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_activity_info_query(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_get_available_lottery_time(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_do_lottery(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_user_get_lottery_record_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

	//MSG
	void process_msg_add_new(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_msg_get_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_msg_del(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_msg_get_session_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_msg_del_session(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_msg_get_newmsg_status(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	
	void process_redpoint_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_redpoint_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

	void process_notice_add(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);
	void process_notice_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply);

	int get_user_detail_info_list(std::set<std::string>& strOpenidSet, std::vector<hoosho::msg::z::UserInfo>& vecPBUserInfoList);
	int get_user_extra_info_list(std::set<std::string>& strOpenidSet, std::vector<hoosho::msg::z::UserExtraInfo>& vecPBUserStarList);
	int get_me_trust_sb_status_list(std::string strSelfOpenid, std::set<std::string>& strThirdOpenidSet, std::vector<hoosho::msg::z::UserTrustSBStatus>& vecPBTrustStatusList);
	uint32_t generate_prize(const std::string strOpenid, std::string& strLotteryNum);

public:
	ExecutorThreadQueue* m_queue;
	lce::cgi::CMysql m_mysql_helper;

	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_db_table_user_info_name;	
	std::string m_db_table_user_cash_name;
	std::string m_db_table_user_trust_name;
	std::string m_db_table_user_star_name;
	std::string m_db_table_order_info_name;
	std::string m_db_table_order_follow_name;
	std::string m_db_table_comment_index_on_orderid_name;
	std::string m_db_table_reply_index_on_origin_comment_name;
	std::string m_db_table_user_activity_info_name;
	
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

	//lottery param
	uint32_t m_percent_0;
	uint32_t m_percent_1;
	uint32_t m_percent_2;

	uint32_t m_prize1_count_max;
	uint32_t m_prize2_count_max;
	
private:
    DECL_LOGGER(logger);
};


#endif


