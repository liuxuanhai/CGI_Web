#ifndef _FEEDS_SERVER_DB_FOLLOW_H_
#define _FEEDS_SERVER_DB_FOLLOW_H_

#include <string>
#include <vector>
#include "cgi/cgi.h"
#include "db_base.h"

class FollowInfo : TableBase
{
public:
	uint64_t m_follow_id;
	string m_content;
	uint64_t m_feed_id;
	uint32_t m_type;
	uint64_t m_origin_comment_id;
	uint64_t m_openid_md5_from;
	uint64_t m_openid_md5_to;
	uint64_t m_create_ts;
	uint64_t m_del_ts;

	//for reserve
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5;  // equal to md5_from

	//for add: notice
	uint64_t m_feed_owner_openid_md5; 
	uint64_t m_origin_comment_owner_openid_md5;
	
	FollowInfo()
	{
		m_content = "";
		m_type = 0;
		m_origin_comment_id = 0;
		m_openid_md5_from = 0;
		m_openid_md5_to = 0;
	 	m_create_ts = 0;
		m_del_ts = 0;
	}
	

	int SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int InsertToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString() const;
};


class CommentIndexOnFeedid : TableBase
{
public:
	uint64_t m_feed_id;
	uint64_t m_comment_id;
	vector<uint64_t> m_comment_id_list;

	//for reserve
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5;

	//for query
	uint64_t m_limit;
	uint64_t m_begin_comment_id;
	uint64_t m_next_comment_id;

	uint64_t m_total; 
	
	CommentIndexOnFeedid()
	{
		m_comment_id_list.clear();	
	}


	int SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString() const;
};

class ReplyIndexOnOriginComment : TableBase
{
public:
	uint64_t m_origin_comment_id;
	uint64_t m_reply_id;
	vector<uint64_t> m_reply_id_list;

	ReplyIndexOnOriginComment()
	{
		m_reply_id_list.clear();	
	}


	int SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString() const;
};


class FollowCommentInfo 
{
public:
	FollowInfo m_comment;
	vector<FollowInfo> m_reply_list;
};




#endif

