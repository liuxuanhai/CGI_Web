#include "msg_content.h"
#include "common_util.h"
#include "global_var.h"

int MsgContent::SelectFromDB(reVecMsgContent replyVec, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	replyVec.clear();
    if(m_amount == 0)
    {
		return GetMsgById(replyVec, mysql, strErrMsg);
    }
    else
    {
    	return GetMsgList(replyVec, mysql, strErrMsg);		
    }

}

int MsgContent::DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{	
    uint64_t session_id = common::util::generate_session_id(m_openid_md5_from, m_openid_md5_to);
    uint16_t table_index = (uint16_t)session_id % 256;
    char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);

    int del_status = m_openid_md5_from > m_openid_md5_to ? 0x02 : 0x01;
    
	std::ostringstream ossSql;
    ossSql.str("");
	ossSql<<"SELECT del_status FROM t_session_msg_"<<strTableIndex
		<<" WHERE session_id = "<<session_id
		<<" AND msg_id = "<<m_id;

	if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return -1;
    }

    if(mysql.GetRowCount() && mysql.Next())
    {
    	del_status = del_status | atoi(mysql.GetRow(0));

		/*
		 *BOTH openid_a and openid_b del this message, then del this msg form TABLE: t_session_msg_ and t_msg_content_
		 */
		 
    	if(del_status == 0x03)	//DELETE
    	{
			ossSql.str("");
			ossSql<<"DELETE FROM t_session_msg_"<<strTableIndex
				<<" WHERE session_id = "<<session_id
				<<" AND msg_id = "<<m_id;

			if(!mysql.Query(ossSql.str()))
		    {
		        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		        return -1;
		    }

			table_index = (uint16_t)m_id % 256;
			sprintf(strTableIndex, "%02x", table_index);
		    ossSql.str("");
			ossSql<<"DELETE FROM t_msg_content_"<<strTableIndex
				<<" WHERE msg_id = "<<m_id;
				
			if(!mysql.Query(ossSql.str()))
		    {
		        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		        return -1;
		    }
				
    	}
    	else	//UPDATE
    	{
			ossSql.str("");
			ossSql<<"UPDATE t_session_msg_"<<strTableIndex
				<<" SET del_status = "<<del_status
				<<" WHERE session_id = "<<session_id
				<<" AND msg_id = "<<m_id;

			if(!mysql.Query(ossSql.str()))
		    {
		        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		        return -1;
		    }
    	}
	}
	else
	{
		strErrMsg = "SELECT E_NOT_EXIST: " + ossSql.str();
		return 0;
	}

	return 1;
}

