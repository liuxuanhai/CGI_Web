#ifndef _NOTICE_SERVER_DAO_H_
#define _NOTICE_SERVER_DAO_H_

#include <string>
#include <vector>
#include "cgi/cgi.h"

//-----------------------------------------sys_notice--------------------------------------//
typedef struct DaoSysNotice
{
	uint64_t m_pa_appid_md5;
	uint64_t m_create_ts;
	string m_title;
	string m_content;

	static string m_table_name;

	DaoSysNotice() :
			m_pa_appid_md5(0), m_create_ts(0), m_title(""), m_content("")
	{
	}
	DaoSysNotice(uint64_t pa_appid_md5, uint64_t create_ts, string title, string content) :
			m_create_ts(create_ts), m_title(title), m_content(content)
	{
	}

	int SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int UpdateToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int InsertToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	std::string ToString() const;
} DaoSysNotice;

typedef struct DaoSysNoticeByAppidAndLimitTs
{
	uint64_t m_pa_appid_md5;
	uint64_t m_limit_lower_ts;
	uint64_t m_limit_upper_ts;
	uint32_t m_pagesize;
	std::vector<DaoSysNotice> m_dao_sys_notice_list;

	static string m_table_name;
	uint32_t m_queryflag; // 1:noticenotifysysnotice  2: sysnotice(new)
	DaoSysNoticeByAppidAndLimitTs() :
			m_pa_appid_md5(0), m_limit_lower_ts(0),m_limit_upper_ts(0),m_pagesize(0)
	{
	}
	DaoSysNoticeByAppidAndLimitTs(uint64_t pa_appid_md5, uint64_t limit_lower_ts, uint64_t limit_upper_ts, uint32_t pagesize) :
			m_pa_appid_md5(pa_appid_md5), m_limit_lower_ts(limit_lower_ts),m_limit_upper_ts(limit_upper_ts), m_pagesize(pagesize)
	{
	}

	int SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	static bool CreateTsDes(const DaoSysNotice &a, const DaoSysNotice &b)
	{
		return a.m_create_ts > b.m_create_ts;
	}

	std::string ToString() const;
} DaoSysNoticeByAppidAndLimitTs;

typedef struct DaoNoticeState
{
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5;
	uint32_t m_type; //1:sys 2:user
	uint64_t m_last_used_ts;

	static string m_table_name_prefix;

	DaoNoticeState() :
			m_pa_appid_md5(0), m_openid_md5(0), m_type(0), m_last_used_ts(0)
	{
	}
	DaoNoticeState(uint64_t pa_appid_md5, uint64_t openid_md5, uint32_t type) :
			m_pa_appid_md5(pa_appid_md5), m_openid_md5(openid_md5), m_type(type), m_last_used_ts(0)
	{
	}
	void Split();
	int SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int UpdateToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString() const;
} DaoNoticeState;

typedef struct DaoNoticeStateTypeList
{
	uint64_t m_pa_appid_md5;
	uint64_t m_openid_md5;
//	uint32_t m_type; //1:sys 2:user
	uint64_t m_last_used_ts_like;
	uint64_t m_last_used_ts_comment;
	uint64_t m_last_used_ts_follow;
//	uint64_t m_last_used_ts_message;

	static string m_table_name_prefix;

	DaoNoticeStateTypeList() :
			m_pa_appid_md5(0), m_openid_md5(0), m_last_used_ts_like(0), m_last_used_ts_comment(0), m_last_used_ts_follow(0)
	//, m_last_used_ts_message(0)
	{
	}
	DaoNoticeStateTypeList(uint64_t pa_appid_md5, uint64_t openid_md5, uint64_t last_used_ts_like, uint64_t last_used_ts_comment,
			uint64_t last_used_ts_follow, uint64_t last_used_ts_message) :
			m_pa_appid_md5(pa_appid_md5), m_openid_md5(openid_md5), m_last_used_ts_like(last_used_ts_like), m_last_used_ts_comment(
					last_used_ts_comment), m_last_used_ts_follow(last_used_ts_follow)
	//, m_last_used_ts_message(last_used_ts_message)
	{
	}

	int SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	std::string ToString() const;
} DaoNoticeStateTypeList;
//--------------------------------------------------notice_user------------------------------------//
typedef struct DaoNoticeUser
{
	uint64_t m_openid_md5;
	uint64_t m_pa_appid_md5;
	uint64_t m_create_ts;
	uint32_t m_type;
	uint32_t m_status;
	string m_extra_data_0;
	string m_extra_data_1;
	string m_extra_data_2;

	static string m_table_name_prefix;

	DaoNoticeUser() :
			m_openid_md5(0), m_pa_appid_md5(0), m_create_ts(0), m_type(0), m_status(0), m_extra_data_0(""), m_extra_data_1(""), m_extra_data_2("")
	{
	}
	DaoNoticeUser(uint64_t openid_md5, uint64_t pa_appid_md5, uint64_t create_ts, uint32_t type, uint32_t status, string extra_data_0,
			string extra_data_1, string extra_data_2) :
			m_create_ts(create_ts), m_extra_data_0(extra_data_0), m_extra_data_1(extra_data_1), m_extra_data_2(extra_data_2)
	{
	}

	int SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int UpdateToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int InsertToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	std::string ToString() const;
} DaoNoticeUser;

typedef struct DaoNoticeUserByAll
{
public:
	struct TypeStatus
	{
		uint32_t type;
		uint32_t status;
		TypeStatus(uint32_t iType, uint32_t iStatus)
		{
			type = iType;
			status = iStatus;
		}
	};

	uint64_t m_openid_md5;
	uint64_t m_pa_appid_md5;
	uint64_t m_limit_ts;
	uint64_t m_limit_ts_like;
	uint64_t m_limit_ts_comment;
	uint64_t m_limit_ts_follow;
	uint64_t m_limit_ts_message;

	uint32_t m_type;
	uint32_t m_pagesize;
	uint32_t m_queryflag; // 1:noticenotify  2: noticenotifyTypeList 3:noticeuserrecord
	std::vector<DaoNoticeUser> m_dao_notice_user_list;
	std::vector<TypeStatus> m_type_status_list;

	static string m_table_name_prefix;

	DaoNoticeUserByAll() :
			m_openid_md5(0), m_pa_appid_md5(0), m_limit_ts(0), m_type(0), m_pagesize(0), m_queryflag(0)
	{
	}
	DaoNoticeUserByAll(uint64_t openid_md5, uint64_t pa_appid_md5, uint64_t limit_ts, uint32_t type, uint32_t pagesize) :
			m_openid_md5(openid_md5), m_pa_appid_md5(pa_appid_md5), m_limit_ts(limit_ts), m_type(type), m_pagesize(pagesize), m_queryflag(0)
	{
	}

	int SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	static bool CreateTsDes(const DaoNoticeUser &a, const DaoNoticeUser &b)
	{
		return a.m_create_ts > b.m_create_ts;
	}

	std::string ToString() const;
} DaoNoticeUserByAll;
#endif

