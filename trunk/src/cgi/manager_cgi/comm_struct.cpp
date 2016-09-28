#include "comm_struct.h"
#include "comm_def.h"

lce::cgi::CMysql TableBase::g_user_mysql;
lce::cgi::CMysql TableBase::g_feeds_mysql;
lce::cgi::CMysql TableBase::g_pa_mysql;
IMPL_LOGGER(TableBase,logger);


int ManagerInfo::SelectFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "select * from " << m_table_name << " where uin = '" << sql_escape(m_uin) << "'";
	if(!g_user_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_user_mysql.GetErrMsg();
        return -1;
    }
    if(g_user_mysql.GetRowCount() && g_user_mysql.Next())
    {
        m_passwd = g_user_mysql.GetRow(1);
        m_pa_appid_md5 = strtoul(g_user_mysql.GetRow(2), NULL, 10);
        m_openid_md5 = strtoul(g_user_mysql.GetRow(3), NULL, 10);
        m_create_ts = strtoul(g_user_mysql.GetRow(4), NULL, 10);
        m_admin = strtoul(g_user_mysql.GetRow(5), NULL, 10);
        return TABLE_BASE_RET_OK;
    }
    return TABLE_BASE_RET_NOT_EXIST;
}

int ManagerInfo::DeleteFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "delete from " << m_table_name << " where uin = '" << sql_escape(m_uin) << "'";
	if(!g_user_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_user_mysql.GetErrMsg();
        return -1;
    }
    return TABLE_BASE_RET_OK;
}

int ManagerInfo::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "insert into " << m_table_name
			<< " set uin = '" << sql_escape(m_uin) << "'"
			<< ", passwd = '" << sql_escape(m_passwd) << "'"
			<< ", pa_appid_md5 = " << m_pa_appid_md5
			<< ", openid_md5 = " << m_openid_md5
			<< ", create_ts = " << m_create_ts
			<< ", admin = " << m_admin
			<< " on duplicate key"
			<< " update passwd = '" << sql_escape(m_passwd) << "'";

	if(!g_user_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_user_mysql.GetErrMsg();
			return -1;
	}

	return TABLE_BASE_RET_OK;
}

std::string ManagerInfo::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss << "{"
        << "uin: " << m_uin
        << ", passwd: " << m_passwd
        << ", pa_appid_md5: " << m_pa_appid_md5
        << ", openid_md5: " << m_openid_md5
        << ", create_ts: " << m_create_ts
        << ", admin: " << m_admin
        << "}";

    return oss.str();
}

lce::cgi::CAnyValue ManagerInfo::ToAnyValue()
{
	lce::cgi::CAnyValue stAnyValue;
	stAnyValue["uin"] = m_uin;
	//stAnyValue["passwd"] = m_passwd;
	stAnyValue["pa_appid_md5"] = int_2_str(m_pa_appid_md5);
	stAnyValue["openid_md5"] = int_2_str(m_openid_md5);
	stAnyValue["create_ts"] = m_create_ts;
	stAnyValue["admin"] = m_admin;
	
	return stAnyValue;
}

int ManagerInfoIndexByOpenidMd5::SelectFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "select * from " << m_table_name << " where openid_md5 = " << m_openid_md5;
	if(!g_user_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_user_mysql.GetErrMsg();
        return -1;
    }
    if(g_user_mysql.GetRowCount() && g_user_mysql.Next())
    {
    	m_uin = g_user_mysql.GetRow(0);
        m_passwd = g_user_mysql.GetRow(1);
        m_pa_appid_md5 = strtoul(g_user_mysql.GetRow(2), NULL, 10);
        //m_openid_md5 = strtoul(g_user_mysql.GetRow(3), NULL, 10);
        m_create_ts = strtoul(g_user_mysql.GetRow(4), NULL, 10);
        m_admin = strtoul(g_user_mysql.GetRow(5), NULL, 10);
        return TABLE_BASE_RET_OK;
    }
    return TABLE_BASE_RET_NOT_EXIST;
}