int MsgContent::AddToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	/*
	 *INSERT INTO TABLE: t_msg_content_index
	 */
	 
	m_id = common::util::generate_unique_id();
	uint16_t table_index = (uint16_t)m_id % 256;

	 
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);

	std::ostringstream ossSql;
    ossSql.str("");
    ossSql<<"INSERT INTO t_msg_content_"<<strTableIndex
   		<<" VALUES ("<<m_id
   		<<", '"<<sql_escape(m_content)
   		<<"', "<<m_openid_md5_from
   		<<", "<<m_openid_md5_to
   		<<")";

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return -1;
    }


	/*
	 *INSERT INTO TABLE: t_session_msg_index
	 */
	 
    uint64_t session_id = common::util::generate_session_id(m_openid_md5_from, m_openid_md5_to);
    table_index = (uint16_t)session_id % 256;
    sprintf(strTableIndex, "%02x", table_index);
    m_create_ts = (uint64_t)time(NULL);
    int del_status = 0x00;
    ossSql.str("");
    ossSql<<"INSERT INTO t_session_msg_"<<strTableIndex
   		<<" VALUES ("<<session_id
   		<<", "<<m_id
   		<<", "<<m_create_ts
   		<<", "<<del_status
   		<<")";

    if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return -1;
    }

    /*
     *INSERT INTO TABLE: t_session_user_index
     */
    
    table_index = (uint16_t)m_openid_md5_from % 256;
    sprintf(strTableIndex, "%02x", table_index);
	
    //before insert, check data exists or not
	ossSql.str("");
	ossSql<<"SELECT * FROM t_session_user_"<<strTableIndex
		<<" WHERE session_id = "<<session_id;
		
	if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return -1;
    }

    if(mysql.GetRowCount()== 0)
    {
		ossSql.str("");
	    ossSql<<"INSERT INTO t_session_user_"<<strTableIndex
	    	<<" VALUES ("<<m_openid_md5_from
	    	<<", "<<m_openid_md5_to
	    	<<", "<<session_id
	    	<<", "<<m_create_ts
	    	<<", 0)";
	    	
	    if(!mysql.Query(ossSql.str()))
	    {
	        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
	        return -1;
	    }

		table_index = (uint16_t)m_openid_md5_to % 256;
    	sprintf(strTableIndex, "%02x", table_index);
	    ossSql.str("");
	    ossSql<<"INSERT INTO t_session_user_"<<strTableIndex
	    	<<" VALUES ("<<m_openid_md5_to
	    	<<", "<<m_openid_md5_from
	    	<<", "<<session_id
	    	<<", "<<m_create_ts
	    	<<", 1)";
	    	
	    if(!mysql.Query(ossSql.str()))
	    {
	        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
	        return -1;
	    }
    }
	else
	{
		table_index = (uint16_t)m_openid_md5_to % 256;
    	sprintf(strTableIndex, "%02x", table_index);
	    ossSql.str("");
	    ossSql<<"UPDATE t_session_user_"<<strTableIndex
	    	<<" SET newmsg_status=1 WHERE openid_md5_from="<<m_openid_md5_to
	    	<<" AND openid_md5_to="<<m_openid_md5_from;
	    	
	    if(!mysql.Query(ossSql.str()))
	    {
	        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
	        return -1;
	    }
	}
    

	return 0;
}

int MsgContent::GetMsgById(reVecMsgContent replyVec, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	uint64_t session_id = common::util::generate_session_id(m_openid_md5_from, m_openid_md5_to);
	uint16_t table_index = (uint16_t)session_id % 256;
	char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);

	::hoosho::commstruct::MsgContent stMsgContent;

	stMsgContent.set_id(m_id);

	std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"SELECT "
		<<" create_ts FROM t_session_msg_"<<strTableIndex
		<<" WHERE session_id = "<<session_id
		<<" AND msg_id = "<<m_id;
		
	if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return -1;
    }
    if(mysql.GetRowCount()&&mysql.Next())
    {
		stMsgContent.set_create_ts(strtoul(mysql.GetRow(0), NULL, 10));
    }
    else
    {
		strErrMsg = "SELECT E_NOT_EXIST: " + ossSql.str();
		return 0;
    }


    table_index = (uint16_t)m_id % 256;
    sprintf(strTableIndex, "%02x", table_index);
	ossSql.str("");
	ossSql<<"SELECT content, openid_md5_from, openid_md5_to FROM t_msg_content_"<<strTableIndex
		<<" WHERE msg_id = "<<m_id;

	if(!mysql.Query(ossSql.str()))
	{
	    strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
	    return -1;
	}

	if(mysql.GetRowCount()&&mysql.Next())
	{		
		stMsgContent.set_content(mysql.GetRow(0));
		stMsgContent.set_openid_md5_from(strtoul(mysql.GetRow(1), NULL, 10));
		stMsgContent.set_openid_md5_to(strtoul(mysql.GetRow(2), NULL, 10));
	}    
	else
	{
		strErrMsg = "SELECT E_NOT_EXIST: " + ossSql.str();
		return 0;
	}

	replyVec.push_back(stMsgContent);
	
	return 1;		
}

