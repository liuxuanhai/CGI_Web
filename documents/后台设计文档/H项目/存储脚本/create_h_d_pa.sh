#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_prefix=""
table_def=""

##########################################################################################################
#######                               公众号信息库 d_pa                                       	##########
##########################################################################################################
db_name="d_pa"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} created!!!"

#公众号信息表，单表
table_name="t_pa_info"
table_def="("
table_def+="appid_md5 bigint unsigned not null default 0,"                          	#公告号appid_md5
table_def+="appid char(128) not null default '',"									 	#公众号appid
table_def+="name char(255) not null default '',"										#公众号名称
table_def+="simple_desc blob not null default '',"										#公众号简介
table_def+="portrait_pic_id bigint unsigned not null default 0,"					    #公众号头像图片id 
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (appid_md5)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#公众号标签(appid)表，单表
table_name="t_pa_mark_field"
table_def="("
table_def+="appid_md5 bigint unsigned not null default 0,"								#公众号appid_md5
table_def+="field int unsigned not null default 0,"										#公众号领域
table_def+="primary key (appid_md5, field)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#公众号达人表，按appid_md5%256分表
table_name_prefix="t_pa_expert"
table_def="("
table_def+="appid_md5 bigint unsigned not null default 0,"								#公众号appid_md5
table_def+="openid_md5 bigint unsigned not null default 0,"								#达人openid_md5
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="extra_data_0 blob not null default '',"										#拓展字段
table_def+="primary key (appid_md5, openid_md5)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#公众号号主自定义Tab表，单表
table_name="t_pa_tab"
table_def="("
table_def+="appid_md5 bigint unsigned not null default 0,"								#公众号appid_md5
table_def+="id int unsigned not null default 0,"										#tab id
table_def+="name char(255) not null default '',"										#tab名称
table_def+="extra_data_0 blob not null default '',"										#拓展字段
table_def+="primary key (appid_md5, id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

