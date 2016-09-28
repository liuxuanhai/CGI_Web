#include "executor_thread_feed.h"
#include "global_var.h"
#include "wx_api_pay.h"
#include "common_util.h"
#include "util/lce_util.h"
#include "dao.h"
#include "wx_api_message.h"
#include "wx_api_download_resource.h"
#include "wx_api_parse_ipaddr.h"

IMPL_LOGGER(ExecutorThreadFeed, logger)

ExecutorThreadFeed::ExecutorThreadFeed()
{
    m_queue = NULL;
}

ExecutorThreadFeed::~ExecutorThreadFeed()
{
}

int ExecutorThreadFeed::init(ExecutorThreadQueue * queue)
{
    m_queue = queue;

	const lce::app::Config& stConfig = g_server->config();

	m_db_ip = stConfig.get_string_param("DB", "ip"); 
	m_db_user = stConfig.get_string_param("DB", "user"); 
	m_db_passwd = stConfig.get_string_param("DB", "passwd"); 
	m_db_name = stConfig.get_string_param("DB", "db_name"); 
	m_db_table_user_info_name = stConfig.get_string_param("DB", "table_user_info_name");

	m_pa_appid = stConfig.get_string_param("PA", "appid");  	

	m_user_upload_ticket_limit = stConfig.get_int_param("TICKET", "user_upload_ticket_limit");

    assert(m_mysql_helper.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd));	

    return 0;
}


