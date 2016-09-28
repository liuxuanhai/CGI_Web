#include "db_feed.h"

IMPL_LOGGER(FeedInfo,logger)
IMPL_LOGGER(FeedIndexOnAppid,logger)

int FeedInfo::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg,
                           const std::string& strTableNameFeedIndexPrefix)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_feed_id % 256)
		<<" where feed_id=" << m_feed_id;

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

    if(mysql.GetRowCount() && mysql.Next())
    {
		m_pa_appid_md5 = strtoul(mysql.GetRow(1), NULL, 10);
		m_openid_md5 = strtoul(mysql.GetRow(2), NULL, 10);
		m_cover_pic_id = strtoul(mysql.GetRow(3), NULL, 10);
		m_pic_id_list = mysql.GetRow(4);
		m_content = mysql.GetRow(5);
		m_num_favorite = strtoul(mysql.GetRow(6), NULL, 10);
		m_num_share = strtoul(mysql.GetRow(7), NULL, 10);
		m_num_comment = strtoul(mysql.GetRow(8), NULL, 10);
		m_num_read = strtoul(mysql.GetRow(9), NULL, 10);
		m_create_ts = strtoul(mysql.GetRow(10), NULL, 10);
		m_del_ts = strtoul(mysql.GetRow(11), NULL, 10);
		m_feed_type = strtoul(mysql.GetRow(12), NULL, 10);
		m_extra_data_0 = mysql.GetRow(13);
		m_extra_data_1 = mysql.GetRow(14);
		m_extra_data_2 = mysql.GetRow(15);

        if(m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_TOPIC_FEED)
        {
            std::ostringstream ossSql;
            ossSql.str("");
            ossSql << "select count(*) from " << strTableNameFeedIndexPrefix
                << IntToHexStr(m_pa_appid_md5 % 256)
                <<" where pa_appid_md5 = " << m_pa_appid_md5
                <<" and origin_feed_id = " << m_feed_id
                <<" and feed_type = " << ::hoosho::commenum::FEEDS_FEED_TYPE_FEED_OF_TOPIC;

            if(!mysql.Query(ossSql.str()))
            {
                strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
                return DB_RET_FAIL;
            }

            if(mysql.GetRowCount() && mysql.Next())
            {
                m_num_feed = strtoul(mysql.GetRow(0), NULL, 10);;
                return DB_RET_OK;
            }
        }
        return DB_RET_OK;
    }

	return DB_RET_NOT_EXIST;
}

int FeedInfo::InsertToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
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

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}


int FeedInfo::UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
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


    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}

int FeedInfo::DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;

    ossSql.str("");
    ossSql << "delete from " << strTableNamePrefix
			<< IntToHexStr(m_feed_id % 256)
			<<" where feed_id=" << m_feed_id;

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }
	if(!mysql.GetAffectedRows())
	{
		return DB_RET_NOT_EXIST;
	}
	return DB_RET_OK;
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

int FeedIndexOnAppid::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_feed_id_list.clear();
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
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
	if(m_order_type == ::hoosho::commenum::FEEDS_FEED_LIST_ORDER_BY_TIME)
	{
		if(m_begin_feed_id)
		{
			ossSql << " and feed_id < " << m_begin_feed_id;
		}
		ossSql << " order by feed_id desc limit " << m_limit;
	}
	else if(m_order_type == ::hoosho::commenum::FEEDS_FEED_LIST_ORDER_BY_FAVORITE)
	{
		ossSql << " order by num_favorite desc limit " << m_offset << "," << m_limit;
	}
	
	strErrMsg = ossSql.str();

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	if(!mysql.GetRowCount())
	{
		return DB_RET_NOT_EXIST;
	}


    while(mysql.GetRowCount() && mysql.Next())
    {
		m_feed_id = strtoul(mysql.GetRow(2), NULL, 10);
		m_feed_id_list.push_back(m_feed_id);
    }


	/******************************************/

