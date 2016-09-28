#include "comm_struct_card.h"
#include "comm_def_card.h"
lce::cgi::CMysql TableBaseCard::g_hx_mysql;

int CardPicture::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where id='"<<sql_escape(m_id)<<"'";

	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}

	if(g_hx_mysql.GetRowCount() && g_hx_mysql.Next())
	{
		m_data = "";
		m_data.assign(g_hx_mysql.GetRow(1), g_hx_mysql.GetRowLength(1));
		m_status = atoi(g_hx_mysql.GetRow(2));
		m_uin = g_hx_mysql.GetRow(3);
		m_createtimestamp = ::atoi(g_hx_mysql.GetRow(4));
		m_pic_usage = ::atoi(g_hx_mysql.GetRow(4));

		return TABLE_BASE_RET_OK;
	}

	strErrMsg = "CardPicture not exists!!, id=" + m_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int CardPicture::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id='"<<sql_escape(m_id)<<"'"
			<<", data='"<<sql_escape(m_data)<<"'"
			<<", status="<<m_status
			<<", uin='"<<sql_escape(m_uin)<<"'"
			<<", create_timestamp="<<m_createtimestamp
			<<", pic_usage="<<m_pic_usage
			<<" on duplicate key"
			<<" update data='"<<sql_escape(m_data)<<"'"
			<<", status="<<m_status
			<<", uin='"<<sql_escape(m_uin)<<"'"
			<<", create_timestamp="<<m_createtimestamp
			<<", pic_usage="<<m_pic_usage;
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string CardPicture::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"id: "<<m_id
        <<", data_size: "<<m_data.size()
        <<", status: "<<m_status
        <<", uin: "<<m_uin
        <<", create_timestamp: "<<m_createtimestamp
        <<", pic_usage: "<<m_pic_usage
        <<"}";

    return oss.str();
}
int WxAccountAuthInfo::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where auth_appid='"<<sql_escape(m_auth_appid)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}

	if(g_hx_mysql.GetRowCount() && g_hx_mysql.Next())
	{
		m_auth_access_token = g_hx_mysql.GetRow(1);
		m_auth_refresh_token = g_hx_mysql.GetRow(2);
		m_expire_timestamp = ::atoi(g_hx_mysql.GetRow(3));
		return TABLE_BASE_RET_OK;
	}

	strErrMsg = "WxAccountAuthInfo not exists!! auth_appid=" + m_auth_appid;
	return TABLE_BASE_RET_NOT_EXIST;
}

int WxAccountAuthInfo::DeleteFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"delete from "<<m_table_name<<" where auth_appid='"<<sql_escape(m_auth_appid)<<"'";

	if(!g_hx_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
		return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

int WxAccountAuthInfo::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set auth_appid='"<<sql_escape(m_auth_appid)<<"'"
			<<", auth_access_token='"<<sql_escape(m_auth_access_token)<<"'"
			<<", auth_refresh_token='"<<sql_escape(m_auth_refresh_token)<<"'"
			<<", expire_timestamp="<<m_expire_timestamp
			<<" on duplicate key"
			<<" update auth_access_token='"<<sql_escape(m_auth_access_token)<<"'"
			<<", auth_refresh_token='"<<sql_escape(m_auth_refresh_token)<<"'"
			<<", expire_timestamp="<<m_expire_timestamp;
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string WxAccountAuthInfo::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"auth_appid: "<<m_auth_appid
		<<", auth_access_token: "<<m_auth_access_token
		<<", auth_refresh_token: "<<m_auth_refresh_token
		<<", expire_timestamp: "<<m_expire_timestamp
        <<"}";

    return oss.str();
}

void WxAccountAuthInfo::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["auth_appid"] = m_auth_appid;
    stAnyValue["auth_access_token"] = m_auth_access_token;
    stAnyValue["expire_timestamp"] = m_expire_timestamp;

}
int OptionsInfo::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where id="<<m_id;
	if(!g_hx_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
		return -1;
	}

	if(g_hx_mysql.GetRowCount() && g_hx_mysql.Next())
	{;
		m_value = g_hx_mysql.GetRow(1);
		return TABLE_BASE_RET_OK;
	}

	strErrMsg = "OptionsInfo not exists!!, id=" + int_2_str(m_id);
	return TABLE_BASE_RET_NOT_EXIST;
}

int OptionsInfo::UpdateToDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id="<<m_id
			<<", value='"<<sql_escape(m_value)<<"'"
			<<" on duplicate key"
			<<" update value='"<<sql_escape(m_value)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string OptionsInfo::ToString()
{
	std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"id: "<<m_id
        <<", value: "<<m_value
        <<"}";

    return oss.str();
}



int LandingPageInfo::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where page_id="<<m_page_id;
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}

	if(g_hx_mysql.GetRowCount() && g_hx_mysql.Next())
	{
		m_uin = g_hx_mysql.GetRow(1);
		m_url = g_hx_mysql.GetRow(2);
		m_extra_data = g_hx_mysql.GetRow(3);
		return TABLE_BASE_RET_OK;
	}

	strErrMsg = "LandingPageInfo not exists!! page_id=" + m_page_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int LandingPageInfo::DeleteFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"delete from "<<m_table_name<<" where page_id="<<m_page_id;

	if(!g_hx_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
		return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

int LandingPageInfo::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set page_id="<<m_page_id
			<<", uin='"<<sql_escape(m_uin)<<"'"
			<<", url='"<<sql_escape(m_url)<<"'"
			<<", extra_data='"<<sql_escape(m_extra_data)<<"'"
			<<" on duplicate key"
			<<" update uin='"<<sql_escape(m_uin)<<"'"
			<<", url='"<<sql_escape(m_url)<<"'"
			<<", extra_data='"<<sql_escape(m_extra_data)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string LandingPageInfo::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"page_id: "<<m_page_id
		<<", uin: "<<m_uin
		<<", url: "<<m_url
		<<", extra_data: "<<m_extra_data
        <<"}";

    return oss.str();
}

