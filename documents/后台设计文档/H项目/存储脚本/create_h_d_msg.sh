#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_prefix=""
table_def=""

##########################################################################################################
#######                               私信库 d_msg			                                       #######
##########################################################################################################
db_name="d_msg"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} created!!!"

#消息内容表，按msgid%256, 分256表
table_name_prefix="t_msg_content_"
table_def="("
table_def+="msg_id bigint unsigned not null default 0,"                          	#消息Id
table_def+="content blob not null default '',"									 	#消息内容
table_def+="openid_md5_from bigint unsigned not null default 0,"					#谁发
table_def+="openid_md5_to bigint unsigned not null default 0,"						#发给谁
table_def+="primary key (msg_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#会话消息关系表，按session_id%256, 分256表
table_name_prefix="t_session_msg_"
table_def="("
table_def+="session_id bigint unsigned not null default 0,"							#会话Id
table_def+="msg_id bigint unsigned not null default 0,"                          	#消息Id
table_def+="create_ts bigint unsigned not null default 0,"							#创建时间
table_def+="del_status tinyint unsigned not null default 0,"						#删除状态，第0位表示openid_md5小者删除，第1位表示打折
table_def+="primary key (session_id, msg_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#会话用户关系表，按openid_md5_from%256, 分256表
table_name_prefix="t_session_user_"
table_def="("
table_def+="openid_md5_from bigint unsigned not null default 0,"					#用户A openid_md5
table_def+="openid_md5_to bigint unsigned not null default 0,"						#用户B openid_md5
table_def+="session_id bigint unsigned not null default 0,"                         #会话Id
table_def+="create_ts bigint unsigned not null default 0,"							#创建时间
table_def+="newmsg_status tinyint(1) not null default 0,"							#是否有新的消息
table_def+="primary key (openid_md5_from, openid_md5_to)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#用户红点表，按openid_md5%256,分256表
table_name_prefix="t_red_point_"
table_def="("
table_def+="openid_md5 bigint unsigned not null default 0,"							#用户openid_md5
table_def+="pa_appid_md5 bigint unsigned not null default 0,"						#公众号appid_md5
table_def+="type bigint unsigned not null default 0,"								#红点类型 [1:点赞 2:评论 3:粉丝 4:私信 5:系统消息]
table_def+="value bigint unsigned not null default 0,"								#新增数量
table_def+="create_ts bigint unsigned not null default 0,"							#创建时间
table_def+="primary key (openid_md5, pa_appid_md5, type)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;i++))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#系统消息，单表
table_name="t_notice_common"
table_def="("
table_def+="pa_appid_md5 bigint unsigned not null default 0,"                          	#公告号appid_md5
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="title blob not null default '',"											#标题
table_def+="content blob not null default '',"											#正文
table_def+="primary key (pa_appid_md5, create_ts)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#个人通知表，按openid_md5%256, 分256表
table_name_prefix="t_notice_user_"
table_def="("
table_def+="openid_md5 bigint unsigned not null default 0,"                             #用户openid_md5 
table_def+="pa_appid_md5 bigint unsigned not null default 0,"                           #公众号appid_md5
table_def+="create_ts bigint unsigned not null default 0,"                              #创建时间
table_def+="type tinyint unsigned not null default 0,"                           		#个人通知类型 [1.点赞 2.评论 3.粉丝 ]
table_def+="status tinyint unsigned not null default 0,"                           		#状态0:未读，1：已读
table_def+="extra_data_0 blob not null default '',"                             		#扩展字段0 [type=1/2 表示被点赞或评论的feed_id, type=3表示粉丝openid_md5]
table_def+="extra_data_1 blob not null default '',"                             		#扩展字段1 [type=1 表示点赞用户openid_md5, type=2 表示origin_comment_id]
table_def+="extra_data_2 blob not null default '',"                             		#扩展字段2 [type=2 表示follow_id]
table_def+="primary key (openid_md5, pa_appid_md5,type,create_ts)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
index=`printf "%02x" $i`
table_name="${table_name_prefix}${index}"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"
done

