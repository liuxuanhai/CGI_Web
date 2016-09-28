#include "db_follow.h"


int FollowInfo::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_follow_id % 256)
		<<" where follow_id=" << m_follow_id;

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

    if(mysql.GetRowCount() && mysql.Next())
    {
		m_content = mysql.GetRow(1);
		m_feed_id = strtoul(mysql.GetRow(2), NULL, 10);
		m_type = strtoul(mysql.GetRow(3), NULL, 10);
		m_origin_comment_id = strtoul(mysql.GetRow(4), NULL, 10);
		m_openid_md5_from = strtoul(mysql.GetRow(5), NULL, 10);
		m_openid_md5_to = strtoul(mysql.GetRow(6), NULL, 10);
		m_create_ts = strtoul(mysql.GetRow(7), NULL, 10);
		m_del_ts = strtoul(mysql.GetRow(8), NULL, 10);
	
        return DB_RET_OK;
    }

	return DB_RET_NOT_EXIST;
}

int FollowInfo::InsertToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_follow_id % 256)
    	<<" set follow_id=" << m_follow_id
		<<", content='" << lce::cgi::CMysql::MysqlEscape(m_content) << "'"
		<<", feed_id=" << m_feed_id
		<<", type=" << m_type
		<<", origin_comment_id=" << m_origin_comment_id
		<<", openid_md5_from=" << m_openid_md5_from
		<<", openid_md5_to=" << m_openid_md5_to
		<<", create_ts=" << m_create_ts
		<<", del_ts=" << m_del_ts
	;

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}


int FollowInfo::UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_follow_id % 256)
    	<<" set follow_id=" << m_follow_id
		<<", content='" << lce::cgi::CMysql::MysqlEscape(m_content) << "'"
		<<", feed_id=" << m_feed_id
		<<", type=" << m_type
		<<", origin_comment_id=" << m_origin_comment_id
		<<", openid_md5_from=" << m_openid_md5_from
		<<", openid_md5_to=" << m_openid_md5_to
		<<", create_ts=" << m_create_ts
		<<", del_ts=" << m_del_ts
		<<" on duplicate key"
		<<" update content='" << lce::cgi::CMysql::MysqlEscape(m_content) << "'"
		<<", feed_id=" << m_feed_id
		<<", type=" << m_type
		<<", origin_comment_id=" << m_origin_comment_id
		<<", openid_md5_from=" << m_openid_md5_from
		<<", openid_md5_to=" << m_openid_md5_to
		<<", create_ts=" << m_create_ts
		<<", del_ts=" << m_del_ts;

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}

int FollowInfo::DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;

    ossSql.str("");
    ossSql << "delete from " << strTableNamePrefix
			<< IntToHexStr(m_follow_id % 256)
			<< " where follow_id=" << m_follow_id;

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


std::string FollowInfo::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< "follow_id:" << m_follow_id
		<< "content:" << m_content
		<< ", feed_id:" << m_feed_id
		<< ", type:" << m_type
		<< ", origin_comment_id:" << m_origin_comment_id
		<< ", openid_md5_from:" << m_openid_md5_from
		<< ", openid_md5_to:" << m_openid_md5_to
		<< ", create_ts = " << m_create_ts
		<< ", del_ts = " << m_del_ts
		<< "}";
	return oss.str();
}

int CommentIndexOnFeedid::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_comment_id_list.clear();
	std::ostringstream ossSql;

	//count total num
	ossSql.str("");
	ossSql << "select count(*) from " << strTableNamePrefix
		<< IntToHexStr(m_feed_id % 256)
		<<" where feed_id=" << m_feed_id;

	if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		return DB_RET_FAIL;
	}

	if(mysql.Next())
	{
		m_total = strtoul(mysql.GetRow(0), NULL, 10);
	}
	
/*	
	if(!m_total)
	{
		return DB_RET_NOT_EXIST;
	}
*/	

	//get detail
	ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_feed_id % 256)
		<<" where feed_id=" << m_feed_id;
	if(m_begin_comment_id)
	{
		ossSql <<" and comment_id < " << m_begin_comment_id;
	}
	ossSql << " order by comment_id desc limit " << m_limit;
	
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
		m_comment_id_list.push_back(strtoul(mysql.GetRow(1), NULL, 10) );
    }

	return DB_RET_OK;
}

int CommentIndexOnFeedid::UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_feed_id % 256)
		<<" set feed_id=" << m_feed_id
    	<<", comment_id=" << m_comment_id;
	
    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}

int CommentIndexOnFeedid::DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "delete from " << strTableNamePrefix
			<< IntToHexStr(m_feed_id % 256)
			<<" where feed_id=" << m_feed_id
			<<" and comment_id =" << m_comment_id;
	
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


std::string CommentIndexOnFeedid::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< " feed_id:" << m_feed_id
		<< "[";
	for(size_t i = 0; i < m_comment_id_list.size(); i++)
	{
		oss << m_comment_id_list[i] << ", ";
	}
	oss	<< "]"
		<< "}";
	return oss.str();
}



int ReplyIndexOnOriginComment::SelectFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	m_reply_id_list.clear();
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "select * from " << strTableNamePrefix
		<< IntToHexStr(m_origin_comment_id % 256)
		<<" where origin_comment_id=" << m_origin_comment_id
		<<" order by reply_id asc";

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
		m_reply_id = strtoul(mysql.GetRow(1), NULL, 10);
		m_reply_id_list.push_back(m_reply_id);
    }

	return DB_RET_OK;
}

int ReplyIndexOnOriginComment::UpdateToDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into " << strTableNamePrefix
		<< IntToHexStr(m_origin_comment_id % 256)
		<<" set origin_comment_id=" << m_origin_comment_id
    	<<", reply_id=" << m_reply_id;
	
    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return DB_RET_FAIL;
    }

	return DB_RET_OK;
}

int ReplyIndexOnOriginComment::DeleteFromDB(const std::string& strTableNamePrefix, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql << "delete from " << strTableNamePrefix
			<< IntToHexStr(m_origin_comment_id % 256)
			<<" where origin_comment_id=" << m_origin_comment_id
			<<" and reply_id =" << m_reply_id;
	
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


std::string ReplyIndexOnOriginComment::ToString() const
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<< " origin_comment_id:" << m_origin_comment_id
		<< "[";
	for(size_t i = 0; i < m_reply_id_list.size(); i++)
	{
		oss << m_reply_id_list[i] << ", ";
	}
	oss	<< "]"
		<< "}";
	return oss.str();
}