void LandingPageInfo::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["page_id"] = m_page_id;
    stAnyValue["uin"] = m_uin;
    stAnyValue["url"] = m_url;
    stAnyValue["extra_data"] = m_extra_data;
}
int CardUserPassword::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select password from "<<m_table_name<<" where uin='"<<sql_escape(m_uin)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}

	if(g_hx_mysql.GetRowCount() && g_hx_mysql.Next())
	{
		m_password= g_hx_mysql.GetRow(0);
		return TABLE_BASE_RET_OK;
	}

	strErrMsg = "CardUserPassword not exists!!, uin=" + m_uin;
	return TABLE_BASE_RET_NOT_EXIST;
}
int CardUserPassword::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set uin='"<<sql_escape(m_uin)<<"'"
			<<", password='"<<sql_escape(m_password)<<"'"
			<<" on duplicate key"
			<<" update password='"<<sql_escape(m_password)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string CardUserPassword::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"uin: "<<m_uin
        <<", password: "<<m_password
        <<"}";

    return oss.str();
}

int CardUserInfo::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where uin='"<<sql_escape(m_uin)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}

	if(g_hx_mysql.GetRowCount() && g_hx_mysql.Next())
	{
		m_nick = g_hx_mysql.GetRow(1);
		m_organization_name = g_hx_mysql.GetRow(2);
		m_contacts_name = g_hx_mysql.GetRow(3);
		m_phone = g_hx_mysql.GetRow(4);
		m_bind_wx_account = g_hx_mysql.GetRow(5);
		m_bind_alipay_account = g_hx_mysql.GetRow(6);
		m_bind_weibo_account = g_hx_mysql.GetRow(7);
		m_bind_merchant_id = g_hx_mysql.GetRow(8);
		m_developer_id = g_hx_mysql.GetRow(9);
		m_operator_id = g_hx_mysql.GetRow(10);
		m_usertype = ::atoi(g_hx_mysql.GetRow(11));
		m_createtimestamp = ::atoi(g_hx_mysql.GetRow(12));
		m_lasttimestamp = ::atoi(g_hx_mysql.GetRow(13));
		m_question1 =g_hx_mysql.GetRow(14);
		m_answer1 =g_hx_mysql.GetRow(15);
		m_question2 =g_hx_mysql.GetRow(16);
		m_answer2 =g_hx_mysql.GetRow(17);
		m_question3 =g_hx_mysql.GetRow(18);
		m_answer3 =g_hx_mysql.GetRow(19);
		m_status =::atoi(g_hx_mysql.GetRow(20));
		m_wx_msg=g_hx_mysql.GetRow(21);
		m_extra_data = g_hx_mysql.GetRow(22);

		return TABLE_BASE_RET_OK;
	}

	strErrMsg = "CardUserInfo not exists!!, uin=" + m_uin;
	return TABLE_BASE_RET_NOT_EXIST;
}

