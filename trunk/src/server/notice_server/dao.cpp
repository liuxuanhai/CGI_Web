
#include "global_var.h"
#include "dao.h"

extern string DaoSysNotice::m_table_name;
extern string DaoSysNoticeByAppidAndLimitTs::m_table_name;
extern string DaoNoticeState::m_table_name_prefix;
extern string DaoNoticeStateTypeList::m_table_name_prefix;
extern string DaoNoticeUser::m_table_name_prefix;
extern string DaoNoticeUserByAll::m_table_name_prefix;

log4cplus::Logger logger = log4cplus::Logger::getInstance("dao");

int DaoSysNotice::SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "select * from " << m_table_name <<" where pa_appid_md5=" << m_pa_appid_md5 << " and create_ts=" << m_create_ts;

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if (!mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		return -1;
	}

	if (mysql.GetRowCount() && mysql.Next())
	{
		m_title = mysql.GetRow(2);
		m_content = mysql.GetRow(3);
		return 1;
	}

	return 0;
}

int DaoSysNotice::UpdateToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "insert into "<<m_table_name
			<<" set pa_appid_md5="<<m_pa_appid_md5
			<<", create_ts="<<m_create_ts
			<<", title='"<<lce::cgi::CMysql::MysqlEscape(m_title)<<"'"
			<<", content='"<<lce::cgi::CMysql::MysqlEscape(m_content)<<"'"
			<<" on duplicate key"
			<<" update create_ts="<<m_create_ts
			<<", title='"<<lce::cgi::CMysql::MysqlEscape(m_title)<<"'"
			<<", content='"<<lce::cgi::CMysql::MysqlEscape(m_content)<<"'";

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if (!mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		return -1;
	}

	return 0;
}
int DaoSysNotice::InsertToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "insert into "<<m_table_name
			<<" set pa_appid_md5="<<m_pa_appid_md5
			<<", create_ts="<<m_create_ts
			<<", title='"<<lce::cgi::CMysql::MysqlEscape(m_title)<<"'"
			<<", content='"<<lce::cgi::CMysql::MysqlEscape(m_content)<<"'";

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if (!mysql.Query(ossSql.str()))
	{
		string strMsg = mysql.GetErrMsg();
		if(strMsg.substr(sizeof("Duplicate"))=="Duplicate")
		{
			return 2;
		}
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + strMsg;
		return -1;
	}

	return 0;
}
int DaoSysNotice::DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "delete from "<<m_table_name<<" where pa_appid_md5=" << m_pa_appid_md5 << " and create_ts=" << m_create_ts;

	LOG4CPLUS_DEBUG(logger, ossSql.str());
	
	if (!mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		return -1;
	}

	return 0;
}

std::string DaoSysNotice::ToString() const
{
	std::ostringstream oss;
	oss << "{ "
		<<" pa_appid_md5: "<<m_pa_appid_md5
		<<", create_ts: "<<m_create_ts
		<<", title: "<<m_title
		<<", content: "<<m_content
		<< "}";
	return oss.str();
}
int DaoSysNoticeByAppidAndLimitTs::SelectFromDB(lce::cgi::CMysql& mysql,std::string& strErrMsg)
{
	this->m_dao_sys_notice_list.clear();

    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name
			<<" where pa_appid_md5="<<m_pa_appid_md5;

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if(m_limit_upper_ts>0)
	{
		ossSql<<" and create_ts <" <<m_limit_upper_ts;
	}
	else if(m_limit_lower_ts>0)
	{
		ossSql<<" and create_ts >" <<m_limit_lower_ts;
	}
	ossSql<<" order by create_ts desc";
	if(m_pagesize>0)
	{
		ossSql<<" limit "<<m_pagesize;
	}

	LOG4CPLUS_DEBUG(logger, ossSql.str());


	if(!mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
			return -1;
	}

    if(0 == mysql.GetRowCount())
    {
        strErrMsg = "SysNotice not exists!!,pa_appid_md5=";
        strErrMsg+=m_pa_appid_md5;
        strErrMsg+=",limit_upper_ts=";
        strErrMsg+=m_limit_upper_ts;
        strErrMsg+=",limit_lower_ts=";
        strErrMsg+=m_limit_lower_ts;
        strErrMsg+=",pagesize=";
        strErrMsg+=m_pagesize;
	    return 0;
    }
    if(m_queryflag==2)
    {
		while(mysql.Next())
		{
			DaoSysNotice stDaoSysNotice;
			stDaoSysNotice.m_pa_appid_md5 = strtoul(mysql.GetRow(0),NULL,10);
			stDaoSysNotice.m_create_ts = strtoul(mysql.GetRow(1),NULL,10);
			stDaoSysNotice.m_title = mysql.GetRow(2);
			stDaoSysNotice.m_content = mysql.GetRow(3);

			m_dao_sys_notice_list.push_back(stDaoSysNotice);
		}
		std::sort(m_dao_sys_notice_list.begin(), m_dao_sys_notice_list.end(), CreateTsDes);
    }


	return 1;
}

