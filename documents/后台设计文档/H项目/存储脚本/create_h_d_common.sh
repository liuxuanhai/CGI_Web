#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_prefix=""
table_def=""

##########################################################################################################
#######                               公共库 d_common			                                   #######
##########################################################################################################
db_name="d_common"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} created!!!"

#图片表，按picid%256, 分256表
table_name_prefix="t_pic_"
table_def="("
table_def+="pic_id bigint unsigned not null default 0,"                          	#图片Id
table_def+="data mediumblob not null default '',"									#图片内容
table_def+="openid_md5 bigint unsigned not null default 0,"							#谁上传的
table_def+="create_ts bigint unsigned not null default 0,"							#创建时间
table_def+="primary key (pic_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