int CardUserInfo::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set uin='"<<sql_escape(m_uin)<<"'"
			<<", nick='"<<sql_escape(m_nick)<<"'"
			<<", organization_name='"<<sql_escape(m_organization_name)<<"'"
			<<", contacts_name='"<<sql_escape(m_contacts_name)<<"'"
			<<", phone='"<<sql_escape(m_phone)<<"'"
			<<", bind_wx_account='"<<sql_escape(m_bind_wx_account)<<"'"
			<<", bind_alipay_account='"<<sql_escape(m_bind_alipay_account)<<"'"
			<<", bind_weibo_account='"<<sql_escape(m_bind_weibo_account)<<"'"
			<<", bind_merchant_id='"<<sql_escape(m_bind_merchant_id)<<"'"
			<<", developer_id='"<<sql_escape(m_developer_id)<<"'"
			<<", operator_id='"<<sql_escape(m_operator_id)<<"'"
			<<", usertype="<<m_usertype
			<<", create_timestamp="<<m_createtimestamp
			<<", last_timestamp="<<m_lasttimestamp
			<<", question1='"<<sql_escape(m_question1)<<"'"
			<<", answer1='"<<sql_escape(m_answer1)<<"'"
			<<", question2='"<<sql_escape(m_question1)<<"'"
			<<", answer2='"<<sql_escape(m_answer1)<<"'"
			<<", question3='"<<sql_escape(m_question1)<<"'"
			<<", answer3='"<<sql_escape(m_answer1)<<"'"
			<<", status="<<m_status
			<<", wx_msg='"<<sql_escape(m_wx_msg)<<"'"
			<<", extra_data='"<<sql_escape(m_extra_data)<<"'"
			<<" on duplicate key"
			<<" update nick='"<<sql_escape(m_nick)<<"'"
			<<", organization_name='"<<sql_escape(m_organization_name)<<"'"
			<<", contacts_name='"<<sql_escape(m_contacts_name)<<"'"
			<<", phone='"<<sql_escape(m_phone)<<"'"
			<<", bind_wx_account='"<<sql_escape(m_bind_wx_account)<<"'"
			<<", bind_alipay_account='"<<sql_escape(m_bind_alipay_account)<<"'"
			<<", bind_weibo_account='"<<sql_escape(m_bind_weibo_account)<<"'"
			<<", bind_merchant_id='"<<sql_escape(m_bind_merchant_id)<<"'"
			<<", developer_id='"<<sql_escape(m_developer_id)<<"'"
			<<", operator_id='"<<sql_escape(m_operator_id)<<"'"
			<<", usertype="<<m_usertype
			<<", last_timestamp="<<m_lasttimestamp
			<<", question1='"<<sql_escape(m_question1)<<"'"
			<<", answer1='"<<sql_escape(m_answer1)<<"'"
			<<", question2='"<<sql_escape(m_question2)<<"'"
			<<", answer2='"<<sql_escape(m_answer2)<<"'"
			<<", question3='"<<sql_escape(m_question3)<<"'"
			<<", answer3='"<<sql_escape(m_answer3)<<"'"
			<<", status="<<m_status
			<<", wx_msg='"<<sql_escape(m_wx_msg)<<"'"
			<<", extra_data='"<<sql_escape(m_extra_data)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string CardUserInfo::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"uin: "<<m_uin
        <<", nick: "<<m_nick
        <<", organization_name: "<<m_organization_name
		<<", contacts_name: "<<m_contacts_name
		<<", phone: "<<m_phone
		<<", bind_wx_account: "<<m_bind_wx_account
		<<", bind_alipay_account: "<<m_bind_alipay_account
		<<", bind_weibo_account: "<<m_bind_weibo_account
		<<", bind_merchant_id: "<<m_bind_merchant_id
		<<", developer_id: "<<m_developer_id
		<<", operator_id: "<<m_operator_id
		<<", usertype: "<<m_usertype
		<<", last_timestamp: "<<m_lasttimestamp
		<<", question1: "<<m_question1
		<<", question2: "<<m_question2
		<<", question3: "<<m_question3
		<<", status: "<<m_status
		<<", wx_msg: "<<m_wx_msg
		<<", extra_data: "<<m_extra_data
        <<"}";

    return oss.str();
}

void CardUserInfo::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["uin"] = m_uin;
    stAnyValue["nick"] = m_nick;
    stAnyValue["organization_name"] = m_organization_name;
    stAnyValue["contacts_name"] = m_contacts_name;
    stAnyValue["phone"] = m_phone;
    stAnyValue["bind_wx_account"] = m_bind_wx_account;
    stAnyValue["bind_alipay_account"] = m_bind_alipay_account;
    stAnyValue["bind_weibo_account"] = m_bind_weibo_account;
    stAnyValue["bind_merchant_id"] = m_bind_merchant_id;
    stAnyValue["developer_id"] = m_developer_id;
    stAnyValue["operator_id"] = m_operator_id;
    stAnyValue["usertype"] = m_usertype;
    stAnyValue["last_timestamp"] = m_lasttimestamp;
    stAnyValue["question1"] = m_question1;
    stAnyValue["question2"] = m_question2;
    stAnyValue["question3"] = m_question3;
    stAnyValue["status"] = m_status;
    stAnyValue["wx_msg"] = m_wx_msg;
    stAnyValue["extra_data"] = m_extra_data;



}

