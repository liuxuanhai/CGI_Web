#include <cgi/cgi_any_value.h>
#include <util/str_op.h>
#include "executor_thread.h"
#include "global_var.h"
#include "wx_api_message.h"
#include "jsoncpp/json.h"

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
    assert(m_mysql_helper.Init(stConfig.get_string_param("DB", "ip")
                               , stConfig.get_string_param("DB", "db_name")
                               , stConfig.get_string_param("DB", "user")
                               , stConfig.get_string_param("DB", "passwd")
                               , stConfig.get_int_param("DB", "port")));

	assert(m_ano_mysql_helper.Init(stConfig.get_string_param("DB", "ip")
                               , stConfig.get_string_param("DB", "db_name")
                               , stConfig.get_string_param("DB", "user")
                               , stConfig.get_string_param("DB", "passwd")
                               , stConfig.get_int_param("DB", "port")));

    m_table_name_feed = stConfig.get_string_param("DB", "table_name_feed");
    m_table_name_follow = stConfig.get_string_param("DB", "table_name_follow");
    m_table_name_listen = stConfig.get_string_param("DB", "table_name_listen");
    m_table_name_history = stConfig.get_string_param("DB", "table_name_history");
    m_table_name_user = stConfig.get_string_param("DB", "table_name_user");

    m_pa_appid = stConfig.get_string_param("PA_INFO", "appid");

    m_new_follow_template_id = stConfig.get_string_param("TEMPLATE_NEW_FOLLOW", "template_id");
    m_new_follow_color = stConfig.get_string_param("TEMPLATE_NEW_FOLLOW", "color");
    m_new_follow_url = stConfig.get_string_param("TEMPLATE_NEW_FOLLOW", "url");
    m_new_follow_first = stConfig.get_string_param("TEMPLATE_NEW_FOLLOW", "first");
    m_new_follow_keyword1 = stConfig.get_string_param("TEMPLATE_NEW_FOLLOW", "keyword1");
    m_new_follow_keyword2 = stConfig.get_string_param("TEMPLATE_NEW_FOLLOW", "keyword2");
    m_new_follow_keyword3 = stConfig.get_string_param("TEMPLATE_NEW_FOLLOW", "keyword3");
    m_new_follow_remark = stConfig.get_string_param("TEMPLATE_NEW_FOLLOW", "remark");

    m_listen_follow_template_id = stConfig.get_string_param("TEMPLATE_LISTEN_FOLLOW", "template_id");
    m_listen_follow_color = stConfig.get_string_param("TEMPLATE_LISTEN_FOLLOW", "color");
    m_listen_follow_url = stConfig.get_string_param("TEMPLATE_LISTEN_FOLLOW", "url");
    m_listen_follow_first = stConfig.get_string_param("TEMPLATE_LISTEN_FOLLOW", "first");
    m_listen_follow_keyword1 = stConfig.get_string_param("TEMPLATE_LISTEN_FOLLOW", "keyword1");
    m_listen_follow_keyword2 = stConfig.get_string_param("TEMPLATE_LISTEN_FOLLOW", "keyword2");
    m_listen_follow_keyword3 = stConfig.get_string_param("TEMPLATE_LISTEN_FOLLOW", "keyword3");
    m_listen_follow_remark = stConfig.get_string_param("TEMPLATE_LISTEN_FOLLOW", "remark");

    m_listen_price = stConfig.get_int_param("ORDER", "listen_price");
    m_share_percent_default = stConfig.get_int_param("ORDER", "share_percent_default");
    m_share_percent_good = stConfig.get_int_param("ORDER", "share_percent_good");
    m_share_percent_bad = stConfig.get_int_param("ORDER", "share_percent_bad");

    m_auto_comment_time = stConfig.get_int_param("FEEDS", "auto_comment_time") ;

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
        ExecutorThreadResponseElement reply(request.m_request_type, request.m_fsm_id);

        switch(request.m_request_type)
        {
            case ExecutorThreadRequestType::T_CHECK_OUT_TRADE_NO_UNIQUE:
                process_check_out_trade_no_unique(request, reply);
                break;

            case ExecutorThreadRequestType::T_GET_FEED_LIST:
                process_get_feed_list(request, reply);
                break;

            case ExecutorThreadRequestType::T_GET_FEED_DETAIL:
                process_get_feed_detail(request, reply);
                break;

            case ExecutorThreadRequestType::T_ADD_FEED:
                process_add_feed(request, reply);
                break;

            case ExecutorThreadRequestType::T_GET_FOLLOW_LIST:
                process_get_follow_list(request, reply);
                break;

            case ExecutorThreadRequestType::T_GET_FOLLOW_DETAIL:
                process_get_follow_detail(request, reply);
                break;

            case ExecutorThreadRequestType::T_ADD_FOLLOW:
                process_add_follow(request, reply);
                break;

            case ExecutorThreadRequestType::T_GET_LISTEN_LIST:
                process_get_listen_list(request, reply);
                break;

            case ExecutorThreadRequestType::T_GET_LISTEN_DETAIL:
                process_get_listen_detail(request, reply);
                break;

            case ExecutorThreadRequestType::T_ADD_LISTEN:
                process_add_listen(request, reply);
                break;

            case ExecutorThreadRequestType::T_COMMENT_FOLLOW:
                process_comment_follow(request, reply);
                break;

            case ExecutorThreadRequestType::T_GET_HISTORY_LIST:
                process_get_history_list(request, reply);
                break;

            case ExecutorThreadRequestType::T_CHECK_LISTEN:
                process_check_listen(request, reply);
                break;

            case ExecutorThreadRequestType::T_GET_COMMNET_FOLLOW:
                process_get_comment_follow(request, reply);
                break;

			case ExecutorThreadRequestType::T_SET_OUT_TRADE_NO:
				process_set_out_trade_no(request, reply);
				break;

			case ExecutorThreadRequestType::T_CHECK_BUSINESS_ID_VALID:
				process_check_business_id_valid(request, reply);
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

static void feed_info_db_2_pb(::lce::cgi::CMysql& mysql, ::hoosho::j::commstruct::FeedInfo& stFeedInfo)
{
    stFeedInfo.set_feed_id(strtoul(mysql.GetRow(0), NULL, 10));
    stFeedInfo.set_openid(mysql.GetRow(1));
    stFeedInfo.set_content(mysql.GetRow(2));
    stFeedInfo.set_ts(strtoul(mysql.GetRow(3), NULL, 10));
    stFeedInfo.set_num_follow(strtoul(mysql.GetRow(4), NULL, 10));
    stFeedInfo.set_num_listen(strtoul(mysql.GetRow(5), NULL, 10));
    stFeedInfo.set_need_sex(strtoul(mysql.GetRow(6), NULL, 10));
}

static void follow_info_db_2_pb(::lce::cgi::CMysql& mysql, ::hoosho::j::commstruct::FollowInfo& stFollowInfo)
{
    stFollowInfo.set_follow_id(strtoul(mysql.GetRow(0), NULL, 10));
    stFollowInfo.set_feed_id(strtoul(mysql.GetRow(1), NULL, 10));
    stFollowInfo.set_openid(mysql.GetRow(2));
    stFollowInfo.set_content_type(strtoul(mysql.GetRow(3), NULL, 10));
    stFollowInfo.set_content_id(mysql.GetRow(4));
    stFollowInfo.set_content_duration(strtoul(mysql.GetRow(5), NULL, 10));
    stFollowInfo.set_ts(strtoul(mysql.GetRow(6), NULL, 10));
    stFollowInfo.set_num_listen(strtoul(mysql.GetRow(7), NULL, 10));
    stFollowInfo.set_num_comment_good(strtoul(mysql.GetRow(8), NULL, 10));
    stFollowInfo.set_num_comment_default(strtoul(mysql.GetRow(9), NULL, 10));
    stFollowInfo.set_num_comment_bad(strtoul(mysql.GetRow(10), NULL, 10));
    stFollowInfo.set_out_trade_no(mysql.GetRow(11));
}

static void listen_info_db_2_pb(::lce::cgi::CMysql& mysql, ::hoosho::j::commstruct::ListenInfo& stListenInfo)
{
    stListenInfo.set_listen_id(strtoul(mysql.GetRow(0), NULL, 10));
    stListenInfo.set_openid(mysql.GetRow(1));
    stListenInfo.set_follow_id(strtoul(mysql.GetRow(2), NULL, 10));
    stListenInfo.set_feed_id(strtoul(mysql.GetRow(3), NULL, 10));
    stListenInfo.set_ts(strtoul(mysql.GetRow(4), NULL, 10));
    stListenInfo.set_comment_type(strtoul(mysql.GetRow(5), NULL, 10));
    stListenInfo.set_out_trade_no(mysql.GetRow(6));
}

static int set_default_comment_and_get_follow_user(::lce::cgi::CMysql& mysql, const uint64_t& qwListenId, const uint64_t& qwFollowId,
												   string& strOpenid, const string& strTableListen, const string& strTableFollow)
{
	ostringstream oss;

	//set
	oss.str("");
	oss << "UPDATE " << strTableListen << " SET comment_type = " << ::hoosho::j::commenum::COMMENT_TYPE_DEFAULT
	<< " WHERE listen_id = " << qwListenId
	<< " AND comment_type = " << ::hoosho::j::commenum::COMMENT_TYPE_NULL;
	if(!mysql.Query(oss.str()))
	{
        return -1;
	}
	if(!mysql.GetAffectedRows())
	{
		return -1;
	}

	//get
	oss.str("");
	oss << "SELECT openid FROM " << strTableFollow << " WHERE follow_id = " << qwFollowId;
	if(!mysql.Query(oss.str()))
	{
		return -1;
	}
	if(!mysql.GetAffectedRows())
	{
		return -1;
	}
    if(mysql.GetRowCount() && mysql.Next())
    {
        strOpenid = mysql.GetRow(0);
    }

	return 0;
}

int ExecutorThread::send_template_message_of_new_follow(const uint64_t& qwFollowId, const uint64_t& qwFeedId)
{
    //获得 提问者、提问内容、回答者、回答时间
    std::string strFeedOpenid;
    std::string strFollowOpenid;
    std::string strContent;
    uint64_t qwTs;

    ostringstream oss;
    oss.str("");
    oss << "SELECT openid, content FROM " << m_table_name_feed << " WHERE feed_id = " << qwFeedId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
        strFeedOpenid = m_mysql_helper.GetRow(0);
        strContent = m_mysql_helper.GetRow(1);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "not data found, sql = " << oss.str());
        return -1;
    }

    oss.str("");
    oss << "SELECT openid, ts FROM " << m_table_name_follow << " WHERE follow_id = " << qwFollowId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
        strFollowOpenid = m_mysql_helper.GetRow(0);
        qwTs = strtoul(m_mysql_helper.GetRow(1), NULL, 10);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "not data found, sql = " << oss.str());
        return -1;
    }

    std::string strFollowName;
    //openid -> name
    oss.str("");
    oss << "SELECT nickname FROM " << m_table_name_user << " WHERE openid = '" << sql_escape(strFollowOpenid) << "'";
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
        strFollowName = m_mysql_helper.GetRow(0);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "not data found, sql = " << oss.str());
        return -1;
    }

    //8 hour
    long dwTs = qwTs + 60 * 60 * 8;
    struct tm *p;
    p = gmtime(&dwTs);
    char bufTs[80];
    strftime(bufTs, 80, "%Y年%m月%d日 %H:%M", p);
    LOG4CPLUS_DEBUG(logger, "ts_str = " << bufTs);

    std::string strUrl = m_new_follow_url;
    strUrl.replace(strUrl.find("FEEDID"), strlen("FEEDID"), lce::util::StringOP::TypeToStr(qwFeedId));

    Json::Value any;
    any["touser"] = strFeedOpenid;
    any["template_id"] = m_new_follow_template_id;
    any["url"] = strUrl;
    any["data"]["first"]["value"] = m_new_follow_first;
    any["data"]["first"]["color"] = m_new_follow_color;
    any["data"][m_new_follow_keyword1]["value"] = strContent;
    any["data"][m_new_follow_keyword1]["color"] = m_new_follow_color;
    any["data"][m_new_follow_keyword2]["value"] = strFollowName;
    any["data"][m_new_follow_keyword2]["color"] = m_new_follow_color;
    any["data"][m_new_follow_keyword3]["value"] = bufTs;
    any["data"][m_new_follow_keyword3]["color"] = m_new_follow_color;
    any["data"]["remark"]["value"] = m_new_follow_remark;
    any["data"]["remark"]["color"] = m_new_follow_color;

    std::string strPostData = any.toJsonString();
    LOG4CPLUS_DEBUG(logger, "postdata = " << strPostData);

    //调用微信api
    common::wxapi::WXAPIMessage wxapiMessage;
    std::string strBaseAccessToken;
    std::string strJSAPITicket;
    g_server_processor_token->get_pa_info(m_pa_appid, strBaseAccessToken, strJSAPITicket);

    int iRet = 0;

    if(wxapiMessage.SendTemplateMessage(strBaseAccessToken, strPostData, iRet) < 0)
    {
        LOG4CPLUS_ERROR(logger, "WXAPI.SendTemplateMessage failed, errcode = " << iRet);
        return -1;
    }
    return 0;
}

