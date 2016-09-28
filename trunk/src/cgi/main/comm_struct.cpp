#include "comm_struct.h"
#include "comm_def.h"

lce::cgi::CMysql TableBase::g_common_mysql;
lce::cgi::CMysql TableBase::g_main_mysql;

int CommonPicture::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where id='"<<sql_escape(m_id)<<"'";

	if(!g_common_mysql.Query(ossSql.str())) 
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(g_common_mysql.GetRowCount() && g_common_mysql.Next())
	{
		m_data = "";
		m_type = atoi(g_common_mysql.GetRow(1));
		m_data.assign(g_common_mysql.GetRow(2), g_common_mysql.GetRowLength(2));
		m_source= atoi(g_common_mysql.GetRow(3));
		m_owner_uin = g_common_mysql.GetRow(4);

		return TABLE_BASE_RET_OK;
	}
	
	strErrMsg = "Picture not exists!!, id=" + m_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int CommonPicture::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id='"<<sql_escape(m_id)<<"'"
			<<", type="<<m_type
			<<", source="<<m_source
			<<", data='"<<sql_escape(m_data)<<"'"
			<<", owner_uin='"<<sql_escape(m_owner_uin)<<"'"
			<<" on duplicate key"
			<<" update data='"<<sql_escape(m_data)<<"'"
			<<", type="<<m_type
			<<", source="<<m_source
			<<", owner_uin='"<<sql_escape(m_owner_uin)<<"'";
	if(!g_common_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string CommonPicture::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"id: "<<m_id
        <<", data_size: "<<m_data.size()
        <<", auth_type: "<<m_type
        <<", source: "<<m_source
        <<", owner: "<<m_owner_uin
        <<"}";

    return oss.str();
}

int Document::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where id='"<<sql_escape(m_id)<<"'";

	if(!g_common_mysql.Query(ossSql.str())) 
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(g_common_mysql.GetRowCount() && g_common_mysql.Next())
	{
		m_owner_uin = g_common_mysql.GetRow(4);
		m_data = "";
		m_type = atoi(g_common_mysql.GetRow(1));
		m_data.assign(g_common_mysql.GetRow(2), g_common_mysql.GetRowLength(2));
		m_source= atoi(g_common_mysql.GetRow(3));
		

		return TABLE_BASE_RET_OK;
	}
	
	strErrMsg = "Document not exists!!, id=" + m_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int Document::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id='"<<sql_escape(m_id)<<"'"
			<<", type="<<m_type
			<<", source="<<m_source
			<<", data='"<<sql_escape(m_data)<<"'"
			<<", owner_uin='"<<sql_escape(m_owner_uin)<<"'"
			<<" on duplicate key"
			<<" update data='"<<sql_escape(m_data)<<"'"
			<<", type="<<m_type
			<<", source="<<m_source
			<<", owner_uin='"<<sql_escape(m_owner_uin)<<"'";
	if(!g_common_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string Document::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"id: "<<m_id
        <<", data_size: "<<m_data.size()
        <<", doc_type:"<<m_type
        <<", source: "<<m_source
        <<", owner: "<<m_owner_uin
        <<"}";

    return oss.str();
}

int Password::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select password from "<<m_table_name<<" where uin='"<<sql_escape(m_uin)<<"'";

	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(g_main_mysql.GetRowCount() && g_main_mysql.Next())
	{
		m_password= g_main_mysql.GetRow(0);
		return TABLE_BASE_RET_OK;
	}

	strErrMsg = "Password not exists!!, uin=" + m_uin;
	return TABLE_BASE_RET_NOT_EXIST;
}

int Password::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set uin='"<<sql_escape(m_uin)<<"'"
			<<", password='"<<sql_escape(m_password)<<"'"
			<<" on duplicate key"
			<<" update password='"<<sql_escape(m_password)<<"'";
	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string Password::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"uin: "<<m_uin
        <<", password: "<<m_password
        <<"}";

    return oss.str();
}

int UserInfo::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where uin='"<<sql_escape(m_uin)<<"'"; 
	if(!g_main_mysql.Query(ossSql.str())) 
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(g_main_mysql.GetRowCount() && g_main_mysql.Next())
	{
		m_user_type = ::atoi(g_main_mysql.GetRow(1));
		m_nick = g_main_mysql.GetRow(2); 
		m_headportait_url = g_main_mysql.GetRow(3);
		m_contactor_name = g_main_mysql.GetRow(4);
		m_contactor_phone_no = g_main_mysql.GetRow(5);
		m_contactor_wx = g_main_mysql.GetRow(6);
		m_contactor_qq = g_main_mysql.GetRow(7);
		m_organization_name = g_main_mysql.GetRow(8);
		m_dev_id = g_main_mysql.GetRow(9);
		m_oper_id = g_main_mysql.GetRow(10);
		m_ts = ::atoi(g_main_mysql.GetRow(11));
		return TABLE_BASE_RET_OK;
	}
	
	strErrMsg = "UserInfo not exists!!, uin=" + m_uin;
	return TABLE_BASE_RET_NOT_EXIST;
}

int UserInfo::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set uin='"<<sql_escape(m_uin)<<"'"
			<<", user_type="<<m_user_type
			<<", nick='"<<sql_escape(m_nick)<<"'"
			<<", headportait='"<<sql_escape(m_headportait_url)<<"'"
			<<", contactor_name='"<<sql_escape(m_contactor_name)<<"'"
			<<", contactor_phone_no='"<<sql_escape(m_contactor_phone_no)<<"'"
			<<", contactor_wx='"<<sql_escape(m_contactor_wx)<<"'"
			<<", contactor_qq='"<<sql_escape(m_contactor_qq)<<"'"
			<<", organization_name='"<<sql_escape(m_organization_name)<<"'"
			<<", dev_id='"<<sql_escape(m_dev_id)<<"'"
			<<", oper_id='"<<sql_escape(m_oper_id)<<"'"
			<<", ts="<<m_ts
			<<" on duplicate key"
			<<" update user_type="<<m_user_type
			<<", nick='"<<sql_escape(m_nick)<<"'"
			<<", headportait='"<<sql_escape(m_headportait_url)<<"'"
			<<", contactor_name='"<<sql_escape(m_contactor_name)<<"'"
			<<", contactor_phone_no='"<<sql_escape(m_contactor_phone_no)<<"'"
			<<", contactor_wx='"<<sql_escape(m_contactor_wx)<<"'"
			<<", contactor_qq='"<<sql_escape(m_contactor_qq)<<"'"
			<<", organization_name='"<<sql_escape(m_organization_name)<<"'"
			<<", dev_id='"<<sql_escape(m_dev_id)<<"'"
			<<", oper_id='"<<sql_escape(m_oper_id)<<"'"
			<<", ts="<<m_ts;
	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string UserInfo::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"uin: "<<m_uin
        <<", user_type: "<<m_user_type
        <<", nick: "<<m_nick
        <<", headportait="<<m_headportait_url
        <<", contactor_name="<<m_contactor_name
		<<", contactor_phone_no="<<m_contactor_phone_no
		<<", contactor_wx="<<m_contactor_wx
		<<", contactor_qq="<<m_contactor_qq
		<<", organization_name="<<m_organization_name
		<<", dev_id="<<m_dev_id
		<<", oper_id="<<m_oper_id
		<<", ts="<<m_ts;

    return oss.str();
}

void UserInfo::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["uin"] = m_uin;
	stAnyValue["user_type"] = m_user_type;
	stAnyValue["nick"] = m_nick;
	stAnyValue["headportait"] = m_headportait_url;
	stAnyValue["contactor_name"] = m_contactor_name;
	stAnyValue["contactor_phone_no"] = m_contactor_phone_no;
	stAnyValue["contactor_wx"] = m_contactor_wx;
	stAnyValue["contactor_qq"] = m_contactor_qq;
	stAnyValue["organization_name"] = m_organization_name;
	stAnyValue["dev_id"] = m_dev_id;
	stAnyValue["oper_id"] = m_oper_id;
    stAnyValue["ts"] = m_ts;
}