int CardInfo::SelectFromDB(std::string& strErrMsg)
{
		std::ostringstream ossSql;
		ossSql.str("");
		ossSql<<"select * from "<<m_table_name<<" where id='"<<sql_escape(m_id)<<"'";
		if(!g_hx_mysql.Query(ossSql.str()))
		{
				strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
				return -1;
		}

		if(g_hx_mysql.GetRowCount() && g_hx_mysql.Next())
		{
			m_status = ::atoi(g_hx_mysql.GetRow(1));
			m_type = ::atoi(g_hx_mysql.GetRow(2));

		    m_logourl = g_hx_mysql.GetRow(3);
		    m_title = g_hx_mysql.GetRow(4);
		    m_merchant_id = g_hx_mysql.GetRow(5);
		    m_extra_data= g_hx_mysql.GetRow(6);
		    m_activate_form= g_hx_mysql.GetRow(7);
		    m_owned = ::atoi(g_hx_mysql.GetRow(8));
		    m_wx_msg = g_hx_mysql.GetRow(9);
		    m_date_info = g_hx_mysql.GetRow(10);
		    m_quantity = ::atoi(g_hx_mysql.GetRow(11));
		    m_total_quantity =::atoi(g_hx_mysql.GetRow(12));
		    m_create_timestamp = ::atoi(g_hx_mysql.GetRow(13));
		    m_expire_timestamp = ::atoi(g_hx_mysql.GetRow(14));

			return TABLE_BASE_RET_OK;
		}

		strErrMsg = "CardInfo not exists!!, CardInfo_id=" + m_id;
		return TABLE_BASE_RET_NOT_EXIST;
}
int CardInfo::UpdateToDB(std::string& strErrMsg)
{
	  std::ostringstream ossSql;
		ossSql.str("");
		ossSql<<"insert into "<<m_table_name
				<<" set id='"<<sql_escape(m_id)<<"'"
				<<", status="<<m_status
				<<", type="<<m_type
				<<", logourl='"<<sql_escape(m_logourl)<<"'"
				<<", title='"<<sql_escape(m_title)<<"'"
				<<", merchant_id='"<<sql_escape(m_merchant_id)<<"'"
				<<", extra_data='"<<sql_escape(m_extra_data)<<"'"
				<<", activate_form='"<<sql_escape(m_activate_form)<<"'"
				<<", owned="<<m_owned
				<<", wx_msg='"<<sql_escape(m_wx_msg)<<"'"
				<<", date_info='"<<sql_escape(m_date_info)<<"'"
				<<", quantity="<<m_quantity
				<<", total_quantity="<<m_total_quantity
				<<", create_timestamp="<<m_create_timestamp
				<<", expire_timestamp="<<m_expire_timestamp
				<<" on duplicate key"
				<<" update status="<<m_status
				<<", extra_data = '"<<sql_escape(m_extra_data)<<"'"
				<<", activate_form='"<<sql_escape(m_activate_form)<<"'"
				<<", status="<<m_status
				<<", type="<<m_type
				<<", logourl='"<<sql_escape(m_logourl)<<"'"
				<<", title='"<<sql_escape(m_title)<<"'"
				<<", wx_msg='"<<sql_escape(m_wx_msg)<<"'"
				<<", date_info='"<<sql_escape(m_date_info)<<"'"
				<<", quantity="<<m_quantity
				<<", total_quantity="<<m_total_quantity
				<<", create_timestamp="<<m_create_timestamp
				<<", expire_timestamp="<<m_expire_timestamp;


		if(!g_hx_mysql.Query(ossSql.str()))
		{
				strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
				return TABLE_BASE_RET_UPDATE_FAILED;
		}

		return TABLE_BASE_RET_OK;
}
std::string CardInfo::ToString()
{
	std::ostringstream oss;
	    oss.str("");
	    oss<<"{"
	        <<"id: "<<m_id
	        <<",status: "<<m_status
	        <<",type: "<<m_type
			<<",logourl: "<<m_logourl
			<<",title: "<<m_title
			<<",merchant_id: "<<m_merchant_id
			<<",extra_data: "<<m_extra_data
			<<",activate form: "<<m_activate_form
			<<",owned: "<<m_owned
			<<",wx_msg: "<<m_wx_msg
			<<",date_info: "<<m_date_info
			<<",quantity: "<<m_quantity
			<<",total_quantity: "<<m_total_quantity
			<<",create_timestamp: "<<m_create_timestamp
			<<",expire_timestamp: "<<m_expire_timestamp
	        <<"}";

	    return oss.str();
}
void CardInfo::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["id"] = m_id;
    stAnyValue["status"]=m_status;
    stAnyValue["type"]=m_type;
    stAnyValue["logourl"] = m_logourl;
    stAnyValue["title"] = m_title;
    stAnyValue["merchant_id"] = m_merchant_id;
    stAnyValue["extra_data"] = m_extra_data;
    stAnyValue["activate_form"] = m_activate_form;
    stAnyValue["owned"] = m_owned;
    stAnyValue["wx_msg"] = m_wx_msg;
    stAnyValue["date_info"]=m_date_info;
    stAnyValue["quantity"]=m_quantity;
    stAnyValue["total_quantity"] = m_total_quantity;
    stAnyValue["create_timestamp"]=m_create_timestamp;
    stAnyValue["expire_timestamp"]=m_expire_timestamp;


}


int CardInfoIndexByMerchantid::SelectFromDB(std::string& strErrMsg)
{
	m_cardinfo_list.clear();

    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where merchant_id='"<<sql_escape(m_merchant_id)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}

    if(0 == g_hx_mysql.GetRowCount())
    {
        strErrMsg = "CardInfo not exists!!, merchant_id=" + m_merchant_id;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	while(g_hx_mysql.Next())
	{
		CardInfo stCardInfo(m_table_name);
		stCardInfo.m_id = g_hx_mysql.GetRow(0);
		stCardInfo.m_status = ::atoi(g_hx_mysql.GetRow(1));
		stCardInfo.m_type = ::atoi(g_hx_mysql.GetRow(2));

		stCardInfo.m_logourl = g_hx_mysql.GetRow(3);
		stCardInfo.m_title = g_hx_mysql.GetRow(4);
		stCardInfo.m_merchant_id = g_hx_mysql.GetRow(5);
		stCardInfo.m_extra_data= g_hx_mysql.GetRow(6);
		stCardInfo.m_activate_form= g_hx_mysql.GetRow(7);
		stCardInfo.m_owned = ::atoi(g_hx_mysql.GetRow(8));
		stCardInfo.m_wx_msg = g_hx_mysql.GetRow(9);
		stCardInfo.m_date_info = g_hx_mysql.GetRow(10);
		stCardInfo.m_quantity = ::atoi(g_hx_mysql.GetRow(11));
		stCardInfo.m_total_quantity =::atoi(g_hx_mysql.GetRow(12));
		stCardInfo.m_create_timestamp = ::atoi(g_hx_mysql.GetRow(13));
		stCardInfo.m_expire_timestamp = ::atoi(g_hx_mysql.GetRow(14));
		m_cardinfo_list.push_back(stCardInfo);
	}

	return TABLE_BASE_RET_OK;
}

std::string CardInfoIndexByMerchantid::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_cardinfo_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_cardinfo_list[i].ToString()<<", ";
    }

    oss<<"]";

    return oss.str();
}