int ExecutorThread::send_template_message_of_listen_follow(const uint64_t& qwListenId)
{
    //获得 回答者、提问内容、评价内容、评价时间
    std::string strFollowOpenid;
    std::string strContent;
    uint64_t qwCommentType;
    uint64_t qwTs;

    uint64_t qwFeedId;
    uint64_t qwFollowId;

    ostringstream oss;
    oss.str("");
    oss << "SELECT comment_type, ts, feed_id, follow_id FROM " << m_table_name_listen << " WHERE listen_id = " << qwListenId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
        qwCommentType = strtoul(m_mysql_helper.GetRow(0), NULL, 10);
        qwTs = strtoul(m_mysql_helper.GetRow(1), NULL, 10);
        qwFeedId = strtoul(m_mysql_helper.GetRow(2), NULL, 10);
        qwFollowId = strtoul(m_mysql_helper.GetRow(3), NULL, 10);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "not data found, sql = " << oss.str());
        return -1;
    }

    //差评，不分钱给回答者，不通知
    if(qwCommentType == ::hoosho::j::commenum::COMMENT_TYPE_NULL)
    {
        LOG4CPLUS_ERROR(logger, "empty comment type, listen_id = " << qwListenId);
        return -1;
    }
    if(qwCommentType == ::hoosho::j::commenum::COMMENT_TYPE_BAD)
    {
        LOG4CPLUS_DEBUG(logger, "listen_id = " << qwListenId << " comment_type = bad, not need to notify");
        return -1;
    }
    double dbAmount = 0;

    if(qwCommentType == ::hoosho::j::commenum::COMMENT_TYPE_GOOD)
    {
        dbAmount = m_listen_price * m_share_percent_good / 100.0;
    }
    else if(qwCommentType == ::hoosho::j::commenum::COMMENT_TYPE_DEFAULT)
    {
        dbAmount = m_listen_price * m_share_percent_default / 100.0;
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "invalid comment type = " << qwCommentType);
        return -1;
    }
    std::string strAmount;
    strAmount = lce::util::StringOP::TypeToStr(dbAmount / 100) + "元";

    oss.str("");
    oss << "SELECT content FROM " << m_table_name_feed << " WHERE feed_id = " << qwFeedId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
        strContent = m_mysql_helper.GetRow(0);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "not data found, sql = " << oss.str());
        return -1;
    }

    oss.str("");
    oss << "SELECT openid FROM " << m_table_name_follow << " WHERE follow_id = " << qwFollowId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
        strFollowOpenid = m_mysql_helper.GetRow(0);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "not data found, sql = " << oss.str());
        return -1;
    }

    //8 hour
    long dwTs = qwTs + 60 * 60 * 8;
    struct tm *p;
    p = gmtime(&dwTs);
    char bufTs[80];
    strftime(bufTs, 80, "%Y年%m月%d日 %H:%M", p);
    LOG4CPLUS_DEBUG(logger, "ts_str = " << bufTs);

    std::string strUrl = m_listen_follow_url;
    strUrl.replace(strUrl.find("FEEDID"), strlen("FEEDID"), lce::util::StringOP::TypeToStr(qwFeedId));

    Json::Value any;
    any["touser"] = strFollowOpenid;
    any["template_id"] = m_listen_follow_template_id;
    any["url"] = strUrl;
    any["data"]["first"]["value"] = m_listen_follow_first;
    any["data"]["first"]["color"] = m_listen_follow_color;
    any["data"][m_listen_follow_keyword1]["value"] = strContent;
    any["data"][m_listen_follow_keyword1]["color"] = m_listen_follow_color;
    any["data"][m_listen_follow_keyword2]["value"] = strAmount;
    any["data"][m_listen_follow_keyword2]["color"] = m_listen_follow_color;
    any["data"][m_listen_follow_keyword3]["value"] = lce::util::StringOP::TypeToStr(bufTs);
    any["data"][m_listen_follow_keyword3]["color"] = m_listen_follow_color;
    any["data"]["remark"]["value"] = m_listen_follow_remark;
    any["data"]["remark"]["color"] = m_listen_follow_color;

    std::string strPostData = any.toJsonString();
    LOG4CPLUS_DEBUG(logger, "postdata = " << strPostData);

    //调用微信api
    common::wxapi::WXAPIMessage wxapiMessage;
    std::string strBaseAccessToken;
    std::string strJSAPITicket;
    g_server_processor_token->get_pa_info(m_pa_appid, strBaseAccessToken, strJSAPITicket);

    int iRet = 0;

    if(wxapiMessage.SendTemplateMessage(strBaseAccessToken, strPostData, iRet) < 0)
    {
        LOG4CPLUS_ERROR(logger, "WXAPI.SendTemplateMessage failed, errcode = " << iRet);
        return -1;
    }
    return 0;
}