void ExecutorThreadFeed::run()
{
    LOG4CPLUS_TRACE(logger, "ExecutorThreadFeed "<<pthread_self()<<" running ...");

    while(true)
    {
        if(m_queue->request_empty())
        {
            usleep(10);
            continue;
        }

        ExecutorThreadRequestElement request = m_queue->get_request();
        m_queue->pop_request();
		
        LOG4CPLUS_TRACE(logger, "ExecutorThreadFeed get request "<<request.ToString());
        ExecutorThreadResponseElement reply(request.m_request_type, request.m_fsm_type, request.m_fsm_id, request.m_self_user_id);

        switch(request.m_request_type)
        {        	
        	case ExecutorThreadRequestType::T_GET_GOOD_TYPE:
        		process_get_good_type(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_GET_GOOD_INFO:
        		process_get_good_info(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_ADD_TICKET:
        		process_add_ticket(request, reply);
        		break;        	
			case ExecutorThreadRequestType::T_GET_USER_FEED_LIST:
        		process_get_user_feed_list(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_GET_FEED_LIST:
        		process_get_feed_list(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_GET_USER_SHOW_LIST:
        		process_get_user_show_list(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_GET_SHOW_LIST:
        		process_get_show_list(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_GET_SHOW_DETAIL:
        		process_get_show_detail(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_ADD_SHOW:
        		process_add_show(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_GET_USER_COLLECT_LIST:
        		process_get_user_collect_list(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_ADD_COLLECT:
        		process_add_collect(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_GET_USER_FEED_CONTEND_ID_LIST:
        		process_get_user_feed_contend_id_list(request, reply);
        		break;     	
        	case ExecutorThreadRequestType::T_GET_BANNER_LIST:
        		process_get_banner_list(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_DEL_COLLECT:
        		process_del_collect(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_CHECK_USER_COLLECT:
        		process_check_user_collect(request, reply);
        		break;
        	case ExecutorThreadRequestType::T_GET_USER_LUCK_LIST:
        		process_get_user_luck_list(request, reply);
        		break;
			case ExecutorThreadRequestType::T_GET_FEED_DETATIL:
        		process_get_feed_detail(request, reply);
        		break; 
			case ExecutorThreadRequestType::T_GET_FEED_JOIN_USER_LIST:
				process_get_feed_join_user_list(request, reply);
				break;
			case ExecutorThreadRequestType::T_GET_USER_TICKET_LIST:
				process_get_user_ticket_list(request, reply);
				break;
        	
			
			default:
				LOG4CPLUS_ERROR(logger, "ExecutorThreadFeed fail, request_type: "<<request.m_request_type);
				reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
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

void ExecutorThreadFeed::process_get_good_type(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoGoodType stDaoGoodType;
	reply.m_good_type_list.clear();
	if(DAO_RET_OK != stDaoGoodType.GetGoodTypeList(reply.m_good_type_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_get_good_info(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoGoodInfo stDaoGoodInfo;
	reply.m_good_info_list.clear();
	if(DAO_RET_OK != stDaoGoodInfo.GetGoodInfoList( reply.m_good_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_add_ticket(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{	
	//1.check if available_num = total_join_num - approved_num - pending_num > 0 ?
	DaoFeedInfo stDaoFeedInfo;
	uint64_t qwAvailableNum = 0;
	if(DAO_RET_OK != stDaoFeedInfo.GetAvailableUploadNumByFeedId(request.m_ticket_info.feed_id(), qwAvailableNum, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	if(!qwAvailableNum)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_UPLOAD_TICKET_LIMIT;
		return;
	}

	//2.check already upload ticket time on same feed_id
	DaoTicketInfo stDaoTicketInfo;
	uint32_t iCount;
	if(DAO_RET_OK != stDaoTicketInfo.GetUserUploadTimeByFeedId(request.m_self_user_id, request.m_ticket_info.feed_id(), iCount, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	if(iCount >= m_user_upload_ticket_limit)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_UPLOAD_TICKET_LIMIT;
		return;
	}
	
	
	//3.down pic from wx, insert into t_pic_info
	std::string strPics = request.m_ticket_info.ticket_pics();	
	DaoPicInfo stDaoPicInfo(m_pa_appid);
	if(DAO_RET_OK != stDaoPicInfo.DownLoadAddPics(strPics, request.m_self_user_id, PIC_TYPE_PRIVATE, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
		return;
	}		

	//4.insert into t_ticket_info
	uint64_t qwTicketid = ::common::util::generate_unique_id();
	request.m_ticket_info.set_ticket_id(qwTicketid);
	request.m_ticket_info.set_user_id(request.m_self_user_id);
	request.m_ticket_info.set_create_ts(timems());

	::common::wxapi::WXAPIParseIPAddr stWXAPIParseIPAddr;
	std::string strIPCity = "";
	stWXAPIParseIPAddr.ParseIPAddr(request.m_ticket_info.user_ip(), strIPCity);
	request.m_ticket_info.set_user_ip(strIPCity + " IP: " + request.m_ticket_info.user_ip());
	request.m_ticket_info.set_ticket_pics(strPics);
	
	if(DAO_RET_OK != stDaoTicketInfo.AddTicket(request.m_ticket_info, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	reply.m_ticket_id = qwTicketid;

	return;
}

void ExecutorThreadFeed::process_get_user_feed_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	uint64_t qwUserid = request.m_self_user_id;
	if(0 != request.m_user_id)
	{
		qwUserid = request.m_user_id;
	}

	//1.Get feed_info_list
	DaoTicketInfo stDaoTicketInfo;
	std::set<uint64_t> qwFeedIdList;
	if(DAO_RET_OK != stDaoTicketInfo.GetUserFeedIdList(qwUserid, request.m_offset, request.m_limit, qwFeedIdList, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	
	DaoFeedInfo stDaoFeedInfo;
	reply.m_feed_info_list.clear();
	if(DAO_RET_OK != stDaoFeedInfo.GetUserFeedInfoList(qwFeedIdList, reply.m_feed_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2.Get good_info_list
	std::set<uint64_t> qwGoodidList;
	for(size_t i = 0; i < reply.m_feed_info_list.size(); i++)
	{
		qwGoodidList.insert(reply.m_feed_info_list[i].good_id());
	}

	DaoGoodInfo stDaoGoodInfo;
	reply.m_good_info_list.clear();
	if(DAO_RET_OK != stDaoGoodInfo.GetGoodInfoListByGoodid(qwGoodidList, reply.m_good_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_get_feed_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.Get feed_info_list
	DaoFeedInfo stDaoFeedInfo;
	reply.m_feed_info_list.clear();
	if(DAO_RET_OK != stDaoFeedInfo.GetFeedInfoListByGoodId(request.m_good_id, request.m_begin_feed_id, request.m_limit, reply.m_feed_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2.Get good_info_list
	std::set<uint64_t> qwGoodidList;	
	for(size_t i = 0; i < reply.m_feed_info_list.size(); i++)
	{
		qwGoodidList.insert(reply.m_feed_info_list[i].good_id());	
	}

	DaoGoodInfo stDaoGoodInfo;
	reply.m_good_info_list.clear();
	if(DAO_RET_OK != stDaoGoodInfo.GetGoodInfoListByGoodid(qwGoodidList, reply.m_good_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_get_user_show_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoShowInfo stDaoShowInfo;
	reply.m_show_info_list.clear();
	if(DAO_RET_OK != stDaoShowInfo.GetShowInfoList(request.m_user_id, request.m_begin_show_id, request.m_limit, reply.m_show_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_get_show_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoShowInfo stDaoShowInfo;
	reply.m_show_info_list.clear();
	if(DAO_RET_OK != stDaoShowInfo.GetShowInfoList(0, request.m_begin_show_id, request.m_limit, reply.m_show_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_get_show_detail(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.Get show_info
	DaoShowInfo stDaoShowInfo;
	if(DAO_RET_OK != stDaoShowInfo.GetSHowDetailInfoByShowid(request.m_show_id, reply.m_show_info, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2.Get feed_info
	DaoFeedInfo stDaoFeedInfo;
	if(DAO_RET_OK != stDaoFeedInfo.GetFeedInfoByFeedid(reply.m_show_info.feed_id(), reply.m_feed_info, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//3.Get user_info
	DaoUserInfo stDaoUserInfo;
	if(DAO_RET_OK != stDaoUserInfo.GetUserInfoByPrimaryKey(reply.m_show_info.user_id(), m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	
	reply.m_user_info.CopyFrom(stDaoUserInfo.m_user_info); 	
	return;
}


void ExecutorThreadFeed::process_add_show(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.before add, check show_status in t_feed_info
	DaoFeedInfo stDaoFeedInfo;
	uint32_t iRet = stDaoFeedInfo.CheckShowStatus(request.m_show_info.feed_id(), m_mysql_helper);
	if(DAO_RET_FAIL == iRet)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}
	else if(DAO_RET_NOT_EXIST == iRet)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_PARAM_ERR;
		return;
	}
	
	if(DAO_RET_UNSHOWED != iRet)
	{
		reply.m_result_code = ExecutorThreadRequestType::E_SHOW_REPEATED;
		return;
	}

	//2.down pic from wx, insert into t_pic_info
	std::string strPics = request.m_show_info.show_pics();
	DaoPicInfo stDaoPicInfo(m_pa_appid);
	if(DAO_RET_OK != stDaoPicInfo.DownLoadAddPics(strPics, request.m_self_user_id, PIC_TYPE_PUBLIC, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_INNER_ERR;
		return;
	}	

	//3.insert into t_show_info
	request.m_show_info.set_user_id(request.m_self_user_id);
	request.m_show_info.set_create_ts(timems());
	request.m_show_info.set_show_pics(strPics);

	DaoShowInfo stDaoShowInfo;
	if(DAO_RET_OK != stDaoShowInfo.AddShowInfo(request.m_show_info, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	reply.m_show_id = request.m_show_info.show_id();

	return;
}

void ExecutorThreadFeed::process_get_user_collect_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoCollectInfo stDaoCollectInfo;
	if(DAO_RET_OK != stDaoCollectInfo.GetCollectInfoList(request.m_self_user_id, request.m_begin_ts, request.m_limit, reply.m_collect_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	std::set<uint64_t> qwFeedIdList;
	for(size_t i = 0; i < reply.m_collect_info_list.size(); i++)
	{
		qwFeedIdList.insert(reply.m_collect_info_list[i].feed_id());
	}

	DaoFeedInfo stDaoFeedInfo;
	if(DAO_RET_OK != stDaoFeedInfo.GetUserFeedInfoList(qwFeedIdList, reply.m_feed_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2.Get good_info_list
	std::set<uint64_t> qwGoodidList;
	for(size_t i = 0; i < reply.m_feed_info_list.size(); i++)
	{
		qwGoodidList.insert(reply.m_feed_info_list[i].good_id());
	}

	DaoGoodInfo stDaoGoodInfo;
	reply.m_good_info_list.clear();
	if(DAO_RET_OK != stDaoGoodInfo.GetGoodInfoListByGoodid(qwGoodidList, reply.m_good_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_add_collect(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoCollectInfo stDaoCollectInfo;
	request.m_collect_info.set_user_id(request.m_self_user_id);
	request.m_collect_info.set_create_ts(timems());

	if(DAO_RET_OK != stDaoCollectInfo.AddCollectInfo(request.m_collect_info, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_get_feed_join_user_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.Get user_id_list
	DaoTicketInfo stDaoTicketInfo;
	reply.m_contend_info_list.clear();
	if(DAO_RET_OK != stDaoTicketInfo.GetContendListByFeedid(request.m_feed_id, request.m_begin_ticket_id, request.m_limit, reply.m_contend_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2.Get user_info_list
	std::set<uint64_t> qwUseridList;
	for(size_t i = 0; i < reply.m_contend_info_list.size(); i++)
	{
		qwUseridList.insert(reply.m_contend_info_list[i].user_id());
	}

	DaoUserInfo stDaoUserInfo;
	reply.m_user_info_list.clear();
	if(DAO_RET_OK != stDaoUserInfo.GetUserInfoList(qwUseridList, reply.m_user_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_get_user_feed_contend_id_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	uint64_t qwSearchUserid = request.m_user_id;
	if(0 == qwSearchUserid)
		qwSearchUserid = request.m_self_user_id;

	DaoTicketInfo stDaoTicketInfo;
	reply.m_feed_contend_info_list.clear();

	std::set<uint64_t>::iterator it = request.m_feed_id_list.begin();
	std::set<uint64_t> qwContendidList;
	for(;  it != request.m_feed_id_list.end(); it++)
	{
		uint64_t qwFeedid = *it;
		qwContendidList.clear();
		if(DAO_RET_OK != stDaoTicketInfo.GetUserContendIdListByFeedid(qwSearchUserid, qwFeedid, qwContendidList, m_mysql_helper))
		{
			continue;
		}

		::hoosho::msg::s::FeedContendInfo stFeedContendInfo;
		stFeedContendInfo.set_feed_id(qwFeedid);
		for(std::set<uint64_t>::iterator it2 = qwContendidList.begin(); it2 != qwContendidList.end(); it2++)
		{
			stFeedContendInfo.add_contend_id_list(*it2);
		}

		reply.m_feed_contend_info_list.push_back(stFeedContendInfo);
	}

	return; 
}

void ExecutorThreadFeed::process_get_banner_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoBannerInfo stDaoBannerInfo;
	if(DAO_RET_OK != stDaoBannerInfo.GetBannerInfoList(request.m_begin_banner_id, request.m_limit, reply.m_banner_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_del_collect(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoCollectInfo stDaoCollectInfo;
	if(DAO_RET_OK != stDaoCollectInfo.DelCollect(request.m_self_user_id, request.m_feed_id, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_check_user_collect(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	DaoCollectInfo stDaoCollectInfo;
	if(DAO_RET_OK != stDaoCollectInfo.CheckUserCollect(request.m_self_user_id, request.m_feed_id_list, reply.m_user_collect_flag_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_get_user_luck_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.Get feed_info_list
	uint64_t qwSearchUserid = request.m_user_id;
	if(0 == qwSearchUserid)
		qwSearchUserid = request.m_self_user_id;
	
	DaoFeedInfo stDaoFeedInfo;
	reply.m_feed_info_list.clear();
	if(DAO_RET_OK != stDaoFeedInfo.GetUserLuckFeedList(qwSearchUserid, request.m_begin_feed_id, request.m_limit, reply.m_feed_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2.Get good_info_list
	std::set<uint64_t> qwGoodidList;
	for(size_t i = 0; i < reply.m_feed_info_list.size(); i++)
	{
		qwGoodidList.insert(reply.m_feed_info_list[i].good_id());
	}

	DaoGoodInfo stDaoGoodInfo;
	reply.m_good_info_list.clear();
	if(DAO_RET_OK != stDaoGoodInfo.GetGoodInfoListByGoodid(qwGoodidList, reply.m_good_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

void ExecutorThreadFeed::process_get_feed_detail(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.Get feed_info_list
	DaoFeedInfo stDaoFeedInfo;
	reply.m_feed_info_list.clear();
	std::set<uint64_t>::iterator it = request.m_feed_id_list.begin();
	for(; it != request.m_feed_id_list.end(); it++)
	{
		uint64_t qwFeedid = *it;
		::hoosho::msg::s::FeedInfo stFeedInfo;
		if(DAO_RET_OK != stDaoFeedInfo.GetFeedInfoByFeedid(qwFeedid, stFeedInfo,m_mysql_helper))
		{
			reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
			return;
		}

		reply.m_feed_info_list.push_back(stFeedInfo);
	}

	//2.Get good_info_list
	std::set<uint64_t> qwGoodidList;
	for(size_t i = 0; i < reply.m_feed_info_list.size(); i++)
	{
		qwGoodidList.insert(reply.m_feed_info_list[i].good_id());
	}

	DaoGoodInfo stDaoGoodInfo;
	reply.m_good_info_list.clear();
	if(DAO_RET_OK != stDaoGoodInfo.GetGoodInfoListByGoodid(qwGoodidList, reply.m_good_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;	
}

void ExecutorThreadFeed::process_get_user_ticket_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
	//1.Get ticket_info_list
	DaoTicketInfo stDaoTicketInfo;	
	if(DAO_RET_OK != stDaoTicketInfo.GetTicketInfoListByUserid(request.m_self_user_id, request.m_begin_ticket_id, request.m_limit, reply.m_ticket_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//2.Get feed_info_list
	std::set<uint64_t> qwFeedIdList;
	for(size_t i = 0; i < reply.m_ticket_info_list.size(); i++)
	{
		qwFeedIdList.insert(reply.m_ticket_info_list[i].feed_id());
	}

	DaoFeedInfo stDaoFeedInfo;
	if(DAO_RET_OK != stDaoFeedInfo.GetUserFeedInfoList(qwFeedIdList, reply.m_feed_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	//3.Get good_info_list
	std::set<uint64_t> qwGoodidList;
	for(size_t i = 0; i < reply.m_feed_info_list.size(); i++)
	{
		qwGoodidList.insert(reply.m_feed_info_list[i].good_id());
	}

	DaoGoodInfo stDaoGoodInfo;
	reply.m_good_info_list.clear();
	if(DAO_RET_OK != stDaoGoodInfo.GetGoodInfoListByGoodid(qwGoodidList, reply.m_good_info_list, m_mysql_helper))
	{
		reply.m_result_code = ExecutorThreadRequestType::E_DB_ERR;
		return;
	}

	return;
}

