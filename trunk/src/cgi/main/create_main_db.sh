#!/bin/sh

db_name="main"

echo "create database if not exists $db_name" | mysql -uroot -p.hoosho,9942


# table password
password_table_def="(
	  uin char(255) not null default ''
	, password char(255) not null default ''
	, primary key (uin)
)ENGINE=InnoDB DEFAULT CHARSET=utf8	
"
echo "create table if not exists password $password_table_def"  | mysql -uroot -p.hoosho,9942 $db_name

#table user_info
user_info_table_def="( 
	  uin char(255) not null default ''
	, user_type tinyint unsigned not null default 0	
	, nick char(255) not null default ''
	, headportait char(255) not null default ''
	, contactor_name char(255) not null default ''
	, contactor_phone_no char(255) not null default ''
	, contactor_wx char(255) not null default ''
	, contactor_qq char(255) not null default ''
	, organization_name char(255) not null default ''
	, dev_id char(255) not null default ''
	, oper_id char(255) not null default ''
	, ts bigint unsigned not null default 0
	, primary key (uin)
	)ENGINE=InnoDB DEFAULT CHARSET=utf8	
"
echo "create table if not exists user_info $user_info_table_def" | mysql -uroot -p.hoosho,9942 $db_name


#table dev_info
dev_info_table_def="( 
	  id char(255) not null default ''
	, uin char(255) not null default ''
	, type tinyint unsigned not null default 0
	, status tinyint unsigned not null default 0
	, expire_ts bigint unsigned not null default 0
	, evaluate_active tinyint unsigned not null default 0
	, evaluate_quality tinyint unsigned not null default 0
	, evaluate_ontime tinyint unsigned not null default 0
	, region_area char(255) not null default ''
	, introduction blob not null default ''
	, dev_field bigint unsigned not null default 0
	, dev_language bigint unsigned not null default 0
	, show_case mediumblob not null default ''
	, company_name char(255) not null default ''
	, company_detail_address char(255) not null default ''
	, company_phone_no char(255) not null default ''
	, company_email char(255) not null default ''
	, company_business_licence_pic_url char(255) not null default ''
	, company_code char(255) not null default ''
	, company_corporation_name char(255) not null default ''
	, deal_num int unsigned not null default 0
	, primary key (id)
	)ENGINE=InnoDB DEFAULT CHARSET=utf8	
"
echo "create table if not exists dev_info $dev_info_table_def" | mysql -uroot -p.hoosho,9942 $db_name


#table oper_info
oper_info_table_def="( 
	  id char(255) not null default ''
	, uin char(255) not null default ''
	, status tinyint unsigned not null default 0
	, expire_ts bigint unsigned not null default 0
	, company_introduction blob not null default ''
	, company_name char(255) not null default ''
	, company_detail_address char(255) not null default ''
	, company_phone_no char(255) not null default ''
	, company_email char(255) not null default ''
	, company_business_licence_pic_url char(255) not null default ''
	, company_code char(255) not null default ''
	, company_corporation_name char(255) not null default ''
	, primary key (id)
	)ENGINE=InnoDB DEFAULT CHARSET=utf8	
"
echo "create table if not exists oper_info $oper_info_table_def" | mysql -uroot -p.hoosho,9942 $db_name


#table wx_public_account
wx_public_account_table_def="( 
	 wx char(255) not null default ''
	, oper_id char(255) not null default ''
	, name char(255) not null default ''
	, head_portrait_url char(255) not null default ''
	, qr_code_url char(255) not null default ''
	, introduction blob not null default ''
	, tag bigint unsigned not null default 0
	, fans_num bigint unsigned not null default 0
	, read_num bigint unsigned not null default 0
	, identified tinyint unsigned not null default 0
	, identified_info char(255) not null default ''
	, price_multi_pic_text_first_soft int unsigned not null default 0
	, price_multi_pic_text_first_hard int unsigned not null default 0
	, price_multi_pic_text_second_soft int unsigned not null default 0
	, price_multi_pic_text_second_hard int unsigned not null default 0
	, price_multi_pic_text_third_soft int unsigned not null default 0
	, price_multi_pic_text_third_hard int unsigned not null default 0
	, price_single_pic_text_soft int unsigned not null default 0
	, price_single_pic_text_hard int unsigned not null default 0
	, status tinyint unsigned not null default 0
	, primary key (wx)
	, index(oper_id)
	)ENGINE=InnoDB DEFAULT CHARSET=utf8	