int ExecutorThread::func_get_feed_detail(const std::vector <uint64_t> &vecFeedId, const std::string& strOpenid,
                                         std::vector <hoosho::j::commstruct::FeedInfo> &vecFeedInfo)
{
    std::ostringstream oss;

    vecFeedInfo.clear();
    ::hoosho::j::commstruct::FeedInfo stFeedInfo;
    for(size_t i = 0; i < vecFeedId.size(); i++)
    {
        oss.str("");
        oss << "SELECT * FROM " << m_table_name_feed << " WHERE feed_id = " << vecFeedId[i];

        if(!m_mysql_helper.Query(oss.str()))
        {
            LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
            return -1;
        }

        if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
        {
            feed_info_db_2_pb(m_mysql_helper, stFeedInfo);
            vecFeedInfo.push_back(stFeedInfo);

            func_add_history(vecFeedId[i], strOpenid);
        }
    }
    return 0;
}
int ExecutorThread::func_get_follow_detail(const std::vector<uint64_t>& vecFollowId,
                                           std::vector<hoosho::j::commstruct::FollowInfo>& vecFollowInfo)
{
    std::ostringstream oss;

    vecFollowInfo.clear();
    ::hoosho::j::commstruct::FollowInfo stFollowInfo;
    for(size_t i = 0; i < vecFollowId.size(); i++)
    {
        oss.str("");
        oss << "SELECT * FROM " << m_table_name_follow << " WHERE follow_id = " << vecFollowId[i];

        if(!m_mysql_helper.Query(oss.str()))
        {
            LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
            return -1;
        }

        if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
        {
            follow_info_db_2_pb(m_mysql_helper, stFollowInfo);
            vecFollowInfo.push_back(stFollowInfo);
        }
    }
    return 0;
}


