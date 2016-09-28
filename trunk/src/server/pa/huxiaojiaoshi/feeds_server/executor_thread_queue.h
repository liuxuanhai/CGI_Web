#ifndef _FEEDS_SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _FEEDS_SERVER_EXECUTOR_THREAD_QUEUE_H_

#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include "memory/duplex_queue.h"
#include "msg.pb.h"

using namespace std;

class ExecutorThreadRequestType
{
public:
    enum Type
    {
        T_CHECK_OUT_TRADE_NO_UNIQUE,

        T_GET_FEED_LIST,
        T_GET_FEED_DETAIL,
        T_ADD_FEED,

        T_GET_FOLLOW_LIST,
        T_GET_FOLLOW_DETAIL,
        T_ADD_FOLLOW,

        T_GET_LISTEN_LIST,
        T_GET_LISTEN_DETAIL,
        T_ADD_LISTEN,
        T_COMMENT_FOLLOW,

        T_GET_HISTORY_LIST,

        T_CHECK_LISTEN,
        T_GET_COMMNET_FOLLOW,

        T_SET_OUT_TRADE_NO,
        T_CHECK_BUSINESS_ID_VALID,
    };

    enum Error
    {
        E_OK = 0,
        E_NOT_EXIST,
        E_DB_FAIL,
        E_INVALID_PARAM,
        E_NO_PAY,
		E_LISTEN_SELF,
        E_FOLLOW_HAS_COMMENT,
    };

    static const std::string type_string(int type)
    {
        switch(type)
        {
            case T_CHECK_OUT_TRADE_NO_UNIQUE:
                return "ExecutorThreadRequestType::T_CHECK_OUT_TRADE_NO_UNIQUE";

            case T_GET_FEED_LIST:
                return "ExecutorThreadRequestType::T_GET_FEED_LIST";

            case T_GET_FEED_DETAIL:
                return "ExecutorThreadRequestType::T_GET_FEED_DETAIL";

            case T_ADD_FEED:
                return "ExecutorThreadRequestType::T_ADD_FEED";

            case T_GET_FOLLOW_LIST:
                return "ExecutorThreadRequestType::T_GET_FOLLOW_LIST";

            case T_GET_FOLLOW_DETAIL:
                return "ExecutorThreadRequestType::T_GET_FOLLOW_DETAIL";

            case T_ADD_FOLLOW:
                return "ExecutorThreadRequestType::T_ADD_FOLLOW";

            case T_GET_LISTEN_LIST:
                return "ExecutorThreadRequestType::T_GET_LISTEN_LIST";

            case T_GET_LISTEN_DETAIL:
                return "ExecutorThreadRequestType::T_GET_LISTEN_DETAIL";

            case T_ADD_LISTEN:
                return "ExecutorThreadRequestType::T_ADD_LISTEN";

            case T_COMMENT_FOLLOW:
                return "ExecutorThreadRequestType::T_COMMENT_FOLLOW";

            case T_GET_HISTORY_LIST:
                return "ExecutorThreadRequestType::T_GET_HISTORY_LIST";

            case T_CHECK_LISTEN:
                return "ExecutorThreadRequestType::T_CHECK_LISTEN";

            case T_GET_COMMNET_FOLLOW:
                return "ExecutorThreadRequestType::T_GET_COMMNET_FOLLOW";

            case T_SET_OUT_TRADE_NO:
                return "ExecutorThreadRequestType::T_SET_OUT_TRADE_NO";

            case T_CHECK_BUSINESS_ID_VALID:
                return "ExecutorThreadRequestType::T_CHECK_BUSINESS_ID_VALID";

            default:
                return "ExecutorThreadRequestType::T_UNKNOWN";
        }
    }