//	if(m_feed_type == ::hoosho::commenum::FEEDS_FEED_TYPE_FEED_OF_TOPIC)
//	{
	ossSql.str("");
	ossSql << "select count(*) from " << strTableNamePrefix
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
	if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

    if(!mysql.GetRowCount())
	{
		return DB_RET_NOT_EXIST;
	}


    while(mysql.GetRowCount() && mysql.Next())
    {
		m_total = strtoul(mysql.GetRow(0), NULL, 10);
    }

//	}

	return DB_RET_OK;
}

int FeedIndexOnAppid::SelectFromDBByFeedid(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_feed_id_list.clear();
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<<" where pa_appid_md5=" << m_pa_appid_md5
		<<" and feed_id=" << m_feed_id;
	
	strErrMsg = ossSql.str();

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	if(!mysql.GetRowCount())
	{
		return DB_RET_NOT_EXIST;
	}

    if(mysql.GetRowCount() && mysql.Next())
    {
		m_pa_appid_md5 = strtoul(mysql.GetRow(0), NULL, 10);
		m_openid_md5 = strtoul(mysql.GetRow(1), NULL, 10);
		m_feed_id = strtoul(mysql.GetRow(2), NULL, 10);
		m_feed_type = strtoul(mysql.GetRow(3), NULL, 10);
		m_origin_feed_id = strtoul(mysql.GetRow(4), NULL, 10);
		m_num_favorite = strtoul(mysql.GetRow(5), NULL, 10);
    }

	return DB_RET_OK;
}


int FeedIndexOnAppid::UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<<" set pa_appid_md5=" << m_pa_appid_md5
		<<", openid_md5=" << m_openid_md5
    	<<", feed_id=" << m_feed_id
		<<", feed_type=" << m_feed_type
		<<", origin_feed_id=" << m_origin_feed_id
		<<", num_favorite=" << m_num_favorite
		<<" on duplicate key "
		<<" update num_favorite=" << m_num_favorite
	;

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}

int FeedIndexOnAppid::DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "delete from " << strTableNamePrefix
			<< IntToHexStr(m_pa_appid_md5 % 256)
			<<" where feed_id=" << m_feed_id
			<<" and pa_appid_md5 =" << m_pa_appid_md5;


    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }
	if(!mysql.GetAffectedRows())
	{
		return DB_RET_NOT_EXIST;
	}
	return DB_RET_OK;
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

int FeedReport::UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<<" set pa_appid_md5=" << m_pa_appid_md5
		<<", openid_md5_from=" << m_openid_md5_from
		<<", openid_md5_to=" << m_openid_md5_to
    	<<", feed_id=" << m_feed_id
		<<", report_ts=" << m_report_ts
		<<", report_reason='" << lce::cgi::CMysql::MysqlEscape(m_report_reason) << "'"
		<<" on duplicate key update"
		<<" report_ts=" << m_report_ts
		<<", report_reason='" << lce::cgi::CMysql::MysqlEscape(m_report_reason) << "'";
	strErrMsg = ossSql.str();
    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}

std::string FeedReport::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< "pa_appid_md5 = " << m_pa_appid_md5
		<< ", openid_md5_from = " << m_openid_md5_from
		<< ", openid_md5_to = " << m_openid_md5_to
    	<< ", feed_id = " << m_feed_id
		<< ", report_ts = " << m_report_ts
		<< ", report_reason = " << m_report_reason
		<< "}";
	return oss.str();

}

int UserForbid::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<<" where pa_appid_md5=" << m_pa_appid_md5
		<<" and openid_md5= " << m_openid_md5;
	
    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

    if(mysql.GetRowCount() && mysql.Next())
    {
		m_until_ts= strtoul(mysql.GetRow(2), NULL, 10);
		return DB_RET_OK;
    }
	return DB_RET_NOT_EXIST;	
}

int UserForbid::UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	
		std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<<" set pa_appid_md5=" << m_pa_appid_md5
		<<", openid_md5=" << m_openid_md5
		<<", until_ts=" << m_until_ts
		<<" on duplicate key update "
		<<" until_ts=" << m_until_ts;

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}

std::string UserForbid::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< "pa_appid_md5 = " << m_pa_appid_md5
		<< ", openid_md5 = " << m_openid_md5
		<< ", until_ts = " << m_until_ts
		<< "}";
	return oss.str();

}