int ManagerInfoList::SelectFromDB(std::string& strErrMsg)
{
	m_manager_info_list.clear();
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "select * from " << m_table_name
		<< " where pa_appid_md5 = " << m_pa_appid_md5
		<< " and admin = " << MANAGER_TYPE_NORMAL
		<< " order by create_ts asc";
	if(!g_user_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_user_mysql.GetErrMsg();
        return -1;
    }
	if(!g_user_mysql.GetRowCount())
	{
		return TABLE_BASE_RET_NOT_EXIST;
	}
    while(g_user_mysql.GetRowCount() && g_user_mysql.Next())
    {
    	ManagerInfo stManagerInfo;
		stManagerInfo.m_uin = g_user_mysql.GetRow(0);
        stManagerInfo.m_passwd = g_user_mysql.GetRow(1);
        stManagerInfo.m_pa_appid_md5 = strtoul(g_user_mysql.GetRow(2), NULL, 10);
        stManagerInfo.m_openid_md5 = strtoul(g_user_mysql.GetRow(3), NULL, 10);
        stManagerInfo.m_create_ts = strtoul(g_user_mysql.GetRow(4), NULL, 10);
        stManagerInfo.m_admin = strtoul(g_user_mysql.GetRow(5), NULL, 10);
		m_manager_info_list.push_back(stManagerInfo);
    }
	return TABLE_BASE_RET_OK;
}

std::string ManagerInfoList::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss << "pa_appid_md5: " << m_pa_appid_md5
        << ", list: [";

    for(size_t i = 0; i < m_manager_info_list.size(); i++)
    {
        if(i)
        {
            oss << ", ";
        }
        oss << m_manager_info_list[i].ToString();
    }
    oss << " ]";
    return oss.str();
}

int UserForbidInfo::SelectFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "select * from " << m_table_name
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<< " where pa_appid_md5 = " << m_pa_appid_md5
		<< " and openid_md5 = " << m_openid_md5;
		
	if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_user_mysql.GetErrMsg();
        return -1;
    }


    if(g_feeds_mysql.GetRowCount() && g_feeds_mysql.Next())
    {
    	m_until_ts = strtoul(g_feeds_mysql.GetRow(2), NULL, 10);
		return TABLE_BASE_RET_OK;
    }
	return TABLE_BASE_RET_NOT_EXIST;
}

int UserForbidInfo::UpdateToDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "insert into "<< m_table_name
			<< IntToHexStr(m_pa_appid_md5 % 256)
			<< " set pa_appid_md5 = " << m_pa_appid_md5
			<< ", openid_md5 = " << m_openid_md5
			<< ", until_ts = " << m_until_ts
			<< " on duplicate key update"
			<< " until_ts = " << m_until_ts;
	strErrMsg = ossSql.str();
	if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_feeds_mysql.GetErrMsg();
        return -1;
    }

	return TABLE_BASE_RET_OK;
}

lce::cgi::CAnyValue FeedReportInfo::ToAnyValue()const
{
	lce::cgi::CAnyValue stAnyValue;
	stAnyValue["pa_appid_md5"] = int_2_str(m_pa_appid_md5);
	stAnyValue["openid_md5_from"] = int_2_str(m_openid_md5_from);
	stAnyValue["openid_md5_to"] = int_2_str(m_openid_md5_to);
	stAnyValue["feed_id"] = int_2_str(m_feed_id);
	stAnyValue["report_ts"] = m_report_ts;
	stAnyValue["report_reason"] = m_report_reason;
	return stAnyValue;
}



int UserForbidInfoList::SelectFromDB(std::string& strErrMsg)
{
	m_user_forbid_info_list.clear();
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "select * from " << m_table_name
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<< " where pa_appid_md5 = " << m_pa_appid_md5;
		
	if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_feeds_mysql.GetErrMsg();
        return -1;
    }

	if(!g_feeds_mysql.GetRowCount())
	{
		return TABLE_BASE_RET_NOT_EXIST;
	}
	
    while(g_feeds_mysql.GetRowCount() && g_feeds_mysql.Next())
    {
    	UserForbidInfo stUserForbidInfo(m_table_name);
    	stUserForbidInfo.m_openid_md5 = strtoul(g_feeds_mysql.GetRow(1), NULL, 10);
    	stUserForbidInfo.m_until_ts = strtoul(g_feeds_mysql.GetRow(2), NULL, 10);
		m_user_forbid_info_list.push_back( stUserForbidInfo );
    }
	return TABLE_BASE_RET_OK;
}