int EntityShopInfo::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where sid='"<<sql_escape(m_sid)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}

	if(g_hx_mysql.GetRowCount() && g_hx_mysql.Next())
	{
		m_uin = g_hx_mysql.GetRow(1);
		m_PoiId = g_hx_mysql.GetRow(2);
		m_CreateTime  = ::atol(g_hx_mysql.GetRow(3));
		m_update_status  = ::atol(g_hx_mysql.GetRow(4));
		m_err_msg = g_hx_mysql.GetRow(5);
		m_extra_data = g_hx_mysql.GetRow(6);
		m_business_name = g_hx_mysql.GetRow(7);
		m_branch_name = g_hx_mysql.GetRow(8);
		m_province =g_hx_mysql.GetRow(9);
		m_city = g_hx_mysql.GetRow(10);
		m_district = g_hx_mysql.GetRow(11);
		m_address =g_hx_mysql.GetRow(12);
		m_available_state =::atoi(g_hx_mysql.GetRow(13));

		return TABLE_BASE_RET_OK;
	}

	strErrMsg = "EntityShopInfo not exists!! auth_appid=" + m_sid;
	return TABLE_BASE_RET_NOT_EXIST;
}

int EntityShopInfo::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set sid='"<<sql_escape(m_sid)<<"'"
			<<", uin='"<<sql_escape(m_uin)<<"'"
			<<", PoiId='"<<sql_escape(m_PoiId)<<"'"
			<<", CreateTime="<<m_CreateTime
			<<", update_status="<<m_update_status
			<<", err_msg='"<<sql_escape(m_err_msg)<<"'"
			<<", extra_data='"<<sql_escape(m_extra_data)<<"'"
			<<", business_name='"<<sql_escape(m_business_name)<<"'"
			<<", branch_name='"<<sql_escape(m_branch_name)<<"'"
			<<", province='"<<sql_escape(m_province)<<"'"
			<<", city='"<<sql_escape(m_city)<<"'"
			<<", district='"<<sql_escape(m_district)<<"'"
			<<", address='"<<sql_escape(m_address)<<"'"
			<<", available_state="<<m_available_state
			<<" on duplicate key"
			<<" update uin='"<<sql_escape(m_uin)<<"'"
			<<", PoiId='"<<sql_escape(m_PoiId)<<"'"
			<<", CreateTime="<<m_CreateTime
			<<", update_status="<<m_update_status
			<<", err_msg='"<<sql_escape(m_err_msg)<<"'"
			<<", extra_data='"<<sql_escape(m_extra_data)<<"'"
			<<", business_name='"<<sql_escape(m_business_name)<<"'"
			<<", branch_name='"<<sql_escape(m_branch_name)<<"'"
			<<", province='"<<sql_escape(m_province)<<"'"
			<<", city='"<<sql_escape(m_city)<<"'"
			<<", district='"<<sql_escape(m_district)<<"'"
			<<", address='"<<sql_escape(m_address)<<"'"
			<<", available_state="<<m_available_state;
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string EntityShopInfo::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"sid: "<<m_sid
		<<", uin: "<<m_uin
		<<", PoiId: "<<m_PoiId
		<<", CreateTime: "<<m_CreateTime
		<<", update_status: "<<m_update_status
		<<", err_msg: "<<m_err_msg
		<<", extra_data: "<<m_extra_data
		<<", business_name: "<<m_business_name
		<<", branch_name: "<<m_branch_name
		<<", province: "<<m_province
		<<", city: "<< m_city
		<<", district: "<<m_district
		<<", address: "<<m_address
		<<", available_state: "<<m_available_state
        <<"}";

    return oss.str();
}

void EntityShopInfo::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["sid"] = m_sid;
    stAnyValue["uin"] = m_uin;
    stAnyValue["PoiId"] = m_PoiId;
    stAnyValue["CreateTime"] = m_CreateTime;
    stAnyValue["update_status"] = m_update_status;
    stAnyValue["err_msg"] = m_err_msg;
    stAnyValue["extra_data"] = m_extra_data;
    stAnyValue["business_name"]= m_business_name;
    stAnyValue["branch_name"]= m_branch_name;
    stAnyValue["province"]=m_province;
    stAnyValue["city"]= m_city;
    stAnyValue["district"]= m_district;
    stAnyValue["address"]=m_address;
    stAnyValue["available_state"]= m_available_state;

}



int WXEntityShopInfo::SelectFromDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where sid='"<<sql_escape(m_sid)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}

	if(g_hx_mysql.GetRowCount() && g_hx_mysql.Next())
	{
		m_uin = g_hx_mysql.GetRow(1);
		m_PoiId = g_hx_mysql.GetRow(2);
		m_CreateTime  = ::atol(g_hx_mysql.GetRow(3));
		m_update_status  = ::atol(g_hx_mysql.GetRow(4));
		m_err_msg = g_hx_mysql.GetRow(5);
		m_extra_data = g_hx_mysql.GetRow(6);
		m_business_name = g_hx_mysql.GetRow(7);
		m_branch_name = g_hx_mysql.GetRow(8);
		m_province =g_hx_mysql.GetRow(9);
		m_city = g_hx_mysql.GetRow(10);
		m_district = g_hx_mysql.GetRow(11);
		m_address =g_hx_mysql.GetRow(12);
		m_available_state =::atoi(g_hx_mysql.GetRow(13));
		m_expire_timestamp_cached = ::atol(g_hx_mysql.GetRow(14));

		return TABLE_BASE_RET_OK;
	}

	strErrMsg = "WXEntityShopInfo not exists!! auth_appid=" + m_sid;
	return TABLE_BASE_RET_NOT_EXIST;
}

