#include "executor_thread.h"
#include "global_var.h"

IMPL_LOGGER(ExecutorThread, logger)

ExecutorThread::ExecutorThread()
{
    m_queue = NULL;
}

ExecutorThread::~ExecutorThread()
{
}

int ExecutorThread::init(ExecutorThreadQueue * queue)
{
    m_queue = queue;
    const lce::app::Config& stConfig = g_server->config();
	db_ip = stConfig.get_string_param("DB", "ip"); 
	db_user = stConfig.get_string_param("DB", "user"); 
	db_passwd = stConfig.get_string_param("DB", "passwd"); 
	db_name = stConfig.get_string_param("DB", "db_name"); 
	db_table_user_info_name_prefix = stConfig.get_string_param("DB", "table_user_info_name_prefix"); 
	db_table_user_password_name_prefix = stConfig.get_string_param("DB", "table_user_password_name_prefix"); 
	db_table_user_follow_name_prefix = stConfig.get_string_param("DB", "table_user_follow_name_prefix"); 
	db_table_user_appid_power_name = stConfig.get_string_param("DB", "table_user_appid_power_name"); 
	db_table_user_zombie_name = stConfig.get_string_param("DB", "table_user_zombie_name"); 
	
    assert(m_mysql_helper.Init(db_ip, db_name, db_user, db_passwd));
    
	return 0;
}

void ExecutorThread::run()
{
    LOG4CPLUS_TRACE(logger, "ExecutorThread "<<pthread_self()<<" running ...");

    while(true)
    {
        if(m_queue->request_empty())
        {
            usleep(10);
            continue;
        }

        ExecutorThreadRequestElement request = m_queue->get_request();
        m_queue->pop_request();
		
        LOG4CPLUS_TRACE(logger, "ExecutorThread get request, "<<request.ToString());
        ExecutorThreadResponseElement reply(request.m_request_type, request.m_fsm_type, request.m_fsm_id);

        switch(request.m_request_type)
        {					
            case ExecutorThreadRequestType::T_USERINFO_QUERY:
				process_user_info_get(request, reply);
            	break;

			case ExecutorThreadRequestType::T_USERINFO_UPDATE:
				process_user_info_update(request, reply);
				break;	

			case ExecutorThreadRequestType::T_USERINFO_BATCH_QUERY:
				process_user_info_get_batch(request, reply);
				break;

			case ExecutorThreadRequestType::T_USERFANS_LIST:
				process_user_fans_list(request, reply);
				break;
					
			case ExecutorThreadRequestType::T_USERFANS_NUM:
				process_user_fans_num(request, reply);
				break;	
				
			case ExecutorThreadRequestType::T_USERFOLLOWS_LIST:
				process_user_follows_list(request, reply);
				break;
				
			case ExecutorThreadRequestType::T_FOLLOW:
				process_follow(request, reply);
				break;

			case ExecutorThreadRequestType::T_POWER_QUERY:
				process_power_get(request, reply);
				break;

			case ExecutorThreadRequestType::T_ZOMBIE_QUERY:
				process_zombies_get(request, reply);
				break;

			case ExecutorThreadRequestType::T_ZOMANAGER_QUERY:
				process_zomanager_get(request, reply);
				break;
			
            default:
                LOG4CPLUS_ERROR(logger, "ExecutorThread get unknown type:"
					           <<request.m_request_type<<"("<<ExecutorThreadRequestType::type_string(request.m_request_type)<<")");
                break;
        }

        if(!request.m_need_reply)//no need reply
        {
			continue;
        }

        while(m_queue->reply_full())
		{
			usleep(50);
		}
		
        m_queue->push_reply(reply);
    }
}

