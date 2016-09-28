#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_prefix=""
table_def=""

##########################################################################################################
#######                               用户信息库 d_user		                                       #######
##########################################################################################################
db_name="d_user"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} created!!!"

#用户信息表，按虎笑openid_md5%256, 分256表
table_name_prefix="t_user_info_"
table_def="("
table_def+="openid_md5 bigint unsigned not null default 0,"                          	#虎笑openid_md5
table_def+="openid char(128) not null default '',"									 	#虎笑openid
table_def+="portrait_pic_id bigint unsigned not null default 0,"						#头像图片id
table_def+="portrait_pic_url_wx blob not null default '',"								#微信头像url
table_def+="nick tinyblob not null default '',"											#昵称
table_def+="nick_wx tinyblob not null default '',"										#微信昵称
table_def+="addr_country char(128) not null default '',"								#国家
table_def+="addr_province char(128) not null default '',"								#省份
table_def+="addr_city char(128) not null default '',"									#城市
table_def+="addr_country_md5 bigint unsigned not null default 0,"						#国家md5
table_def+="addr_province_md5 bigint unsigned not null default 0,"						#省份md5
table_def+="addr_city_md5 bigint unsigned not null default 0,"							#城市md5
table_def+="sex tinyint unsigned not null default 0,"									#性别(1:男,2:女,0:未知)
table_def+="phone_no bigint unsigned not null default 0,"								#手机号码
table_def+="email char(255) not null default '',"										#邮箱
table_def+="self_desc blob not null default '',"										#个性签名
table_def+="main_page_cover_pic_id bigint unsigned not null default 0,"					#主页封面图片id
table_def+="hoosho_no bigint unsigned not null default 0,"								#虎笑帐号
table_def+="birthday_ts bigint unsigned not null default 0,"							#生日
table_def+="create_ts bigint unsigned not null default 0,"								#注册时间
table_def+="last_update_ts bigint unsigned not null default 0,"							#最近一次修改信息时间
table_def+="primary key (openid_md5)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#用户关注表，按虎笑openid_md5_from%256, 分256表
table_name_prefix="t_user_follow_"
table_def="("
table_def+="openid_md5_from bigint unsigned not null default 0,"						#用户A
table_def+="openid_md5_to bigint unsigned not null default 0,"							#用户B
table_def+="relation bigint unsigned not null default 0,"								#关系(1：A关注B, 2:A被B关注)
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (openid_md5_from, openid_md5_to, relation)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#用户密码表，按虎笑帐号%256, 分256表
table_name_prefix="t_user_password_"
table_def="("
table_def+="hoosho_no bigint unsigned not null default 0,"								#虎笑帐号
table_def+="hoosho_password char(255) not null default '',"								#虎笑密码
table_def+="openid_md5 bigint unsigned not null default 0,"								#虎笑openid_md5
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (hoosho_no)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#用户权限表, 单表
table_name="t_user_appid_power"
table_def="("
table_def+="openid_md5 bigint unsigned not null default 0,"								#虎笑openid_md5
table_def+="pa_appid_md5 bigint unsigned not null default 0,"							#公众号appid_md5
table_def+="power bigint unsigned not null default 0,"								#权限值
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (openid_md5, pa_appid_md5, power)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#用户标签(appid)表，单表
table_name="t_user_mark_appid"
table_def="("
table_def+="openid_md5 bigint unsigned not null default 0,"								#虎笑openid_md5
table_def+="pa_appid_md5 bigint unsigned not null default 0,"							#公众号appid_md5
table_def+="pa_openid bigint unsigned not null default 0,"								#公众号openid
table_def+="is_follow bigint unsigned not null default 0,"								#是否关注了公众号
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (openid_md5, pa_appid_md5)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#用户全局配置
table_name="t_user_config"
table_def="("
table_def+="c_key bigint unsigned not null default 0,"								#配置key
table_def+="c_value bigint unsigned not null default 0,"							#配置value
table_def+="primary key (c_key)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#僵尸用户(zombie)表，单表
table_name="t_user_zombie"
table_def="("
table_def+="uin char(255) not null default '',"										#虎笑管理员用户uin
table_def+="openid_md5 bigint unsigned not null default 0,"							#僵尸用户openid_md5
table_def+="primary key (uin, openid_md5)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#管理者信息(manager)表，单表
table_name="t_manager"
table_def="("
table_def+="uin char(255) not null default '',"										#虎笑管理员用户uin
table_def+="passwd char(255) not null default '',"									#密码
table_def+="pa_appid_md5 bigint unsigned not null default 0,"						#公众号appid_md5
table_def+="openid_md5 bigint unsigned not null default 0,"							#系统生成的openid_md5
table_def+="create_ts bigint unsigned not null default 0,"							#创建时间
table_def+="admin bigint unsigned not null default 0,"								#权限（1：号主，0：普通管理员）
table_def+="primary key (uin)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#公众号 UV 统计表，按年月分表
table_name_prefix="t_pa_uv_"
table_def="("
table_def+="pa_appid_md5 bigint unsigned not null default 0,"						#公众号appid_md5
table_def+="openid_md5 bigint unsigned not null default 0,"							#用户openid_md5
table_def+="create_ts bigint unsigned not null default 0,"							#日期 例：20160523
table_def+="primary key (pa_appid_md5, openid_md5)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=2016;i<2116;++i))
do
	year=`printf "%02d" $i`
	for((j=1;j<=12;++j))
	do
		month=`printf "%02d" $j`
		table_name="${table_name_prefix}${year}${month}"
		echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
		echo "TABLE $table_name created!"
	done
done
