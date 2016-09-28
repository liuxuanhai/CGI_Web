#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_prefix=""
table_def=""

##########################################################################################################
#######                               feeds信息库 d_feeds		                                   #######
##########################################################################################################
db_name="d_feeds"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} created!!!"

#feed数据表，按feed_id%256, 分256表
table_name_prefix="t_feed_"
table_def="("
table_def+="feed_id bigint unsigned not null default 0,"                          		#feed id
table_def+="pa_appid_md5 bigint unsigned not null default 0,"                          	#公众号appid_md5
table_def+="openid_md5 bigint unsigned not null default 0,"                          	#创建者openid_md5
table_def+="cover_pic_id bigint unsigned not null default 0,"							#封面图片id
table_def+="pic_id_list blob not null default '',"										#图片id列表
table_def+="content blob not null default '',"											#feed内容
table_def+="num_favorite bigint unsigned not null default 0,"							#点赞数
table_def+="num_share bigint unsigned not null default 0,"								#分享数
table_def+="num_comment bigint unsigned not null default 0,"							#评论数
table_def+="num_read bigint unsigned not null default 0,"								#阅读数
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="del_ts bigint unsigned not null default 0,"									#删除时间
table_def+="feed_type bigint unsigned not null default 0,"								#feed类型（1：普通feed， 2：话题feed， 4：话题下的feed， 8：精华feed，16：banner feed）
table_def+="extra_data_0 blob not null default '', "									#预留字段0 
																						#（1）当feed类型为[话题下的feed]时，表示此条feed所属的话题feed的id)
																						#（2）当feed类型为[banner feed]时，表示此条feed的banner feed子类型)  （1：推广、广告， 2：（精华）推送）
table_def+="extra_data_1 blob not null default '', "									#预留字段1
																						#（1）当feed类型为[banner feed]，且
																						#		(①banner feed子类型为[推广]时，表示要跳转的url)
																						#		(②banner feed子类型为[推送]时，表示要推送的feed id)
table_def+="extra_data_2 blob not null default '', "									#预留字段2
table_def+="primary key (feed_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#feed 按appid索引表，按appid_md5%256, 分256表
table_name_prefix="t_feed_index_on_appid_"
table_def="("
table_def+="pa_appid_md5 bigint unsigned not null default 0,"							#公众号appid_md5
table_def+="openid_md5 bigint unsigned not null default 0,"								#文章作者openid_md5
table_def+="feed_id bigint unsigned not null default 0,"                          		#feed id
table_def+="feed_type bigint unsigned not null default 0,"								#feed类型（1：普通feed， 2：话题feed， 4：话题下的feed， 8：精华feed，16：banner feed）
table_def+="origin_feed_id bigint unsigned not null default 0, "						#当feed类型为[话题下的feed]时，表示此条feed所属的话题feed的id
table_def+="num_favorite bigint unsigned not null default 0,"							#点赞数
table_def+="primary key (pa_appid_md5, feed_id),"
table_def+="index(openid_md5)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#feed点赞表，按feed_id%256, 分256表
table_name_prefix="t_feed_favorite_"
table_def="("
table_def+="feed_id bigint unsigned not null default 0,"                          		#feed id
table_def+="openid_md5 bigint unsigned not null default 0,"								#点赞人
table_def+="create_ts bigint unsigned not null default 0,"								#点赞时间
table_def+="primary key (feed_id, openid_md5)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#feed收藏表，按appid%256, 分256表
table_name_prefix="t_feed_collect_"
table_def="("
table_def+="pa_appid_md5 bigint unsigned not null default 0,"							#公众号appid_md5
table_def+="feed_id bigint unsigned not null default 0,"                          		#feed id
table_def+="openid_md5 bigint unsigned not null default 0,"								#收藏人
table_def+="create_ts bigint unsigned not null default 0,"								#收藏时间
table_def+="primary key (pa_appid_md5, feed_id, openid_md5)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done


#跟帖表，按follow_id%256, 分256表
#注意：跟帖分两种：评论和回复，type字段注明两种的区别
table_name_prefix="t_feed_follow_"
table_def="("
table_def+="follow_id bigint unsigned not null default 0,"                          	#跟帖id
table_def+="content blob not null default '',"                          				#跟帖内容
table_def+="feed_id bigint unsigned not null default 0,"                          		#feed id
table_def+="type tinyint unsigned not null default 0,"                          		#类型(1:评论， 2：回复)
table_def+="origin_comment_id bigint unsigned not null default 0,"						#如果类型为2， 这个字段表示回复哪条评论
table_def+="openid_md5_from bigint unsigned not null default 0,"                        #主动者openid_md5
table_def+="openid_md5_to bigint unsigned not null default 0,"                        	#被动者openid_md5
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="del_ts bigint unsigned not null default 0,"									#删除时间
table_def+="primary key (follow_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done


#评论 按feedid索引表，按feed_id%256, 分256表
#注意，此表只保存评论和feed的关系，回复和评论的关系不需要插入到此index表
table_name_prefix="t_comment_index_on_feedid_"
table_def="("
table_def+="feed_id bigint unsigned not null default 0,"                          		#feed id
table_def+="comment_id bigint unsigned not null default 0,"                          	#评论id
table_def+="primary key (feed_id, comment_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done

#回复 按原始评论id索引表，按origin_comment_id%256, 分256表
#注意，此表只保存回复和原始评论的关系，feed和评论的关系不需要插入到此index表
table_name_prefix="t_reply_index_on_origin_comment_"
table_def="("
table_def+="origin_comment_id bigint unsigned not null default 0,"                      #评论id
table_def+="reply_id bigint unsigned not null default 0,"                          		#回复id
table_def+="primary key (origin_comment_id, reply_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done


#feed举报表，按appid%256, 分256表
table_name_prefix="t_feed_report_"
table_def="("
table_def+="pa_appid_md5 bigint unsigned not null default 0,"                      #appid
table_def+="openid_md5_from bigint unsigned not null default 0,"                   #举报人openid
table_def+="openid_md5_to bigint unsigned not null default 0,"                     #被举报人openid
table_def+="feed_id bigint unsigned not null default 0,"                           #被举报feedid
table_def+="report_ts bigint unsigned not null default 0,"                         #举报时间
table_def+="report_reason blob not null default '',"                               #举报原因
table_def+="primary key (pa_appid_md5, openid_md5_from, openid_md5_to, feed_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done


#用户禁言表，按appid%256, 分256表
table_name_prefix="t_user_forbid_"
table_def="("
table_def+="pa_appid_md5 bigint unsigned not null default 0,"                      #appid
table_def+="openid_md5 bigint unsigned not null default 0,"                        #被禁言人openid
table_def+="until_ts bigint unsigned not null default 0,"						   #禁言结束的时间
table_def+="primary key (pa_appid_md5, openid_md5)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
for((i=0;i<256;++i))
do
	index=`printf "%02x" $i`
	table_name="${table_name_prefix}${index}"
	echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
	echo "TABLE $table_name created!"
done