int WXEntityShopInfo::UpdateToDB(std::string& strErrMsg)
{
    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set sid='"<<sql_escape(m_sid)<<"'"
			<<", uin='"<<sql_escape(m_uin)<<"'"
			<<", PoiId='"<<sql_escape(m_PoiId)<<"'"
			<<", CreateTime="<<m_CreateTime
			<<", update_status="<<m_update_status
			<<", err_msg='"<<sql_escape(m_err_msg)<<"'"
			<<", extra_data='"<<sql_escape(m_extra_data)<<"'"
			<<", business_name='"<<sql_escape(m_business_name)<<"'"
			<<", branch_name='"<<sql_escape(m_branch_name)<<"'"
			<<", province='"<<sql_escape(m_province)<<"'"
			<<", city='"<<sql_escape(m_city)<<"'"
			<<", district='"<<sql_escape(m_district)<<"'"
			<<", address='"<<sql_escape(m_address)<<"'"
			<<", available_state="<<m_available_state
			<<", expire_timestamp_cached="<<m_expire_timestamp_cached
			<<" on duplicate key"
			<<" update uin='"<<sql_escape(m_uin)<<"'"
			<<", PoiId='"<<sql_escape(m_PoiId)<<"'"
			<<", CreateTime="<<m_CreateTime
			<<", update_status="<<m_update_status
			<<", err_msg='"<<sql_escape(m_err_msg)<<"'"
			<<", extra_data='"<<sql_escape(m_extra_data)<<"'"
			<<", business_name='"<<sql_escape(m_business_name)<<"'"
			<<", branch_name='"<<sql_escape(m_branch_name)<<"'"
			<<", province='"<<sql_escape(m_province)<<"'"
			<<", city='"<<sql_escape(m_city)<<"'"
			<<", district='"<<sql_escape(m_district)<<"'"
			<<", address='"<<sql_escape(m_address)<<"'"
			<<", available_state="<<m_available_state
			<<", expire_timestamp_cached="<<m_expire_timestamp_cached;
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}

	return TABLE_BASE_RET_OK;
}

std::string WXEntityShopInfo::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{"
        <<"sid: "<<m_sid
		<<", uin: "<<m_uin
		<<", PoiId: "<<m_PoiId
		<<", CreateTime: "<<m_CreateTime
		<<", update_status: "<<m_update_status
		<<", err_msg: "<<m_err_msg
		<<", extra_data: "<<m_extra_data
		<<", business_name: "<<m_business_name
		<<", branch_name: "<<m_branch_name
		<<", province: "<<m_province
		<<", city: "<< m_city
		<<", district: "<<m_district
		<<", address: "<<m_address
		<<", available_state: "<<m_available_state
		<<", expire_timestamp_cached: "<<m_expire_timestamp_cached
        <<"}";

    return oss.str();
}

void WXEntityShopInfo::ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const
{
    stAnyValue.clear();
    stAnyValue["sid"] = m_sid;
    stAnyValue["uin"] = m_uin;
    stAnyValue["PoiId"] = m_PoiId;
    stAnyValue["CreateTime"] = m_CreateTime;
    stAnyValue["update_status"] = m_update_status;
    stAnyValue["err_msg"] = m_err_msg;
    stAnyValue["extra_data"] = m_extra_data;
    stAnyValue["business_name"]= m_business_name;
    stAnyValue["branch_name"]= m_branch_name;
    stAnyValue["province"]=m_province;
    stAnyValue["city"]= m_city;
    stAnyValue["district"]= m_district;
    stAnyValue["address"]=m_address;
    stAnyValue["available_state"]= m_available_state;
    stAnyValue["expire_timestamp_cached"] = m_expire_timestamp_cached;

}

int EntityShopsInfoIndexByUin::SelectFromDB(std::string& strErrMsg)
{
	m_entityshops_info_list.clear();

    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where uin='"<<sql_escape(m_uin)<<"' and available_state != " << ENTITYSHOP_DELETE;
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}

    if(0 == g_hx_mysql.GetRowCount())
    {
        strErrMsg = "EntityShopInfo not exists!!, uin=" + m_uin;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	while(g_hx_mysql.Next())
	{
		EntityShopInfo stEntityShopInfo(m_table_name);
		stEntityShopInfo.m_sid = g_hx_mysql.GetRow(0);
		stEntityShopInfo.m_uin = g_hx_mysql.GetRow(1);
		stEntityShopInfo.m_PoiId = g_hx_mysql.GetRow(2);
		stEntityShopInfo.m_CreateTime  = ::atol(g_hx_mysql.GetRow(3));
		stEntityShopInfo.m_update_status  = ::atol(g_hx_mysql.GetRow(4));
		stEntityShopInfo.m_err_msg = g_hx_mysql.GetRow(5);
		stEntityShopInfo.m_extra_data = g_hx_mysql.GetRow(6);
		stEntityShopInfo.m_business_name = g_hx_mysql.GetRow(7);
		stEntityShopInfo.m_branch_name = g_hx_mysql.GetRow(8);
		stEntityShopInfo.m_province = g_hx_mysql.GetRow(9);
		stEntityShopInfo.m_city = g_hx_mysql.GetRow(10);
		stEntityShopInfo.m_district = g_hx_mysql.GetRow(11);
		stEntityShopInfo.m_address = g_hx_mysql.GetRow(12);
		stEntityShopInfo.m_available_state = ::atoi(g_hx_mysql.GetRow(13));
		m_entityshops_info_list.push_back(stEntityShopInfo);
	}
	std::sort(m_entityshops_info_list.begin(), m_entityshops_info_list.end(), create_timestampDes);
	return TABLE_BASE_RET_OK;
}