static void user_info_db_2_pb(::hoosho::commstruct::UserDetailInfo& stUserDetailInfo
							, ::lce::cgi::CMysql& mysql)
{
	stUserDetailInfo.set_openid_md5(strtoul(mysql.GetRow(0), NULL, 10));
	stUserDetailInfo.set_openid(mysql.GetRow(1));
	stUserDetailInfo.set_portrait_pic_id(strtoll(mysql.GetRow(2), NULL, 10));
	stUserDetailInfo.set_portrait_pic_url_wx(mysql.GetRow(3));
	stUserDetailInfo.set_nick(mysql.GetRow(4));
	stUserDetailInfo.set_nick_wx(mysql.GetRow(5));
	stUserDetailInfo.set_addr_country(mysql.GetRow(6));
	stUserDetailInfo.set_addr_province(mysql.GetRow(7));
	stUserDetailInfo.set_addr_city(mysql.GetRow(8));
	stUserDetailInfo.set_sex(strtoul(mysql.GetRow(12), NULL, 10));
	stUserDetailInfo.set_phone_no(strtoul(mysql.GetRow(13), NULL, 10));
	stUserDetailInfo.set_email(mysql.GetRow(14));
	stUserDetailInfo.set_self_desc(mysql.GetRow(15));
	stUserDetailInfo.set_main_page_cover_pic_id(strtoul(mysql.GetRow(16), NULL, 10));
	stUserDetailInfo.set_hoosho_no(strtoul(mysql.GetRow(17), NULL, 10));
	stUserDetailInfo.set_birthday_ts(strtoul(mysql.GetRow(18), NULL, 10));
	
	/*
		+------------------------+---------------------+------+-----+---------+-------+
	| Field                  | Type                | Null | Key | Default | Extra |
	+------------------------+---------------------+------+-----+---------+-------+
	0| openid_md5             | bigint(20) unsigned | NO   | PRI | 0       |       |
	1| openid                 | char(128)           | NO   |     |         |       |
	2| portrait_pic_id        | bigint(20) unsigned | NO   |     | 0       |       |
	3| portait_pic_url_wx     | blob                | NO   |     | NULL    |       |
	4| nick                   | char(64)            | NO   |     |         |       |
	5| nick_wx                | char(64)            | NO   |     |         |       |
	6| addr_country           | char(128)           | NO   |     |         |       |
	7| addr_province          | char(128)           | NO   |     |         |       |
	8| addr_city              | char(128)           | NO   |     |         |       |
	9| addr_country_md5       | bigint(20) unsigned | NO   |     | 0       |       |
	10| addr_province_md5      | bigint(20) unsigned | NO   |     | 0       |       |
	11| addr_city_md5          | bigint(20) unsigned | NO   |     | 0       |       |
	12| sex                    | tinyint(3) unsigned | NO   |     | 0       |       |
	13| phone_no               | bigint(20) unsigned | NO   |     | 0       |       |
	14| email                  | char(255)           | NO   |     |         |       |
	15| self_desc              | blob                | NO   |     | NULL    |       |
	16| main_page_cover_pic_id | bigint(20) unsigned | NO   |     | 0       |       |
	17| hoosho_no              | bigint(20) unsigned | NO   |     | 0       |       |
	18| birthday_ts            | bigint(20) unsigned | NO   |     | 0       |       |
	19| create_ts              | bigint(20) unsigned | NO   |     | 0       |       |
	20| last_update_ts         | bigint(20) unsigned | NO   |     | 0       |       |
	+------------------------+---------------------+------+-----+---------+-------+
	*/
}

void ExecutorThread::process_user_info_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	uint64_t qwOpenidMD5 = 0;
	OPENID_TO_MD5(request.m_user_detail_info_from.openid(), qwOpenidMD5);

	std::string strTableName = "";
	UINT64_TO_TABLENAME(db_table_user_info_name_prefix, qwOpenidMD5, strTableName);

	std::ostringstream oss;
	oss.str("");
	oss<<"select * from "<<strTableName<<" where openid_md5="<<qwOpenidMD5;
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_info_get failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

    if(0 == m_mysql_helper.GetRowCount() || !m_mysql_helper.Next())
    {
    	LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_info_get failed, userinfo not exists for qwOpenidMD5="<<qwOpenidMD5);
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
        return;
    }

	user_info_db_2_pb(reply.m_user_detail_info, m_mysql_helper);
	return;
}