int ExecutorThread::func_add_history(const uint64_t qwFeedId, const std::string& strOpenid)
{
    if(strOpenid.empty() || strOpenid == "0")
    {
        return 0;
    }
    std::ostringstream oss;
        oss.str("");
        oss << "INSERT INTO " << m_table_name_history
        << " SET openid = '" << sql_escape(strOpenid) << "'"
        << ", feed_id = " << qwFeedId
        << ", ts = " << time(NULL)
        << " ON DUPLICATE KEY"
        << " UPDATE ts = " << time(NULL);

        if(!m_mysql_helper.Query(oss.str()))
        {
            LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
            return -1;
        }
    return 0;
}


int ExecutorThread::check_out_trade_no_unique(const std::string& strOutTradeNo)
{
    ostringstream oss;

    //query t_follow
    oss.str("");
    oss << "SELECT * FROM " << m_table_name_follow << " WHERE out_trade_no = '" << sql_escape(strOutTradeNo) << "'";
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
        ::hoosho::j::commstruct::FollowInfo stFollowInfo;
        follow_info_db_2_pb(m_mysql_helper, stFollowInfo);
        LOG4CPLUS_ERROR(logger, "out_trade_no has been used in t_follow, msg = " << stFollowInfo.Utf8DebugString());
        return -1;
    }


    //query t_listen
    oss.str("");
    oss << "SELECT * FROM " << m_table_name_listen << " WHERE out_trade_no = '" << sql_escape(strOutTradeNo) << "'";
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
        ::hoosho::j::commstruct::ListenInfo stListenInfo;
        listen_info_db_2_pb(m_mysql_helper, stListenInfo);
        LOG4CPLUS_ERROR(logger, "out_trade_no has been used in t_listen, msg = " << stListenInfo.Utf8DebugString());
        return -1;
    }
    return 0;
}

int ExecutorThread::func_inc_num_follow(const uint64_t &qwFeedId)
{
    ostringstream oss;
    oss.str("");
    oss << "SELECT num_follow FROM " << m_table_name_feed << " WHERE feed_id = " << qwFeedId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(!m_mysql_helper.GetRowCount())
    {
        LOG4CPLUS_ERROR(logger, "feed_id = " << qwFeedId << " not found, sql = " << oss.str());
        return -1;
    }

    m_mysql_helper.Next();
    uint64_t qwNumFollow = strtoul(m_mysql_helper.GetRow(0), NULL, 10);
    qwNumFollow++;

    oss.str("");
    oss << "UPDATE " << m_table_name_feed << " SET num_follow = " << qwNumFollow
    << " WHERE feed_id = " << qwFeedId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }

    return 0;
}
int ExecutorThread::func_inc_num_listen(const uint64_t& qwFeedId, const uint64_t& qwFollowId)
{
    ostringstream oss;

    //  t_feed
    oss.str("");
    oss << "SELECT num_listen FROM " << m_table_name_feed << " WHERE feed_id = " << qwFeedId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(!m_mysql_helper.GetRowCount())
    {
        LOG4CPLUS_ERROR(logger, "feed_id = " << qwFeedId << " not found, sql = " << oss.str());
        return -1;
    }

    m_mysql_helper.Next();
    uint64_t qwNumListen = strtoul(m_mysql_helper.GetRow(0), NULL, 10);
    qwNumListen++;

    oss.str("");
    oss << "UPDATE " << m_table_name_feed << " SET num_listen = " << qwNumListen
    << " WHERE feed_id = " << qwFeedId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }

    // t_follow
    oss.str("");
    oss << "SELECT num_listen FROM " << m_table_name_follow << " WHERE follow_id = " << qwFollowId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(!m_mysql_helper.GetRowCount())
    {
        LOG4CPLUS_ERROR(logger, "follow_id = " << qwFollowId << " not found, sql = " << oss.str());
        return -1;
    }

    m_mysql_helper.Next();
    qwNumListen = strtoul(m_mysql_helper.GetRow(0), NULL, 10);
    qwNumListen++;

    oss.str("");
    oss << "UPDATE " << m_table_name_follow << " SET num_listen = " << qwNumListen
    << " WHERE follow_id = " << qwFollowId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }

    return 0;
}

int ExecutorThread::func_inc_num_comment(const uint64_t &qwFollowId, const uint64_t &qwCommentType, std::string& strOpenid)
{
    ostringstream oss;

    //check follow_id, get openid
    oss.str("");
    oss << "SELECT openid FROM " << m_table_name_follow << " WHERE follow_id = " << qwFollowId;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }
    if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
    {
       	strOpenid = m_mysql_helper.GetRow(0);
        LOG4CPLUS_DEBUG(logger, "follow_id = " << qwFollowId << " belong to openid = " << strOpenid);
    }
    else
    {
        LOG4CPLUS_ERROR(logger, "follow_id = " << qwFollowId << " not found");
        return -1;
    }

    //add num_commnet
    oss.str("");
    if(qwCommentType == ::hoosho::j::commenum::COMMENT_TYPE_DEFAULT)
    {
        oss << "UPDATE " << m_table_name_follow << " SET num_comment_default = num_comment_default + 1"
        << " WHERE follow_id = " << qwFollowId;
    }
    else if(qwCommentType == ::hoosho::j::commenum::COMMENT_TYPE_GOOD)
    {
        oss << "UPDATE " << m_table_name_follow << " SET num_comment_good = num_comment_good + 1"
        << " WHERE follow_id = " << qwFollowId;
    }
    else if(qwCommentType == ::hoosho::j::commenum::COMMENT_TYPE_BAD)
    {
        oss << "UPDATE " << m_table_name_follow << " SET num_comment_bad = num_comment_bad + 1"
        << " WHERE follow_id = " << qwFollowId;
    }
    else
    {
        return -1;
    }
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        return -1;
    }

    return 0;
}