std::string DaoSysNoticeByAppidAndLimitTs::ToString() const
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_dao_sys_notice_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_dao_sys_notice_list[i].ToString()<<", ";
    }

    oss<<"]";

    return oss.str();
}
void DaoNoticeState::Split()
{
	if(m_type==NOTICE_USER_FOLLOWED_LT)//||m_type==NOTICE_USER_MESSAGED_LT)
	{
		m_pa_appid_md5=0;
	}
};
int DaoNoticeState::SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	char suffix[4];
	sprintf(suffix,"%02x",(uint32_t)(m_openid_md5%256));
	Split();
	ossSql << "select * from " <<m_table_name_prefix<<"_"<<suffix
			<<" where pa_appid_md5=" <<m_pa_appid_md5
			<< " and openid_md5=" <<m_openid_md5
			<<" and type="<<m_type;

	LOG4CPLUS_DEBUG(logger, ossSql.str());
	
	if (!mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		return -1;
	}

	if (mysql.GetRowCount() && mysql.Next())
	{
		m_last_used_ts = strtoul(mysql.GetRow(3),NULL,10);
		return 1;
	}

	return 0;
}

int DaoNoticeState::UpdateToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	char suffix[4];
	sprintf(suffix,"%02x",(uint32_t)(m_openid_md5%256));
	Split();
	ossSql << "insert into "<<m_table_name_prefix<<"_"<<suffix
			<<" set openid_md5="<<m_openid_md5
			<<", pa_appid_md5="<<m_pa_appid_md5
			<<", type="<<m_type
			<<", last_used_ts="<<m_last_used_ts
			<<" on duplicate key"
			<<" update last_used_ts="<<m_last_used_ts;
			
	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if (!mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		return -1;
	}

	return 0;
}

int DaoNoticeState::DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	char suffix[4];
	sprintf(suffix,"%02x",(uint32_t)(m_openid_md5%256));
	Split();
	ossSql << "delete from "<<m_table_name_prefix<<"_"<<suffix
				<<" where pa_appid_md5=" <<m_pa_appid_md5
				<< " and openid_md5=" <<m_openid_md5
				<<" and type="<<m_type;
				
	LOG4CPLUS_DEBUG(logger, ossSql.str());
	
	if (!mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		return -1;
	}

	return 0;
}