void UserInfo::ToSimpleAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["uin"] = m_uin;
	stAnyValue["contactor_name"] = m_contactor_name;
	stAnyValue["contactor_phone_no"] = m_contactor_phone_no;
	stAnyValue["contactor_wx"] = m_contactor_wx;
	stAnyValue["contactor_qq"] = m_contactor_qq;
	stAnyValue["dev_id"] = m_dev_id;
	stAnyValue["oper_id"] = m_oper_id;
}

int DeveloperInfo::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where id='"<<sql_escape(m_id)<<"'"; 
	if(!g_main_mysql.Query(ossSql.str())) 
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(g_main_mysql.GetRowCount() && g_main_mysql.Next())
	{
		m_uin = g_main_mysql.GetRow(1);
		m_type = ::atol(g_main_mysql.GetRow(2));
		m_status = ::atol(g_main_mysql.GetRow(3));
		m_expire_ts = ::atol(g_main_mysql.GetRow(4));
		m_evaluate_active = ::atol(g_main_mysql.GetRow(5));
		m_evaluate_quality = ::atol(g_main_mysql.GetRow(6));
		m_evaluate_ontime = ::atol(g_main_mysql.GetRow(7));
		m_region_area = g_main_mysql.GetRow(8);
		m_introduction = g_main_mysql.GetRow(9);
		m_dev_field = ::atol(g_main_mysql.GetRow(10));
		m_dev_language = ::atol(g_main_mysql.GetRow(11));
		m_show_case = g_main_mysql.GetRow(12);
		m_company_name =g_main_mysql.GetRow(13);
		m_company_detail_address =g_main_mysql.GetRow(14);
		m_company_phone_no = g_main_mysql.GetRow(15);
		m_company_email = g_main_mysql.GetRow(16);
		m_company_business_licence_pic_url = g_main_mysql.GetRow(17);
		m_company_code = g_main_mysql.GetRow(18);
		m_company_corporation_name = g_main_mysql.GetRow(19);
		m_deal_num = ::atol(g_main_mysql.GetRow(20));
		
		return TABLE_BASE_RET_OK;
	}
	
	strErrMsg = "DeveloperInfo not exists!!, developer_id=" + m_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int DeveloperInfo::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id='"<<sql_escape(m_id)<<"'"
			<<", uin='"<<sql_escape(m_uin)<<"'"
			<<", type="<<m_type
			<<", status="<<m_status
			<<", expire_ts="<<m_expire_ts
			<<", evaluate_active="<<m_evaluate_active
			<<", evaluate_quality="<<m_evaluate_quality
			<<", evaluate_ontime="<<m_evaluate_ontime
			<<", region_area='"<<sql_escape(m_region_area)<<"'"
			<<", introduction='"<<sql_escape(m_introduction)<<"'"
			<<", dev_field="<<m_dev_field
			<<", dev_language="<<m_dev_language
			<<", show_case='"<<sql_escape(m_show_case)<<"'"
			<<", company_name='"<<sql_escape(m_company_name)<<"'"
			<<", company_detail_address='"<<sql_escape(m_company_detail_address)<<"'"
			<<", company_phone_no='"<<sql_escape(m_company_phone_no)<<"'"
			<<", company_email='"<<sql_escape(m_company_email)<<"'"
			<<", company_business_licence_pic_url='"<<sql_escape(m_company_business_licence_pic_url)<<"'"
			<<", company_code='"<<sql_escape(m_company_code)<<"'"
			<<", company_corporation_name='"<<sql_escape(m_company_corporation_name)<<"'"
			<<", deal_num="<<m_deal_num
			<<" on duplicate key"
			<<" update uin='"<<sql_escape(m_uin)<<"'"
			<<", type="<<m_type
			<<", status="<<m_status
			<<", expire_ts="<<m_expire_ts
			<<", evaluate_active="<<m_evaluate_active
			<<", evaluate_quality="<<m_evaluate_quality
			<<", evaluate_ontime="<<m_evaluate_ontime
			<<", region_area='"<<sql_escape(m_region_area)<<"'"
			<<", introduction='"<<sql_escape(m_introduction)<<"'"
			<<", dev_field="<<m_dev_field
			<<", dev_language="<<m_dev_language
			<<", show_case='"<<sql_escape(m_show_case)<<"'"
			<<", company_name='"<<sql_escape(m_company_name)<<"'"
			<<", company_detail_address='"<<sql_escape(m_company_detail_address)<<"'"
			<<", company_phone_no='"<<sql_escape(m_company_phone_no)<<"'"
			<<", company_email='"<<sql_escape(m_company_email)<<"'"
			<<", company_business_licence_pic_url='"<<sql_escape(m_company_business_licence_pic_url)<<"'"
			<<", company_code='"<<sql_escape(m_company_code)<<"'"
			<<", company_corporation_name='"<<sql_escape(m_company_corporation_name)<<"'"
			<<", deal_num="<<m_deal_num;

	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string DeveloperInfo::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"id: "<<m_id
        <<", uin: "<<m_uin
        <<", type: "<<m_type
		<<", status="<<m_status
		<<", expire_ts="<<m_expire_ts
		<<", evaluate_active="<<m_evaluate_active
		<<", evaluate_quality="<<m_evaluate_quality
		<<", evaluate_ontime="<<m_evaluate_ontime
		<<", region_area="<<m_region_area
		<<", introduction="<<m_introduction
		<<", dev_field="<<m_dev_field
		<<", dev_language="<<m_dev_language
		<<", show_case="<<m_show_case
		<<", company_name="<<m_company_name
		<<", company_detail_address="<<m_company_detail_address
		<<", company_phone_no="<<m_company_phone_no
		<<", company_email="<<m_company_email
		<<", company_business_licence_pic_url="<<m_company_business_licence_pic_url
		<<", company_code="<<m_company_code
		<<", company_corporation_name="<<m_company_corporation_name
		<<", deal_num="<<m_deal_num
        <<"}";

    return oss.str();
}

void DeveloperInfo::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["id"]= m_id;
	stAnyValue["uin"]= m_uin;
	stAnyValue["type"]= m_type;
	stAnyValue["status"]= m_status;
	stAnyValue["expire_ts"]=m_expire_ts;
	stAnyValue["evaluate_active"] = m_evaluate_active;
	stAnyValue["evaluate_quality"]  = m_evaluate_quality;
	stAnyValue["evaluate_ontime"] = m_evaluate_ontime;
	stAnyValue["region_area"] = m_region_area;
	stAnyValue["introduction"] = m_introduction;
	stAnyValue["dev_field"] = m_dev_field;
	stAnyValue["dev_language"] = m_dev_language;
	stAnyValue["show_case"] = m_show_case;
	stAnyValue["company_name"] = m_company_name;
	stAnyValue["company_detail_address"] = m_company_detail_address;
	stAnyValue["company_phone_no"] = m_company_phone_no;
	stAnyValue["company_email"] = m_company_email;
	stAnyValue["company_business_licence_pic_url"] = m_company_business_licence_pic_url;
	stAnyValue["company_code"] = m_company_code;
	stAnyValue["company_corporation_name"] = m_company_corporation_name;
	stAnyValue["deal_num"] = m_deal_num;
}


