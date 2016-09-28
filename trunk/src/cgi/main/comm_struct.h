#ifndef _HX_COMM_STRUCT_H_
#define _HX_COMM_STRUCT_H_

#include "cgi/cgi.h"

class TableBase
{
	public:
		enum TABLE_BASE_RET
		{
			TABLE_BASE_RET_OK = 0,
			TABLE_BASE_RET_NOT_EXIST = 10001,
			TABLE_BASE_RET_UPDATE_FAILED = 10002,
			TABLE_BASE_RET_ALREADY_EXIST = 10003,
			TABLE_BASE_RET_AUTH_FAILED = 10004,
		};
		
	public:
		string m_table_name;
		
		static lce::cgi::CMysql g_common_mysql;
		static lce::cgi::CMysql g_main_mysql;

	public:
		TableBase(const std::string& strTableName): m_table_name(strTableName)
		{
		
		}
};



class CommonPicture : public TableBase
{
	public:
		std::string m_id;
		uint32_t m_type;
		std::string m_data;
		uint32_t m_source;
		std::string m_owner_uin;
	public:
		CommonPicture(const std::string& strTableName): TableBase(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
};

class Document : public TableBase
{		
	public:
		std::string m_id;
		uint32_t m_type;
		std::string m_data;
		uint32_t m_source;
		std::string m_owner_uin;
	
	public:
		Document(const std::string& strTableName): TableBase(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
};

class Password: public TableBase
{
	public:
		std::string m_uin;
		std::string m_password;

	public:
		Password(const std::string& strTableName): TableBase(strTableName)
		{
		}

	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
};

class UserInfo : public TableBase
{
	public:
		std::string m_uin;
		std::string m_nick;
		std::string m_headportait_url;
		std::string m_contactor_name;
		std::string m_contactor_phone_no;
		std::string m_contactor_wx;
		std::string m_contactor_qq;
		std::string m_organization_name;
		std::string m_dev_id;
		std::string m_oper_id;
		uint32_t m_user_type;
		uint32_t m_ts;
		
	public:
		UserInfo(const std::string& strTableName): TableBase(strTableName)
		{
		}
		UserInfo(): TableBase("")
		{
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
		void ToSimpleAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};

class DeveloperInfo : public TableBase
{
	public:
		std::string m_id;
		std::string m_uin;
		uint32_t m_type;       //个人 企业
		uint32_t m_status;
		uint64_t m_expire_ts;
		uint32_t m_evaluate_active;
		uint32_t m_evaluate_quality;
		uint32_t m_evaluate_ontime;
		std::string m_region_area;
		std::string m_introduction;
		uint64_t m_dev_field;
		uint64_t m_dev_language;
		std::string m_show_case;
		std::string m_company_name;
		std::string m_company_detail_address;
		std::string m_company_phone_no;
		std::string m_company_email;
		std::string m_company_business_licence_pic_url;
		std::string m_company_code;
		std::string m_company_corporation_name;
		uint32_t m_deal_num;
	public:
		DeveloperInfo(const std::string& strTableName): TableBase(strTableName)
		{
		}
		DeveloperInfo():TableBase("")
		{
		}
	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const ;
};

class DeveloperInfoWithFieldLanguageTypeByOrder : public TableBase
{
	public:
		enum DeveloperInfoOrderBy
		{
			DeveloperInfoOrderBy_NONE = 0,
			DeveloperInfoOrderBy_ACTIVE = 1,
			DeveloperInfoOrderBy_QUALITY = 2,
			DeveloperInfoOrderBy_ONTIME = 3,
			DeveloperInfoOrderBy_STAR = 4,
		};
	public:
		uint64_t m_dev_field;
		uint32_t m_dev_language;
		uint32_t m_type;
		int32_t m_order_by;
		uint32_t m_limit_from;
		uint32_t m_limit_len;
		
		std::vector<DeveloperInfo> m_developer_info_list;

	public:
		DeveloperInfoWithFieldLanguageTypeByOrder(const std::string& strTableName): TableBase(strTableName)
		{
			
		}

