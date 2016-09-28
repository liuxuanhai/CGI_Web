#ifndef _FEEDS_SERVER_DB_COLLECT_H_
#define _FEEDS_SERVER_DB_COLLECT_H_

#include <string>
#include <vector>
#include "cgi/cgi.h"
#include "db_base.h"


class CollectInfo : TableBase
{
public:
	uint64_t m_pa_appid_md5;
	uint64_t m_feed_id;
	uint64_t m_openid_md5;
	uint64_t m_create_ts;
	
	CollectInfo()
	{
	}

	int SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int InsertToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString() const;
};


class CollectList : TableBase
{
public:
	uint64_t m_pa_appid_md5;
	uint64_t m_feed_id;
	uint64_t m_openid_md5;
	uint64_t m_create_ts;
	vector<uint64_t> m_feed_id_list;
	vector<uint64_t> m_create_ts_list;

	//for query
	uint64_t m_begin_create_ts;
	uint64_t m_next_create_ts;
	uint64_t m_limit;
	
	int SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	std::string ToString() const;
};


class FeedListCollect : TableBase
{
public:
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5;
	vector<uint64_t> m_feed_id_list;
	vector<uint64_t> m_collect_list;

	int SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	std::string ToString() const;
};


#endif