int DeveloperInfoWithFieldLanguageTypeByOrder::SelectFromDB(uint32_t& dwTotalNum, std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where status="<<DEVELOPER_STATUS_VERIFY_OK;
	if(m_type != 0)
	{
		ossSql<<" and type="<<m_type;
	}
	ossSql<<" and dev_field&"<<m_dev_field<<"!=0";
	ossSql<<" and dev_language&"<<m_dev_language<<"!=0";
	if(!g_main_mysql.Query(ossSql.str())) 
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(0 == g_main_mysql.GetRowCount())
    {
        strErrMsg = "DeveloperInfo not exists!!, type=" + int_2_str(m_type);
        dwTotalNum = 0;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	m_developer_info_list.clear();
	vector<DeveloperInfo> tmp_developer_info_list;
	while(g_main_mysql.Next())
	{
	    DeveloperInfo stDeveloperInfo(m_table_name);
		stDeveloperInfo.m_id = g_main_mysql.GetRow(0);
        stDeveloperInfo.m_uin = g_main_mysql.GetRow(1);
		stDeveloperInfo.m_type = ::atol(g_main_mysql.GetRow(2));
		stDeveloperInfo.m_status = ::atol(g_main_mysql.GetRow(3));
		stDeveloperInfo.m_expire_ts = ::atol(g_main_mysql.GetRow(4));
		stDeveloperInfo.m_evaluate_active = ::atol(g_main_mysql.GetRow(5));
		stDeveloperInfo.m_evaluate_quality = ::atol(g_main_mysql.GetRow(6));
		stDeveloperInfo.m_evaluate_ontime = ::atol(g_main_mysql.GetRow(7));
		stDeveloperInfo.m_region_area = g_main_mysql.GetRow(8);
		stDeveloperInfo.m_introduction = g_main_mysql.GetRow(9);
		stDeveloperInfo.m_dev_field = ::atol(g_main_mysql.GetRow(10));
		stDeveloperInfo.m_dev_language = ::atol(g_main_mysql.GetRow(11));
		stDeveloperInfo.m_show_case = g_main_mysql.GetRow(12);
		stDeveloperInfo.m_company_name =g_main_mysql.GetRow(13);
		stDeveloperInfo.m_company_detail_address =g_main_mysql.GetRow(14);
		stDeveloperInfo.m_company_phone_no = g_main_mysql.GetRow(15);
		stDeveloperInfo.m_company_email = g_main_mysql.GetRow(16);
		stDeveloperInfo.m_company_business_licence_pic_url = g_main_mysql.GetRow(17);
		stDeveloperInfo.m_company_code = g_main_mysql.GetRow(18);
		stDeveloperInfo.m_company_corporation_name = g_main_mysql.GetRow(19);
		stDeveloperInfo.m_deal_num = ::atoi(g_main_mysql.GetRow(20));
		
		tmp_developer_info_list.push_back(stDeveloperInfo);
	}

	dwTotalNum = tmp_developer_info_list.size();
	
	if(m_limit_from >= tmp_developer_info_list.size())
	{
		return TABLE_BASE_RET_NOT_EXIST;
	}
	
	if(DeveloperInfoOrderBy_ACTIVE == m_order_by)
	{
		std::sort(tmp_developer_info_list.begin(), tmp_developer_info_list.end(), CompareActive);
	}
	else if(DeveloperInfoOrderBy_QUALITY == m_order_by)
	{
		std::sort(tmp_developer_info_list.begin(), tmp_developer_info_list.end(), CompareQuality);
	}
	else if(DeveloperInfoOrderBy_ONTIME == m_order_by)
	{
		std::sort(tmp_developer_info_list.begin(), tmp_developer_info_list.end(), CompareOntime);
	}
	else if(DeveloperInfoOrderBy_STAR== m_order_by)
	{
		std::sort(tmp_developer_info_list.begin(), tmp_developer_info_list.end(), CompareStar);
	}

	for(size_t i=m_limit_from; i<m_limit_from+m_limit_len && i<tmp_developer_info_list.size(); ++i)
	{
		m_developer_info_list.push_back(tmp_developer_info_list[i]);
	}
	
	return TABLE_BASE_RET_OK;
}

std::string DeveloperInfoWithFieldLanguageTypeByOrder::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_developer_info_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_developer_info_list[i].ToString()<<", ";
    }
    
    oss<<"]";

    return oss.str();
}

int OperInfo::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where id='"<<sql_escape(m_id)<<"'"; 
	if(!g_main_mysql.Query(ossSql.str())) 
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
		return -1;
	}

	if(g_main_mysql.GetRowCount() && g_main_mysql.Next())
	{
		m_uin = g_main_mysql.GetRow(1);
		m_status = ::atol(g_main_mysql.GetRow(2));
		m_expire_ts = ::atol(g_main_mysql.GetRow(3));
		m_company_introduction = g_main_mysql.GetRow(4);
		m_company_name =g_main_mysql.GetRow(5);
		m_company_detail_address =g_main_mysql.GetRow(6);
		m_company_phone_no = g_main_mysql.GetRow(7);
		m_company_email = g_main_mysql.GetRow(8);
		m_company_business_licence_pic_url = g_main_mysql.GetRow(9);
		m_company_code = g_main_mysql.GetRow(10);
		m_company_corporation_name = g_main_mysql.GetRow(11);
		
		return TABLE_BASE_RET_OK;
	}
	
	strErrMsg = "OperInfo not exists!!, oper_id=" + m_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int OperInfo::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id='"<<sql_escape(m_id)<<"'"
			<<", uin='"<<sql_escape(m_uin)<<"'"
			<<", status="<<m_status
			<<", expire_ts="<<m_expire_ts
			<<", company_introduction='"<<sql_escape(m_company_introduction)<<"'"
			<<", company_name='"<<sql_escape(m_company_name)<<"'"
			<<", company_detail_address='"<<sql_escape(m_company_detail_address)<<"'"
			<<", company_phone_no='"<<sql_escape(m_company_phone_no)<<"'"
			<<", company_email='"<<sql_escape(m_company_email)<<"'"
			<<", company_business_licence_pic_url='"<<sql_escape(m_company_business_licence_pic_url)<<"'"
			<<", company_code='"<<sql_escape(m_company_code)<<"'"
			<<", company_corporation_name='"<<sql_escape(m_company_corporation_name)<<"'"
			<<" on duplicate key"
			<<" update uin='"<<sql_escape(m_uin)<<"'"
			<<", status="<<m_status
			<<", expire_ts="<<m_expire_ts
			<<", company_introduction='"<<sql_escape(m_company_introduction)<<"'"
			<<", company_name='"<<sql_escape(m_company_name)<<"'"
			<<", company_detail_address='"<<sql_escape(m_company_detail_address)<<"'"
			<<", company_phone_no='"<<sql_escape(m_company_phone_no)<<"'"
			<<", company_email='"<<sql_escape(m_company_email)<<"'"
			<<", company_business_licence_pic_url='"<<sql_escape(m_company_business_licence_pic_url)<<"'"
			<<", company_code='"<<sql_escape(m_company_code)<<"'"
			<<", company_corporation_name='"<<sql_escape(m_company_corporation_name)<<"'";

	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string OperInfo::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"id: "<<m_id
        <<", uin: "<<m_uin
		<<", status="<<m_status
		<<", expire_ts="<<m_expire_ts
		<<", introduction="<<m_company_introduction
		<<", company_name="<<m_company_name
		<<", company_detail_address="<<m_company_detail_address
		<<", company_phone_no="<<m_company_phone_no
		<<", company_email="<<m_company_email
		<<", company_business_licence_pic_url="<<m_company_business_licence_pic_url
		<<", company_code="<<m_company_code
		<<", company_corporation_name="<<m_company_corporation_name
        <<"}";

    return oss.str();
}

void OperInfo::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["id"]= m_id;
	stAnyValue["uin"]= m_uin;
	stAnyValue["status"]= m_status;
	stAnyValue["expire_ts"]=m_expire_ts;
	stAnyValue["introduction"] = m_company_introduction;
	stAnyValue["company_name"] = m_company_name;
	stAnyValue["company_detail_address"] = m_company_detail_address;
	stAnyValue["company_phone_no"] = m_company_phone_no;
	stAnyValue["company_email"] = m_company_email;
	stAnyValue["company_business_licence_pic_url"] = m_company_business_licence_pic_url;
	stAnyValue["company_code"] = m_company_code;
	stAnyValue["company_corporation_name"] = m_company_corporation_name;
}