void ExecutorThread::process_power_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	uint64_t qwOpenidMD5 = request.m_openid_md5;
	uint64_t qwPAAppidMD5 = request.m_pa_appid_md5;
	std::string strTableName = db_table_user_appid_power_name;

	std::ostringstream oss;
	oss.str("");
	oss<<"select power from "<<strTableName<<" where openid_md5="<<qwOpenidMD5
			<<" and pa_appid_md5="<<qwPAAppidMD5;
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_power_get failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

    if(0 == m_mysql_helper.GetRowCount())
    {
    	LOG4CPLUS_ERROR(logger, "ExecutorThread::process_power_get end, power not exists for qwOpenidMD5="<<qwOpenidMD5<<", qwPAAppidMD5"<<qwPAAppidMD5);
    	reply.m_power_list.clear();
        return;
    }

    while(m_mysql_helper.Next())
    {
		reply.m_power_list.push_back(strtoul(m_mysql_helper.GetRow(0), NULL, 10));
    }

	return;
}

void ExecutorThread::process_zombies_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strUin = request.m_uin;
	std::string strTableName = db_table_user_zombie_name;

	std::ostringstream oss;
	oss.str("");
	oss<<"select openid_md5 from "<<strTableName<<" where uin='"<<strUin<<"'";
	//oss<<"select openid_md5 from "<<strTableName;
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_zombies_get failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

    if(0 == m_mysql_helper.GetRowCount())
    {
    	LOG4CPLUS_ERROR(logger, "ExecutorThread::process_zombies_get end, zombies not exists for uin="<<strUin);
    	reply.m_openid_md5_list.clear();
        return;
    }

    while(m_mysql_helper.Next())
    {
		reply.m_openid_md5_list.push_back(strtoul(m_mysql_helper.GetRow(0), NULL, 10));
    }

	return;
}

void ExecutorThread::process_zomanager_get(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	std::string strUin = request.m_uin;
	std::string strTableName = db_table_user_zombie_name;

	std::ostringstream oss;
	oss.str("");
	//oss<<"select openid_md5 from "<<strTableName<<" where uin='"<<strUin<<"'";
	oss<<"select distinct uin from "<<strTableName;
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_zomanager_get failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

    if(0 == m_mysql_helper.GetRowCount())
    {
    	LOG4CPLUS_ERROR(logger, "ExecutorThread::process_zomanager_get end, zomanager not exists");
    	reply.m_openid_md5_list.clear();
        return;
    }

    while(m_mysql_helper.Next())
    {
		reply.m_uin_list.push_back(m_mysql_helper.GetRow(0));
    }

	return;
}