std::string DaoNoticeState::ToString() const
{

	std::ostringstream oss;
	oss << "{ "
		<<" openid_md5: "<<m_openid_md5
		<<" pa_appid_md5: "<<m_pa_appid_md5
		<<", type: "<<m_type
		<<", last_used_ts: "<<m_last_used_ts
		<< "}";
	return oss.str();
}
int DaoNoticeStateTypeList::SelectFromDB(lce::cgi::CMysql& mysql,std::string& strErrMsg)
{


	std::ostringstream ossSql;
	ossSql.str("");
	char suffix[4];
	sprintf(suffix,"%02x",(uint32_t)(m_openid_md5%256));
	ossSql << "select * from " <<m_table_name_prefix<<"_"<<suffix
			<<" where openid_md5=" <<m_openid_md5
			<<" and type >= "<<NOTICE_USER_LIKED_LT
			<<" and type <= "<<NOTICE_USER_FOLLOWED_LT;

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if(!mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
			return -1;
	}

    if(0 == mysql.GetRowCount())
    {
        strErrMsg = "SysNotice not exists!!,sql=\n";
        strErrMsg+=ossSql.str();
	    return 0;
    }

	while(mysql.Next())
	{
		uint64_t pa_appid_md5 = strtoul(mysql.GetRow(1),NULL,10);
		uint32_t type= atol(mysql.GetRow(2));

		switch(type)
		{
		case NOTICE_USER_LIKED_LT:
			if(pa_appid_md5==m_pa_appid_md5)
			{
				m_last_used_ts_like = strtoul(mysql.GetRow(3),NULL,10);
			}
			break;
		case NOTICE_USER_COMMENTED_LT:
			if(pa_appid_md5==m_pa_appid_md5)
			{
				m_last_used_ts_comment = strtoul(mysql.GetRow(3),NULL,10);
			}
			break;
		case NOTICE_USER_FOLLOWED_LT:
			if(pa_appid_md5==0)
			{
				m_last_used_ts_follow = strtoul(mysql.GetRow(3),NULL,10);
			}
			break;
		}

	}

	return 1;
}
std::string DaoNoticeStateTypeList::ToString() const
{
	std::ostringstream oss;
	oss << "{ "
		<<" openid_md5: "<<m_openid_md5
		<<", pa_appid_md5: "<<m_pa_appid_md5
		<<", last_used_ts_like: "<<m_last_used_ts_like
		<<", last_used_ts_comment: "<<m_last_used_ts_comment
		<<", last_used_ts_follow: "<<m_last_used_ts_follow
	//	<<", last_used_ts_message: "<<m_last_used_ts_message
		<< "}";
	return oss.str();
}


int DaoNoticeUser::SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	char suffix[4];
	sprintf(suffix,"%02x",(uint32_t)(m_openid_md5%256));
	if(m_type==NOTICE_USER_FOLLOWED)        //||m_type==NOTICE_USER_MESSAGED)
	{
		m_pa_appid_md5=0;
	}
	ossSql << "select * from " <<m_table_name_prefix<<"_"<<suffix
			<<" where openid_md5="<<m_openid_md5
			<<" and pa_appid_md5=" <<m_pa_appid_md5
			<<" and type="<<m_type
			<< " and create_ts=" <<m_create_ts;

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if (!mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		return -1;
	}

	if (mysql.GetRowCount() && mysql.Next())
	{
		m_type= ::atol(mysql.GetRow(3));
		m_status=::atol (mysql.GetRow(4));
		m_extra_data_0 =mysql.GetRow(5);
		m_extra_data_1 =mysql.GetRow(6);
		m_extra_data_2 =mysql.GetRow(7);
		return 1;
	}

	return 0;
}