int WXPublicAccount::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where wx='"<<sql_escape(m_wx)<<"'"; 
	if(!g_main_mysql.Query(ossSql.str())) 
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
		return -1;
	}

	if(g_main_mysql.GetRowCount() && g_main_mysql.Next())
	{
		m_oper_id = g_main_mysql.GetRow(1);
		m_name = g_main_mysql.GetRow(2);
		m_headportrait_url = g_main_mysql.GetRow(3);
		m_qrcode_url = g_main_mysql.GetRow(4);
		m_introduction = g_main_mysql.GetRow(5);
		m_tag = ::atol(g_main_mysql.GetRow(6));
		m_fans_num = ::atol(g_main_mysql.GetRow(7));
		m_read_num = ::atol(g_main_mysql.GetRow(8));
		m_identified= ::atol(g_main_mysql.GetRow(9));
		m_identified_info = g_main_mysql.GetRow(10);
		m_price_multi_pic_text_first_soft = ::atol(g_main_mysql.GetRow(11));
		m_price_multi_pic_text_first_hard = ::atol(g_main_mysql.GetRow(12));
		m_price_multi_pic_text_second_soft = ::atol(g_main_mysql.GetRow(13));
		m_price_multi_pic_text_second_hard = ::atol(g_main_mysql.GetRow(14));
		m_price_multi_pic_text_third_soft= ::atol(g_main_mysql.GetRow(15));
		m_price_multi_pic_text_third_hard= ::atol(g_main_mysql.GetRow(16));
		m_price_single_pic_text_soft = ::atol(g_main_mysql.GetRow(17));
		m_price_single_pic_text_hard = ::atol(g_main_mysql.GetRow(18));
		m_status = ::atol(g_main_mysql.GetRow(19));

		return TABLE_BASE_RET_OK;
	}
	
	strErrMsg = "WXPublicAccount not exists!!, wx=" + m_wx;
	return TABLE_BASE_RET_NOT_EXIST;
}

int WXPublicAccount::DeleteFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"delete from "<<m_table_name<<" where wx='"<<sql_escape(m_wx)<<"'"; 
	if(!g_main_mysql.Query(ossSql.str())) 
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
		return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

int WXPublicAccount::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set wx='"<<sql_escape(m_wx)<<"'"
			<<", oper_id='"<<sql_escape(m_oper_id)<<"'"
			<<", name='"<<sql_escape(m_name)<<"'"
			<<", head_portrait_url='"<<sql_escape(m_headportrait_url)<<"'"
			<<", qr_code_url='"<<sql_escape(m_qrcode_url)<<"'"
			<<", introduction='"<<sql_escape(m_introduction)<<"'"
			<<", tag="<<m_tag
			<<", fans_num="<<m_fans_num
			<<", read_num="<<m_read_num
			<<", identified="<<m_identified
			<<", identified_info='"<<sql_escape(m_identified_info)<<"'"
			<<", price_multi_pic_text_first_soft="<<m_price_multi_pic_text_first_soft
			<<", price_multi_pic_text_first_hard="<<m_price_multi_pic_text_first_hard
			<<", price_multi_pic_text_second_soft="<<m_price_multi_pic_text_second_soft
			<<", price_multi_pic_text_second_hard="<<m_price_multi_pic_text_second_hard
			<<", price_multi_pic_text_third_soft="<<m_price_multi_pic_text_third_soft
			<<", price_multi_pic_text_third_hard="<<m_price_multi_pic_text_third_hard
			<<", price_single_pic_text_soft="<<m_price_single_pic_text_soft
			<<", price_single_pic_text_hard="<<m_price_single_pic_text_hard
			<<", status="<<m_status
			<<" on duplicate key"
			<<" update oper_id='"<<sql_escape(m_oper_id)<<"'"
			<<", name='"<<sql_escape(m_name)<<"'"
			<<", head_portrait_url='"<<sql_escape(m_headportrait_url)<<"'"
			<<", qr_code_url='"<<sql_escape(m_qrcode_url)<<"'"
			<<", introduction='"<<sql_escape(m_introduction)<<"'"
			<<", tag="<<m_tag
			<<", fans_num="<<m_fans_num
			<<", read_num="<<m_read_num
			<<", identified="<<m_identified
			<<", identified_info='"<<sql_escape(m_identified_info)<<"'"
			<<", price_multi_pic_text_first_soft="<<m_price_multi_pic_text_first_soft
			<<", price_multi_pic_text_first_hard="<<m_price_multi_pic_text_first_hard
			<<", price_multi_pic_text_second_soft="<<m_price_multi_pic_text_second_soft
			<<", price_multi_pic_text_second_hard="<<m_price_multi_pic_text_second_hard
			<<", price_multi_pic_text_third_soft="<<m_price_multi_pic_text_third_soft
			<<", price_multi_pic_text_third_hard="<<m_price_multi_pic_text_third_hard
			<<", price_single_pic_text_soft="<<m_price_single_pic_text_soft
			<<", price_single_pic_text_hard="<<m_price_single_pic_text_hard
			<<", status="<<m_status;

	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string WXPublicAccount::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"wx: "<<m_wx
		<<", oper_id="<<m_oper_id
		<<", name="<<m_name
		<<", head_portrait_url="<<m_headportrait_url
		<<", qr_code_url="<<m_qrcode_url
		<<", introduction="<<m_introduction
		<<", tag="<<m_tag
		<<", fans_num="<<m_fans_num
		<<", read_num="<<m_read_num
		<<", identified="<<m_identified
		<<", identified_info="<<m_identified_info
		<<", price_multi_pic_text_first_soft="<<m_price_multi_pic_text_first_soft
		<<", price_multi_pic_text_first_hard="<<m_price_multi_pic_text_first_hard
		<<", price_multi_pic_text_second_soft="<<m_price_multi_pic_text_second_soft
		<<", price_multi_pic_text_second_hard="<<m_price_multi_pic_text_second_hard
		<<", price_multi_pic_text_third_soft="<<m_price_multi_pic_text_third_soft
		<<", price_multi_pic_text_third_hard="<<m_price_multi_pic_text_third_hard
		<<", price_single_pic_text_soft="<<m_price_single_pic_text_soft
		<<", price_single_pic_text_hard="<<m_price_single_pic_text_hard
		<<", status="<<m_status
        <<"}";

    return oss.str();
}

void WXPublicAccount::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["wx"]=m_wx;
	stAnyValue["oper_id"]= m_oper_id;
	stAnyValue["name"]= m_name;
	stAnyValue["head_portrait_url"] = m_headportrait_url;
	stAnyValue["qrcode_url"] = m_qrcode_url;
	stAnyValue["introduction"] = m_introduction;
	stAnyValue["tag"] = m_tag;
	stAnyValue["fans_num"] = m_fans_num;
	stAnyValue["read_num"] = m_read_num;
	stAnyValue["identified"] = m_identified;
	stAnyValue["identified_info"] = m_identified_info;
	stAnyValue["price_multi_pic_text_first_soft"] = m_price_multi_pic_text_first_soft;
	stAnyValue["price_multi_pic_text_first_hard"] = m_price_multi_pic_text_first_hard;
	stAnyValue["price_multi_pic_text_second_soft"] = m_price_multi_pic_text_second_soft;
	stAnyValue["price_multi_pic_text_second_hard"] = m_price_multi_pic_text_second_hard;
	stAnyValue["price_multi_pic_text_third_soft"] = m_price_multi_pic_text_third_soft;
	stAnyValue["price_multi_pic_text_third_hard"] = m_price_multi_pic_text_third_hard;
	stAnyValue["price_single_pic_text_soft"] = m_price_single_pic_text_soft;
	stAnyValue["price_single_pic_text_hard"] = m_price_single_pic_text_hard;
	stAnyValue["status"] = m_status;
}


