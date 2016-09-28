#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_def=""

##########################################################################################################
#######                               统计库 d_statistic                                           #######
##########################################################################################################
db_name="d_statistic"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} create"

#按天统计数据表， 按ts%256 分256表
table_name_prefix="t_statistic_"
table_def="("
table_def+="ts bigint unsigned not null default 0,"                             #零点时间戳
table_def+="statistic_type int unsigned not null default 0,"                    #统计类型
table_def+="statistic_value bigint unsigned not null default 0,"                #统计结果值
table_def+="primary key (ts, statistic_type)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"

for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done