    static const std::string err_string(int err)
    {
        switch(err)
        {
            case E_OK:
                return "ExecutorThreadRequestType::E_OK";

            case E_NOT_EXIST:
                return "ExecutorThreadRequestType::E_NOT_EXIST";

            case E_DB_FAIL:
                return "ExecutorThreadRequestType::E_DB_FAIL";

            case E_INVALID_PARAM:
                return "ExecutorThreadRequestType::E_INVALID_PARAM";

            case E_NO_PAY:
                return "ExecutorThreadRequestType::E_NO_PAY";

            case E_LISTEN_SELF:
                return "ExecutorThreadRequestType::E_LISTEN_SELF";

            default:
                return "ExecutorThreadRequestType::E_UNKNOWN";
        }
    }
};

class ExecutorThreadRequestElement
{
public:
    ExecutorThreadRequestElement()
    {
    }

    ExecutorThreadRequestElement(int request_type)
    {
        m_request_type = request_type;
        m_need_reply = false;

    }

    void need_reply()
    {
        m_need_reply = true;
    }

    const std::string ToString() const
    {
        std::ostringstream ss;
        ss << "  {"
           << "request_type: " << ExecutorThreadRequestType::type_string(m_request_type)
           << ", fsm_id: " << m_fsm_id
           << ", need_reply: " << m_need_reply << "  ";

        if(m_request_type == ExecutorThreadRequestType::T_CHECK_OUT_TRADE_NO_UNIQUE)
        {
           ss << ", out_trade_no: "  << m_out_trade_no;
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_FEED_LIST)
        {
            ss << ", openid: " << m_openid
               <<", scene_type: " << m_scene_type
               << ", begin_feed_id: " << m_begin_feed_id
               << ", limit: " << m_limit
               << ", need_sex: " << m_need_sex;
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_FEED_DETAIL)
        {
            ss << ", openid: " << m_openid
            << ", feed_id_list: [";
            for(size_t i = 0; i < m_feed_id_list.size(); i++)
            {
                if(i > 0)
                {
                    ss << ", ";
                }
                ss << m_feed_id_list[i];
            }
            ss << " ] ";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_ADD_FEED)
        {
            ss << ", feed_info: {" << m_feed_info.Utf8DebugString() << "}";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_FOLLOW_LIST)
        {
             ss << ", openid: " << m_openid
               <<", scene_type: " << m_scene_type
               << ", feed_id: " << m_feed_id
               << ", begin_follow_id: " << m_begin_follow_id
               << ", limit: " << m_limit;
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_FOLLOW_DETAIL)
        {
            ss << ", follow_id_list: [";
            for(size_t i = 0; i < m_follow_id_list.size(); i++)
            {
                if(i > 0)
                {
                    ss << ", ";
                }
                ss << m_follow_id_list[i];
            }
            ss << " ] ";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_ADD_FOLLOW)
        {
            ss << ", feed_info: {" << m_follow_info.Utf8DebugString() << "}";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_LISTEN_LIST)
        {
              ss << ", openid: " << m_openid
               << ", scene_type: " << m_scene_type
               << ", feed_id: " << m_feed_id
               << ", begin_ts: " << m_begin_ts
               << ", limit: " << m_limit;
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_LISTEN_DETAIL)
        {
            ss << ", openid: " << m_openid
                << ", follow_id: " << m_follow_id
                << ", out_trade_no: " << m_out_trade_no;
        }
        else if(m_request_type == ExecutorThreadRequestType::T_ADD_LISTEN)
        {
            ss << ", listen_info: {" << m_listen_info.Utf8DebugString() << "}";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_COMMENT_FOLLOW)
        {
            ss << ", comment_follow_info: {" << m_comment_follow_info.Utf8DebugString() << "}";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_HISTORY_LIST)
        {
            ss << ", openid: " << m_openid
               << ", begin_ts: " << m_begin_ts
               << ", limit: " << m_limit;
        }
        else if(m_request_type == ExecutorThreadRequestType::T_CHECK_LISTEN)
        {
            ss << ", check listen: [";
            for(size_t i = 0; i < m_check_listen_list.size(); i++)
            {
                if(i)   ss << ", ";
                ss << "{" << m_check_listen_list[i].Utf8DebugString() << "}";
            }
            ss << " ]";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_COMMNET_FOLLOW)
        {
            ss << ", get comment follow: [";
            for(size_t i = 0; i < m_comment_follow_list.size(); i++)
            {
                if(i)   ss << ", ";
                ss << "{" << m_comment_follow_list[i].Utf8DebugString() << "}";
            }
            ss << " ]";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_SET_OUT_TRADE_NO)
        {
            ss << ", out_trade_no: " << m_out_trade_no
            << ", business_id: " << m_business_id
            << ", business_type: " << m_business_type;
        }
        else if(m_request_type == ExecutorThreadRequestType::T_CHECK_BUSINESS_ID_VALID)
        {
            ss << ", business_id: " << m_business_id
            << ", business_type: " << m_business_type;
        }

        return ss.str();
    }

public:
    uint32_t m_request_type;
    uint32_t m_fsm_id;
    bool m_need_reply;