int WXPublicAccountWithTagAndCertifiedStatus::SelectFromDB(uint32_t& dwTotalNum, std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where status="<<WX_PUBLIC_ACCOUNT_VERIFY_OK
		<<" and identified="<<m_cetified_status
		<<" and tag&"<<m_tag<<"!=0";
	if(!g_main_mysql.Query(ossSql.str())) 
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
		return -1;
	}

	if(0 == g_main_mysql.GetRowCount())
    {
        strErrMsg = "WXPublicAccount not exists!!, tag=" + int_2_str(m_tag);
        dwTotalNum = 0;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	m_wx_public_account_list.clear();
	std::vector<WXPublicAccount> tmp_wx_public_account_list;
	while(g_main_mysql.Next())
	{
		WXPublicAccount stWXPublicAccount;
		stWXPublicAccount.m_wx = g_main_mysql.GetRow(0);
		stWXPublicAccount.m_oper_id = g_main_mysql.GetRow(1);
		stWXPublicAccount.m_name = g_main_mysql.GetRow(2);
		stWXPublicAccount.m_headportrait_url = g_main_mysql.GetRow(3);
		stWXPublicAccount.m_qrcode_url = g_main_mysql.GetRow(4);
		stWXPublicAccount.m_introduction = g_main_mysql.GetRow(5);
		stWXPublicAccount.m_tag = ::atol(g_main_mysql.GetRow(6));
		stWXPublicAccount.m_fans_num = ::atol(g_main_mysql.GetRow(7));
		stWXPublicAccount.m_read_num = ::atol(g_main_mysql.GetRow(8));
		stWXPublicAccount.m_identified= ::atol(g_main_mysql.GetRow(9));
		stWXPublicAccount.m_identified_info = g_main_mysql.GetRow(10);
		stWXPublicAccount.m_price_multi_pic_text_first_soft = ::atol(g_main_mysql.GetRow(11));
		stWXPublicAccount.m_price_multi_pic_text_first_hard = ::atol(g_main_mysql.GetRow(12));
		stWXPublicAccount.m_price_multi_pic_text_second_soft = ::atol(g_main_mysql.GetRow(13));
		stWXPublicAccount.m_price_multi_pic_text_second_hard = ::atol(g_main_mysql.GetRow(14));
		stWXPublicAccount.m_price_multi_pic_text_third_soft= ::atol(g_main_mysql.GetRow(15));
		stWXPublicAccount.m_price_multi_pic_text_third_hard= ::atol(g_main_mysql.GetRow(16));
		stWXPublicAccount.m_price_single_pic_text_soft = ::atol(g_main_mysql.GetRow(17));
		stWXPublicAccount.m_price_single_pic_text_hard = ::atol(g_main_mysql.GetRow(18));
		stWXPublicAccount.m_status = ::atol(g_main_mysql.GetRow(19));

		tmp_wx_public_account_list.push_back(stWXPublicAccount);
	}

	dwTotalNum = tmp_wx_public_account_list.size();

	if(m_limit_from >= tmp_wx_public_account_list.size())
	{
		return TABLE_BASE_RET_NOT_EXIST;
	}

	for(size_t i=m_limit_from; i<m_limit_from+m_limit_len && i<tmp_wx_public_account_list.size(); ++i)
	{
		m_wx_public_account_list.push_back(tmp_wx_public_account_list[i]);
	}
	
	return TABLE_BASE_RET_OK;
}

std::string WXPublicAccountWithTagAndCertifiedStatus::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_wx_public_account_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_wx_public_account_list[i].ToString()<<", ";
    }
    
    oss<<"]";

    return oss.str();
}

int WXPublicAccountWithOperId::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where oper_id='"<<sql_escape(m_oper_id)<<"'";
	if(!g_main_mysql.Query(ossSql.str())) 
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
		return -1;
	}

	if(0 == g_main_mysql.GetRowCount())
    {
        strErrMsg = "WXPublicAccount not exists!!, oper_id=" + m_oper_id;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	m_wx_public_account_list.clear();
	while(g_main_mysql.Next())
	{
		WXPublicAccount stWXPublicAccount;
		stWXPublicAccount.m_wx = g_main_mysql.GetRow(0);
		stWXPublicAccount.m_oper_id = g_main_mysql.GetRow(1);
		stWXPublicAccount.m_name = g_main_mysql.GetRow(2);
		stWXPublicAccount.m_headportrait_url = g_main_mysql.GetRow(3);
		stWXPublicAccount.m_qrcode_url = g_main_mysql.GetRow(4);
		stWXPublicAccount.m_introduction = g_main_mysql.GetRow(5);
		stWXPublicAccount.m_tag = ::atol(g_main_mysql.GetRow(6));
		stWXPublicAccount.m_fans_num = ::atol(g_main_mysql.GetRow(7));
		stWXPublicAccount.m_read_num = ::atol(g_main_mysql.GetRow(8));
		stWXPublicAccount.m_identified= ::atol(g_main_mysql.GetRow(9));
		stWXPublicAccount.m_identified_info = g_main_mysql.GetRow(10);
		stWXPublicAccount.m_price_multi_pic_text_first_soft = ::atol(g_main_mysql.GetRow(11));
		stWXPublicAccount.m_price_multi_pic_text_first_hard = ::atol(g_main_mysql.GetRow(12));
		stWXPublicAccount.m_price_multi_pic_text_second_soft = ::atol(g_main_mysql.GetRow(13));
		stWXPublicAccount.m_price_multi_pic_text_second_hard = ::atol(g_main_mysql.GetRow(14));
		stWXPublicAccount.m_price_multi_pic_text_third_soft= ::atol(g_main_mysql.GetRow(15));
		stWXPublicAccount.m_price_multi_pic_text_third_hard= ::atol(g_main_mysql.GetRow(16));
		stWXPublicAccount.m_price_single_pic_text_soft = ::atol(g_main_mysql.GetRow(17));
		stWXPublicAccount.m_price_single_pic_text_hard = ::atol(g_main_mysql.GetRow(18));
		stWXPublicAccount.m_status = ::atol(g_main_mysql.GetRow(19));

		m_wx_public_account_list.push_back(stWXPublicAccount);
	}

	return TABLE_BASE_RET_OK;
}

std::string WXPublicAccountWithOperId::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_wx_public_account_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_wx_public_account_list[i].ToString()<<", ";
    }
    
    oss<<"]";

    return oss.str();
}

int WXPublicAccountFeedBack::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where id='"<<sql_escape(m_id)<<"'"; 
	if(!g_main_mysql.Query(ossSql.str())) 
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(g_main_mysql.GetRowCount() && g_main_mysql.Next())
	{
		m_to_wx = g_main_mysql.GetRow(1); 
		m_from_uin = g_main_mysql.GetRow(2); 
		m_reason = ::atoi(g_main_mysql.GetRow(3));
		m_reason_extra = g_main_mysql.GetRow(4);
		m_ts = ::atoi(g_main_mysql.GetRow(5));
		return TABLE_BASE_RET_OK;
	}
	
	strErrMsg = "WXPublicAccountFeedBack not exists!!, id=" + m_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int WXPublicAccountFeedBack::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id='"<<sql_escape(m_id)<<"'"
			<<", to_wx='"<<sql_escape(m_to_wx)<<"'"
			<<", from_uin='"<<sql_escape(m_from_uin)<<"'"
			<<", reason="<<m_reason
			<<", reason_extra='"<<sql_escape(m_reason_extra)<<"'"
			<<", ts="<<m_ts
			<<" on duplicate key"
			<<" update to_wx='"<<sql_escape(m_to_wx)<<"'"
			<<", from_uin='"<<sql_escape(m_from_uin)<<"'"
			<<", reason="<<m_reason
			<<", reason_extra='"<<sql_escape(m_reason_extra)<<"'"
			<<", ts="<<m_ts;
	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string WXPublicAccountFeedBack::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"id: "<<m_id
        <<", to_wx: "<<m_to_wx
        <<", from_uin: "<<m_from_uin
        <<", reason="<<m_reason
        <<", reason_extra="<<m_reason_extra
		<<", ts="<<m_ts
		<<"}";

    return oss.str();
}

void WXPublicAccountFeedBack::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["id"] = m_id;
	stAnyValue["to_wx"] = m_to_wx;
	stAnyValue["from_uin"] = m_from_uin;
	stAnyValue["reason"] = m_reason;
	stAnyValue["reason_extra"] = m_reason_extra;
	stAnyValue["ts"] = m_ts;
}

