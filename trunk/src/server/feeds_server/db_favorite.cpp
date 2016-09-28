#include "db_favorite.h"

int FavoriteInfo::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_feed_id % 256)
		<<" where feed_id=" << m_feed_id
		<<" and openid_md5 = " << m_openid_md5;
	
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
//		m_openid_md5 = strtoul(mysql.GetRow(1), NULL, 10);
		m_create_ts = strtoul(mysql.GetRow(2), NULL, 10);
    }

	return DB_RET_OK;
}

int FavoriteInfo::InsertToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_feed_id % 256)
		<<" set feed_id=" << m_feed_id
    	<<", openid_md5=" << m_openid_md5
    	<<", create_ts=" << m_create_ts
	;
	
    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}


int FavoriteInfo::UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_feed_id % 256)
		<<" set feed_id=" << m_feed_id
    	<<", openid_md5=" << m_openid_md5
    	<<", create_ts=" << m_create_ts
    	<<" on duplicate key"
		<<" update create_ts=" << m_create_ts;
	
    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}

int FavoriteInfo::DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "delete from " << strTableNamePrefix
			<< IntToHexStr(m_feed_id % 256)
			<<" where feed_id=" << m_feed_id
			<<" and openid_md5=" << m_openid_md5;
	
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


std::string FavoriteInfo::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< "feed_id = " << m_feed_id
	 	<< ", openid_md5 = " << m_openid_md5
		<< ", create_ts = " << m_create_ts 
		<< "}";
	return oss.str();
}

int FavoriteList::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_openid_md5_list.clear();
	m_create_ts_list.clear();
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_feed_id % 256)
		<<" where feed_id=" << m_feed_id;
	
	if(m_begin_create_ts)
	{
		ossSql << " and create_ts < " << m_begin_create_ts;
	}
	ossSql << " order by create_ts desc limit " << m_limit;
	
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
		m_openid_md5 = strtoul(mysql.GetRow(1), NULL, 10);
		m_create_ts = strtoul(mysql.GetRow(2), NULL, 10);
		m_openid_md5_list.push_back(m_openid_md5);
		m_create_ts_list.push_back(m_create_ts);
    }

	return DB_RET_OK;
}

std::string FavoriteList::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< "feed_id = " << m_feed_id
		<< ", (openid, create_ts) = [ ";
	for(size_t i = 0; i < m_openid_md5_list.size(); i++)
	{
	 	oss	<< " (" << m_openid_md5
			<< ", " << m_create_ts << ") "; 
	}
	oss << "] "
		<< "}";
	return oss.str();
}



int FeedListFavorite::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_favorite_list.clear();
	std::ostringstream ossSql;

	for(size_t i = 0; i < m_feed_id_list.size(); i++)
	{
		ossSql.str("");
   		ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_feed_id_list[i] % 256)
		<< " where feed_id=" << m_feed_id_list[i]
		<< " and openid_md5 = " << m_openid_md5;
		
		if(!mysql.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
			return DB_RET_FAIL;
		}
		if(!mysql.GetRowCount())
		{
			m_favorite_list.push_back(0);
		}
		else
		{
			m_favorite_list.push_back(1);
		}
	
	}

	return DB_RET_OK;
}

std::string FeedListFavorite::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
	 	<< " openid_md5 = " << m_openid_md5;
	for(size_t i = 0; i < m_feed_id_list.size(); i++)
	{
		oss << "(feed_id = " << m_feed_id_list[i]
			<< " : " << m_favorite_list[i] << ")";
	}
	oss	<< "}";
	return oss.str();
}