//std::string UserForbidInfoList::ToString()
//{
//}


int FeedReportInfoListIndexByOpenid::SelectFromDB(std::string& strErrMsg)
{
	m_feed_id_list.clear();
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "select distinct(feed_id) from " << m_table_name
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<< " where pa_appid_md5 = " << m_pa_appid_md5
		<< " and openid_md5_to = " << m_openid_md5_to;
		
	if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_feeds_mysql.GetErrMsg();
        return -1;
    }

	if(!g_feeds_mysql.GetRowCount())
	{
		return TABLE_BASE_RET_NOT_EXIST;
	}
	
    while(g_feeds_mysql.GetRowCount() && g_feeds_mysql.Next())
    {
    	m_feed_id_list.push_back( strtoul( g_feeds_mysql.GetRow(0), NULL, 10) );
    }
    return TABLE_BASE_RET_OK;
}

//std::string FeedReportInfoListIndexByOpenid::ToString()
//{
//}


int FeedReportInfoListIndexByFeedid::SelectFromDB(std::string& strErrMsg)
{
	m_feed_report_info_list.clear();
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "select * from " << m_table_name
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<< " where pa_appid_md5 = " << m_pa_appid_md5
		<< " and feed_id = " << m_feed_id;
		
	if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_feeds_mysql.GetErrMsg();
        return -1;
    }

	if(!g_feeds_mysql.GetRowCount())
	{
		return TABLE_BASE_RET_NOT_EXIST;
	}
	
    while(g_feeds_mysql.GetRowCount() && g_feeds_mysql.Next())
    {
    	FeedReportInfo stFeedReportInfo;
    	stFeedReportInfo.m_openid_md5_from = strtoul(g_feeds_mysql.GetRow(1), NULL, 10);
    	stFeedReportInfo.m_openid_md5_to = strtoul(g_feeds_mysql.GetRow(2), NULL, 10);
    	stFeedReportInfo.m_feed_id = strtoul(g_feeds_mysql.GetRow(3), NULL, 10);
    	stFeedReportInfo.m_report_ts = strtoul(g_feeds_mysql.GetRow(4), NULL, 10);
    	stFeedReportInfo.m_report_reason = g_feeds_mysql.GetRow(5);
		m_feed_report_info_list.push_back( stFeedReportInfo );
    }
    return TABLE_BASE_RET_OK;
}

//std::string FeedReportInfoListIndexByFeedid::ToString()
//{
//}



int FeedInfo::SelectFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << m_table_name
		<< IntToHexStr(m_feed_id % 256)
		<<" where feed_id=" << m_feed_id;

    if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_feeds_mysql.GetErrMsg();
        return -1;
    }

    if(g_feeds_mysql.GetRowCount() && g_feeds_mysql.Next())
    {
		m_pa_appid_md5 = strtoul(g_feeds_mysql.GetRow(1), NULL, 10);
		m_openid_md5 = strtoul(g_feeds_mysql.GetRow(2), NULL, 10);
		m_cover_pic_id = strtoul(g_feeds_mysql.GetRow(3), NULL, 10);
		m_pic_id_list = g_feeds_mysql.GetRow(4);
		m_content = g_feeds_mysql.GetRow(5);
		m_num_favorite = strtoul(g_feeds_mysql.GetRow(6), NULL, 10);
		m_num_share = strtoul(g_feeds_mysql.GetRow(7), NULL, 10);
		m_num_comment = strtoul(g_feeds_mysql.GetRow(8), NULL, 10);
		m_num_read = strtoul(g_feeds_mysql.GetRow(9), NULL, 10);
		m_create_ts = strtoul(g_feeds_mysql.GetRow(10), NULL, 10);
		m_del_ts = strtoul(g_feeds_mysql.GetRow(11), NULL, 10);
		m_feed_type = strtoul(g_feeds_mysql.GetRow(12), NULL, 10);
		m_extra_data_0 = g_feeds_mysql.GetRow(13);
		m_extra_data_1 = g_feeds_mysql.GetRow(14);
		m_extra_data_2 = g_feeds_mysql.GetRow(15);

        return TABLE_BASE_RET_OK;
    }

	return TABLE_BASE_RET_NOT_EXIST;
}