int DaoNoticeUser::UpdateToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	char suffix[4];
	sprintf(suffix,"%02x",(uint32_t)(m_openid_md5%256));
	if(m_type==NOTICE_USER_FOLLOWED)//||m_type==NOTICE_USER_MESSAGED)
	{
		m_pa_appid_md5=0;
	}
	ossSql << "insert into "<<m_table_name_prefix<<"_"<<suffix
				<<" set openid_md5="<<m_openid_md5
				<<", pa_appid_md5="<<m_pa_appid_md5
				<<", create_ts="<<m_create_ts
				<<", type="<<m_type
				<<", status="<<m_status
				<<", extra_data_0='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_0)<<"'"
				<<", extra_data_1='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_1)<<"'"
				<<", extra_data_2='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_2)<<"'"
				<<" on duplicate key"
				<<" update status="<<m_status;

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if (!mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		return -1;
	}

	return 0;
}
int DaoNoticeUser::InsertToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	char suffix[4];
	sprintf(suffix,"%02x",(uint32_t)(m_openid_md5%256));
	if(m_type==NOTICE_USER_FOLLOWED)//||m_type==NOTICE_USER_MESSAGED)
	{
		m_pa_appid_md5=0;
	}
	if(m_type == 1)	//点赞不能重复插入
	{
		ossSql<<"select * from "<<m_table_name_prefix<<"_"<<suffix
			<<" where openid_md5="<<m_openid_md5
			<<" and pa_appid_md5="<<m_pa_appid_md5
			<<" and type="<<m_type
			<<" and extra_data_0='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_0)<<"'"
			<<" and extra_data_1='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_1)<<"'";
		
		LOG4CPLUS_DEBUG(logger, ossSql.str());
		
		if (!mysql.Query(ossSql.str()))
		{
			string strMsg = mysql.GetErrMsg();			
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + strMsg;
			return -1;
		}

		if (mysql.GetRowCount() && mysql.Next())	//update create_ts
		{
			//1.delete
			ossSql.str("");
			ossSql<<"delete from "<<m_table_name_prefix<<"_"<<suffix
				<<" where openid_md5="<<m_openid_md5
				<<" and pa_appid_md5="<<m_pa_appid_md5
				<<" and type="<<m_type
				<<" and extra_data_0='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_0)<<"'"
				<<" and extra_data_1='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_1)<<"'";

			LOG4CPLUS_DEBUG(logger, ossSql.str());
			if (!mysql.Query(ossSql.str()))
			{
				string strMsg = mysql.GetErrMsg();			
				strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + strMsg;
				return -1;
			}
			
		}
		
		//insert		
		ossSql.str("");
		ossSql<<"insert into "<<m_table_name_prefix<<"_"<<suffix
			<<" set openid_md5="<<m_openid_md5
			<<", pa_appid_md5="<<m_pa_appid_md5
			<<", create_ts="<<m_create_ts
			<<", type="<<m_type
			<<", status="<<m_status
			<<", extra_data_0='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_0)<<"'"
			<<", extra_data_1='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_1)<<"'"
			<<", extra_data_2='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_2)<<"'";
		
	}	
	else
	{
		ossSql << "insert into "<<m_table_name_prefix<<"_"<<suffix
			<<" set openid_md5="<<m_openid_md5
			<<", pa_appid_md5="<<m_pa_appid_md5
			<<", create_ts="<<m_create_ts
			<<", type="<<m_type
			<<", status="<<m_status
			<<", extra_data_0='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_0)<<"'"
			<<", extra_data_1='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_1)<<"'"
			<<", extra_data_2='"<<lce::cgi::CMysql::MysqlEscape(m_extra_data_2)<<"'";
	}
	

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if (!mysql.Query(ossSql.str()))
	{
		string strMsg = mysql.GetErrMsg();
		if(strMsg.substr(sizeof("Duplicate"))=="Duplicate")
		{
			return 2;
		}
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + strMsg;
		return -1;
	}

	return 0;
}
int DaoNoticeUser::DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	char suffix[4];
	sprintf(suffix,"%02x",(uint32_t)(m_openid_md5%256));
	if(m_type==NOTICE_USER_FOLLOWED)
	//		||m_type==NOTICE_USER_MESSAGED)
	{
		m_pa_appid_md5=0;
	}
	ossSql << "delete from "<<m_table_name_prefix<<"_"<<suffix
			<<" where openid_md5="<<m_openid_md5
			<<" and pa_appid_md5="<<m_pa_appid_md5
			<<" and type="<<m_type
			<< " and create_ts=" << m_create_ts;

	LOG4CPLUS_DEBUG(logger, ossSql.str());

	if (!mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		return -1;
	}

	return 0;
}