	public:
		int SelectFromDB(uint32_t& dwTotalNum, std::string& strErrMsg);
		std::string ToString();
		
		static bool CompareActive(const DeveloperInfo &a,const DeveloperInfo &b)
		{
			return a.m_evaluate_active > b.m_evaluate_active;
		}

		static bool CompareQuality(const DeveloperInfo &a,const DeveloperInfo &b)
		{
			return a.m_evaluate_quality > b.m_evaluate_quality; 
		}
		
		static bool CompareOntime(const DeveloperInfo &a,const DeveloperInfo &b)
		{
			return a.m_evaluate_ontime > b.m_evaluate_ontime; 
		}
		
		static bool CompareStar(const DeveloperInfo &a,const DeveloperInfo &b)
		{
			uint32_t a_star = (a.m_evaluate_active + a.m_evaluate_quality + a.m_evaluate_ontime)/3;
			uint32_t b_star = (b.m_evaluate_active + b.m_evaluate_quality + b.m_evaluate_ontime)/3;
			return a_star > b_star;
		}
};

class OperInfo : public TableBase
{
	public:
		std::string m_id;
		std::string m_uin;
		uint32_t m_status;
		uint64_t m_expire_ts;
		std::string m_company_introduction;
		std::string m_company_name;
		std::string m_company_detail_address;
		std::string m_company_phone_no;
		std::string m_company_email;
		std::string m_company_business_licence_pic_url;
		std::string m_company_code;
		std::string m_company_corporation_name;
	public:
		OperInfo(const std::string& strTableName): TableBase(strTableName)
		{
		}
		OperInfo():TableBase("")
		{
		}
	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const ;
};

class WXPublicAccount : public TableBase
{
	public:
		std::string m_wx;
		std::string m_oper_id;
		std::string m_name;
		
		std::string m_headportrait_url;
		std::string m_qrcode_url;
		std::string m_introduction;
		uint64_t m_tag;
		uint64_t m_fans_num;
		uint64_t m_read_num;
		uint32_t m_identified;
		std::string m_identified_info;
		uint32_t m_price_multi_pic_text_first_soft;
		uint32_t m_price_multi_pic_text_first_hard;
		uint32_t m_price_multi_pic_text_second_soft;
		uint32_t m_price_multi_pic_text_second_hard;
		uint32_t m_price_multi_pic_text_third_soft;
		uint32_t m_price_multi_pic_text_third_hard;
		uint32_t m_price_single_pic_text_soft;
		uint32_t m_price_single_pic_text_hard;
		uint32_t m_status;
		
	public:
		WXPublicAccount(const std::string& strTableName): TableBase(strTableName)
		{
		}

		WXPublicAccount():TableBase("")
		{
			
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		int DeleteFromDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const ;
};

class WXPublicAccountWithTagAndCertifiedStatus : public TableBase
{
	public:
		uint64_t m_tag;
		uint32_t m_cetified_status;
		uint32_t m_limit_from;
		uint32_t m_limit_len;
		
		std::vector<WXPublicAccount> m_wx_public_account_list;
	public:
		WXPublicAccountWithTagAndCertifiedStatus(const std::string& strTableName): TableBase(strTableName)
		{
		}
		
	public:
		int SelectFromDB(uint32_t& dwTotalNum, std::string& strErrMsg);
		std::string ToString();
};

class WXPublicAccountWithOperId : public TableBase
{
	public:
		std::string m_oper_id;
		
		std::vector<WXPublicAccount> m_wx_public_account_list;
	public:
		WXPublicAccountWithOperId(const std::string& strTableName): TableBase(strTableName)
		{
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};



class WXPublicAccountFeedBack : public TableBase
{
	public:
		std::string m_id;
		std::string m_to_wx;
		std::string m_from_uin;
		uint32_t m_reason;
		std::string m_reason_extra;
		uint64_t m_ts;
		
	public:
		WXPublicAccountFeedBack(const std::string& strTableName): TableBase(strTableName)
		{
		}