int DevOrder::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where id='"<<sql_escape(m_id)<<"'"; 
	if(!g_main_mysql.Query(ossSql.str())) 
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(g_main_mysql.GetRowCount() && g_main_mysql.Next())
	{
		m_need_uin = g_main_mysql.GetRow(1); 
		m_dev_id = g_main_mysql.GetRow(2); 
		m_introduction = g_main_mysql.GetRow(3); 
		m_doc_url = g_main_mysql.GetRow(4); 
		m_like_preview_url = g_main_mysql.GetRow(5); 
		m_like_website_url = g_main_mysql.GetRow(6); 
		m_expect_info = g_main_mysql.GetRow(7);
		m_status = ::atoi(g_main_mysql.GetRow(8));
		m_create_ts = ::atoi(g_main_mysql.GetRow(9));
		m_dev_start_ts = ::atoi(g_main_mysql.GetRow(10));
		m_dev_end_ts = ::atoi(g_main_mysql.GetRow(11));
		m_cancel_ts = ::atoi(g_main_mysql.GetRow(12));
		return TABLE_BASE_RET_OK;
	}
	
	strErrMsg = "WXPublicAccountFeedBack not exists!!, id=" + m_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int DevOrder::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id='"<<sql_escape(m_id)<<"'"
			<<", need_uin='"<<sql_escape(m_need_uin)<<"'"
			<<", dev_id='"<<sql_escape(m_dev_id)<<"'"
			<<", introduction='"<<sql_escape(m_introduction)<<"'"
			<<", doc_url='"<<sql_escape(m_doc_url)<<"'"
			<<", like_preview_url='"<<sql_escape(m_like_preview_url)<<"'"
			<<", like_website_url='"<<sql_escape(m_like_website_url)<<"'"
			<<", expect_info='"<<sql_escape(m_expect_info)<<"'"
			<<", status="<<m_status
			<<", create_ts="<<m_create_ts
			<<", dev_start_ts="<<m_dev_start_ts
			<<", dev_end_ts="<<m_dev_end_ts
			<<", cancel_ts="<<m_cancel_ts
			<<" on duplicate key"
			<<" update need_uin='"<<sql_escape(m_need_uin)<<"'"
			<<", dev_id='"<<sql_escape(m_dev_id)<<"'"
			<<", introduction='"<<sql_escape(m_introduction)<<"'"
			<<", doc_url='"<<sql_escape(m_doc_url)<<"'"
			<<", like_preview_url='"<<sql_escape(m_like_preview_url)<<"'"
			<<", like_website_url='"<<sql_escape(m_like_website_url)<<"'"
			<<", expect_info='"<<sql_escape(m_expect_info)<<"'"
			<<", status="<<m_status
			<<", create_ts="<<m_create_ts
			<<", dev_start_ts="<<m_dev_start_ts
			<<", dev_end_ts="<<m_dev_end_ts
			<<", cancel_ts="<<m_cancel_ts;
	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string DevOrder::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"id: "<<m_id
        <<", need_uin: "<<m_need_uin
        <<", dev_id: "<<m_dev_id
        <<", introduction="<<m_introduction
        <<", doc_url="<<m_doc_url
		<<", like_preview_url="<<m_like_preview_url
		<<", like_website_url="<<m_like_website_url
		<<", expect_info="<<m_expect_info
		<<", status="<<m_status
		<<", create_ts="<<m_create_ts
		<<", dev_start_ts="<<m_dev_start_ts
		<<", dev_end_ts="<<m_dev_end_ts
		<<", cancel_ts="<<m_cancel_ts		
		<<"}";

    return oss.str();
}

void DevOrder::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["id"] = m_id;
	stAnyValue["need_uin"] = m_need_uin;
	stAnyValue["dev_id"] = m_dev_id;
	stAnyValue["introduction"] = m_introduction;
	stAnyValue["doc_url"] = m_doc_url;
	stAnyValue["like_preview_url"] = m_like_preview_url;
	stAnyValue["like_website_url"] = m_like_website_url;
	stAnyValue["expect_info"] = m_expect_info;
	stAnyValue["status"] = m_status;
	stAnyValue["create_ts"] = m_create_ts;
	stAnyValue["dev_start_ts"] = m_dev_start_ts;
	stAnyValue["dev_end_ts"] = m_dev_end_ts;
	stAnyValue["cancel_ts"] = m_cancel_ts;
}

int DevOrderWithNeedUin::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where need_uin='"<<sql_escape(m_need_uin)<<"'";
	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

    if(0 == g_main_mysql.GetRowCount())
    {
        strErrMsg = "DevOrder not exists!!, need_uin=" + m_need_uin;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	m_dev_order_list.clear();
	while(g_main_mysql.Next())
	{
		DevOrder stDevOrder;
		stDevOrder.m_id = g_main_mysql.GetRow(0); 
		stDevOrder.m_need_uin = g_main_mysql.GetRow(1); 
		stDevOrder.m_dev_id = g_main_mysql.GetRow(2); 
		stDevOrder.m_introduction = g_main_mysql.GetRow(3); 
		stDevOrder.m_doc_url = g_main_mysql.GetRow(4); 
		stDevOrder.m_like_preview_url = g_main_mysql.GetRow(5); 
		stDevOrder.m_like_website_url = g_main_mysql.GetRow(6); 
		stDevOrder.m_expect_info = g_main_mysql.GetRow(7);
		stDevOrder.m_status = ::atoi(g_main_mysql.GetRow(8));
		stDevOrder.m_create_ts = ::atoi(g_main_mysql.GetRow(9));
		stDevOrder.m_dev_start_ts = ::atoi(g_main_mysql.GetRow(10));
		stDevOrder.m_dev_end_ts = ::atoi(g_main_mysql.GetRow(11));
		stDevOrder.m_cancel_ts = ::atoi(g_main_mysql.GetRow(12));
		m_dev_order_list.push_back(stDevOrder);
	}

	return TABLE_BASE_RET_OK;
}

std::string DevOrderWithNeedUin::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_dev_order_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_dev_order_list[i].ToString()<<", ";
    }

    oss<<"]";

    return oss.str();
}

int DevOrderWithDevId::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where dev_id='"<<sql_escape(m_dev_id)<<"'";
	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

    if(0 == g_main_mysql.GetRowCount())
    {
        strErrMsg = "DevOrder not exists!!, dev_id=" + m_dev_id;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	m_dev_order_list.clear();
	while(g_main_mysql.Next())
	{
		DevOrder stDevOrder;
		stDevOrder.m_id = g_main_mysql.GetRow(0); 
		stDevOrder.m_need_uin = g_main_mysql.GetRow(1); 
		stDevOrder.m_dev_id = g_main_mysql.GetRow(2); 
		stDevOrder.m_introduction = g_main_mysql.GetRow(3); 
		stDevOrder.m_doc_url = g_main_mysql.GetRow(4); 
		stDevOrder.m_like_preview_url = g_main_mysql.GetRow(5); 
		stDevOrder.m_like_website_url = g_main_mysql.GetRow(6); 
		stDevOrder.m_expect_info = g_main_mysql.GetRow(7);
		stDevOrder.m_status = ::atoi(g_main_mysql.GetRow(8));
		stDevOrder.m_create_ts = ::atoi(g_main_mysql.GetRow(9));
		stDevOrder.m_dev_start_ts = ::atoi(g_main_mysql.GetRow(10));
		stDevOrder.m_dev_end_ts = ::atoi(g_main_mysql.GetRow(11));
		stDevOrder.m_cancel_ts = ::atoi(g_main_mysql.GetRow(12));
		m_dev_order_list.push_back(stDevOrder);
	}

	return TABLE_BASE_RET_OK;
}

std::string DevOrderWithDevId::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_dev_order_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_dev_order_list[i].ToString()<<", ";
    }

    oss<<"]";

    return oss.str();
}

