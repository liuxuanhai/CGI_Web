#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_prefix=""
table_def=""

##########################################################################################################
#######                               UE资源库 d_ue			                                   #######
##########################################################################################################
db_name="d_ue"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} created!!!"

#资源表，按resource_id%256, 分256表
table_name_prefix="t_resource_"
table_def="("
table_def+="resource_id bigint unsigned not null default 0,"                          	#资源Id
table_def+="resource_type int unsigned not null default 0,"                          	#资源类型
table_def+="data LongBlob not null default '',"					        #资源raw数据
table_def+="name blob not null default '',"						#资源名称
table_def+="create_ts bigint unsigned not null default 0,"				#创建时间
table_def+="primary key (resource_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done


#内容表，按content_id%256, 分256表
table_name_prefix="t_content_"
table_def="("
table_def+="content_id bigint unsigned not null default 0,"                          	#资源Id
table_def+="data MediumBlob not null default '',"					#资源raw数据
table_def+="create_ts bigint unsigned not null default 0,"				#创建时间
table_def+="primary key (content_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done
