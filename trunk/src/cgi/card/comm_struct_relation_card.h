#ifndef _HX_COMM_STRUCT_RELATION_CARD_H_
#define _HX_COMM_STRUCT_RELATION_CARD_H_
#include "cgi/cgi.h"
#include "comm_struct_card.h"
class TableRelationBaseCard
{
	public:
		enum TABLE_BASE_RET
		{
			TABLE_BASE_RET_OK = 0,
			TABLE_BASE_RET_NOT_EXIST = 10001,
			TABLE_BASE_RET_UPDATE_FAILED = 10002,
		};

	public:
		string m_table_name;
		string m_table_relation_name;
		static lce::cgi::CMysql g_hx_mysql;

	public:
		TableRelationBaseCard(const std::string& strTableName,const std::string& strTableRelationName)
		: m_table_name(strTableName),m_table_relation_name(strTableRelationName)
		{

		}
};
class CardInfoIndexByUin : public TableRelationBaseCard
{
	public:


		std::string m_uin;
		std::string m_merchant_id;
		std::vector<CardInfo> m_card_info_list;
	public:
		CardInfoIndexByUin(const std::string& strTableName,const std::string& strTableRelationName)
		:TableRelationBaseCard(strTableName,strTableRelationName)
		{

		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		static bool create_timestampDes(const CardInfo &a,const CardInfo &b){  return a.m_create_timestamp<b.m_create_timestamp; }

		std::string ToString();

};

#endif /* COM_STRUCT_2_H_ */