"
echo "create table if not exists wx_public_account $wx_public_account_table_def" | mysql -uroot -p.hoosho,9942 $db_name


#table wx_public_account_feedback
wx_public_account_feedback_table_def="( 
	  id char(255) not null default ''
	, to_wx char(255) not null default ''
	, from_uin char(255) not null default ''
	, reason tinyint unsigned not null default 0
	, reason_extra blob not null default ''
	, ts bigint unsigned not null default 0
	, primary key (id)
	, index(to_wx)
	)ENGINE=InnoDB DEFAULT CHARSET=utf8	
"
echo "create table if not exists wx_public_account_feedback $wx_public_account_feedback_table_def" | mysql -uroot -p.hoosho,9942 $db_name


# table dev_order
dev_order_table_def="(
	  id char(255) not null default ''
	, need_uin char(255) not null default ''  
	, dev_id char(255) not null default ''
	, introduction blob not null default ''
	, doc_url char(255) not null default ''
	, like_preview_url mediumblob not null default ''
	, like_website_url mediumblob not null default ''
	, expect_info char(255) not null default ''
	, status tinyint unsigned not null default 0
	, create_ts bigint unsigned not null default 0
	, dev_start_ts bigint unsigned not null default 0
	, dev_end_ts bigint unsigned not null default 0
	, cancel_ts bigint unsigned not null default 0
	, primary key (id)
	, index(need_uin)
	, index(dev_id)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists dev_order $dev_order_table_def"  | mysql -uroot -p.hoosho,9942 $db_name


# table oper_order
oper_order_table_def="(
	  id char(255) not null default ''
	, need_uin char(255) not null default ''  
	, tag bigint unsigned not null default 0
	, wx_list blob not null default ''
	, spread_name char(255) not null default ''
	, spread_type tinyint unsigned not null default 0
	, spread_pos tinyint unsigned not null default 0
	, spread_start_ts bigint unsigned not null default 0
	, spread_certified_pic_url char(255) not null default ''
	, spread_remark blob not null default ''
	, chapter_title char(255) not null default ''	
	, chapter_author char(255) not null default ''	
	, chapter_cover_url char(255) not null default ''	
	, chapter_cover_insert_main_body tinyint unsigned not null default 0
	, chapter_summary mediumblob not null default ''
	, chapter_main_body mediumblob not null default ''
	, chapter_original_url blob not null default ''
	, status tinyint unsigned not null default 0
	, create_ts bigint unsigned not null default 0
	, cancel_ts bigint unsigned not null default 0
	, primary key (id)
	, index(need_uin)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists oper_order $oper_order_table_def"  | mysql -uroot -p.hoosho,9942 $db_name


# table oper_order_reverse
oper_order_reverse_table_def="(
	  wx char(255) not null default ''
	, oper_order_id char(255) not null default ''
	, oper_id char(255) not null default ''
	, primary key (wx, oper_order_id)
	, index(oper_id)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists oper_order_reverse $oper_order_reverse_table_def"  | mysql -uroot -p.hoosho,9942 $db_name

# table oper_order_reverse
oper_order_reverse_table_def="(
	  public_account_id char(255) not null default ''
	, oper_order_id char(255) not null default ''
	, oper_id char(255) not null default ''
	, primary key (public_account_id, oper_order_id)
	, index(oper_id)
)ENGINE=InnoDB DEFAULT CHARSET=utf8
"
echo "create table if not exists oper_order_reverse $oper_order_reverse_table_def"  | mysql -uroot -p.hoosho,9942 $db_name


# table open_order
open_order_table_def="(
	  id char(255) not null default ''
	, need_uin char(255) not null default ''  
	, need_type tinyint unsigned not null default 0
	, extra_info blob not null default ''
	, status tinyint unsigned not null default 0
	, create_ts bigint unsigned not null default 0
	, primary key (id)
	, index(need_uin)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists open_order $open_order_table_def"  | mysql -uroot -p.hoosho,9942 $db_name


