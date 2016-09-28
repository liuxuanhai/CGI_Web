#ifndef _HX_HP_MANAGER_COMM_STRUCT_H_
#define _HX_HP_MANAGER_COMM_STRUCT_H_

#include "msg.pb.h"
#include "cgi/cgi.h"
#include "../cgi_ret_code_def.h"


class TableBase
{
	public:
		enum TABLE_BASE_RET
		{
			TABLE_BASE_RET_OK = 0,
			TABLE_BASE_RET_NOT_EXIST = 10001,
			
			TABLE_BASE_RET_ALREADY_EXIST = 10003,
			
		};

	public:
		string m_table_name;

		static lce::cgi::CMysql g_user_mysql;
		static lce::cgi::CMysql g_feeds_mysql;
		static lce::cgi::CMysql g_pa_mysql;

	public:
		TableBase(const std::string& strTableName): m_table_name(strTableName)
		{

		}

		string IntToHexStr(uint32_t num)
		{
		    char index[4];
			sprintf(index, "%02x", num);
			return string(index);
		}
	DECL_LOGGER(logger);
};

class ManagerInfo : public TableBase
{
	public:
		std::string m_uin;
		std::string m_passwd;
		uint64_t m_pa_appid_md5;
		uint64_t m_openid_md5;
		uint64_t m_create_ts;
		uint64_t m_admin;

	public:
		ManagerInfo(const std::string& strTableName): TableBase(strTableName)
		{

		}
		ManagerInfo(): TableBase("")
		{

		}
	public:
		int SelectFromDB(std::string& strErrMsg);
		int DeleteFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		lce::cgi::CAnyValue ToAnyValue();
};

class ManagerInfoIndexByOpenidMd5 : public TableBase
{
	public:
		std::string m_uin;
		std::string m_passwd;
		uint64_t m_pa_appid_md5;
		uint64_t m_openid_md5;
		uint64_t m_create_ts;
		uint64_t m_admin;

	public:
		ManagerInfoIndexByOpenidMd5(const std::string& strTableName): TableBase(strTableName)
		{

		}
	public:
		int SelectFromDB(std::string& strErrMsg);
};


class ManagerInfoList : public TableBase
{
	public:
		uint64_t m_pa_appid_md5;
		vector<ManagerInfo> m_manager_info_list;

	public:
		ManagerInfoList(const std::string& strTableName): TableBase(strTableName)
		{

		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};

class UserForbidInfo : public TableBase
{
	public:
		uint64_t m_pa_appid_md5;
		uint64_t m_openid_md5;
		uint64_t m_until_ts;

	public:
		UserForbidInfo(const std::string& strTableName): TableBase(strTableName)
		{
		
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
};

class UserForbidInfoList : public TableBase
{
	public:
		uint64_t m_pa_appid_md5;
		vector<UserForbidInfo> m_user_forbid_info_list;
		
	public:
		UserForbidInfoList(const std::string& strTableName): TableBase(strTableName)
		{
		
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};


class FeedReportInfo 
{
	public:
		uint64_t m_pa_appid_md5;
		uint64_t m_openid_md5_from;
		uint64_t m_openid_md5_to;
		uint64_t m_feed_id;
		uint64_t m_report_ts;
		string m_report_reason;

	public:
		lce::cgi::CAnyValue ToAnyValue()const;
};

class FeedReportInfoListIndexByOpenid : public TableBase
{
	public:
		uint64_t m_pa_appid_md5;
		uint64_t m_openid_md5_to;
		vector<uint64_t> m_feed_id_list;

	public:
		FeedReportInfoListIndexByOpenid(const std::string& strTableName): TableBase(strTableName)
		{
		
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};

class FeedReportInfoListIndexByFeedid : public TableBase
{
	public:
		uint64_t m_pa_appid_md5;
		
		uint64_t m_feed_id;
		vector<FeedReportInfo> m_feed_report_info_list;

	public:
		FeedReportInfoListIndexByFeedid(const std::string& strTableName): TableBase(strTableName)
		{
		
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};

class FeedInfo : public TableBase
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
	
		FeedInfo(const std::string& strTableName): TableBase(strTableName)
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
	
	
		int SelectFromDB(std::string& strErrMsg);
		int InsertToDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		int DeleteFromDB(std::string& strErrMsg);
	
		std::string ToString() const;
		
};

class FeedIndexOnAppid : public TableBase
{
public:
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5;
	uint64_t m_feed_id;
	uint64_t m_feed_type;
	uint64_t m_origin_feed_id;

	vector<uint64_t> m_feed_id_list;

	//for query
	uint64_t m_begin_feed_id;
	uint64_t m_next_feed_id;
	uint64_t m_limit;
	uint64_t m_type;

	uint64_t m_total;	//参与话题数


	FeedIndexOnAppid(const std::string& strTableName): TableBase(strTableName), m_total(0)
	{
		m_feed_id_list.clear();
	}

	int SelectFromDB(std::string& strErrMsg);
    int UpdateToDB(std::string& strErrMsg);
    int DeleteFromDB(std::string& strErrMsg);

	std::string ToString() const;

};

class PaInfo : public TableBase
{
public:
	uint64_t m_appid_md5;
	string m_appid;
	string m_name;
	string m_simple_desc;
	uint64_t m_portrait_pic_id;
	uint64_t m_create_ts;
	uint64_t m_num_topic_feed;
	uint64_t m_num_banner_feed;

	PaInfo(const std::string& strTableName): TableBase(strTableName)
	{
	}

	int SelectFromDB(std::string& strErrMsg);
	int UpdateToDB(std::string& strErrMsg);
	int DeleteFromDB(std::string& strErrMsg);

	std::string ToString() const;
	lce::cgi::CAnyValue ToAnyValue();
};

#endif

