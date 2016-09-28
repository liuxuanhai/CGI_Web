#ifndef _FEEDS_SERVER_DB_FEED_H_
#define _FEEDS_SERVER_DB_FEED_H_

#include <string>
#include <vector>
#include "cgi/cgi.h"
#include "db_base.h"

class FeedInfo : TableBase
{
public:
	uint64_t m_feed_id;
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5;
	uint64_t m_cover_pic_id;
	string m_pic_id_list;
	string m_content;
	uint64_t m_num_favorite;
	uint64_t m_num_share;
	uint64_t m_num_comment;
	uint64_t m_num_read;
	uint64_t m_create_ts;
	uint64_t m_del_ts;
	uint64_t m_feed_type;

	string m_extra_data_0;
	string m_extra_data_1;
	string m_extra_data_2;

    uint64_t m_num_feed; //当feed_type=2（话题）时有效，表示话题下的feed数

	uint64_t m_origin_feed_id;

	FeedInfo()
	{
		 m_cover_pic_id = 0;
		 m_pic_id_list = "";
		 m_content = "";
		 m_num_favorite = 0;
		 m_num_share = 0;
		 m_num_comment = 0;
		 m_num_read = 0;
		 m_create_ts = 0;
		 m_del_ts = 0;
		 m_feed_type = 0;

		 m_extra_data_0 = "";
		 m_extra_data_1 = "";
		 m_extra_data_2 = "";

		 m_num_feed = 0;

		 m_origin_feed_id = 0;
	}


	int SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg,
                  const std::string& strTableNameFeedIndexPrefix = "t_feed_index_on_appid_");
	int InsertToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString() const;
	DECL_LOGGER(logger);
};


class FeedIndexOnAppid : TableBase
{
public:
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5;
	uint64_t m_feed_id;
	uint64_t m_feed_type;
	uint64_t m_origin_feed_id;
	uint64_t m_num_favorite;

	vector<uint64_t> m_feed_id_list;

	//for query
	uint64_t m_begin_feed_id;
	uint64_t m_next_feed_id;
	uint64_t m_limit;
	uint64_t m_type;
	uint64_t m_order_type;
	uint64_t m_offset;
	
	uint64_t m_total;	//参与话题数


	FeedIndexOnAppid():m_num_favorite(0), m_total(0)
	{
		m_feed_id_list.clear();
	}

	int SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int SelectFromDBByFeedid(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString() const;

	DECL_LOGGER(logger);
};


class FeedReport : TableBase
{
public:
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5_from;
	uint64_t m_openid_md5_to;
	uint64_t m_feed_id;
	uint64_t m_report_ts;
	std::string m_report_reason;

	FeedReport()
	{
		m_pa_appid_md5 = 0;
		m_openid_md5_from = 0;
		m_openid_md5_to = 0;
		m_feed_id = 0;
		m_report_ts = 0;
		m_report_reason = "";
	}

	int UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString()const;
};

class UserForbid : TableBase
{
public:
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5;
	uint64_t m_until_ts;

	UserForbid()
	{
		m_pa_appid_md5 = 0;
		m_openid_md5 = 0;
		m_until_ts = 0;
	}

	int SelectFromDB(const std::string& strTableNmaePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString()const;
};
	
#endif
