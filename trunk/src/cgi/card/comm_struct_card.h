#ifndef _HX_COMM_STRUCT_CARD_H_
#define _HX_COMM_STRUCT_CARD_H_

#include "cgi/cgi.h"

class TableBaseCard
{
	public:
		enum TABLE_BASE_RET
		{
			TABLE_BASE_RET_OK = 0,
			TABLE_BASE_RET_NOT_EXIST = 10001,
			TABLE_BASE_RET_UPDATE_FAILED = 10002,
			TABLE_BASE_RET_ALREADY_EXIST = 10003,
		};

	public:
		string m_table_name;
		static lce::cgi::CMysql g_hx_mysql;

	public:
		TableBaseCard(const std::string& strTableName): m_table_name(strTableName)
		{

		}
};

class CardPicture : public TableBaseCard
{
	public:
		std::string m_id;
		std::string m_data;
		uint32_t m_status;
		std::string m_uin;
		uint64_t m_createtimestamp;
		uint32_t m_pic_usage;

	public:
		CardPicture(const std::string& strTableName): TableBaseCard(strTableName)
		{
			m_status = 1;
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
};

class WxAccountAuthInfo : public TableBaseCard
{
	public:
		std::string m_auth_appid;
		std::string m_auth_access_token;
		std::string m_auth_refresh_token;
		uint64_t m_expire_timestamp;
	public:
		WxAccountAuthInfo(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int DeleteFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const;
};
class OptionsInfo : public TableBaseCard
{
	public:
		uint32_t m_id;
		std::string m_value;

	public:
		OptionsInfo(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
};
class LandingPageInfo : public TableBaseCard
{
	public:

		int m_page_id;
		std::string m_uin;
		std::string m_url;
		std::string m_extra_data;

	public:
		LandingPageInfo(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int DeleteFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};
class CardUserPassword : public TableBaseCard
{
	public:
		std::string m_uin;
		std::string m_password;

	public:
		CardUserPassword(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
};

class CardUserInfo : public TableBaseCard
{
	public:
		std::string m_uin;
		std::string m_nick;
		std::string m_organization_name;
		std::string m_contacts_name;
		std::string m_phone;
		std::string m_bind_wx_account;
		std::string m_bind_alipay_account;
		std::string m_bind_weibo_account;
		std::string m_bind_merchant_id;
		std::string m_developer_id;
		std::string m_operator_id;
		uint32_t m_usertype;
		uint32_t m_createtimestamp;
		uint32_t m_lasttimestamp;
		std::string m_question1;
		std::string m_answer1;
		std::string m_question2;
		std::string m_answer2;
		std::string m_question3;
		std::string m_answer3;
		uint32_t m_status;
		std::string m_wx_msg;
		std::string m_extra_data;


	public:
		CardUserInfo(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};
class CardInfo : public TableBaseCard
{
public:
		std::string m_id;
		uint32_t m_status;
		uint64_t m_type;
		std::string m_logourl;
		std::string m_title;
		std::string m_merchant_id;
		std::string m_extra_data;
		std::string m_activate_form;
		uint32_t m_owned;
	    std::string m_wx_msg;
	    std::string m_date_info;
	    uint64_t m_quantity;
	    uint64_t m_total_quantity;
	    uint64_t m_create_timestamp;
	    uint64_t m_expire_timestamp;


	public:
		CardInfo(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};

class CardInfoIndexByMerchantid : public TableBaseCard
{
	public:
		std::string m_merchant_id;
		std::vector<CardInfo> m_cardinfo_list;

	public:
		CardInfoIndexByMerchantid(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};
class EntityShopInfo : public TableBaseCard
{
	public:
		std::string m_sid;
		std::string m_uin;
		std::string m_PoiId;
		uint64_t m_CreateTime;
		int m_update_status;
		std::string m_err_msg;
		std::string m_extra_data;
		std::string m_business_name;
        std::string m_branch_name;
        std::string m_province;
        std::string m_city;
        std::string m_district;
        std::string m_address;
		uint32_t m_available_state;

	public:
		EntityShopInfo(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};
class WXEntityShopInfo : public TableBaseCard
{
	public:
		std::string m_sid;
		std::string m_uin;
		std::string m_PoiId;
		uint64_t m_CreateTime;
		int m_update_status;
		std::string m_err_msg;
		std::string m_extra_data;
		std::string m_business_name;
        std::string m_branch_name;
        std::string m_province;
        std::string m_city;
        std::string m_district;
        std::string m_address;
		uint32_t m_available_state;
		uint64_t m_expire_timestamp_cached;

	public:
		WXEntityShopInfo(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};


class EntityShopsInfoIndexByUin : public TableBaseCard
{
	public:
		std::string m_uin;
		std::vector<EntityShopInfo> m_entityshops_info_list;

	public:
		EntityShopsInfoIndexByUin(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		static bool create_timestampDes(const EntityShopInfo &a,const EntityShopInfo &b){  return a.m_CreateTime<b.m_CreateTime; }
		std::string ToString();
};
class EntityShopsInfoIndexByPoiId : public TableBaseCard
{
	public:
		std::string m_poiId;
		EntityShopInfo m_entityshop_info;

	public:
		EntityShopsInfoIndexByPoiId(const std::string& strTableName):TableBaseCard(strTableName),m_entityshop_info(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};


class SubmerchantInfo : public TableBaseCard
{
	public:
		std::string m_id;
		std::string m_brand_name;
		std::string m_logo_url;
		std::string m_uin;
		int m_status;
		std::string m_wx_msg;
		std::string m_extra_data;
		std::string m_protocol;  // media_id
		std::string m_agreement;
		std::string m_operator;
		uint64_t m_create_timestamp;
		uint64_t m_expire_timestamp;

	public:
		SubmerchantInfo(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}
	public:
		int SelectFromDB(std::string& strErrMsg);
		int DeleteFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};

class SubmerchantInfoIndexByUin : public TableBaseCard
{
	public:
		std::string m_uin;
		std::vector<SubmerchantInfo> m_submerchant_info_list;

	public:
		SubmerchantInfoIndexByUin(const std::string& strTableName): TableBaseCard(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		static bool create_timestampDes(const SubmerchantInfo &a,const SubmerchantInfo &b){  return a.m_create_timestamp<b.m_create_timestamp; }
		std::string ToString();

};


#endif