    //for add
    ::hoosho::j::commstruct::FeedInfo m_feed_info;
    ::hoosho::j::commstruct::FollowInfo m_follow_info;
    ::hoosho::j::commstruct::ListenInfo m_listen_info;

    //for get list

    //-common
    uint64_t m_scene_type;
    string m_openid;
    uint64_t m_limit;

    //-feed list
    uint64_t m_need_sex;
    uint64_t m_begin_feed_id;

    //-follow list
    uint64_t m_begin_follow_id;
    uint64_t m_feed_id;

    //-listen list
    uint64_t m_begin_ts;

    //-listen detail
    uint64_t m_follow_id;
    std::string m_out_trade_no;

    //for comment follow
    ::hoosho::j::commstruct::CommentFollowInfo m_comment_follow_info;

    //for get detail
    std::vector<uint64_t> m_feed_id_list;
    std::vector<uint64_t> m_follow_id_list;

    //for check listen
    std::vector<hoosho::j::commstruct::CheckListenInfo> m_check_listen_list;

    //for get commnet follow
    std::vector<hoosho::j::commstruct::CommentFollowInfo> m_comment_follow_list;

    //for set out_trade_no
    uint64_t m_business_id;
    uint32_t m_business_type;
};

class AutoCommentFollowInfo
{
public:
    string m_openid;
    string m_out_trade_no;
    string m_out_trade_openid;
    uint64_t m_comment_type;
    uint64_t m_record_type;

    AutoCommentFollowInfo()
    {

    }

    AutoCommentFollowInfo(const string& openid, const string& out_trade_no, const string& out_trade_openid,
                          const uint64_t& comment_type = ::hoosho::j::commenum::COMMENT_TYPE_DEFAULT,
                            const uint64_t& record_type = 1):m_openid(openid), m_out_trade_no(out_trade_no),
                                                              m_out_trade_openid(out_trade_openid), m_comment_type(comment_type), m_record_type(record_type)
    {

    }
};

class ExecutorThreadResponseElement
{
public:
    ExecutorThreadResponseElement()
    {
    }

    ExecutorThreadResponseElement(uint32_t dwReqType, uint32_t dwFsmId)
        : m_request_type(dwReqType)
        , m_fsm_id(dwFsmId)
        , m_result_code(ExecutorThreadRequestType::E_OK)
    {
    }