std::string DaoNoticeUser::ToString() const
{
	std::ostringstream oss;
	oss << "{ "
		<<" openid_md5: "<<m_openid_md5
		<<", pa_appid_md5: "<<m_pa_appid_md5
		<<", create_ts: "<<m_create_ts
		<<", status: "<<m_status
		<<", type: "<<m_type
		<<", extra_data_0: "<<m_extra_data_0
		<<", extra_data_1: "<<m_extra_data_1
		<<", extra_data_2: "<<m_extra_data_2
		<< "}";
	return oss.str();
}

int DaoNoticeUserByAll::SelectFromDB(lce::cgi::CMysql& mysql,std::string& strErrMsg)
{
	this->m_dao_notice_user_list.clear();

    std::ostringstream ossSql;
	ossSql.str("");
	char suffix[4];
	sprintf(suffix,"%02x",(uint32_t)(m_openid_md5%256));

	if(m_queryflag==NOTICE_RECORD&&(m_type==NOTICE_USER_LIKED||m_type==NOTICE_USER_COMMENTED))
	{
		ossSql<<"select * from "<<m_table_name_prefix<<"_"<<suffix
				<<" where openid_md5="<<m_openid_md5<<" and pa_appid_md5="<<m_pa_appid_md5;
	}
	else
	{
		ossSql<<"select * from "<<m_table_name_prefix<<"_"<<suffix
					<<" where openid_md5="<<m_openid_md5;
	}

	LOG4CPLUS_DEBUG(logger, ossSql.str());
	
	if(m_limit_ts>0)
	{
		if(m_queryflag==NOTICE_RECORD)
		{
			ossSql<<" and create_ts < "<<m_limit_ts;
		}
		else
		{
			ossSql<<" and create_ts > "<<m_limit_ts;
		}
	}

	if(m_queryflag==NOTICE_NOTIFY_TYPE||m_queryflag==NOTICE_RECORD)
	{
		ossSql<<" and type="<<m_type;
		if(m_type==NOTICE_USER_FOLLOWED)//||m_type==NOTICE_USER_MESSAGED)
		{
			m_pa_appid_md5=0;
		}
	}
	ossSql<<"  order by create_ts desc ";
	if(m_pagesize>0)
	{
		ossSql<<" limit "<<m_pagesize;
	}



	if(!mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
			return -1;
	}



    if(0 == mysql.GetRowCount())
    {
        strErrMsg = "NoticeUser not exists!!, openid_md5=";
        strErrMsg+=m_openid_md5;
        strErrMsg+=",pa_appid_md5=";
        strErrMsg+=m_pa_appid_md5;
        strErrMsg+=",limit_ts=";
        strErrMsg+=m_limit_ts;
        strErrMsg+=",type=";
        strErrMsg+=m_type;
        strErrMsg+=",pagesize=";
        strErrMsg+=m_pagesize;
	    return 0;
    }


    if(m_queryflag==NOTICE_NOTIFY_TYPE_LIST||m_queryflag==NOTICE_NOTIFY)
    {
    	TypeStatus type1(NOTICE_USER_LIKED,0),type2(NOTICE_USER_COMMENTED,0),type3(NOTICE_USER_FOLLOWED,0);//type4(NOTICE_USER_MESSAGED,0);
    	int iFlagAll=0;
    	while(mysql.Next())
    	{
    		uint64_t pa_appid_md5 = strtoul(mysql.GetRow(1),NULL,10);
    		uint64_t create_ts=strtoul(mysql.GetRow(2),NULL,10);
    		uint32_t type= ::atol(mysql.GetRow(3));
    		uint32_t status=::atol (mysql.GetRow(4));

    		if(
    			(type==NOTICE_USER_FOLLOWED&&pa_appid_md5==0)
    			//	||(type==NOTICE_USER_MESSAGED&&pa_appid_md5==0)
    				||(pa_appid_md5==m_pa_appid_md5&&type==NOTICE_USER_LIKED)
    				||(pa_appid_md5==m_pa_appid_md5&&type==NOTICE_USER_COMMENTED)
    		  )
    		{
				if(status==0)
				{
					switch(type)
					{
					case NOTICE_USER_LIKED:
						if(create_ts < m_limit_ts_like)
						{
							break;
						}
						type1.status=1;
						iFlagAll=1;
						break;
					case NOTICE_USER_COMMENTED:
						if(create_ts < m_limit_ts_comment)
						{
							break;
						}
						type2.status=1;
						iFlagAll=1;
						break;
					case NOTICE_USER_FOLLOWED:
						if(create_ts < m_limit_ts_follow)
						{
							break;
						}
						type3.status=1;
						iFlagAll=1;
						break;
			/*		case NOTICE_USER_MESSAGED:
						if(create_ts < m_limit_ts_message)
						{
							break;
						}
						type4.status=1;
						iFlagAll=1;
						break;*/
					}
				}
    		}
    	}
    	m_type_status_list.clear();
    	m_type_status_list.push_back(type1);
    	m_type_status_list.push_back(type2);
    	m_type_status_list.push_back(type3);
 //   	m_type_status_list.push_back(type4);
    	if(m_queryflag==NOTICE_NOTIFY)
    	{
    		return iFlagAll;
    	}

    }

    if(m_queryflag==NOTICE_RECORD)
    {
		while(mysql.Next())
		{
			DaoNoticeUser stDaoNoticeUser;
			stDaoNoticeUser.m_openid_md5 = strtoul(mysql.GetRow(0),NULL,10);
			stDaoNoticeUser.m_pa_appid_md5 =strtoul(mysql.GetRow(1),NULL,10);
			stDaoNoticeUser.m_create_ts = strtoul(mysql.GetRow(2),NULL,10);
			stDaoNoticeUser.m_type= ::atol(mysql.GetRow(3));
			stDaoNoticeUser.m_status=::atol (mysql.GetRow(4));
			stDaoNoticeUser.m_extra_data_0 =mysql.GetRow(5);
			stDaoNoticeUser.m_extra_data_1 =mysql.GetRow(6);
			stDaoNoticeUser.m_extra_data_2 =mysql.GetRow(7);
	/*		if((stDaoNoticeUser.m_type==NOTICE_USER_FOLLOWED&&stDaoNoticeUser.m_pa_appid_md5==0)
			//		||(stDaoNoticeUser.m_type==NOTICE_USER_MESSAGED&&stDaoNoticeUser.m_pa_appid_md5==0)
					||(stDaoNoticeUser.m_type==NOTICE_USER_LIKED&&stDaoNoticeUser.m_pa_appid_md5==m_pa_appid_md5)
					||(stDaoNoticeUser.m_type==NOTICE_USER_COMMENTED&&stDaoNoticeUser.m_pa_appid_md5==m_pa_appid_md5))
		   */
			m_dao_notice_user_list.push_back(stDaoNoticeUser);


		}
		std::sort(m_dao_notice_user_list.begin(), m_dao_notice_user_list.end(), CreateTsDes);
		if(m_dao_notice_user_list.size()==0)
		{
		    return 0;
		}
    }


	return 1;
}

std::string DaoNoticeUserByAll::ToString() const
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_dao_notice_user_list.size(); ++i)
    {
    	if(m_dao_notice_user_list[i].m_type==NOTICE_USER_FOLLOWED
   // 			||m_dao_notice_user_list[i].m_type==NOTICE_USER_MESSAGED
    			||(m_dao_notice_user_list[i].m_pa_appid_md5==m_pa_appid_md5))
    	{
    		oss<<"list_%zu: "<<m_dao_notice_user_list[i].ToString()<<", ";
    	}
    }

    oss<<"]";

    return oss.str();
}