void ExecutorThread::process_user_info_update(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	const ::hoosho::commstruct::UserDetailInfo& stUserDetailInfo = request.m_user_detail_info_from;
	//add to t_user_info
	uint64_t qwOpenidMD5 = 0;
	uint64_t qwAddrCountryMD5 = 0;
	uint64_t qwAddrProvinceMD5 = 0;
	uint64_t qwAddrCityMD5 = 0;
	OPENID_TO_MD5(stUserDetailInfo.openid(), qwOpenidMD5);
	OPENID_TO_MD5(stUserDetailInfo.addr_country(), qwAddrCountryMD5);
	OPENID_TO_MD5(stUserDetailInfo.addr_province(), qwAddrProvinceMD5);
	OPENID_TO_MD5(stUserDetailInfo.addr_city(), qwAddrCityMD5);
	
	std::string strTableName = "";
	UINT64_TO_TABLENAME(db_table_user_info_name_prefix, qwOpenidMD5, strTableName);

	std::ostringstream oss;
	oss.str("");
	oss<<"insert into "<<strTableName
		<<" set openid_md5="<<qwOpenidMD5
		<<", openid='"<<sql_escape(stUserDetailInfo.openid())<<"'"
		<<", portrait_pic_id="<<stUserDetailInfo.portrait_pic_id()
		<<", portrait_pic_url_wx='"<<sql_escape(stUserDetailInfo.portrait_pic_url_wx())<<"'"
		<<", nick='"<<sql_escape(stUserDetailInfo.nick())<<"'"
		<<", nick_wx='"<<sql_escape(stUserDetailInfo.nick_wx())<<"'"
		<<", addr_country='"<<sql_escape(stUserDetailInfo.addr_country())<<"'"
		<<", addr_province='"<<sql_escape(stUserDetailInfo.addr_province())<<"'"
		<<", addr_city='"<<sql_escape(stUserDetailInfo.addr_city())<<"'"
		<<", addr_country_md5="<<qwAddrCountryMD5
		<<", addr_province_md5="<<qwAddrProvinceMD5
		<<", addr_city_md5="<<qwAddrCityMD5
		<<", sex="<<stUserDetailInfo.sex()
		<<", phone_no="<<stUserDetailInfo.phone_no()
		<<", email='"<<sql_escape(stUserDetailInfo.email())<<"'"
		<<", self_desc='"<<sql_escape(stUserDetailInfo.self_desc())<<"'"
		<<", main_page_cover_pic_id="<<stUserDetailInfo.main_page_cover_pic_id()
		<<", hoosho_no="<<stUserDetailInfo.hoosho_no()
		<<", birthday_ts="<<stUserDetailInfo.birthday_ts()
		<<", create_ts=unix_timestamp()"
		<<", last_update_ts=unix_timestamp()"
		<<" on duplicate key update"
		<<" openid='"<<sql_escape(stUserDetailInfo.openid())<<"'"
		<<", portrait_pic_id="<<stUserDetailInfo.portrait_pic_id()
		<<", portrait_pic_url_wx='"<<sql_escape(stUserDetailInfo.portrait_pic_url_wx())<<"'"
		<<", nick='"<<sql_escape(stUserDetailInfo.nick())<<"'"
		<<", nick_wx='"<<sql_escape(stUserDetailInfo.nick_wx())<<"'"
		<<", addr_country='"<<sql_escape(stUserDetailInfo.addr_country())<<"'"
		<<", addr_province='"<<sql_escape(stUserDetailInfo.addr_province())<<"'"
		<<", addr_city='"<<sql_escape(stUserDetailInfo.addr_city())<<"'"
		<<", addr_country_md5="<<qwAddrCountryMD5
		<<", addr_province_md5="<<qwAddrProvinceMD5
		<<", addr_city_md5="<<qwAddrCityMD5
		<<", sex="<<stUserDetailInfo.sex()
		<<", phone_no="<<stUserDetailInfo.phone_no()
		<<", email='"<<sql_escape(stUserDetailInfo.email())<<"'"
		<<", self_desc='"<<sql_escape(stUserDetailInfo.self_desc())<<"'"
		<<", main_page_cover_pic_id="<<stUserDetailInfo.main_page_cover_pic_id()
		<<", hoosho_no="<<stUserDetailInfo.hoosho_no()
		<<", birthday_ts="<<stUserDetailInfo.birthday_ts();

	LOG4CPLUS_DEBUG(logger, "sql="<<oss.str());
	
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_info_update failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

	//add t_user_password
	UINT64_TO_TABLENAME(db_table_user_password_name_prefix, stUserDetailInfo.hoosho_no(), strTableName);
	oss.str("");
	oss<<"insert into "<<strTableName
		<<" set hoosho_no="<<stUserDetailInfo.hoosho_no()
		<<", openid_md5="<<qwOpenidMD5
		<<", create_ts=unix_timestamp()"
		<<" on duplicate key update"
		<<" openid_md5="<<qwOpenidMD5;
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_info_update failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

	return;
}

void ExecutorThread::process_user_info_get_batch(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	reply.m_user_detail_info_list.clear();
	
	for(size_t i=0; i<request.m_openid_md5_list.size(); ++i)
	{
		uint64_t qwOpenidMD5 = request.m_openid_md5_list[i];
		std::string strTableName = "";
		UINT64_TO_TABLENAME(db_table_user_info_name_prefix, qwOpenidMD5, strTableName);

		std::ostringstream oss;
		oss.str("");
		oss<<"select * from "<<strTableName<<" where openid_md5="<<qwOpenidMD5;
		
		if(!m_mysql_helper.Query(oss.str()))
	    {
	        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_info_get_batch failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
	        continue;
	    }

	    if(0 == m_mysql_helper.GetRowCount() || !m_mysql_helper.Next())
	    {
	    	LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_info_get_batch failed, userinfo not exists for qwOpenidMD5="<<qwOpenidMD5);
	    	continue;
	    }

		::hoosho::commstruct::UserDetailInfo stUserDetailInfo;
		user_info_db_2_pb(stUserDetailInfo, m_mysql_helper);
		reply.m_user_detail_info_list.push_back(stUserDetailInfo);
	}
	
	return;	
}