int OperOrder::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where id='"<<sql_escape(m_id)<<"'"; 
	if(!g_main_mysql.Query(ossSql.str())) 
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(g_main_mysql.GetRowCount() && g_main_mysql.Next())
	{
		m_need_uin = g_main_mysql.GetRow(1); 
		m_tag = ::atoi(g_main_mysql.GetRow(2));
		m_wx_list = g_main_mysql.GetRow(3); 
		m_spread_name = g_main_mysql.GetRow(4); 
		m_spread_type = ::atoi(g_main_mysql.GetRow(5));
		m_spread_pos = ::atoi(g_main_mysql.GetRow(6));
		m_spread_start_ts = ::atoi(g_main_mysql.GetRow(7));
		m_spread_certified_pic_url = g_main_mysql.GetRow(8); 
		m_spread_ramark = g_main_mysql.GetRow(9); 
		m_chapter_title = g_main_mysql.GetRow(10); 
		m_chapter_author = g_main_mysql.GetRow(11); 
		m_chapter_cover_url = g_main_mysql.GetRow(12); 
		m_chapter_cover_insert_main_body = ::atoi(g_main_mysql.GetRow(13));
		m_chapter_summary = g_main_mysql.GetRow(14); 
		m_chapter_main_body = g_main_mysql.GetRow(15); 
		m_chapter_original_url = g_main_mysql.GetRow(16); 
		m_status = ::atoi(g_main_mysql.GetRow(17));
		m_create_ts = ::atoi(g_main_mysql.GetRow(18));
		m_cancel_ts = ::atoi(g_main_mysql.GetRow(19));
		return TABLE_BASE_RET_OK;
	}
	
	strErrMsg = "WXPublicAccountFeedBack not exists!!, id=" + m_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int OperOrder::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id='"<<sql_escape(m_id)<<"'"
			<<", need_uin='"<<sql_escape(m_need_uin)<<"'"
			<<", tag="<<m_tag
			<<", wx_list='"<<sql_escape(m_wx_list)<<"'"
			<<", spread_name='"<<sql_escape(m_spread_name)<<"'"
			<<", spread_type="<<m_spread_type
			<<", spread_pos="<<m_spread_pos
			<<", spread_start_ts="<<m_spread_start_ts
			<<", spread_certified_pic_url='"<<sql_escape(m_spread_certified_pic_url)<<"'"
			<<", spread_remark='"<<sql_escape(m_spread_ramark)<<"'"
			<<", chapter_title='"<<sql_escape(m_chapter_title)<<"'"
			<<", chapter_author='"<<sql_escape(m_chapter_author)<<"'"
			<<", chapter_cover_url='"<<sql_escape(m_chapter_cover_url)<<"'"
			<<", chapter_cover_insert_main_body="<<m_chapter_cover_insert_main_body
			<<", chapter_summary='"<<sql_escape(m_chapter_summary)<<"'"
			<<", chapter_main_body='"<<sql_escape(m_chapter_main_body)<<"'"
			<<", chapter_original_url='"<<sql_escape(m_chapter_original_url)<<"'"
			<<", status="<<m_status
			<<", create_ts="<<m_create_ts
			<<", cancel_ts="<<m_cancel_ts
			<<" on duplicate key"
			<<" update need_uin='"<<sql_escape(m_need_uin)<<"'"
			<<", tag="<<m_tag
			<<", wx_list='"<<sql_escape(m_wx_list)<<"'"
			<<", spread_name='"<<sql_escape(m_spread_name)<<"'"
			<<", spread_type="<<m_spread_type
			<<", spread_pos="<<m_spread_pos
			<<", spread_start_ts="<<m_spread_start_ts
			<<", spread_certified_pic_url='"<<sql_escape(m_spread_certified_pic_url)<<"'"
			<<", spread_remark='"<<sql_escape(m_spread_ramark)<<"'"
			<<", chapter_title='"<<sql_escape(m_chapter_title)<<"'"
			<<", chapter_author='"<<sql_escape(m_chapter_author)<<"'"
			<<", chapter_cover_url='"<<sql_escape(m_chapter_cover_url)<<"'"
			<<", chapter_cover_insert_main_body="<<m_chapter_cover_insert_main_body
			<<", chapter_summary='"<<sql_escape(m_chapter_summary)<<"'"
			<<", chapter_main_body='"<<sql_escape(m_chapter_main_body)<<"'"
			<<", chapter_original_url='"<<sql_escape(m_chapter_original_url)<<"'"
			<<", status="<<m_status
			<<", create_ts="<<m_create_ts
			<<", cancel_ts="<<m_cancel_ts;
	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string OperOrder::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"id: "<<m_id
        <<", need_uin: "<<m_need_uin
        <<", tag: "<<m_tag
        <<", wx_list="<<m_wx_list
        <<", spread_name="<<m_spread_name
		<<", spread_type="<<m_spread_type
		<<", spread_pos="<<m_spread_pos
		<<", spread_start_ts="<<m_spread_start_ts
		<<", spread_certified_pic_url="<<m_spread_certified_pic_url
		<<", spread_remark="<<m_spread_ramark
		<<", chapter_title="<<m_chapter_title
		<<", chapter_author="<<m_chapter_author
		<<", chapter_cover_url="<<m_chapter_cover_url
		<<", chapter_cover_insert_main_body="<<m_chapter_cover_insert_main_body
		<<", chapter_summary="<<m_chapter_summary
		<<", chapter_main_body="<<m_chapter_main_body
		<<", chapter_original_url="<<m_chapter_original_url
		<<", status="<<m_status
		<<", create_ts="<<m_create_ts
		<<", cancel_ts="<<m_cancel_ts				
		<<"}";

    return oss.str();
}

void OperOrder::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["id"] = m_id;
	stAnyValue["need_uin"] = m_need_uin;
	stAnyValue["tag"] = m_tag;
	stAnyValue["wx_list"] = m_wx_list;
	stAnyValue["spread_name"] = m_spread_name;
	stAnyValue["spread_type"] = m_spread_type;
	stAnyValue["spread_pos"] = m_spread_pos;
	stAnyValue["spread_start_ts"] = m_spread_start_ts;
	stAnyValue["spread_certified_pic_url"] = m_spread_certified_pic_url;
	stAnyValue["spread_remark"] = m_spread_ramark;
	stAnyValue["chapter_title"] = m_chapter_title;
	stAnyValue["chapter_author"] = m_chapter_author;
	stAnyValue["chapter_cover_url"] = m_chapter_cover_url;
	stAnyValue["chapter_cover_insert_main_body"] = m_chapter_cover_insert_main_body;
	stAnyValue["chapter_summary"] = m_chapter_summary;
	stAnyValue["chapter_main_body"] = m_chapter_main_body;
	stAnyValue["chapter_original_url"] = m_chapter_original_url;
	stAnyValue["status"] = m_status;
	stAnyValue["create_ts"] = m_create_ts;
	stAnyValue["cancel_ts"] = m_cancel_ts;
}