void ExecutorThread::process_check_out_trade_no_unique(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
    LOG4CPLUS_TRACE(logger, "process_check_out_trade_no_unique");

    reply.m_out_trade_no = request.m_out_trade_no;
    if(!check_out_trade_no_unique(request.m_out_trade_no))
    {
        reply.m_out_trade_no_unique = true;
        reply.m_result_code = ExecutorThreadRequestType::E_OK;
        return;
    }
    else
    {
        reply.m_out_trade_no_unique = false;
        reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
        return;
    }
}

void ExecutorThread::process_get_feed_list(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
    LOG4CPLUS_TRACE(logger, "process_get_feed_list");

    std::ostringstream oss;
    oss.str("");
    oss << "SELECT * FROM " << m_table_name_feed << " WHERE need_sex & " << request.m_need_sex << " = need_sex";
    if(request.m_begin_feed_id)
    {
        oss << " AND feed_id < " << request.m_begin_feed_id;
    }
    if(request.m_scene_type == ::hoosho::j::commenum::GET_LIST_SCENE_TYPE_PERSON_PAGE)
    {
        oss << " AND openid = '" << sql_escape(request.m_openid) << "'";
    }
    oss << " ORDER BY feed_id DESC";
    oss << " LIMIT " << request.m_limit;
    LOG4CPLUS_TRACE(logger, "sql = " << oss.str());

    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

    if(!m_mysql_helper.GetRowCount())
    {
        LOG4CPLUS_ERROR(logger, "mysql query succ but not data found, sql = " << oss.str());
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
        return;
    }

    reply.m_feed_list.clear();
    ::hoosho::j::commstruct::FeedInfo stFeedInfo;
    while( m_mysql_helper.Next())
    {
        feed_info_db_2_pb(m_mysql_helper, stFeedInfo);
        reply.m_feed_list.push_back(stFeedInfo);
    }
    return;
}