    const std::string ToString() const
    {
        std::ostringstream ss;
        ss << "{"
           << " req_type: " << ExecutorThreadRequestType::type_string(m_request_type)
           << ", fsm_id: " << m_fsm_id
           << ", result_code: " << ExecutorThreadRequestType::err_string(m_result_code) << " ";

        if(m_request_type == ExecutorThreadRequestType::T_CHECK_OUT_TRADE_NO_UNIQUE)
        {
            ss << ", out_trade_no: " << m_out_trade_no
             << ", check_out_trade_no_unique: " << m_out_trade_no_unique;
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_FEED_LIST)
        {
            ss << ", feed_list: [ ";
            for(size_t i = 0; i < m_feed_list.size(); i++)
            {
                if(i > 0) ss << ", ";
                ss << "{" << m_feed_list[i].Utf8DebugString() << "}";
            }
            ss << "] ";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_FEED_DETAIL)
        {
            ss << ", feed_list: [ ";
            for(size_t i = 0; i < m_feed_list.size(); i++)
            {
                if(i > 0) ss << ", ";
                ss << "{" << m_feed_list[i].Utf8DebugString() << "}";
            }
            ss << "] ";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_ADD_FEED)
        {
            ss << ", feed_info: {" << m_feed_info.Utf8DebugString() << "}";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_FOLLOW_LIST)
        {
            ss << ", follow_list: [ ";
            for(size_t i = 0; i < m_follow_list.size(); i++)
            {
                if(i > 0) ss << ", ";
                ss << "{" << m_follow_list[i].Utf8DebugString() << "}";
            }
            ss << "] ";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_FOLLOW_DETAIL)
        {
            ss << ", follow_list: [ ";
            for(size_t i = 0; i < m_follow_list.size(); i++)
            {
                if(i > 0) ss << ", ";
                ss << "{" << m_follow_list[i].Utf8DebugString() << "}";
            }
            ss << "] ";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_ADD_FOLLOW)
        {
            ss << ", follow_info: {" << m_follow_info.Utf8DebugString() << "}";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_LISTEN_LIST)
        {
            ss << ", listen_list: [";
            for(size_t i = 0; i < m_follow_list.size(); i++)
            {
                if(i > 0) ss << ", ";
                ss << "{" << m_follow_list[i].Utf8DebugString() << "}";
            }
            ss << "] ";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_LISTEN_DETAIL)
        {
            ss << ", listen_info: {" << m_listen_info.Utf8DebugString() << "}";
            ss << ", follow_info: {" << m_follow_info.Utf8DebugString() << "}";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_ADD_LISTEN)
        {

        }
        else if(m_request_type == ExecutorThreadRequestType::T_COMMENT_FOLLOW)
        {
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_HISTORY_LIST)
        {
            ss << ", history_list: [";
            for(size_t i = 0; i < m_feed_list.size(); i++)
            {
                if(i > 0) ss << ", ";
                ss << "{" << m_feed_list[i].Utf8DebugString() << "}";
            }
            ss << "] ";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_CHECK_LISTEN)
        {
            ss << ", check listen: [";
            for(size_t i = 0; i < m_check_listen_list.size(); i++)
            {
                if(i)   ss << ", ";
                ss << "{" << m_check_listen_list[i].Utf8DebugString() << "}";
            }
            ss << " ]";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_GET_COMMNET_FOLLOW)
        {
            ss << ", get comment follow: [";
            for(size_t i = 0; i < m_comment_follow_list.size(); i++)
            {
                if(i)   ss << ", ";
                ss << "{" << m_comment_follow_list[i].Utf8DebugString() << "}";
            }
            ss << " ]";
        }
        else if(m_request_type == ExecutorThreadRequestType::T_SET_OUT_TRADE_NO)
        {
        }
        else if(m_request_type == ExecutorThreadRequestType::T_CHECK_BUSINESS_ID_VALID)
        {
        }
        return ss.str();
    }

public:
    uint32_t m_request_type;
    uint32_t m_fsm_id;
    uint32_t m_result_code;

    //check out_trade_no unique
    string m_out_trade_no;
    bool m_out_trade_no_unique;
    string m_openid;
    string m_out_trade_openid;

    //for add
    ::hoosho::j::commstruct::FeedInfo m_feed_info;
    ::hoosho::j::commstruct::FollowInfo m_follow_info;
    ::hoosho::j::commstruct::CommentFollowInfo m_comment_follow_info;

    //for get list/detail
    std::vector<hoosho::j::commstruct::FeedInfo> m_feed_list;
    std::vector<hoosho::j::commstruct::FollowInfo> m_follow_list;
    ::hoosho::j::commstruct::ListenInfo m_listen_info;

    //check listen
    std::vector<hoosho::j::commstruct::CheckListenInfo> m_check_listen_list;
    //get comment follow
    std::vector<hoosho::j::commstruct::CommentFollowInfo> m_comment_follow_list;

    //when get listen list/detail, auto comment
    std::vector<AutoCommentFollowInfo> m_auto_comment_follow_list;
};

typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