int OperOrderWithNeedUin::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where need_uin='"<<sql_escape(m_need_uin)<<"'";
	if(!g_main_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

    if(0 == g_main_mysql.GetRowCount())
    {
        strErrMsg = "OperOrder not exists!!, need_uin=" + m_need_uin;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	m_oper_order_list.clear();
	while(g_main_mysql.Next())
	{
		OperOrder stOperOrder;
		stOperOrder.m_id = g_main_mysql.GetRow(0); 
		stOperOrder.m_need_uin = g_main_mysql.GetRow(1); 
		stOperOrder.m_tag = ::atoi(g_main_mysql.GetRow(2));
		stOperOrder.m_wx_list = g_main_mysql.GetRow(3); 
		stOperOrder.m_spread_name = g_main_mysql.GetRow(4); 
		stOperOrder.m_spread_type = ::atoi(g_main_mysql.GetRow(5));
		stOperOrder.m_spread_pos = ::atoi(g_main_mysql.GetRow(6));
		stOperOrder.m_spread_start_ts = ::atoi(g_main_mysql.GetRow(7));
		stOperOrder.m_spread_certified_pic_url = g_main_mysql.GetRow(8); 
		stOperOrder.m_spread_ramark = g_main_mysql.GetRow(9); 
		stOperOrder.m_chapter_title = g_main_mysql.GetRow(10); 
		stOperOrder.m_chapter_author = g_main_mysql.GetRow(11); 
		stOperOrder.m_chapter_cover_url = g_main_mysql.GetRow(12); 
		stOperOrder.m_chapter_cover_insert_main_body = ::atoi(g_main_mysql.GetRow(13));
		stOperOrder.m_chapter_summary = g_main_mysql.GetRow(14); 
		stOperOrder.m_chapter_main_body = g_main_mysql.GetRow(15); 
		stOperOrder.m_chapter_original_url = g_main_mysql.GetRow(16); 
		stOperOrder.m_status = ::atoi(g_main_mysql.GetRow(17));
		stOperOrder.m_create_ts = ::atoi(g_main_mysql.GetRow(18));
		stOperOrder.m_cancel_ts = ::atoi(g_main_mysql.GetRow(19));
		m_oper_order_list.push_back(stOperOrder);
	}

	return TABLE_BASE_RET_OK;
}

std::string OperOrderWithNeedUin::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_oper_order_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_oper_order_list[i].ToString()<<", ";
    }

    oss<<"]";

    return oss.str();
}

int OperOrderReverse::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where wx='"<<sql_escape(m_wx)<<"'"; 
	ossSql<<" and oper_order_id='"<<sql_escape(m_oper_order_id)<<"'";
	if(!g_main_mysql.Query(ossSql.str())) 
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(g_main_mysql.GetRowCount() && g_main_mysql.Next())
	{
		m_oper_id = ::atoi(g_main_mysql.GetRow(2));
	}
	
	strErrMsg = "WXPublicAccountFeedBack not exists!!, wx=" + m_wx;
	return TABLE_BASE_RET_NOT_EXIST;
}

int OperOrderReverse::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set wx='"<<sql_escape(m_wx)<<"'"
			<<", oper_order_id='"<<sql_escape(m_oper_order_id)<<"'"
			<<", oper_id='"<<sql_escape(m_oper_id)<<"'"
			<<" on duplicate key"
			<<" update oper_id='"<<sql_escape(m_oper_id)<<"'";
	if(!g_main_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
		return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string OperOrderReverse::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"wx: "<<m_wx
        <<", oper_order_id: "<<m_oper_order_id
        <<", oper_id: "<<m_oper_id
		<<"}";

    return oss.str();
}

void OperOrderReverse::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["wx"] = m_wx;
	stAnyValue["oper_order_id"] = m_oper_order_id;
	stAnyValue["oper_id"] = m_oper_id;
}

int OperOrderReverseWithOperId::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where oper_id='"<<sql_escape(m_oper_id)<<"'";
	if(!g_main_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
		return -1;
	}

    if(0 == g_main_mysql.GetRowCount())
    {
        strErrMsg = "OperOrder not exists!!, oper_id=" + m_oper_id;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	m_oper_order_reverse_list.clear();
	while(g_main_mysql.Next())
	{
		OperOrderReverse stOperOrderReverse;
		stOperOrderReverse.m_wx = g_main_mysql.GetRow(0); 
		stOperOrderReverse.m_oper_order_id = g_main_mysql.GetRow(1); 
		stOperOrderReverse.m_oper_id = g_main_mysql.GetRow(2);
		m_oper_order_reverse_list.push_back(stOperOrderReverse);
	}

	return TABLE_BASE_RET_OK;
}

std::string OperOrderReverseWithOperId::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_oper_order_reverse_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_oper_order_reverse_list[i].ToString()<<", ";
    }

    oss<<"]";

    return oss.str();
}

int OperOrderReverseWithWx::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where wx='"<<sql_escape(m_wx)<<"'";
	if(!g_main_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
		return -1;
	}

    if(0 == g_main_mysql.GetRowCount())
    {
        strErrMsg = "OperOrder not exists!!, wx=" + m_wx;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	m_oper_order_reverse_list.clear();
	while(g_main_mysql.Next())
	{
		OperOrderReverse stOperOrderReverse;
		stOperOrderReverse.m_wx = g_main_mysql.GetRow(0); 
		stOperOrderReverse.m_oper_order_id = g_main_mysql.GetRow(1); 
		stOperOrderReverse.m_oper_id = g_main_mysql.GetRow(2);
		m_oper_order_reverse_list.push_back(stOperOrderReverse);
	}

	return TABLE_BASE_RET_OK;
}

std::string OperOrderReverseWithWx::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_oper_order_reverse_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_oper_order_reverse_list[i].ToString()<<", ";
    }

    oss<<"]";

    return oss.str();
}




int OpenOrder::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where id='"<<sql_escape(m_id)<<"'"; 
	if(!g_main_mysql.Query(ossSql.str())) 
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
			return -1;
	}

	if(g_main_mysql.GetRowCount() && g_main_mysql.Next())
	{
		m_need_uin = g_main_mysql.GetRow(1);
		m_need_type = ::atoi(g_main_mysql.GetRow(2));
		m_extra_info = g_main_mysql.GetRow(3);
		m_status = ::atoi(g_main_mysql.GetRow(4));
		m_create_ts = ::atoi(g_main_mysql.GetRow(5));
	}
	
	strErrMsg = "OpenOrder not exists!!, id=" + m_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int OpenOrder::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id='"<<sql_escape(m_id)<<"'"
			<<", need_uin='"<<sql_escape(m_need_uin)<<"'"
			<<", need_type="<<m_need_type
			<<", extra_info='"<<sql_escape(m_extra_info)<<"'"
			<<", status="<<m_status
			<<", create_ts="<<m_create_ts
			<<" on duplicate key"
			<<" update need_uin='"<<sql_escape(m_need_uin)<<"'"
			<<", need_type="<<m_need_type
			<<", extra_info='"<<sql_escape(m_extra_info)<<"'"
			<<", status="<<m_status
			<<", create_ts="<<m_create_ts;
	if(!g_main_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
		return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string OpenOrder::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"id: "<<m_id
        <<", need_uin: "<<m_need_uin
        <<", need_type: "<<m_need_type
        <<", extra_info: "<<m_extra_info
        <<", status: "<<m_status
        <<", create_ts: "<<m_create_ts
		<<"}";

    return oss.str();
}

void OpenOrder::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["id"] = m_id;
	stAnyValue["need_uin"] = m_need_uin;
	stAnyValue["need_type"] = m_need_type;
	stAnyValue["extra_info"] = m_extra_info;
	stAnyValue["status"] = m_status;
	stAnyValue["create_ts"] = m_create_ts;
}


int OpenOrderWithNeedUin::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where need_uin='"<<sql_escape(m_need_uin)<<"'";
	if(!g_main_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_main_mysql.GetErrMsg();
		return -1;
	}

    if(0 == g_main_mysql.GetRowCount())
    {
        strErrMsg = "OpenOrder not exists!!, need_uin=" + m_need_uin;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	m_open_order_list.clear();
	while(g_main_mysql.Next())
	{
		OpenOrder stOpenOrder;
		stOpenOrder.m_id = g_main_mysql.GetRow(0); 
		stOpenOrder.m_need_uin = g_main_mysql.GetRow(1); 
		stOpenOrder.m_need_type = ::atoi(g_main_mysql.GetRow(2));
		stOpenOrder.m_extra_info = g_main_mysql.GetRow(3);
		stOpenOrder.m_status = ::atoi(g_main_mysql.GetRow(4));
		stOpenOrder.m_create_ts = ::atoi(g_main_mysql.GetRow(5));
		m_open_order_list.push_back(stOpenOrder);
	}

	return TABLE_BASE_RET_OK;
}

std::string OpenOrderWithNeedUin::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_open_order_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_open_order_list[i].ToString()<<", ";
    }

    oss<<"]";

    return oss.str();
}