void ExecutorThread::process_get_feed_detail(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
    LOG4CPLUS_TRACE(logger, "process_get_feed_detail");

    if(func_get_feed_detail(request.m_feed_id_list, request.m_openid, reply.m_feed_list) < 0)
    {
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

    if(!reply.m_feed_list.size())
    {
        LOG4CPLUS_ERROR(logger, "mysql query succ but not data found");
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
    }
    return;
}

void ExecutorThread::process_add_feed(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_add_feed");

    ::hoosho::j::commstruct::FeedInfo& stFeedInfo = request.m_feed_info;
    std::ostringstream oss;
    oss.str("");
    oss << "INSERT INTO " << m_table_name_feed
    << " SET feed_id = " << stFeedInfo.feed_id()
    << ", openid = '" << sql_escape(stFeedInfo.openid()) << "'"
	<< ", content = '" << sql_escape(stFeedInfo.content()) << "'"
	<< ", ts = " << stFeedInfo.ts()
	<< ", num_follow = " << stFeedInfo.num_follow()
	<< ", num_listen = " << stFeedInfo.num_listen()
	<< ", need_sex = " << stFeedInfo.need_sex();

    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

    return;
}


void ExecutorThread::process_get_follow_list(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
    LOG4CPLUS_TRACE(logger, "process_get_follow_list");

    std::ostringstream oss;
    oss.str("");
    oss << "SELECT * FROM " << m_table_name_follow;
    if(request.m_scene_type == ::hoosho::j::commenum::GET_LIST_SCENE_TYPE_PERSON_PAGE)
    {
        oss << " WHERE openid = '" << sql_escape(request.m_openid) << "'";
    }
    else if(request.m_scene_type == ::hoosho::j::commenum::GET_LIST_SCENE_TYPE_MAIN_PAGE)
    {
        oss << " WHERE feed_id = " << request.m_feed_id;
    }
    else
    {
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        LOG4CPLUS_ERROR(logger, "invalid scene_type = " << request.m_scene_type);
        return;;
    }
    if(request.m_begin_follow_id)
    {
        oss << " AND follow_id < " << request.m_begin_follow_id;
    }
	oss << " AND out_trade_no != ''";
    oss << " ORDER BY follow_id DESC";
    oss << " LIMIT " << request.m_limit;
    LOG4CPLUS_TRACE(logger, "sql = " << oss.str());

    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

    if(!m_mysql_helper.GetRowCount())
    {
        LOG4CPLUS_ERROR(logger, "mysql query succ but not data found, sql = " << oss.str());
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
        return;
    }

    reply.m_follow_list.clear();
    ::hoosho::j::commstruct::FollowInfo stFollowInfo;
    while( m_mysql_helper.Next())
    {
        follow_info_db_2_pb(m_mysql_helper, stFollowInfo);
        reply.m_follow_list.push_back(stFollowInfo);
    }
    return;

}



void ExecutorThread::process_get_follow_detail(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_get_follow_detail");


    if(func_get_follow_detail(request.m_follow_id_list, reply.m_follow_list) < 0)
    {
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

    if(!reply.m_follow_list.size())
    {
        LOG4CPLUS_ERROR(logger, "mysql query succ but not data found");
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
        return;
    }
    return;
}
void ExecutorThread::process_add_follow(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_add_follow");

    std::ostringstream oss;
    //save
    ::hoosho::j::commstruct::FollowInfo& stFollowInfo = request.m_follow_info;
    oss.str("");
    oss << "INSERT INTO " << m_table_name_follow
    << " SET follow_id = " << stFollowInfo.follow_id()
    << ", feed_id = '" << stFollowInfo.feed_id() << "'"
    << ", openid = '" << sql_escape(stFollowInfo.openid()) << "'"
    << ", content_type = '" << stFollowInfo.content_type() << "'"
    << ", content_id = '" << sql_escape(stFollowInfo.content_id()) << "'"
    << ", content_duration = '" << stFollowInfo.content_duration() << "'"
    << ", ts = '" << stFollowInfo.ts() << "'"
    << ", num_listen = " << stFollowInfo.num_listen()
    << ", num_comment_good = " << stFollowInfo.num_comment_good()
    << ", num_comment_default = " << stFollowInfo.num_comment_default()
    << ", num_comment_bad = " << stFollowInfo.num_comment_bad()
    << ", out_trade_no = '" << sql_escape(stFollowInfo.out_trade_no()) << "'";

    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

//    func_inc_num_follow(stFollowInfo.feed_id());

    return;
}
void ExecutorThread::process_get_listen_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_get_listen_list");

    ostringstream oss;
    oss.str("");
    oss << "SELECT * FROM " << m_table_name_listen << " WHERE openid = '" << sql_escape(request.m_openid) << "'";
    if(request.m_begin_ts)
    {
        oss << " AND ts < " << request.m_begin_ts;
    }
	oss << " AND out_trade_no != ''";
    oss << " ORDER BY listen_id DESC";
    oss << " LIMIT " << request.m_limit;

    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

    if(!m_mysql_helper.GetRowCount())
    {
        LOG4CPLUS_ERROR(logger, "mysql query succ but not data found, sql = " << oss.str());
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
        return;
    }
    request.m_follow_id_list.clear();
	reply.m_auto_comment_follow_list.clear();
    while(m_mysql_helper.Next())
    {
		hoosho::j::commstruct::ListenInfo stListenInfo;
		listen_info_db_2_pb(m_mysql_helper, stListenInfo);
        request.m_follow_id_list.push_back(stListenInfo.follow_id());

		string strOpenid;
		if(stListenInfo.comment_type() == ::hoosho::j::commenum::COMMENT_TYPE_NULL &&
				time(NULL) - stListenInfo.ts() > m_auto_comment_time)
		{
			//use m_mysql_helper here!!!
			if(!set_default_comment_and_get_follow_user(m_ano_mysql_helper, stListenInfo.listen_id(), stListenInfo.follow_id(), strOpenid, m_table_name_listen, m_table_name_follow))
			{
				AutoCommentFollowInfo stAutoCommentFollowInfo(strOpenid, stListenInfo.out_trade_no(), stListenInfo.openid());
				reply.m_auto_comment_follow_list.push_back(stAutoCommentFollowInfo);

				//发送微信模板消息
                send_template_message_of_listen_follow(stListenInfo.listen_id());
			}
		}
        LOG4CPLUS_TRACE(logger, request.m_follow_id_list[request.m_follow_id_list.size() - 1]);
    }
    LOG4CPLUS_TRACE(logger, request.m_follow_id_list.size());
    if(func_get_follow_detail(request.m_follow_id_list, reply.m_follow_list) < 0)
    {
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

    if(!reply.m_follow_list.size())
    {
        LOG4CPLUS_ERROR(logger, "mysql query succ but not data found.");
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
        return;
    }
    return;;
}
void ExecutorThread::process_get_listen_detail(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_listen_detail");

    ostringstream oss;

    //获取回答信息
    oss.str("");
    oss << "SELECT * FROM " << m_table_name_follow << " WHERE follow_id = " << request.m_follow_id;
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }
    if(!m_mysql_helper.GetRowCount())
    {
        LOG4CPLUS_ERROR(logger, "follow_id = " << request.m_follow_id << " not found, sql = " << oss.str());
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
        return;
    }
    m_mysql_helper.Next();
    follow_info_db_2_pb(m_mysql_helper, reply.m_follow_info);
    if(reply.m_follow_info.openid() == request.m_openid)
    {
        LOG4CPLUS_DEBUG(logger, "openid = " << request.m_openid
                                << " listen his follow with id = " << reply.m_follow_info.openid());
        reply.m_result_code = ExecutorThreadRequestType::E_LISTEN_SELF;
        return;
    }


    //查找是否有偷听记录
    oss.str("");
    oss << "SELECT * FROM " << m_table_name_listen << " WHERE openid = '" << sql_escape(request.m_openid) << "'"
    << " AND follow_id = " << request.m_follow_id;
	oss << " AND out_trade_no != ''";
    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }
    if(!m_mysql_helper.GetRowCount())
    {
        LOG4CPLUS_ERROR(logger, "mysql query succ, but not data found, sql = " << oss.str());
        reply.m_result_code = ExecutorThreadRequestType::E_NO_PAY;
        return;
    }

    m_mysql_helper.Next();
    listen_info_db_2_pb(m_mysql_helper, reply.m_listen_info);
	const ::hoosho::j::commstruct::ListenInfo& stListenInfo = reply.m_listen_info;

	reply.m_auto_comment_follow_list.clear();
	if(stListenInfo.comment_type() == ::hoosho::j::commenum::COMMENT_TYPE_NULL &&
			time(NULL) - stListenInfo.ts() > m_auto_comment_time)
	{
		string strOpenid;
		//use m_mysql_helper here!!!
		if(!set_default_comment_and_get_follow_user(m_mysql_helper, stListenInfo.listen_id(), stListenInfo.follow_id(), strOpenid, m_table_name_listen, m_table_name_follow))
		{
			reply.m_listen_info.set_comment_type(::hoosho::j::commenum::COMMENT_TYPE_DEFAULT);
			AutoCommentFollowInfo stAutoCommentFollowInfo(strOpenid, stListenInfo.out_trade_no(), stListenInfo.openid());
			reply.m_auto_comment_follow_list.push_back(stAutoCommentFollowInfo);

			//发送微信模板消息
            send_template_message_of_listen_follow(stListenInfo.listen_id());
		}
	}

    return;
}



void ExecutorThread::process_add_listen(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_add_listen");

	ostringstream oss;

	//get
	oss.str("");
	oss << "SELECT openid FROM " << m_table_name_follow << " WHERE follow_id = " << request.m_listen_info.follow_id();
	if(!m_mysql_helper.Query(oss.str()))
	{
		LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
	}

	if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
	{
		string strOpenid = m_mysql_helper.GetRow(0);
		if(strOpenid == request.m_listen_info.openid())
		{
			LOG4CPLUS_ERROR(logger, "follow_id = " << request.m_listen_info.follow_id()
									<< " belong to openid = " << request.m_listen_info.openid());
			reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
			return;
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "follow_id = " << request.m_listen_info.follow_id() << " not found, sql = " << oss.str());
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
		return;
	}

    //save
    oss.str("");
    oss << "INSERT INTO " << m_table_name_listen
    << " SET listen_id = " << request.m_listen_info.listen_id()
    << ", openid = '" << sql_escape(request.m_listen_info.openid()) << "'"
    << ", follow_id = " << request.m_listen_info.follow_id()
    << ", feed_id = " << request.m_listen_info.feed_id()
    << ", ts = " << request.m_listen_info.ts()
    << ", comment_type = " << request.m_listen_info.comment_type()
    << ", out_trade_no = '" << sql_escape(request.m_listen_info.out_trade_no()) << "'";

    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

//    func_inc_num_listen(request.m_listen_info.feed_id(), request.m_listen_info.follow_id());
}

