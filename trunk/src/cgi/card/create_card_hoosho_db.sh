#!/bin/sh

db_name="card"

echo "create database if not exists $db_name" | mysql -uroot -p.hoosho,9942


# table card_user_password
card_user_password_table_def="(
	 uin char(255) not null default ''
	,password char(255) not null default ''
	,primary key (uin)
)ENGINE=InnoDB DEFAULT CHARSET=utf8	
"
echo "create table if not exists card_user_password $card_user_password_table_def"  | mysql -uroot -p.hoosho,9942 $db_name

# table card_user_info
card_user_info_table_def="(
	  uin char(255) not null default ''
	, nick char(255) not null default ''
	, organization_name char(255) not null default ''
	, contacts_name char(255) not null default ''
	, phone char(32) not null default ''
	, bind_wx_account char(255) not null default ''
	, bind_alipay_account char(255) not null default ''
	, bind_weibo_account char(255) not null default ''
	, bind_hooshouin char(255) not null default ''
	, bind_merchant_id char(255) not null default ''
	, developer_id char(255) not null default ''
	, operator_id char(255) not null default ''
	, usertype int unsigned not null default 1
	, create_timestamp int unsigned not null default 0
	, last_timestamp int unsigned not null default 0
	, question1 char(255) not null default ''
	, answer1 char(255) not null default ''
	, question2 char(255) not null default ''
	, answer2 char(255) not null default ''
	, question3 char(255) not null default ''
	, answer3 char(255) not null default ''
	, status int unsigned not null default 0
	, wx_msg char(255) not null default ''
	, extra_data blob not null default ''
	, primary key (uin)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 	
"
echo "create table if not exists card_user_info $card_user_info_table_def"  | mysql -uroot -p.hoosho,9942 $db_name
#table wx_account_auth_info
wx_account_auth_info_table_def="(
	 auth_appid char(255) not null default ''
	, auth_access_token char(255) not null default ''
	, auth_refresh_token char(255) not null default ''
	, expire_timestamp int unsigned not null default 0
	, primary key (auth_appid)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists wx_account_auth_info $wx_account_auth_info_table_def"  | mysql -uroot -p.hoosho,9942 $db_name


#table card_info
card_info_table_def="(
	  id char(255) not null default ''
	, status int not null default 0
	, type int not null default 0  	
	, logourl char(255) default ''
	, title char(255) not null default ''
	, merchant_id char(255) not null default ''
	, extra_data blob not null default ''
	, activate_form blob not null default ''
	, owned int not null
	, wx_msg char(255) not null default ''
	, date_info char(255) not null default ''
	, quantity int not null default 0
	, total_quantity int not null default 0
	, create_timestamp int unsigned not null default 0
	, expire_timestamp int unsigned not null default 0
	, primary key (id)
	, index(merchant_id,create_timestamp)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists card_info $card_info_table_def"  | mysql -uroot -p.hoosho,9942 $db_name


#table card_get_info
card_get_info_table_def="(
	  id char(255) not null default ''
	, status int not null default 0
	, type int not null default 0  	
	, logourl char(255) default ''
	, title char(255) not null default ''
	, points int not null default 0
	, balance int not null default 0
	, level int not null default 0
	, openid char(255) not null default ''
	, timestamp int unsigned not null default 0
	, uin char(255) not null default ''
	, card_id char(255) not null default ''
	, extra_data blob not null default ''	
	, primary key (id)
	, index(uin)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists card_get_info $card_get_info_table_def"  | mysql -uroot -p.hoosho,9942 $db_name


#table membercard_info
membercard_info_table_def="(
	  id char(255) not null default ''
	, status int not null default 0
	, type int not null default 0  	
	, cardid char(255) default ''
	, openid char(255) not null default ''
	, create_timestamp int unsigned not null default 0
	, extra_data blob not null default ''	
	, primary key (id)
	, index(cardid)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists membercard_info $membercard_info_table_def"  | mysql -uroot -p.hoosho,9942 $db_name

#table entity_shop_info
entity_shop_info_table_def="(
	  sid char(255) not null default ''
	, uin char(255) not null default ''
	, PoiId char(255) not null default ''
	, CreateTime int unsigned not null default 0
	, update_status int unsigned not null default 0
	, err_msg blob not null default ''
	, extra_data blob not null default ''	
	, business_name char(255) not null default ''
        , branch_name char(255) not null default ''
        , province char(255) not null default ''
        , city char(255) not null default ''
	, district char(255) not null default ''
        , address char(255) not null default ''
	, available_state int unsigned not null default 0
	, primary key (sid)
	, index(uin,CreateTime) 
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists entity_shop_info $entity_shop_info_table_def"  | mysql -uroot -p.hoosho,9942 $db_name

#table wx_entity_shop_info
wx_entity_shop_info_table_def="(
	  sid char(255) not null default ''
	, uin char(255) not null default ''
	, PoiId char(255) not null default ''
	, CreateTime int unsigned not null default 0
	, update_status int unsigned not null default 0
	, err_msg blob not null default ''
	, extra_data blob not null default ''	
	, business_name char(255) not null default ''
        , branch_name char(255) not null default ''
        , province char(255) not null default ''
        , city char(255) not null default ''
	, district char(255) not null default ''
        , address char(255) not null default ''
	, available_state int unsigned not null default 0
	, expire_timestamp_cached int unsigned not null default 0
	, primary key (sid) 
	, index(PoiId) 
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists wx_entity_shop_info $wx_entity_shop_info_table_def"  | mysql -uroot -p.hoosho,9942 $db_name

#table landing_page_info
landing_page_info_table_def="(
	  page_id int unsigned not null default 0
	, uin char(255) not null default ''
	, url char(255) not null default ''
	, extra_data blob not null default ''
	, primary key (page_id)
	, index(uin)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists landing_page_info $landing_page_info_table_def"  | mysql -uroot -p.hoosho,9942 $db_name

# table card_picture
card_pic_table_def="(
	  id char(255) not null default ''
	, data mediumblob not null default '' 
	, status int unsigned not null default 0
	, uin char(255) not null default ''
	, create_timestamp int unsigned not null default 0
	, pic_usage int unsigned not null default 0
	, primary key (id)
	, index(uin)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 	
"
echo "create table if not exists card_picture $card_pic_table_def"  | mysql -uroot -p.hoosho,9942 $db_name

#table submerchant_info
submerchant_info_def="(
	id char(255) not null default ''
	, brand_name char(255) not null default ''
	, logo_url char(255) not null default ''
	, uin char(255) not null default ''
	, status int unsigned not null default 0
	, wx_msg char(255) not null default ''
	, extra_data blob not null default ''
	, protocol char(255) not null default ''
	, agreement char(255) not null default ''
	, operator char(255) not null default ''
	, create_timestamp int unsigned not null default 0
	, expire_timestamp int unsigned not null default 0
	, primary key(id)
	, index(uin,create_timestamp)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 	
"
echo "create table if not exists submerchant_info $submerchant_info_def"  | mysql -uroot -p.hoosho,9942 $db_name

#table options
options_info_table_def="(
	  id int unsigned not null default 0
	, value char(255) not null default ''
	, primary key (id)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 
"
echo "create table if not exists options_info $options_info_table_def"  | mysql -uroot -p.hoosho,9942 $db_name