int FeedInfo::InsertToDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << m_table_name
		<< IntToHexStr(m_feed_id % 256)
    	<<" set feed_id=" << m_feed_id
		<<", pa_appid_md5=" << m_pa_appid_md5
		<<", openid_md5=" << m_openid_md5
		<<", cover_pic_id=" << m_cover_pic_id
		<<", pic_id_list='" << lce::cgi::CMysql::MysqlEscape(m_pic_id_list) << "'"
		<<", content='" << lce::cgi::CMysql::MysqlEscape(m_content) << "'"
		<<", num_favorite=" << m_num_favorite
		<<", num_share=" << m_num_share
		<<", num_comment=" << m_num_comment
		<<", num_read=" << m_num_read
		<<", create_ts=" << m_create_ts
		<<", del_ts=" << m_del_ts
		<<", feed_type=" << m_feed_type
		<<", extra_data_0='" << lce::cgi::CMysql::MysqlEscape(m_extra_data_0) << "'"
		<<", extra_data_1='" << lce::cgi::CMysql::MysqlEscape(m_extra_data_1) << "'"
		<<", extra_data_2='" << lce::cgi::CMysql::MysqlEscape(m_extra_data_2) << "'"
	;

    if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_feeds_mysql.GetErrMsg();
        return -1;
    }

	return TABLE_BASE_RET_OK;
}


int FeedInfo::UpdateToDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << m_table_name
		<< IntToHexStr(m_feed_id % 256)
    	<<" set feed_id=" << m_feed_id
		<<", pa_appid_md5=" << m_pa_appid_md5
		<<", openid_md5=" << m_openid_md5
		<<", cover_pic_id=" << m_cover_pic_id
		<<", pic_id_list='" << lce::cgi::CMysql::MysqlEscape(m_pic_id_list) << "'"
		<<", content='" << lce::cgi::CMysql::MysqlEscape(m_content) << "'"
		<<", num_favorite=" << m_num_favorite
		<<", num_share=" << m_num_share
		<<", num_comment=" << m_num_comment
		<<", num_read=" << m_num_read
		<<", create_ts=" << m_create_ts
		<<", del_ts=" << m_del_ts
		<<", feed_type=" << m_feed_type
		<<", extra_data_0='" << lce::cgi::CMysql::MysqlEscape(m_extra_data_0) << "'"
		<<", extra_data_1='" << lce::cgi::CMysql::MysqlEscape(m_extra_data_1) << "'"
		<<", extra_data_2='" << lce::cgi::CMysql::MysqlEscape(m_extra_data_2) << "'"

		<<" on duplicate key"
		<<" update pa_appid_md5=" << m_pa_appid_md5
		<<", openid_md5=" << m_openid_md5
		<<", cover_pic_id=" << m_cover_pic_id
		<<", pic_id_list='" << lce::cgi::CMysql::MysqlEscape(m_pic_id_list) << "'"
		<<", content='" << lce::cgi::CMysql::MysqlEscape(m_content) << "'"
		<<", num_favorite=" << m_num_favorite
		<<", num_share=" << m_num_share
		<<", num_comment=" << m_num_comment
		<<", num_read=" << m_num_read
		<<", create_ts=" << m_create_ts
		<<", del_ts=" << m_del_ts
		<<", feed_type=" << m_feed_type
		<<", extra_data_0='" << lce::cgi::CMysql::MysqlEscape(m_extra_data_0) << "'"
		<<", extra_data_1='" << lce::cgi::CMysql::MysqlEscape(m_extra_data_1) << "'"
		<<", extra_data_2='" << lce::cgi::CMysql::MysqlEscape(m_extra_data_2) << "'"
	;


    if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_feeds_mysql.GetErrMsg();
        return -1;
    }

	return TABLE_BASE_RET_OK;
}

int FeedInfo::DeleteFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;

    ossSql.str("");
    ossSql << "delete from " << m_table_name
			<< IntToHexStr(m_feed_id % 256)
			<<" where feed_id=" << m_feed_id;

    if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_feeds_mysql.GetErrMsg();
        return -1;
    }
	if(!g_feeds_mysql.GetAffectedRows())
	{
		return TABLE_BASE_RET_NOT_EXIST;
	}
	return TABLE_BASE_RET_OK;
}