void ExecutorThread::process_user_fans_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	/*
		mysql> desc t_user_follow_ff;
+-----------------+---------------------+------+-----+---------+-------+
| Field           | Type                | Null | Key | Default | Extra |
+-----------------+---------------------+------+-----+---------+-------+
| openid_md5_from | bigint(20) unsigned | NO   | PRI | 0       |       |
| openid_md5_to   | bigint(20) unsigned | NO   | PRI | 0       |       |
| relation        | bigint(20) unsigned | NO   | PRI | 0       |       |
| create_ts       | bigint(20) unsigned | NO   |     | 0       |       |
+-----------------+---------------------+------+-----+---------+-------+
	*/
	uint64_t qwOpenidMD5 = request.m_user_detail_info_from.openid_md5();

	std::string strTableName = "";
	UINT64_TO_TABLENAME(db_table_user_follow_name_prefix, qwOpenidMD5, strTableName);

	std::ostringstream oss;
	oss.str("");
	oss<<"select openid_md5_to from "<<strTableName
			<<" where openid_md5_from="<<qwOpenidMD5
			<<" and relation=2";// 1: from follow to ; 2: to follow from
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_fans_list failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

    if(0 == m_mysql_helper.GetRowCount())
    {
    	LOG4CPLUS_DEBUG(logger, "ExecutorThread::process_user_fans_list, no fans found for qwOpenidMD5="<<qwOpenidMD5);
        return;
    }

	reply.m_openid_md5_list.clear();
    while(m_mysql_helper.Next())
    {
		reply.m_openid_md5_list.push_back(strtoul(m_mysql_helper.GetRow(0), NULL, 10));
    }

	return;
}

void ExecutorThread::process_user_fans_num(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	reply.m_map_openid_md5_2_fans_num.clear();
	
	for(size_t i=0; i<request.m_openid_md5_list.size(); ++i)
	{
		uint64_t qwOpenidMD5 = request.m_openid_md5_list[i];
		std::string strTableName = "";
		UINT64_TO_TABLENAME(db_table_user_follow_name_prefix, qwOpenidMD5, strTableName);

		std::ostringstream oss;
		oss.str("");
		oss<<"select count(openid_md5_to) from "<<strTableName
			<<" where openid_md5_from="<<qwOpenidMD5
			<<" and relation=2";
		if(!m_mysql_helper.Query(oss.str()))
	    {
	        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_fans_num failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
	        continue;
	    }

	    if(0 == m_mysql_helper.GetRowCount() || !m_mysql_helper.Next())
	    {
	    	LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_fans_num failed, count(openid_md5_to) select not succ!!!");
	    	continue;
	    }

	    uint32_t dwFansNum = atol(m_mysql_helper.GetRow(0));
		reply.m_map_openid_md5_2_fans_num.insert(std::make_pair(qwOpenidMD5, dwFansNum));
	}
	
	return;	
}

void ExecutorThread::process_user_follows_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	/*
		mysql> desc t_user_follow_ff;
+-----------------+---------------------+------+-----+---------+-------+
| Field           | Type                | Null | Key | Default | Extra |
+-----------------+---------------------+------+-----+---------+-------+
| openid_md5_from | bigint(20) unsigned | NO   | PRI | 0       |       |
| openid_md5_to   | bigint(20) unsigned | NO   | PRI | 0       |       |
| relation        | bigint(20) unsigned | NO   | PRI | 0       |       |
| create_ts       | bigint(20) unsigned | NO   |     | 0       |       |
+-----------------+---------------------+------+-----+---------+-------+
	*/
	uint64_t qwOpenidMD5 = request.m_user_detail_info_from.openid_md5();

	std::string strTableName = "";
	UINT64_TO_TABLENAME(db_table_user_follow_name_prefix, qwOpenidMD5, strTableName);

	std::ostringstream oss;
	oss.str("");
	oss<<"select openid_md5_to from "<<strTableName
			<<" where openid_md5_from="<<qwOpenidMD5
			<<" and relation=1";  // 1: from follow to ; 2: to follow from
	if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_user_follows_list failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
        return;
    }

    if(0 == m_mysql_helper.GetRowCount())
    {
    	LOG4CPLUS_DEBUG(logger, "ExecutorThread::process_user_follows_list, no follows found for qwOpenidMD5="<<qwOpenidMD5);
        return;
    }

	reply.m_openid_md5_list.clear();
    while(m_mysql_helper.Next())
    {
		reply.m_openid_md5_list.push_back(strtoul(m_mysql_helper.GetRow(0), NULL, 10));
    }

	return;
}



