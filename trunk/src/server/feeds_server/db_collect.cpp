#include "db_collect.h"

int CollectInfo::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_pa_appid_md5% 256)
		<<" where pa_appid_md5 = " << m_pa_appid_md5
		<<" and feed_id = " << m_feed_id
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
		m_create_ts = strtoul(mysql.GetRow(3), NULL, 10);
    }

	return DB_RET_OK;
}

int CollectInfo::InsertToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<<" set feed_id=" << m_feed_id
		<<", pa_appid_md5=" << m_pa_appid_md5
    	<<", openid_md5=" << m_openid_md5
    	<<", create_ts=" << m_create_ts
	;
	strErrMsg = ossSql.str();
    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}


int CollectInfo::UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<<" set feed_id=" << m_feed_id
		<<", pa_appid_md5=" << m_pa_appid_md5
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

int CollectInfo::DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "delete from " << strTableNamePrefix
			<< IntToHexStr(m_pa_appid_md5 % 256)
			<<" where feed_id=" << m_feed_id
			<<" and pa_appid_md5=" << m_pa_appid_md5
			<<" and openid_md5=" << m_openid_md5;
	strErrMsg = ossSql.str();
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


std::string CollectInfo::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< "feed_id = " << m_feed_id
		<< ", pa_appid_md5 = " << m_pa_appid_md5
	 	<< ", openid_md5 = " << m_openid_md5
		<< ", create_ts = " << m_create_ts 
		<< "}";
	return oss.str();
}

int CollectList::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_feed_id_list.clear();
	m_create_ts_list.clear();
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<<" where pa_appid_md5=" << m_pa_appid_md5
		<<" and openid_md5=" << m_openid_md5;
	
	if(m_begin_create_ts)
	{
		ossSql << " and create_ts < " << m_begin_create_ts;
	}
	ossSql << " order by create_ts desc limit " << m_limit;

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
		m_feed_id = strtoul(mysql.GetRow(1), NULL, 10);
		m_create_ts = strtoul(mysql.GetRow(3), NULL, 10);
		m_feed_id_list.push_back(m_feed_id);
		m_create_ts_list.push_back(m_create_ts);
    }

	return DB_RET_OK;
}

std::string CollectList::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< "openid_md5 = " << m_openid_md5
		<< ", pa_appid = " << m_pa_appid_md5
		<< ", (feedid, create_ts) = [ ";
	for(size_t i = 0; i < m_feed_id_list.size(); i++)
	{
	 	oss	<< " (" << m_openid_md5
			<< ", " << m_create_ts << ") "; 
	}
	oss << "] "
		<< "}";
	return oss.str();
}



int FeedListCollect::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_collect_list.clear();
	std::ostringstream ossSql;

	for(size_t i = 0; i < m_feed_id_list.size(); i++)
	{
		ossSql.str("");
   		ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_pa_appid_md5 % 256)
		<< " where feed_id=" << m_feed_id_list[i]
		<< " and openid_md5 = " << m_openid_md5
		<< " and pa_appid_md5 = " << m_pa_appid_md5;
		
		if(!mysql.Query(ossSql.str()))
		{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
			return DB_RET_FAIL;
		}
		if(!mysql.GetRowCount())
		{
			m_collect_list.push_back(0);
		}
		else
		{
			m_collect_list.push_back(1);
		}
	
	}

	return DB_RET_OK;
}

std::string FeedListCollect::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
	 	<< " openid_md5 = " << m_openid_md5
	 	<< ", pa_appid = " << m_pa_appid_md5;
	for(size_t i = 0; i < m_feed_id_list.size(); i++)
	{
		oss << "(feed_id = " << m_feed_id_list[i]
			<< " : " << m_collect_list[i] << ")";
	}
	oss	<< "}";
	return oss.str();
}