std::string FeedInfo::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< "feed_id:" << m_feed_id
		<< ", pa_appid_md5 = " << m_pa_appid_md5
		<< ", openid_md5 = " << m_openid_md5
		<< ", cover_pic_id = " << m_cover_pic_id
		<< ", pic_id_list = " << m_pic_id_list
		<< ", content = " << m_content
		<< ", num_favorite = " << m_num_favorite
		<< ", num_share = " << m_num_share
		<< ", num_comment = " << m_num_comment
		<< ", num_read = " << m_num_read
		<< ", create_ts = " << m_create_ts
		<< ", del_ts = " << m_del_ts
		<<", feed_type=" << m_feed_type
		<<", extra_data_0='" << m_extra_data_0 << "'"
		<<", extra_data_1='" << m_extra_data_1 << "'"
		<<", extra_data_2='" << m_extra_data_2 << "'"
		<< "}";
	return oss.str();
}


int FeedIndexOnAppid::SelectFromDB(std::string& strErrMsg)
{
	m_feed_id_list.clear();
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << m_table_name
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<<" where pa_appid_md5=" << m_pa_appid_md5;
	if(m_type == ::hoosho::commenum::FEEDS_FEED_LIST_BY_OPENID)
	{
		ossSql << " and openid_md5 = " << m_openid_md5;
	}
	//feed_type
	ossSql << " and feed_type & " << m_feed_type << " = feed_type";
	//origin feed id
	if(m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_FEED_OF_TOPIC)
	{
		ossSql << " and origin_feed_id = " << m_origin_feed_id;
	}
	if(m_begin_feed_id)
	{
		ossSql << " and feed_id < " << m_begin_feed_id;
	}
	ossSql << " order by feed_id desc limit " << m_limit;

	strErrMsg = ossSql.str();

    if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_feeds_mysql.GetErrMsg();
        return -1;
    }

	if(!g_feeds_mysql.GetRowCount())
	{
		return TABLE_BASE_RET_NOT_EXIST;
	}


    while(g_feeds_mysql.GetRowCount() && g_feeds_mysql.Next())
    {
		m_feed_id = strtoul(g_feeds_mysql.GetRow(2), NULL, 10);
		m_feed_id_list.push_back(m_feed_id);
    }


	/******************************************/

//	if(m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_FEED_OF_TOPIC)
//	{
	ossSql.str("");
	ossSql << "select count(*) from " << m_table_name
	<< IntToHexStr(m_pa_appid_md5 % 256)
	<<" where pa_appid_md5=" << m_pa_appid_md5
	<<" and feed_type & " << m_feed_type << " = feed_type";

	if(m_type == ::hoosho::commenum::FEEDS_FEED_LIST_BY_OPENID)
	{
		ossSql << " and openid_md5 = " << m_openid_md5;
	}
	if(m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_FEED_OF_TOPIC)
	{
		ossSql << " and origin_feed_id = " << m_origin_feed_id;
	}
	LOG4CPLUS_TRACE(logger, "sql = " << ossSql.str());
	if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_feeds_mysql.GetErrMsg();
        return -1;
    }

    if(!g_feeds_mysql.GetRowCount())
	{
		return TABLE_BASE_RET_NOT_EXIST;
	}


    while(g_feeds_mysql.GetRowCount() && g_feeds_mysql.Next())
    {
		m_total = strtoul(g_feeds_mysql.GetRow(0), NULL, 10);
    }

//	}

	return TABLE_BASE_RET_OK;
}

int FeedIndexOnAppid::UpdateToDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << m_table_name
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<<" set pa_appid_md5=" << m_pa_appid_md5
		<<", openid_md5=" << m_openid_md5
    	<<", feed_id=" << m_feed_id
		<<", feed_type=" << m_feed_type
		<<", origin_feed_id=" << m_origin_feed_id
	;

    if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_feeds_mysql.GetErrMsg();
        return -1;
    }

	return TABLE_BASE_RET_OK;
}