std::string EntityShopsInfoIndexByUin::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_entityshops_info_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_entityshops_info_list[i].ToString()<<", ";
    }

    oss<<"]";

    return oss.str();
}
int EntityShopsInfoIndexByPoiId::SelectFromDB(std::string& strErrMsg)
{

    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where PoiId='"<<sql_escape(m_poiId)<<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}
	if(g_hx_mysql.GetRowCount()==0)
	{
		strErrMsg = "EntityShopInfo not exists!! PoiId=" + m_poiId;
		return TABLE_BASE_RET_NOT_EXIST;
	}
	if(g_hx_mysql.GetRowCount()!=1)
	{
		strErrMsg = "EntityShopInfo exists!! But more than 1!, PoiId=" + m_poiId;
		return -1;
	}
	if(g_hx_mysql.GetRowCount()&& g_hx_mysql.Next())
	{
		m_entityshop_info.m_sid = g_hx_mysql.GetRow(0);
		m_entityshop_info.m_uin = g_hx_mysql.GetRow(1);
		m_entityshop_info.m_PoiId = g_hx_mysql.GetRow(2);
		m_entityshop_info.m_CreateTime  = ::atol(g_hx_mysql.GetRow(3));
		m_entityshop_info.m_update_status  = ::atol(g_hx_mysql.GetRow(4));
		m_entityshop_info.m_err_msg = g_hx_mysql.GetRow(5);
		m_entityshop_info.m_extra_data = g_hx_mysql.GetRow(6);
		m_entityshop_info.m_business_name = g_hx_mysql.GetRow(7);
		m_entityshop_info.m_branch_name = g_hx_mysql.GetRow(8);
		m_entityshop_info.m_province = g_hx_mysql.GetRow(9);
		m_entityshop_info.m_city = g_hx_mysql.GetRow(10);
		m_entityshop_info.m_district = g_hx_mysql.GetRow(11);
		m_entityshop_info.m_address = g_hx_mysql.GetRow(12);
		m_entityshop_info.m_available_state = ::atoi(g_hx_mysql.GetRow(13));
		return TABLE_BASE_RET_OK;
	}
	strErrMsg = "EntityShopInfo exist!! PoiId=" + m_poiId;
	strErrMsg +=",RowCount=";
	strErrMsg +=g_hx_mysql.GetRowCount();
	return -1;
}

std::string EntityShopsInfoIndexByPoiId::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"{";
    oss<<"list_%zu: "<<m_entityshop_info.ToString();
    oss<<"}";

    return oss.str();
}
int SubmerchantInfo::SelectFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "select * from " << m_table_name << " where id = '" << sql_escape(m_id) <<"'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_hx_mysql.GetErrMsg();
			return -1;
	}
	if(g_hx_mysql.GetRowCount() && g_hx_mysql.Next())
	{
		m_brand_name = g_hx_mysql.GetRow(1);
		m_logo_url = g_hx_mysql.GetRow(2);
		m_uin = g_hx_mysql.GetRow(3);
		m_status = atoi(g_hx_mysql.GetRow(4));
		m_wx_msg = g_hx_mysql.GetRow(5);
		m_extra_data = g_hx_mysql.GetRow(6);
		m_protocol = g_hx_mysql.GetRow(7);
		m_agreement = g_hx_mysql.GetRow(8);
		m_operator = g_hx_mysql.GetRow(9);
		m_create_timestamp = atoi(g_hx_mysql.GetRow(10));
		m_expire_timestamp = atoi(g_hx_mysql.GetRow(11));
		return TABLE_BASE_RET_OK;
	}

	strErrMsg = "SubmerchantInfo not exists!! id = " + m_id;
	return TABLE_BASE_RET_NOT_EXIST;
}

int SubmerchantInfo::DeleteFromDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql << "delete from " << m_table_name << " where id = '" << m_id << "'";
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query erros, sql = " + ossSql.str() + ", msg = " + g_hx_mysql.GetErrMsg();
			return TABLE_BASE_RET_UPDATE_FAILED;
	}
					
	return TABLE_BASE_RET_OK;
}

