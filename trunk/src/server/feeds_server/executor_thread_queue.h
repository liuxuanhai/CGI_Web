#ifndef _FEEDS_SERVER_EXECUTOR_THREAD_QUEUE_H_
#define _FEEDS_SERVER_EXECUTOR_THREAD_QUEUE_H_

#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include "memory/duplex_queue.h"
#include "db_feed.h"
#include "db_follow.h"
#include "db_favorite.h"
#include "db_collect.h"

using namespace std;

    class ExecutorThreadRequestType
    {
    public:
        enum Type
        {
        	T_FEED_DETAIL_QUERY,
            T_FEED_QUERY,
            T_FEED_ADD,
            T_FEED_DELETE,

			T_REPORT_FEED_ADD,
			T_USER_FORBID_QUERY,

			T_FOLLOW_DETAIL_QUERY,
			T_FOLLOW_LIST_QUERY,
			T_FOLLOW_COMMENT_DETAIL_QUERY,
			T_FOLLOW_COMMENT_QUERY,
			T_FOLLOW_ADD,
			T_FOLLOW_DELETE,

			T_FEED_LIST_FAVORITE_QUERY,
			T_FAVORITE_QUERY,
			T_FAVORITE_ADD,
			T_FAVORITE_DELETE,

			T_FEED_LIST_COLLECT_QUERY,
			T_COLLECT_QUERY,
			T_COLLECT_ADD,
			T_COLLECT_DELETE,
        };

        enum Error
        {
            E_OK = 0,
            E_NOT_EXIST,
            E_DB_FAIL,
            E_INVALID_PARAM,
            E_FEED_DELETED,
            E_REPLY_TO_SELF_INVALID,
        };

        static const std::string type_string(int type)
        {
            switch(type)
            {
            	case T_FEED_DETAIL_QUERY:
					return "ExecutorThreadRequestType::T_FEED_DETAIL_QUERY";

                case T_FEED_QUERY:
                    return "ExecutorThreadRequestType::T_FEED_QUERY";

				case T_FEED_ADD:
					return "ExecutorThreadRequestType::T_FEED_ADD";

            	case T_FEED_DELETE:
					return "ExecutorThreadRequestType::T_FEED_DELETE";


				case T_REPORT_FEED_ADD:
					return "ExecutorThreadRequestType::T_REPORT_FEED_ADD";

            	case T_USER_FORBID_QUERY:
					return "ExecutorThreadRequestType::T_USER_FORBID_QUERY";


				case T_FOLLOW_DETAIL_QUERY:
					return "ExecutorThreadRequestType::T_FOLLOW_DETAIL_QUERY";

				case T_FOLLOW_LIST_QUERY:
					return "ExecutorThreadRequestType::T_FOLLOW_LIST_QUERY";

				case T_FOLLOW_COMMENT_DETAIL_QUERY:
                    return "ExecutorThreadRequestType::T_FOLLOW_COMMENT_DETAIL_QUERY";

                case T_FOLLOW_COMMENT_QUERY:
                    return "ExecutorThreadRequestType::T_FOLLOW_COMMENT_QUERY";

				case T_FOLLOW_ADD:
					return "ExecutorThreadRequestType::T_FOLLOW_ADD";

            	case T_FOLLOW_DELETE:
					return "ExecutorThreadRequestType::T_FOLLOW_DELETE";


				case T_FEED_LIST_FAVORITE_QUERY:
					return "ExecutorThreadRequestType::T_FEED_LIST_FAVORITE_QUERY";

                case T_FAVORITE_QUERY:
                    return "ExecutorThreadRequestType::T_FAVORITE_QUERY";

				case T_FAVORITE_ADD:
					return "ExecutorThreadRequestType::T_FAVORITE_ADD";

            	case T_FAVORITE_DELETE:
					return "ExecutorThreadRequestType::T_FAVORITE_DELETE";


				case T_FEED_LIST_COLLECT_QUERY:
					return "ExecutorThreadRequestType::T_FEED_LIST_COLLECT_QUERY";

                case T_COLLECT_QUERY:
                    return "ExecutorThreadRequestType::T_COLLECT_QUERY";

				case T_COLLECT_ADD:
					return "ExecutorThreadRequestType::T_COLLECT_ADD";

            	case T_COLLECT_DELETE:
					return "ExecutorThreadRequestType::T_COLLECT_DELETE";


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

				case E_FEED_DELETED:
					return "ExecutorThreadRequestType::E_FEED_DELETED";

				case E_REPLY_TO_SELF_INVALID:
					return "ExecutorThreadRequestType::E_REPLY_TO_SELF_INVALID";

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

			  if(m_request_type == ExecutorThreadRequestType::T_FEED_DETAIL_QUERY)
			  {
				  	ss << ", feed_id_list: [";
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
			  else if(m_request_type == ExecutorThreadRequestType::T_FEED_QUERY)
			  {
					ss << ", appid: " << m_feed_index_on_appid.m_pa_appid_md5
						<< ", openid: " << m_feed_index_on_appid.m_openid_md5
						<< ", begin_feed_id: " << m_feed_index_on_appid.m_begin_feed_id
						<< ", limit: " << m_feed_index_on_appid.m_limit
						<< ", type: " << m_feed_index_on_appid.m_type;
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FEED_ADD || m_request_type == ExecutorThreadRequestType::T_FEED_DELETE)
			  {
              		ss << ", feed_info: " << m_feed_info.ToString();
			  }

			  else if(m_request_type == ExecutorThreadRequestType::T_REPORT_FEED_ADD)
			  {
					ss << ", report_feed: " << m_feed_report.ToString();
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_USER_FORBID_QUERY)
			  {
					ss << ", user_forbid: " << m_user_forbid.ToString();
			  }
			  
			  else if(m_request_type == ExecutorThreadRequestType::T_FOLLOW_DETAIL_QUERY)
			  {
					ss << ", follow_id_list: [ ";
					for(size_t i = 0; i < m_follow_id_list.size(); i++)
					{
						if(i > 0)
						{
							ss << ", ";
						}
						ss << m_follow_id_list[i];
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FOLLOW_LIST_QUERY)
			  {
			  		ss << " fuck out " ;
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FOLLOW_COMMENT_DETAIL_QUERY)
			  {
					ss << ", comment_id_list: [ ";
					for(size_t i = 0; i < m_comment_id_list.size(); i++)
					{
						if(i > 0)
						{
							ss << ", ";
						}
						ss << m_comment_id_list[i];
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FOLLOW_COMMENT_QUERY)
			  {
					ss << ", appid: " << m_comment_index_on_feedid.m_pa_appid_md5
						<< ", openid: " << m_comment_index_on_feedid.m_openid_md5
						<< ", feed_id: " << m_comment_index_on_feedid.m_feed_id
						<< ", begin_comment_id: " << m_comment_index_on_feedid.m_begin_comment_id
						<< ", limit: " << m_comment_index_on_feedid.m_limit
						;
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FOLLOW_ADD || m_request_type == ExecutorThreadRequestType::T_FOLLOW_DELETE)
			  {
					ss << ", follow_info: " << m_follow_info.ToString();
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FEED_LIST_FAVORITE_QUERY)
			  {
					ss << ", appid: " << m_feed_list_favorite.m_pa_appid_md5
						<< ", openid: " << m_feed_list_favorite.m_openid_md5
						<< ", feed_id_list: [";
					for(size_t i = 0; i < m_feed_list_favorite.m_feed_id_list.size(); i++)
					{
						if(i > 0)
						{
							ss << ", ";
						}
						ss << m_feed_list_favorite.m_feed_id_list[i];
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FAVORITE_QUERY)
			  {
				  	ss << ", appid: " << m_favorite_list.m_pa_appid_md5
					  << ", openid: " << m_favorite_list.m_openid_md5
					  << ", feed_id: " << m_favorite_list.m_feed_id
					  << ", begin_create_ts: " << m_favorite_list.m_begin_create_ts
					  << ", limit: " << m_favorite_list.m_limit
					  ;
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FAVORITE_ADD || m_request_type == ExecutorThreadRequestType::T_FAVORITE_DELETE)
			  {
					ss << ", favorite_info: " << m_favorite_info.ToString();
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FEED_LIST_COLLECT_QUERY)
			  {
					ss << ", appid: " << m_feed_list_collect.m_pa_appid_md5
						<< ", openid: " << m_feed_list_collect.m_openid_md5
						<< ", feed_id_list: [";
					for(size_t i = 0; i < m_feed_list_collect.m_feed_id_list.size(); i++)
					{
						if(i > 0)
						{
							ss << ", ";
						}
						ss << m_feed_list_collect.m_feed_id_list[i];
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_COLLECT_QUERY)
			  {
				  	ss << ", appid: " << m_collect_list.m_pa_appid_md5
					  << ", openid: " << m_collect_list.m_openid_md5
					  << ", begin_create_ts: " << m_collect_list.m_begin_create_ts
					  << ", limit: " << m_collect_list.m_limit
					  ;
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_COLLECT_ADD || m_request_type == ExecutorThreadRequestType::T_COLLECT_DELETE)
			  {
					ss << ", collect_info: " << m_collect_info.ToString();
			  }
              ss<<"  }  ";


            return ss.str();
        }

    public:
    	uint32_t m_request_type;
        uint32_t m_fsm_id;

		//for add & delete
        FeedInfo m_feed_info;
        FeedReport m_feed_report;
        UserForbid m_user_forbid;
		FollowInfo m_follow_info;
		FavoriteInfo m_favorite_info; // also for query
		CollectInfo m_collect_info;

		//for query
		FeedIndexOnAppid m_feed_index_on_appid;
		CommentIndexOnFeedid m_comment_index_on_feedid;  //feed_id & begin_comment_id  >> commnet_tree
		vector<uint64_t> m_feed_id_list;  // (query feed detail) id_lsit >> feed_info_list
		vector<uint64_t> m_follow_id_list;// (query follow detail) id_list >> follow_info_list
		vector<uint64_t> m_comment_id_list;// (query follow_comment detail) id_list >> [follow_comment]_info_list
		FeedListFavorite m_feed_list_favorite; // query feed_list favorite
		FavoriteList m_favorite_list;
		FeedListCollect m_feed_list_collect; 	// query feed_list collect
		CollectList m_collect_list;
		/*
		uint64_t m_pa_appid_md5;
		uint64_t m_openid_md5;
		uint64_t m_feed_id;
		uint64_t m_follow_id;

		uint64_t m_begin_feed_id;
		uint64_t m_begin_comment_id;
		uint64_t m_begin_openid_md5;

		uint64_t m_limit;
		*/

		//
        bool m_need_reply;
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
            , m_total(0)
        {
        }

        const std::string ToString() const
        {
            std::ostringstream ss;
            ss << "{"
			   << " req_type: " << ExecutorThreadRequestType::type_string(m_request_type)
               << ", fsm_id: " << m_fsm_id
               << ", result_code: " << ExecutorThreadRequestType::err_string(m_result_code) << " ";

			  if(m_request_type == ExecutorThreadRequestType::T_FEED_DETAIL_QUERY)
			  {
				  	ss << ", feed_list: [ ";
					for(size_t i = 0; i < m_feed_list.size(); i++)
					{
						if(i > 0) ss << ", ";
						ss << m_feed_list[i].ToString();
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FEED_QUERY)
			  {
			  		ss << ", total = " << m_total;
					ss << ", feed_list: [ ";
					for(size_t i = 0; i < m_feed_list.size(); i++)
					{
						if(i > 0) ss << ", ";
						ss << m_feed_list[i].ToString();
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FEED_ADD || m_request_type == ExecutorThreadRequestType::T_FEED_DELETE)
			  {
              		ss << ", feed_info: " << m_feed_info.ToString();
			  }
			  
			  else if(m_request_type == ExecutorThreadRequestType::T_USER_FORBID_QUERY)
			  {
					ss << ", user_forbid: " << m_user_forbid.ToString();
			  }
			  
			  else if(m_request_type == ExecutorThreadRequestType::T_FOLLOW_DETAIL_QUERY)
			  {
					ss << ", follow_comment_list: [ ";
					for(size_t i = 0; i < m_follow_list.size(); i++)
					{
						if(i > 0) ss << ", ";
						ss << m_follow_list[i].ToString();
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FOLLOW_LIST_QUERY)
			  {
			  		ss << ",  fuck out " ;
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FOLLOW_COMMENT_DETAIL_QUERY)
			  {
					ss << ", follow_comment_list: [ ";
					for(size_t i = 0; i < m_follow_comment_list.size(); i++)
					{
						if(i > 0) ss << ", ";
						ss << "{comment: " << m_follow_comment_list[i].m_comment.ToString();
						ss << ", reply_list: [ ";
						for(size_t j = 0; j < m_follow_comment_list[i].m_reply_list.size(); j++)
						{
							if(i > 0) ss << ", ";
							ss << "{" << m_follow_comment_list[i].m_reply_list[j].ToString() << "}";
						}
						ss << " ] ";
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FOLLOW_COMMENT_QUERY)
			  {
					ss << ", follow_comment_list: [ ";
					for(size_t i = 0; i < m_follow_comment_list.size(); i++)
					{
						if(i > 0) ss << ", ";
						ss << "{comment: " << m_follow_comment_list[i].m_comment.ToString();
						ss << ", reply_list: [ ";
						for(size_t j = 0; j < m_follow_comment_list[i].m_reply_list.size(); j++)
						{
							if(i > 0) ss << ", ";
							ss << "{" << m_follow_comment_list[i].m_reply_list[j].ToString() << "}";
						}
						ss << " ] ";
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FOLLOW_ADD || m_request_type == ExecutorThreadRequestType::T_FOLLOW_DELETE)
			  {
					ss << ", follow_info: " << m_follow_info.ToString();
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FEED_LIST_FAVORITE_QUERY)
			  {
					ss << ", appid: " << m_feed_list_favorite.m_pa_appid_md5
						<< ", openid: " << m_feed_list_favorite.m_openid_md5
						<< ", feed_id_list: [";
					for(size_t i = 0; i < m_feed_list_favorite.m_feed_id_list.size(); i++)
					{
						if(i > 0)
						{
							ss << ", ";
						}
						ss << "{" << m_feed_list_favorite.m_feed_id_list[i] << ": " << m_feed_list_favorite.m_favorite_list[i] << "}";
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FAVORITE_QUERY)
			  {
			  		ss << ", {openid, ts}: [ ";
					for(size_t i = 0; i < m_favorite_list.m_openid_md5_list.size(); i++)
					{
						if(i > 0)
						{
							ss << ", ";
						}
						ss << "{" << m_favorite_list.m_openid_md5_list[i] << ", " << m_favorite_list.m_create_ts_list[i] << "}";
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_FAVORITE_ADD || m_request_type == ExecutorThreadRequestType::T_FAVORITE_DELETE)
			  {
					ss << ", favorite_info: " << m_favorite_info.ToString();
			  }

			  else if(m_request_type == ExecutorThreadRequestType::T_FEED_LIST_COLLECT_QUERY)
			  {
					ss << ", appid: " << m_feed_list_collect.m_pa_appid_md5
						<< ", openid: " << m_feed_list_collect.m_openid_md5
						<< ", feed_id_list: [";
					for(size_t i = 0; i < m_feed_list_collect.m_feed_id_list.size(); i++)
					{
						if(i > 0)
						{
							ss << ", ";
						}
						ss << "{" << m_feed_list_collect.m_feed_id_list[i] << ": " << m_feed_list_collect.m_collect_list[i] << "}";
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_COLLECT_QUERY)
			  {
			  		ss << ", {openid, ts}: [ ";
					for(size_t i = 0; i < m_collect_list.m_feed_id_list.size(); i++)
					{
						if(i > 0)
						{
							ss << ", ";
						}
						ss << "{" << m_collect_list.m_feed_id_list[i] << ", " << m_collect_list.m_create_ts_list[i] << "}";
					}
					ss << "] ";
			  }
			  else if(m_request_type == ExecutorThreadRequestType::T_COLLECT_ADD || m_request_type == ExecutorThreadRequestType::T_COLLECT_DELETE)
			  {
					ss << ", collect_info: " << m_collect_info.ToString();
			  }

              ss<<"  }  ";


            return ss.str();
        }

    public:
        uint32_t m_request_type;
        uint32_t m_fsm_id;
        uint32_t m_result_code;

		//for add
        FeedInfo m_feed_info;
		FollowInfo m_follow_info;
		FavoriteInfo m_favorite_info;
		CollectInfo m_collect_info;

		/*
		uint64_t m_count;

		*/
		/*
		//for query
		uint64_t m_next_feed_id;
		uint64_t m_next_comment_id;
		uint64_t m_next_openid_md5;
		*/

		//for query
		vector<FeedInfo> m_feed_list;
		vector<FollowInfo> m_follow_list;
		vector<FollowCommentInfo> m_follow_comment_list;
//		vector<FavoriteInfo> m_favorite_list;
		vector<uint64_t> m_openid_md5_list;
		FeedListFavorite m_feed_list_favorite;
		FavoriteList m_favorite_list;
		uint64_t m_total;
//		uint64_t m_joinTopicNum;	//参与话题数
		FeedListCollect m_feed_list_collect;
		CollectList m_collect_list;
		UserForbid m_user_forbid;
    };

    typedef lce::memory::DuplexQueue<ExecutorThreadRequestElement, ExecutorThreadResponseElement> ExecutorThreadQueue;

#endif