int FeedIndexOnAppid::DeleteFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "delete from " << m_table_name
			<< IntToHexStr(m_pa_appid_md5 % 256)
			<<" where feed_id=" << m_feed_id
			<<" and pa_appid_md5 =" << m_pa_appid_md5;

	LOG4CPLUS_DEBUG(logger, ossSql.str());
    if(!g_feeds_mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_feeds_mysql.GetErrMsg();
        return -1;
    }
	if(!g_feeds_mysql.GetAffectedRows())
	{
		return TABLE_BASE_RET_NOT_EXIST;
	}
	return TABLE_BASE_RET_OK;
}


std::string FeedIndexOnAppid::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< "pa_appid_md5 = " << m_pa_appid_md5
		<< ", openid_md5 = " << m_openid_md5
		<< ", feed_type=" << m_feed_type
		<< ", origin_feed_id=" << m_origin_feed_id
		<< ", total = " << m_total
		<< "feed_id_list:"
		<< "[";
	for(size_t i = 0; i < m_feed_id_list.size(); i++)
	{
		oss << m_feed_id_list[i] << ", ";
	}
	oss	<< "]"
		<< "}";
	return oss.str();
}


int PaInfo::SelectFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "select * from " << m_table_name << " where appid_md5 = " << m_appid_md5;
	if(!g_pa_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_pa_mysql.GetErrMsg();
		return -1;
	}
	if(g_pa_mysql.GetRowCount() && g_pa_mysql.Next())
	{
		m_appid = g_pa_mysql.GetRow(1);
		m_name = g_pa_mysql.GetRow(2);
		m_simple_desc = g_pa_mysql.GetRow(3);
		m_portrait_pic_id = strtoul(g_pa_mysql.GetRow(4), NULL, 10);
		m_create_ts = strtoul(g_pa_mysql.GetRow(5), NULL, 10);
		m_num_topic_feed = strtoul(g_pa_mysql.GetRow(6), NULL, 10);
		m_num_banner_feed = strtoul(g_pa_mysql.GetRow(7), NULL, 10);
		return TABLE_BASE_RET_OK;
	}
	return TABLE_BASE_RET_NOT_EXIST;
}

int PaInfo::DeleteFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "delete from " << m_table_name << " where appid_md5 = " << m_appid_md5;
	if(!g_pa_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_pa_mysql.GetErrMsg();
		return -1;
	}
	return TABLE_BASE_RET_OK;
}

int PaInfo::UpdateToDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "insert into " << m_table_name
	<< " set appid_md5 = " << m_appid_md5
	<< ", appid = '" << sql_escape(m_appid) << "'"
	<< ", name = '" << sql_escape(m_name) << "'"
	<< ", simple_desc = '" << sql_escape(m_simple_desc) << "'"
	<< ", create_ts = " << m_create_ts
	<< ", num_topic_feed = " << m_num_topic_feed
	<< ", num_banner_feed = " << m_num_banner_feed
	<< " on duplicate key"
	<< " update appid = '" << sql_escape(m_appid) << "'"
   	<< ", name = '" << sql_escape(m_name) << "'"
   	<< ", simple_desc = '" << sql_escape(m_simple_desc) << "'"
   	<< ", create_ts = " << m_create_ts
   	<< ", num_topic_feed = " << m_num_topic_feed
   	<< ", num_banner_feed = " << m_num_banner_feed;

	if(!g_pa_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_pa_mysql.GetErrMsg();
		return -1;
	}

	return TABLE_BASE_RET_OK;
}

std::string PaInfo::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
	<< "appid_md5 = " << m_appid_md5
	<< ", appid = '" << (m_appid) << "'"
	<< ", name = '" << (m_name) << "'"
	<< ", simple_desc = '" << (m_simple_desc) << "'"
	<< ", create_ts = " << m_create_ts
	<< ", num_topic_feed = " << m_num_topic_feed
	<< ", num_banner_feed = " << m_num_banner_feed
	<< "}";

	return oss.str();
}

lce::cgi::CAnyValue PaInfo::ToAnyValue()
{
    lce::cgi::CAnyValue stAnyValue;

    stAnyValue["appid_md5"] = m_appid_md5;
    stAnyValue["appid"] = m_appid;
    stAnyValue["name"] = m_name;
    stAnyValue["simple_desc"] = m_simple_desc;
    stAnyValue["create_ts"] = m_create_ts;
    stAnyValue["num_topic_feed"] = m_num_topic_feed;
    stAnyValue["num_banner_feed"] = m_num_banner_feed;
    return stAnyValue;
}