int SubmerchantInfo::UpdateToDB(std::string& strErrMsg)
{
	std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"insert into "<<m_table_name
			<<" set id='"<<sql_escape(m_id)<<"'"
			<<", brand_name='"<<sql_escape(m_brand_name)<<"'"
			<<", logo_url='"<<sql_escape(m_logo_url)<<"'"
			<<", uin='"<<sql_escape(m_uin)<<"'"
			<<", status="<<m_status
			<<", wx_msg='"<<sql_escape(m_wx_msg)<<"'"
			<<", extra_data='"<<sql_escape(m_extra_data)<<"'"
			<<", protocol='"<<sql_escape(m_protocol)<<"'"
			<<", agreement='"<<sql_escape(m_agreement)<<"'"
			<<", operator='"<<sql_escape(m_operator)<<"'"
			<<", create_timestamp="<<m_create_timestamp
			<<", expire_timestamp="<<m_expire_timestamp
			<<" on duplicate key"
			<<" update id ='"<<sql_escape(m_id)<<"'"
			<<", brand_name='"<<sql_escape(m_brand_name)<<"'"
			<<", logo_url='"<<sql_escape(m_logo_url)<<"'"
			<<", uin='"<<sql_escape(m_uin)<<"'"
			<<", status="<<m_status
			<<", wx_msg='"<<sql_escape(m_wx_msg)<<"'"
			<<", extra_data='"<<sql_escape(m_extra_data)<<"'"
			<<", protocol='"<<sql_escape(m_protocol)<<"'"
			<<", agreement='"<<sql_escape(m_agreement)<<"'"
			<<", operator='"<<sql_escape(m_operator)<<"'"
			<<", create_timestamp="<<m_create_timestamp
			<<", expire_timestamp="<<m_expire_timestamp;
			
	if(!g_hx_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql = " + ossSql.str() + ", msg = " + g_hx_mysql.GetErrMsg();
		return TABLE_BASE_RET_UPDATE_FAILED;
	}
	return TABLE_BASE_RET_OK;
}
		
std::string SubmerchantInfo::ToString()
{
	std::ostringstream oss;
	oss.str("");
	oss << "{"
		<<"merchant_id: "<<m_id
		<<", brand_name: "<<m_brand_name
		<<", logo: "<<m_logo_url
		<<", uin: "<<m_uin
		<<", status: "<<m_status
		<<", wx_msg: "<<m_wx_msg
		<<", extra_data: "<<m_extra_data
		<<", protocol: "<<m_protocol
		<<", agreement: "<<m_agreement
		<<", operator: "<<m_operator
		<<", create_timestamp: "<<m_create_timestamp
		<<", expire_timestamp: "<<m_expire_timestamp
		<< "}";
	return oss.str();
}

void SubmerchantInfo::ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const
{
	stAnyValue.clear();
	stAnyValue["merchant_id"] = m_id;
	stAnyValue["brand_name"] = m_brand_name;
	stAnyValue["logo_url"] = m_logo_url;
	stAnyValue["uin"] = m_uin;
	stAnyValue["status"] = m_status;
	stAnyValue["wx_msg"] = m_wx_msg;
	stAnyValue["extra_data"] = m_extra_data;
	stAnyValue["protocol"] = m_protocol;
	stAnyValue["agreement"] = m_agreement;
	stAnyValue["operator"] = m_operator;
	stAnyValue["create_timestamp"]= m_create_timestamp;
	stAnyValue["expire_timestamp"]= m_expire_timestamp;
}

int SubmerchantInfoIndexByUin::SelectFromDB(std::string& strErrMsg)
{
	m_submerchant_info_list.clear();

    std::ostringstream ossSql;
	ossSql.str("");
	ossSql<<"select * from "<<m_table_name<<" where uin='"<<sql_escape(m_uin)<<"' and status != " << SUBMERCHANT_DELETE;
	if(!g_hx_mysql.Query(ossSql.str()))
	{
			strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
			return -1;
	}

    if(0 == g_hx_mysql.GetRowCount())
    {
        strErrMsg = "SubmerchantInfo not exists!!, uin=" + m_uin;
	    return TABLE_BASE_RET_NOT_EXIST;
    }

	while(g_hx_mysql.Next())
	{
		SubmerchantInfo stSubmerchantInfo(m_table_name);
		stSubmerchantInfo.m_id = g_hx_mysql.GetRow(0);
		stSubmerchantInfo.m_brand_name = g_hx_mysql.GetRow(1);
		stSubmerchantInfo.m_logo_url = g_hx_mysql.GetRow(2);
		stSubmerchantInfo.m_uin = g_hx_mysql.GetRow(3);
		stSubmerchantInfo.m_status = ::atoi(g_hx_mysql.GetRow(4));
		stSubmerchantInfo.m_wx_msg = g_hx_mysql.GetRow(5);
		stSubmerchantInfo.m_extra_data = g_hx_mysql.GetRow(6);
		stSubmerchantInfo.m_protocol = g_hx_mysql.GetRow(7);
		stSubmerchantInfo.m_agreement = g_hx_mysql.GetRow(8);
		stSubmerchantInfo.m_operator = g_hx_mysql.GetRow(9);
		stSubmerchantInfo.m_create_timestamp = atoi(g_hx_mysql.GetRow(10));
		stSubmerchantInfo.m_expire_timestamp = atoi(g_hx_mysql.GetRow(11));
		m_submerchant_info_list.push_back(stSubmerchantInfo);
	}
	std::sort(m_submerchant_info_list.begin(), m_submerchant_info_list.end(), create_timestampDes);
	return TABLE_BASE_RET_OK;
}

std::string SubmerchantInfoIndexByUin::ToString()
{
    std::ostringstream oss;
    oss.str("");
    oss<<"[";
    for(size_t i=0; i!=m_submerchant_info_list.size(); ++i)
    {
        oss<<"list_%zu: "<<m_submerchant_info_list[i].ToString()<<", ";
    }

    oss<<"]";

    return oss.str();
}