void ExecutorThread::process_follow(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	/*
		mysql> desc t_user_follow_ff;
+-----------------+---------------------+------+-----+---------+-------+
| Field           | Type                | Null | Key | Default | Extra |
+-----------------+---------------------+------+-----+---------+-------+
| openid_md5_from | bigint(20) unsigned | NO   | PRI | 0       |       |
| openid_md5_to   | bigint(20) unsigned | NO   | PRI | 0       |       |
| relation        | bigint(20) unsigned | NO   | PRI | 0       |       |
| create_ts       | bigint(20) unsigned | NO   |     | 0       |       |
+-----------------+---------------------+------+-----+---------+-------+
	*/
	uint64_t qwOpenidMD5From = request.m_user_detail_info_from.openid_md5();
	uint64_t qwOpenidMD5To = request.m_user_detail_info_to.openid_md5();
	uint32_t dwRelationFromTo = request.m_relation_from_to; //1:follow op, 2:cancel_follow op

	if(1 == dwRelationFromTo) 
	{
		// ADD From follow To
		{
			std::string strTableName = "";
			UINT64_TO_TABLENAME(db_table_user_follow_name_prefix, qwOpenidMD5From, strTableName);
			std::ostringstream oss;
			oss.str("");
			oss<<"insert into "<<strTableName
				<<" set openid_md5_from="<<qwOpenidMD5From
				<<", openid_md5_to="<<qwOpenidMD5To
				<<", relation=1"
				<<", create_ts=unix_timestamp()"
				<<"on duplicate key"
				<<" update create_ts=unix_timestamp()";
			if(!m_mysql_helper.Query(oss.str()))
		    {
		        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_follow failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		        return;
		    }
		}
		
		//ADD to is followed by From
		{
			std::string strTableName = "";
			UINT64_TO_TABLENAME(db_table_user_follow_name_prefix, qwOpenidMD5To, strTableName);
			std::ostringstream oss;
			oss.str("");
			oss<<"insert into "<<strTableName
				<<" set openid_md5_from="<<qwOpenidMD5To
				<<", openid_md5_to="<<qwOpenidMD5From
				<<", relation=2"
				<<", create_ts=unix_timestamp()"
				<<"on duplicate key"
				<<" update create_ts=unix_timestamp()";
			if(!m_mysql_helper.Query(oss.str()))
		    {
		        LOG4CPLUS_ERROR(logger, "ExecutorThread::process_follow failed, sql="<<oss.str()<<", msg="<<m_mysql_helper.GetErrMsg());
		        reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		        return;
		    }
		}
	}
	else if(2 == dwRelationFromTo)
	{
		// DELETE From follow To
		{
			std::string strTableName = "";
			UINT64_TO_TABLENAME(db_table_user_follow_name_prefix, qwOpenidMD5From, strTableName);
			std::ostringstream oss;
			oss.str("");
			oss<<"delete from "<<strTableName
				<<" where openid_md5_from="<<qwOpenidMD5From
				<<" and openid_md5_to="<<qwOpenidMD5To
				<<" and relation=1";
			m_mysql_helper.Query(oss.str());
		}
		
		//ADD to is followed by From
		{
			std::string strTableName = "";
			UINT64_TO_TABLENAME(db_table_user_follow_name_prefix, qwOpenidMD5To, strTableName);
			std::ostringstream oss;
			oss.str("");
			oss<<"delete from "<<strTableName
				<<" where openid_md5_from="<<qwOpenidMD5To
				<<" and openid_md5_to="<<qwOpenidMD5From
				<<" and relation=2";
			m_mysql_helper.Query(oss.str());
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "ExecutorThread::process_follow failed, unkown relationFromTo="<<dwRelationFromTo);
        reply.m_result_code = ExecutorThreadRequestType::E_FUCK;
        return;
	}
	
	return;
}