		WXPublicAccountFeedBack():TableBase("")
		{
			
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue)const ;
};

class DevOrder : public TableBase
{
	public:
		std::string m_id;
		std::string m_need_uin;
		std::string m_dev_id;
		std::string m_introduction;
		std::string m_doc_url;
		std::string m_like_preview_url;
		std::string m_like_website_url;
		std::string m_expect_info;
		uint32_t m_status;
		uint64_t m_create_ts;
		uint64_t m_dev_start_ts;
		uint64_t m_dev_end_ts;
		uint64_t m_cancel_ts;
		
	public:
		DevOrder(const std::string& strTableName): TableBase(strTableName)
		{
		}

		DevOrder():TableBase("")
		{

		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};

class DevOrderWithNeedUin : public TableBase
{
	public:
		std::string m_need_uin;
		std::vector<DevOrder> m_dev_order_list;
		
	public:
		DevOrderWithNeedUin(const std::string& strTableName): TableBase(strTableName)
		{
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};

class DevOrderWithDevId : public TableBase
{
	public:
		std::string m_dev_id;
		std::vector<DevOrder> m_dev_order_list;
		
	public:
		DevOrderWithDevId(const std::string& strTableName): TableBase(strTableName)
		{
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};

class OperOrder : public TableBase
{
	public:
		std::string m_id;
		std::string m_need_uin;
		uint64_t m_tag;
		std::string m_wx_list; //list
		std::string m_spread_name;
		uint32_t m_spread_type;
		uint32_t m_spread_pos;
		uint64_t m_spread_start_ts;
		std::string m_spread_certified_pic_url;
		std::string m_spread_ramark;
		std::string m_chapter_title;
		std::string m_chapter_author;
		std::string m_chapter_cover_url;
		uint32_t m_chapter_cover_insert_main_body;
		std::string m_chapter_summary;
		std::string m_chapter_main_body;
		std::string m_chapter_original_url;
		uint32_t m_status;
		uint64_t m_create_ts;
		uint64_t m_cancel_ts;
		
	public:
		OperOrder(const std::string& strTableName): TableBase(strTableName)
		{
		}

		OperOrder():TableBase("")
		{

		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};

class OperOrderWithNeedUin : public TableBase
{
	public:
		std::string m_need_uin;
		std::vector<OperOrder> m_oper_order_list;
		
	public:
		OperOrderWithNeedUin(const std::string& strTableName): TableBase(strTableName)
		{
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};


class OperOrderReverse : public TableBase
{
	public:
		std::string m_wx; //一个大号
		std::string m_oper_order_id;
		std::string m_oper_id;
		
	public:
		OperOrderReverse(const std::string& strTableName): TableBase(strTableName)
		{
		}

		OperOrderReverse():TableBase("")
		{

		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};


class OperOrderReverseWithOperId : public TableBase
{
	public:
		std::string m_oper_id;
		std::vector<OperOrderReverse> m_oper_order_reverse_list;
		
	public:
		OperOrderReverseWithOperId(const std::string& strTableName): TableBase(strTableName)
		{
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};

class OperOrderReverseWithWx : public TableBase
{
	public:
		std::string m_wx;
		std::vector<OperOrderReverse> m_oper_order_reverse_list;
		
	public:
		OperOrderReverseWithWx(const std::string& strTableName): TableBase(strTableName)
		{
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};



class OpenOrder : public TableBase
{
	public:
		std::string m_id;
		std::string m_need_uin;
		uint64_t m_need_type;
		std::string m_extra_info;
		uint32_t m_status;
		uint64_t m_create_ts;
		
	public:
		OpenOrder(const std::string& strTableName): TableBase(strTableName)
		{
		}

		OpenOrder():TableBase("")
		{
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		int UpdateToDB(std::string& strErrMsg);
		std::string ToString();
		void ToAnyValue(lce::cgi::CAnyValue& stAnyValue) const;
};

class OpenOrderWithNeedUin : public TableBase
{
	public:
		std::string m_need_uin;
		std::vector<OpenOrder> m_open_order_list;
		
	public:
		OpenOrderWithNeedUin(const std::string& strTableName): TableBase(strTableName)
		{
		}
		
	public:
		int SelectFromDB(std::string& strErrMsg);
		std::string ToString();
};

#endif