void ExecutorThread::process_comment_follow(ExecutorThreadRequestElement &request, ExecutorThreadResponseElement &reply)
{
    ostringstream oss;
    const ::hoosho::j::commstruct::CommentFollowInfo& stCommentFollowInfo = request.m_comment_follow_info;
    //find
    oss.str("");
    oss << "SELECT listen_id, comment_type, out_trade_no FROM " << m_table_name_listen
    << " WHERE openid = '" << sql_escape(stCommentFollowInfo.openid()) << "'"
    << " AND follow_id = " << stCommentFollowInfo.follow_id()
			<< " AND out_trade_no != ''";

    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }
    if(!m_mysql_helper.GetRowCount())
    {
        LOG4CPLUS_ERROR(logger, "mysql query succ, but not data found, sql = " << oss.str());
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
        return;
    }
    m_mysql_helper.Next();
    uint64_t qwListenId = strtoul(m_mysql_helper.GetRow(0), NULL, 10);
    uint64_t qwCommentType = strtoul(m_mysql_helper.GetRow(1), NULL, 10);
    string strOutTradeNo = m_mysql_helper.GetRow(2);
	if(strOutTradeNo.empty())
	{
		LOG4CPLUS_ERROR(logger, "listen_id = " << qwListenId << " has null out_trade_no");
        reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
        return;
	}

    if(qwCommentType != ::hoosho::j::commenum::COMMENT_TYPE_NULL)
    {
        LOG4CPLUS_ERROR(logger, "listen_id = " << qwListenId << " has been comment with type = " << qwCommentType);
        reply.m_result_code = ExecutorThreadRequestType::E_FOLLOW_HAS_COMMENT;
        return;
    }


    //save
    oss.str("");
    oss << "UPDATE " << m_table_name_listen
    << " SET comment_type = " << stCommentFollowInfo.comment_type()
    << " WHERE listen_id = " << qwListenId;

    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

    //add num_comment
    if(func_inc_num_comment(stCommentFollowInfo.follow_id(), stCommentFollowInfo.comment_type(), reply.m_openid) < 0)
    {
        LOG4CPLUS_ERROR(logger, "add num_commnet failed");
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

	reply.m_out_trade_openid = stCommentFollowInfo.openid();
    reply.m_comment_follow_info = request.m_comment_follow_info;
    reply.m_out_trade_no = strOutTradeNo;
	
	//发送微信模板消息
    send_template_message_of_listen_follow(qwListenId);

    return;
}

void ExecutorThread::process_get_history_list(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_get_history_list");

    ostringstream oss;
    oss.str("");
    oss << "SELECT feed_id FROM " << m_table_name_history
    << " WHERE openid = '" << sql_escape(request.m_openid) << "'";
    if(request.m_begin_ts)
    {
        oss << " AND ts < " << request.m_begin_ts;
    }
    oss << " ORDER BY ts DESC";
    oss << " LIMIT " << request.m_limit;

    if(!m_mysql_helper.Query(oss.str()))
    {
        LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }

    if(!m_mysql_helper.GetRowCount())
    {
        LOG4CPLUS_ERROR(logger, "mysql query succ, but not data found, sql = " << oss.str());
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
        return;
    }

    request.m_feed_id_list.clear();
    while(m_mysql_helper.Next())
    {
        request.m_feed_id_list.push_back(strtoul(m_mysql_helper.GetRow(0), NULL, 10));
    }

    if(func_get_feed_detail(request.m_feed_id_list, request.m_openid, reply.m_feed_list) < 0)
    {
        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        return;
    }
}
void ExecutorThread::process_check_listen(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_check_listen");

    ostringstream oss;
    oss.str("");

    reply.m_check_listen_list.clear();
    for(size_t i = 0; i < request.m_check_listen_list.size(); i++)
    {
        ::hoosho::j::commstruct::CheckListenInfo stCheckListenInfo;
        stCheckListenInfo.CopyFrom(request.m_check_listen_list[i]);

        //自己的回答，不用支付
        oss.str("");
        oss << "SELECT openid FROM " << m_table_name_follow
        << " WHERE follow_id = " << stCheckListenInfo.follow_id();
        if(!m_mysql_helper.Query(oss.str()))
        {
            LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str()
                                    << ", errmsg = " << m_mysql_helper.GetErrMsg());
            continue;
        }
        if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
        {
            string strOpenid = m_mysql_helper.GetRow(0);
            if(strOpenid == stCheckListenInfo.openid())
            {
                stCheckListenInfo.set_listen(true);
                reply.m_check_listen_list.push_back(stCheckListenInfo);
                continue;
            }
        }

        oss.str("");
        oss << "SELECT out_trade_no FROM " << m_table_name_listen
        << " WHERE openid = '" << sql_escape(stCheckListenInfo.openid()) << "'"
        << " AND follow_id = " << stCheckListenInfo.follow_id()
        << " AND out_trade_no != ''";

        if(!m_mysql_helper.Query(oss.str()))
        {
            LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str()
                                    << ", errmsg = " << m_mysql_helper.GetErrMsg());
            continue;
        }

//		string strOutTradeNo = "";
//		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
//		{
//			strOutTradeNo = m_mysql_helper.GetRow(0);
//		}
//        if(!m_mysql_helper.GetRowCount() || strOutTradeNo.empty())
        if(!m_mysql_helper.GetRowCount())
        {
            stCheckListenInfo.set_listen(0);
        }
        else
        {
            stCheckListenInfo.set_listen(1);
        }
        reply.m_check_listen_list.push_back(stCheckListenInfo);
    }
    if(!reply.m_check_listen_list.size())
    {
        reply.m_result_code = ExecutorThreadRequestType::E_NOT_EXIST;
        return;
    }
    return;
}

