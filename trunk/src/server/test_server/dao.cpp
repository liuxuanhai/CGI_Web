#include "dao.h"


int DaoUserInfo::SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"select * from test_user_info where id="<<m_id;

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return -1;
    }

    if(mysql.GetRowCount() && mysql.Next())
    {
    	m_name = mysql.GetRow(1);
    	m_age = atol(mysql.GetRow(2));
    	
        return 1;
    }

	return 0;
}

int DaoUserInfo::UpdateToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"insert into test_user_info"
    	<<" set id="<<m_id
    	<<", name='"<<lce::cgi::CMysql::MysqlEscape(m_name)<<"'"
    	<<", age="<<m_age
    	<<" on duplicate key"
    	<<" update name='"<<lce::cgi::CMysql::MysqlEscape(m_name)<<"'"
    	<<", age="<<m_age;

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return -1;
    }

	return 0;
}

int DaoUserInfo::DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"delete from test_user_info where id="<<m_id;

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return -1;
    }

	return 0;
}


std::string DaoUserInfo::ToString() const
{
	std::ostringstream oss;
	oss<<"{id:"<<m_id<<", name="<<m_name<<", age="<<m_age<<"}";
	return oss.str();
}


