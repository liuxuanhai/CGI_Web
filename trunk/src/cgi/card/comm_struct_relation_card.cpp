#include "comm_struct_relation_card.h"
#include "comm_def_card.h"
lce::cgi::CMysql TableRelationBaseCard::g_hx_mysql;

int CardInfoIndexByUin::SelectFromDB(std::string& strErrMsg)
{

	m_card_info_list.clear();
	std::ostringstream ossSql;

	ossSql.str("");
	ossSql << "select * from " << m_table_name << " where "
			<< " status !="<<CARD_DELETE<<" and "
		  <<" merchant_id in (select id from " << m_table_relation_name << " where uin='" << sql_escape(m_uin)<<"'";
	if (!m_merchant_id.empty())
	{
		ossSql << " and where id='" << sql_escape(m_merchant_id)<<"'";
	}

	ossSql << " )";
	if (!g_hx_mysql.Query(ossSql.str()))
	{
		strErrMsg = "mysql query error, sql=" + ossSql.str() + ", msg=" + g_hx_mysql.GetErrMsg();
		return -1;
	}

	if (0 == g_hx_mysql.GetRowCount())
	{
		strErrMsg = "CardInfo not exists!!, uin= " + m_uin;
		strErrMsg += " , merchant_id= " + m_merchant_id;
		return TABLE_BASE_RET_NOT_EXIST;
	}

	while (g_hx_mysql.Next())
	{
		CardInfo stCardInfo(m_table_name);
		stCardInfo.m_id = g_hx_mysql.GetRow(0);
		stCardInfo.m_status = ::atoi(g_hx_mysql.GetRow(1));
		stCardInfo.m_type = ::atoi(g_hx_mysql.GetRow(2));

		stCardInfo.m_logourl = g_hx_mysql.GetRow(3);
		stCardInfo.m_title = g_hx_mysql.GetRow(4);
		stCardInfo.m_merchant_id = g_hx_mysql.GetRow(5);
		stCardInfo.m_extra_data = g_hx_mysql.GetRow(6);
		stCardInfo.m_activate_form = g_hx_mysql.GetRow(7);
		stCardInfo.m_owned = ::atoi(g_hx_mysql.GetRow(8));
		stCardInfo.m_wx_msg = g_hx_mysql.GetRow(9);
		stCardInfo.m_date_info = g_hx_mysql.GetRow(10);
		stCardInfo.m_quantity = ::atoi(g_hx_mysql.GetRow(11));
		stCardInfo.m_total_quantity = ::atoi(g_hx_mysql.GetRow(12));
		stCardInfo.m_create_timestamp = ::atoi(g_hx_mysql.GetRow(13));
		stCardInfo.m_expire_timestamp = ::atoi(g_hx_mysql.GetRow(14));
		m_card_info_list.push_back(stCardInfo);
	}
	std::sort(m_card_info_list.begin(), m_card_info_list.end(), create_timestampDes);
	return TABLE_BASE_RET_OK;
}
std::string CardInfoIndexByUin::ToString()
{
	std::ostringstream oss;
	oss.str("");
	oss << "[";
	for (size_t i = 0; i != m_card_info_list.size(); ++i)
	{
		oss << "list_%zu: " << m_card_info_list[i].ToString() << ", ";
	}

	oss << "]";

	return oss.str();
}