void ExecutorThread::process_get_comment_follow(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_get_comment_follow");

    ostringstream oss;
    reply.m_comment_follow_list.clear();
    for(size_t i = 0; i < request.m_comment_follow_list.size(); i++)
    {
        ::hoosho::j::commstruct::CommentFollowInfo stCommentFollowInfo;
        stCommentFollowInfo.CopyFrom(request.m_comment_follow_list[i]);

        oss.str("");
        oss << "SELECT comment_type, out_trade_no FROM " << m_table_name_listen
        << " WHERE openid = '" << sql_escape(stCommentFollowInfo.openid()) << "'"
        << " AND follow_id = " << stCommentFollowInfo.follow_id()
		<< " AND out_trade_no != ''";

        if(!m_mysql_helper.Query(oss.str()))
        {
            LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
			reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
            return;
        }
        if(!m_mysql_helper.GetRowCount())
        {
            LOG4CPLUS_ERROR(logger, "openid = " << stCommentFollowInfo.openid()
                                   << ", follow_id = " << stCommentFollowInfo.follow_id()
                                   << " not found in table " << m_table_name_listen);
            continue;
        }
        m_mysql_helper.Next();
        stCommentFollowInfo.set_comment_type(strtoul(m_mysql_helper.GetRow(0), NULL, 10));

        reply.m_comment_follow_list.push_back(stCommentFollowInfo);
    }
}

void ExecutorThread::process_set_out_trade_no(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_set_out_trade_no");

	ostringstream oss;
	oss.str("");
	if(request.m_business_type == ::hoosho::j::commenum::FEEDS_BUSINESS_TYPE_FOLLOW)
	{
		oss.str("");
		oss << "SELECT feed_id FROM " << m_table_name_follow << " WHERE follow_id = " << request.m_business_id;
		if(!m_mysql_helper.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
        	reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
        	return;
		}
		uint64_t qwFeedId;
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			qwFeedId = strtoul(((string)m_mysql_helper.GetRow(0)).c_str(), NULL, 10);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "not data found, sql = " << oss.str());
	        reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
	        return;
		}

		oss.str("");
		oss << "UPDATE " << m_table_name_follow << " SET out_trade_no = '" << sql_escape(request.m_out_trade_no) << "'"
				<< " WHERE follow_id = " << request.m_business_id;
		if(!m_mysql_helper.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
	        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
	        return;
		}
		if(!m_mysql_helper.GetAffectedRows())
		{
			LOG4CPLUS_ERROR(logger, "follow_id = " <<request.m_business_id << " not found, sql = " << oss.str());
	        reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
	        return;
		}

		func_inc_num_follow(qwFeedId);

        //发送微信模板消息
        send_template_message_of_new_follow(request.m_business_id, qwFeedId);

	}
	else if(request.m_business_type == ::hoosho::j::commenum::FEEDS_BUSINESS_TYPE_LISTEN)
	{
		oss.str("");
		//!!!!!!!!!!
		oss << "SELECT feed_id, follow_id FROM " << m_table_name_listen << " WHERE listen_id = " << request.m_business_id;
		if(!m_mysql_helper.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
	        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
	        return;
		}
		uint64_t qwFeedId;
		uint64_t qwFollowId;
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			qwFeedId = strtoul( ((string)m_mysql_helper.GetRow(0)).c_str(), NULL, 10);
			qwFollowId = strtoul( ((string)m_mysql_helper.GetRow(1)).c_str(), NULL, 10);
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "not data found, sql = " << oss.str());
	        reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
	        return;
		}

		oss.str("");
		oss << "UPDATE " << m_table_name_listen << " SET out_trade_no = '" << sql_escape(request.m_out_trade_no) << "'"
				<< " WHERE listen_id = " << request.m_business_id;
		if(!m_mysql_helper.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
	        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
	        return;
		}
		if(!m_mysql_helper.GetAffectedRows())
		{
			LOG4CPLUS_ERROR(logger, "listen_id = " <<request.m_business_id << " not found, sql = " << oss.str());
	        reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
	        return;
		}

		func_inc_num_listen(qwFeedId, qwFollowId);
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "unknow bussiness_type = " << request.m_business_type);
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
	}
}
void ExecutorThread::process_check_business_id_valid(ExecutorThreadRequestElement& request, ExecutorThreadResponseElement& reply)
{
    LOG4CPLUS_TRACE(logger, "process_check_business_id_valid");
	ostringstream oss;
	oss.str("");
	if(request.m_business_type == ::hoosho::j::commenum::FEEDS_BUSINESS_TYPE_FOLLOW)
	{
		oss << "SELECT out_trade_no FROM " << m_table_name_follow
				<< " WHERE follow_id = " << request.m_business_id;
		if(!m_mysql_helper.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
	        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
	        return;
		}
		string strOutTradeNo = "";
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			strOutTradeNo= m_mysql_helper.GetRow(0);
		}
		if(!m_mysql_helper.GetRowCount() || strOutTradeNo.empty())
		{
			LOG4CPLUS_ERROR(logger, "follow_id = " << request.m_business_id << " not found, sql = " << oss.str());
	        reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
	        return;
		}
	}
	else if(request.m_business_type == ::hoosho::j::commenum::FEEDS_BUSINESS_TYPE_LISTEN)
	{
		oss << "SELECT out_trade_no FROM " << m_table_name_listen
				<< " WHERE listen_id = " << request.m_business_id;
		LOG4CPLUS_DEBUG(logger, "oss.str() = " << oss.str());
		if(!m_mysql_helper.Query(oss.str()))
		{
			LOG4CPLUS_ERROR(logger, "mysql query failed, sql = " << oss.str() << ", errmsg = " << m_mysql_helper.GetErrMsg());
	        reply.m_result_code = ExecutorThreadRequestType::E_DB_FAIL;
	        return;
		}
		string strOutTradeNo = "";
		if(m_mysql_helper.GetRowCount() && m_mysql_helper.Next())
		{
			strOutTradeNo = m_mysql_helper.GetRow(0);
		}
		if(!m_mysql_helper.GetRowCount() || strOutTradeNo.empty())
		{
			LOG4CPLUS_ERROR(logger, "listen_id = " <<request.m_business_id << " not found, sql = " << oss.str());
	        reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
	        return;
		}
	}
	else
	{
		LOG4CPLUS_ERROR(logger, "unknow bussiness_type = " << request.m_business_type);
		reply.m_result_code = ExecutorThreadRequestType::E_INVALID_PARAM;
	}
}



