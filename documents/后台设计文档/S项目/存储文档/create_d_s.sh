#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_def=""

##########################################################################################################
#######                               S项目库 d_s           		                               #######
##########################################################################################################
db_name="d_s"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} created!!!"

#用户信息表，单表
table_name="t_user_info"
table_def="("
table_def+="user_id bigint unsigned not null default 0,"								#用户平台ID
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="nickname tinyblob not null default '',"										#微信昵称
table_def+="sex tinyint unsigned not null default 0,"									#性别(1:男,2:女,0:未知)
table_def+="headimgurl blob not null default '',"										#微信头像url
table_def+="self_desc blob not null default '',"										#自我描述
table_def+="phone char(32) not null default '',"										#联系电话
table_def+="identity_status tinyint unsigned not null default 0,"						#认证状态 [0:未认证 1:提交待审核 2:审核通过 3:审核失败]
table_def+="user_type tinyint unsigned not null default 0,"								#用户类型 [0:普通用户]
table_def+="default_addr_num bigint unsigned not null default 0,"						#默认地址信息编号
table_def+="extra_data blob not null default '',"										#扩展信息 
table_def+="primary key (user_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#实名信息表，单表
table_name="t_user_identity_info"
table_def="("
table_def+="user_id bigint unsigned not null default 0,"								#用户平台ID
table_def+="identity_status tinyint unsigned not null default 0,"						#认证状态 [0:未认证 1:提交待审核 2:审核通过 3:审核失败]
table_def+="phone char(32) not null default '',"										#手机号
table_def+="identity_num char(32) not null default '',"									#身份证号
table_def+="identity_pic1_id bigint unsigned not null default 0,"						#认证图片1
table_def+="identity_pic2_id bigint unsigned not null default 0,"						#认证图片2
table_def+="identity_pic3_id bigint unsigned not null default 0,"						#认证图片3
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="check_ts bigint unsigned not null default 0,"								#审核时间
table_def+="check_mgr char(64) not null default '',"									#审核人
table_def+="primary key (user_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#收货地址表，单表
table_name="t_user_recv_addr_info"
table_def="("
table_def+="user_id bigint unsigned not null default 0,"								#用户平台ID
table_def+="addr_num bigint unsigned not null default 0,"								#地址编号
table_def+="recv_name char(32) not null default '',"									#收货人
table_def+="identity_num char(32) not null default '',"									#身份证ID
table_def+="phone char(32) not null default '',"										#手机号
table_def+="post_num char(32) not null default '',"										#邮编
table_def+="addr_province char(32) not null default '',"								#省
table_def+="addr_city char(32) not null default '',"									#城市
table_def+="addr_district char(32) not null default '',"								#区
table_def+="addr_detail char(255) not null default '',"									#详细地址
table_def+="primary key (user_id, addr_num)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#feed索引表，单表
table_name="t_feed_index_on_good_id"
table_def="("
table_def+="good_id bigint unsigned not null default 0,"								#商品ID
table_def+="feed_id bigint unsigned not null default 0,"								#期号feedid
table_def+="good_type bigint unsigned not null default 0,"								#商品类型 
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (good_id, feed_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#商品信息表，单表
table_name="t_good_info"
table_def="("
table_def+="good_id bigint unsigned not null default 0,"								#商品id 编号 
table_def+="good_id_text char(64) not null default '',"									#商品id 文字 
table_def+="good_type bigint unsigned not null default 0,"								#商品类型 
table_def+="title char(128) not null default '',"										#标题
table_def+="good_desc blob not null default '',"										#描述
table_def+="show_pics blob not null default '',"										#展示图片 [123|124|125|126...]
table_def+="detail_pic_id bigint unsigned not null default 0,"							#图片详情
table_def+="total_join_num bigint unsigned not null default 0,"							#总需参与次数
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (good_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#商品类型表，单表
table_name="t_good_type"
table_def="("
table_def+="good_type bigint unsigned not null default 0,"								#商品类型 编号 
table_def+="good_type_text char(64) not null default '',"								#商品类型 文字 
table_def+="good_type_icon char(64) not null default '',"								#商品类型 图标 
table_def+="extra_data blob not null default '',"										#扩展字段
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (good_type)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#消费类型表，单表
table_name="t_consume_type"
table_def="("
table_def+="consume_type bigint unsigned not null default 0,"							#消费类型 编号 
table_def+="consume_type_text char(64) not null default '',"							#消费类型 文字 
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (consume_type)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#Feed表，单表
table_name="t_feed_info"
table_def="("
table_def+="feed_id bigint unsigned not null default 0,"								#期号feedid
table_def+="good_id bigint unsigned not null default 0,"								#商品id
table_def+="total_join_num bigint unsigned not null default 0,"							#总需参与次数
table_def+="current_join_num bigint unsigned not null default 0,"						#当前参与次数
table_def+="approved_num bigint unsigned not null default 0,"							#审核通过次数
table_def+="pending_num bigint unsigned not null default 0,"							#已上传小票等待审核次数	
																						#[可上传次数 = 总需参与次数 - 审核通过次数 - 待审核次数]
																						#[当前参与次数 = 审核通过次数 + 审核失败次数 + 待审核次数]
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="feed_status tinyint unsigned not null default 0,"							#揭晓状态 [1:未揭晓，正在参与 2:等待揭晓 3:已经揭晓]
table_def+="finish_ts bigint unsigned not null default 0,"								#完成时间
table_def+="luck_contend_id bigint unsigned not null default 0,"                        #中奖的 夺宝ID
table_def+="luck_user_id bigint unsigned not null default 0,"                           #中奖的 用户平台ID
table_def+="luck_ts bigint unsigned not null default 0,"                                #开奖时间
table_def+="show_status tinyint unsigned not null default 0,"							#晒单状态 [0:未晒单 1:已经晒过单]
table_def+="recv_addr blob not null default '',"                                        #用户收货地址
table_def+="express_name char(64) not null default '',"                                 #快递名称
table_def+="express_num char(64) not null default 0,"                                   #快递单号
table_def+="extra_data blob not null default '',"										#扩展信息
table_def+="primary key (feed_id), index(good_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#小票信息表，单表
table_name="t_ticket_info"
table_def="("
table_def+="ticket_id bigint unsigned not null default 0,"								#小票id 
table_def+="ticket_pics blob not null default '',"										#小票图片id 
table_def+="user_id bigint unsigned not null default 0,"								#用户平台ID
table_def+="user_ip char(32) not null default '',"										#用户ip
table_def+="feed_id bigint unsigned not null default 0,"								#期号
table_def+="check_status tinyint unsigned not null default 0,"							#审核状态 [1:待审核 2:审核通过 3:审核失败]
table_def+="contend_id bigint unsigned not null default 0,"								#夺宝ID [审核后才生成，同一期号下的夺宝ID逐一递增]
table_def+="ticket_price bigint unsigned not null default 0,"							#小票金额
table_def+="consume_type tinyint unsigned not null default 0,"							#消费类型
table_def+="good_id bigint unsigned not null default 0,"								#商品id
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="check_ts bigint unsigned not null default 0,"								#审核时间
table_def+="check_mgr char(64) not null default '',"									#审核人
table_def+="extra_data blob not null default '',"										#扩展信息
table_def+="primary key (ticket_id), index(contend_id), index(feed_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#收藏表，单表
table_name="t_collect_info"
table_def="("
table_def+="user_id bigint unsigned not null default 0,"								#用户平台ID
table_def+="feed_id bigint unsigned not null default 0,"								#期号
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (user_id, feed_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"



#晒单表，单表
table_name="t_show_info"
table_def="("
table_def+="show_id bigint unsigned not null default 0,"								#晒单ID
table_def+="user_id bigint unsigned not null default 0,"								#用户平台ID
table_def+="feed_id bigint unsigned not null default 0,"								#期号
table_def+="title char(128) not null default '',"										#标题
table_def+="show_desc blob not null default '',"										#描述
table_def+="show_pics blob not null default '',"										#图片 [123|124|125|...]
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (show_id), index(user_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#banner表，单表
table_name="t_banner_info"
table_def="("
table_def+="banner_id bigint unsigned not null default 0,"								#banner id
table_def+="pic_id bigint unsigned not null default 0,"									#图片ID
table_def+="content_type tinyint unsigned not null default 0,"							#内容类型 [1:feedid 2:跳转链接]
table_def+="content char(255) not null default '',"										#根据类型来判断，可以是指定的Feedid,也可以存放跳转链接
table_def+="del_status tinyint unsigned not null default 0,"							#删除状态 [0:未删除 1:已删除]
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key(banner_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#普通图片表，单表
table_name="t_pic_info"
table_def="("
table_def+="pic_id bigint unsigned not null default 0,"									#图片ID
table_def+="content mediumblob not null default '',"									#图片二进制
table_def+="user_id bigint unsigned not null default 0,"								#上传者用户平台ID
table_def+="type bigint unsigned not null default 0,"									#图片类型 [0:不需鉴权；1:需要鉴权，如小票、身份证信息]
table_def+="primary key (pic_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


