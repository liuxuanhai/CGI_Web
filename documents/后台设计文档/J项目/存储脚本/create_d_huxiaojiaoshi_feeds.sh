#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_def=""

##########################################################################################################
#######                               J项目库 d_huxiaojiaoshi	                                   #######
##########################################################################################################
db_name="d_huxiaojiaoshi"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} created!!!"

#feed数据表
table_name="t_feed"
table_def=""
table_def="("
table_def+="feed_id bigint unsigned not null default 0,"                          		#feed id
table_def+="openid char(128) not null default 0,"                          				#创建者openid
table_def+="content blob not null default '',"											#feed内容
table_def+="ts bigint unsigned not null default 0,"										#创建时间
table_def+="num_follow bigint unsigned not null default 0,"								#回答数
table_def+="num_listen bigint unsigned not null default 0,"								#总偷听数
table_def+="need_sex bigint unsigned not null default 0,"								#限制回答者的性别【0：不限制，1：仅限男生，2：仅限女生】
table_def+="primary key (feed_id),"
table_def+="index (openid(128))"														
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"

echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#follow数据表
table_name="t_follow"
table_def=""
table_def+="("
table_def+="follow_id bigint unsigned not null default 0,"								#follow_id
table_def+="feed_id bigint unsigned not null default 0,"								#feed_id
table_def+="openid char(128) not null default '',"										#回答者openid
table_def+="content_type bigint unsigned not null default 0,"							#多媒体类型【1：音频，2：视频】
table_def+="content_id char(128) not null default '',"									#多媒体id
table_def+="content_duration bigint unsigned not null default 0,"						#多媒体长度
table_def+="ts bigint unsigned not null default 0,"										#创建时间
table_def+="num_listen bigint unsigned not null default 0,"								#偷听人数
table_def+="num_comment_good bigint unsigned not null default 0,"						#评价“好”次数
table_def+="num_comment_default bigint unsigned not null default 0,"					#评价“一般”次数
table_def+="num_comment_bad bigint unsigned not null default 0,"						#评价“差”次数
table_def+="out_trade_no char(128) not null default '',"									#支付订单号
table_def+="primary key(follow_id),"
table_def+="index (feed_id),"
table_def+="index (openid(128))"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"

echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#listen数据表
table_name="t_listen"
table_def=""
table_def+="("
table_def+="listen_id bigint unsigned not null default 0,"								#listen_id
table_def+="openid char(128) not null default '',"										#偷听者openid
table_def+="follow_id bigint unsigned not null default 0,"								#follow_id
table_def+="feed_id bigint unsigned not null default 0,"								#feed_id
table_def+="ts bigint unsigned not null default 0,"										#偷听时间
table_def+="comment_type bigint unsigned not null default 0,"							#评价类型【0：一般，1：好，2：差】
table_def+="out_trade_no char(128) not null default '',"									#支付订单号
table_def+="primary key(listen_id),"				
table_def+="index (follow_id),"
table_def+="index (openid(128))"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"

echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#history数据表
table_name="t_history"
table_def=""
table_def+="("
table_def+="openid char(128) not null default '',"										#用户
table_def+="feed_id bigint unsigned not null default 0,"								#问题
table_def+="ts bigint unsigned not null default 0,"										#浏览时间
table_def+="primary key(openid, feed_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"

echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"