int MsgContent::GetMsgList(reVecMsgContent replyVec, lce::cgi::CMysql& mysql, std::string& strErrMsg)
{
	uint64_t session_id = common::util::generate_session_id(m_openid_md5_from, m_openid_md5_to);
    uint16_t table_index = (uint16_t)m_openid_md5_from % 256;
    char strTableIndex[4];
	sprintf(strTableIndex, "%02x", table_index);

    int del_status = m_openid_md5_from > m_openid_md5_to ? 0x02 : 0x01;

    std::ostringstream ossSql;
    ossSql.str("");
	ossSql<<"UPDATE t_session_user_"<<strTableIndex
		<<" SET newmsg_status=0"
		<<" WHERE openid_md5_from="<<m_openid_md5_from
		<<" AND openid_md5_to="<<m_openid_md5_to;

	if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return -1;
    }


	table_index = (uint16_t)session_id % 256;
    sprintf(strTableIndex, "%02x", table_index);
    ossSql.str("");
    if(m_id == 0)	//GET LATEST
    {
		ossSql<<"SELECT "
			<<" msg_id, create_ts FROM t_session_msg_"<<strTableIndex
			<<" WHERE session_id = "<<session_id
			<<" AND del_status != "<<del_status
			<<" ORDER BY create_ts DESC LIMIT "<<m_amount;
    }
    else	//GET OLD
    {
		ossSql<<"SELECT "
			<<" msg_id, create_ts FROM t_session_msg_"<<strTableIndex
			<<" WHERE session_id = "<<session_id
			<<"	AND msg_id < "<<m_id
			<<" AND del_status != "<<del_status
			<<" ORDER BY create_ts DESC LIMIT "<<m_amount;
    }
	

	if(!mysql.Query(ossSql.str()))
    {
        strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
        return -1;
    }
    
    uint64_t id_create_ts_array[m_amount][2];
    int i=0;    
	if(mysql.GetRowCount())
	{
		while(mysql.Next())
		{			
			id_create_ts_array[i][0] = common::util::charToUint64_t(mysql.GetRow(0));		//msg_id
			id_create_ts_array[i][1] = common::util::charToUint64_t(mysql.GetRow(1));		//create_ts	
			i++;
		}
	}
	else
	{
		strErrMsg = "SELECT E_NOT_EXIST: " + ossSql.str();
		return 1;		
	}

	while(i>0)
	{
		i--;
		::hoosho::commstruct::MsgContent stMsgContent;
		
		
		stMsgContent.set_id(id_create_ts_array[i][0]);
		stMsgContent.set_create_ts(id_create_ts_array[i][1]);

		uint64_t msg_id = id_create_ts_array[i][0];

		table_index = (uint16_t)msg_id % 256;
		sprintf(strTableIndex, "%02x", table_index);
		ossSql.str("");
		ossSql<<"SELECT content, openid_md5_from, openid_md5_to FROM t_msg_content_"<<strTableIndex
			<<" WHERE msg_id = "<<msg_id;

		if(!mysql.Query(ossSql.str()))
		{
		    strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + mysql.GetErrMsg();
		    return -1;
		}

		if(mysql.GetRowCount()&&mysql.Next())
		{
			stMsgContent.set_content(mysql.GetRow(0));
			stMsgContent.set_openid_md5_from(strtoul(mysql.GetRow(1), NULL, 10));
			stMsgContent.set_openid_md5_to(strtoul(mysql.GetRow(2), NULL, 10));
		}    
		else
		{
			strErrMsg = "SELECT E_NOT_EXIST: " + ossSql.str();
			return 1;
		}	
		replyVec.push_back(stMsgContent);
	}

	return 1;    
}

std::string MsgContent::ToString() const
{
	std::ostringstream oss;
	oss<<"{id:"<<m_id<<", create_ts="<<m_create_ts<<", content="<<m_content<<", openid_md5_from="<<m_openid_md5_from<<", openid_md5_to="<<m_openid_md5_to<<"}";
	return oss.str();
}


