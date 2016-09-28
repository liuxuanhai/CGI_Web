#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_def=""

##########################################################################################################
#######                               Z项目库 d_z           		                               #######
##########################################################################################################
db_name="d_z"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} created!!!"

#用户信息表，单表
table_name="t_user_info"
table_def="("
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="nickname tinyblob not null default '',"										#微信昵称
table_def+="sex tinyint unsigned not null default 0,"									#性别(1:男,2:女,0:未知)
table_def+="headimgurl blob not null default '',"										#微信头像url
table_def+="self_desc blob not null default '',"										#自我描述
table_def+="phone char(32) not null default '',"										#联系电话
table_def+="user_type tinyint unsigned not null default 1,"								#用户类型(0/1:普通用户，2：sb僵尸)
table_def+="user_flag bigint unsigned not null default 0,"								#标志位(64位，[0位:是否新用户]:0否 1是)
table_def+="user_score bigint unsigned not null default 0,"								#用户积分
table_def+="primary key (openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#用户现金表，单表
table_name="t_user_cash"
table_def="("
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="balance bigint unsigned not null default 0,"								#余额
table_def+="pending bigint unsigned not null default 0,"								#待处理的提现冻结资金
table_def+="primary key (openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#用户提现申请表，单表
table_name="t_outcome_req"
table_def="("
table_def+="table_id bigint unsigned not null default 0,"								#表id
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="amount bigint unsigned not null default 0,"									#单笔提现申请金额
table_def+="state bigint unsigned not null default 0,"									#处理状态(1:等待处理 2:审核成功,处理完成 3:审核失败,返回)
table_def+="create_ts bigint unsigned not null default 0,"								#提交时间
table_def+="primary key (table_id),"
table_def+="index(openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#用户现金流水表，单表
table_name="t_cash_flow"
table_def="("
table_def+="table_id bigint unsigned not null default 0,"								#表id
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="out_trade_no char(64) not null default '',"									#商户订单号
table_def+="amount bigint unsigned not null default 0,"									#交易额
table_def+="add_or_reduce tinyint unsigned not null default 0,"							#交易类型(1:收入, 2:支出)
table_def+="type tinyint unsigned not null default 0,"									#流水类型(1:充值, 2:下单支出, 3:打赏支出, 4:抢单收益, 5:订单取消退款, 6:用户打赏收入, 7:提现 8.系统打赏收入)
table_def+="balance bigint unsigned not null default 0,"								#每次产生现金流水后，都统计一次账户可提现余额
table_def+="create_ts bigint unsigned not null default 0,"								#交易时间
table_def+="primary key (table_id),"
table_def+="index(openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#用户评价表，单表
table_name="t_user_star"
table_def="("
table_def+="openid char(128) not null default '',"										#微信openid
table_def+="orderid bigint unsigned not null default 0,"								#订单号
table_def+="star tinyint unsigned not null default 0,"									#评价分数
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (openid, orderid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#用户信任表，单表
table_name="t_user_trust"
table_def="("
table_def+="openid_from char(128) not null default '',"									#被谁信任
table_def+="openid_to char(128) not null default '',"									#信任谁
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (openid_from, openid_to)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#平台订单表，单表
table_name="t_order_info"
table_def="("
table_def+="orderid bigint unsigned not null default 0,"								#订单id
table_def+="openid_master char(128) not null default '',"								#提交人
table_def+="openid_slave char(128) not null default '',"								#确认抢单人
table_def+="order_visible tinyint unsigned not null default 0,"							#订单可见性(1:仅信任的人可见 2:所有人可见)
table_def+="order_status tinyint unsigned not null default 0,"							#订单状态(1:等待抢单 2:已被抢 3:已完成 4:已取消)
table_def+="pay_status tinyint unsigned not null default 0,"							#支付状态(1:未支付 2:已支付)
table_def+="addr_to_longitude bigint unsigned not null default 0,"						#目的位置经度
table_def+="addr_to_latitude bigint unsigned not null default 0,"						#目的位置纬度
table_def+="addr_to char(128) not null default '',"										#目的地址
table_def+="addr_to_detail char(255) not null default '',"								#目的详细地址
table_def+="order_desc blob not null default '',"										#订单描述
table_def+="order_receiver_name char(32) not null default '',"							#收货人
table_def+="order_receiver_phone char(32) not null default '',"							#收货人电话
table_def+="price bigint unsigned not null default 0,"									#订单价钱(单位: 分)
table_def+="expect_from_ts bigint unsigned not null default 0,"							#期望送达起始时间戳
table_def+="expect_to_ts bigint unsigned not null default 0,"							#期望送达结束时间戳
table_def+="pay_ts bigint unsigned not null default 0,"									#支付时间戳
table_def+="create_ts bigint unsigned not null default 0,"								#订单创建时间
table_def+="finish_ts bigint unsigned not null default 0,"								#订单完成时间
table_def+="fentch_ts bigint unsigned not null default 0,"								#订单被sb抢了时间
table_def+="cancel_ts bigint unsigned not null default 0,"								#订单取消时间
table_def+="star_from_master bigint unsigned not null default 0,"						#下单人 对 抢单人 的评价星级
table_def+="star_from_slave bigint unsigned not null default 0,"						#抢单人 对 下单人 的评价星级
table_def+="openid_waiting blob not null default '',"									#候选抢单人(openid|createts,...)
table_def+="extra_data blob not null default '',"										#订单其他信息
table_def+="order_type tinyint unsigned not null default 1,"							#订单类型(0/1:普通订单，2:小票，其他：待分配)
table_def+="media_type tinyint unsigned not null default 1,"							#订单媒体类型(0/1:普通文本，2：语音，3：图片)
table_def+="primary key (orderid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#平台订单媒体表，存放单独的二进制内容，（音频， 视频等）
table_name="t_order_media_info"
table_def="("
table_def+="order_id bigint unsigned not null default 0,"								#订单id
table_def+="media_type bigint unsigned not null default 0,"								#媒体类型
table_def+="media_content mediumblob not null default '',"								#媒体内容
table_def+="primary key (order_id, media_type)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#用户抢单状态表，存放用户抢单的状态
table_name="t_order_fetch_state"
table_def="("
table_def+="order_id bigint unsigned not null default 0,"								#订单id
table_def+="fetch_openid char(128) not null default '',"								#抢单人
table_def+="fetch_state tinyint unsigned not null default 1,"							#抢单状态(1:等待下单者确认；2：被确认)
table_def+="fentch_ts bigint unsigned not null default 0,"								#抢单时间
table_def+="accpet_ts bigint unsigned not null default 0,"								#被接受时间
table_def+="primary key (order_id, fetch_openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#订单打赏表
table_name="t_order_reward"
table_def="("
table_def+="out_trade_no char(64) not null default '',"									#微信下单的商户订单号
table_def+="orderid bigint unsigned not null default 0,"								#订单id
table_def+="openid_from char(128) not null default '',"									#打赏人
table_def+="openid_to char(128) not null default '',"									#被打赏人
table_def+="reward_ts bigint unsigned not null default 0,"								#打赏时间戳
table_def+="amount bigint unsigned not null default 0,"									#打赏金额
table_def+="pay_status tinyint unsigned not null default 0,"							#支付状态(1:未支付 2:已支付)
table_def+="pay_ts bigint unsigned not null default 0,"									#支付时间戳
table_def+="primary key(out_trade_no),"
table_def+="index(orderid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#微信统一下单表，单表
table_name="t_wx_outtrade"
table_def="("
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="out_trade_no char(64) not null default '',"									#商户订单号,  对应OrderInfo的OrderId
table_def+="prepay_id char(128) not null default '',"									#预支付交易会话标示
table_def+="body blob not null default '',"												#商品描述
table_def+="detail blob not null default '',"											#商品详情
table_def+="total_fee bigint unsigned not null default 0,"								#总金额
table_def+="attach char(128) not null default '',"										#附加数据
table_def+="fee_type char(16) not null default '',"										#货币类型
table_def+="spbill_create_ip char(32) not null default '',"								#终端IP
table_def+="time_start char(32) not null default '',"									#交易起始时间
table_def+="time_expire char(32) not null default '',"									#交易结束时间
table_def+="goods_tag char(64) not null default '',"									#商品标记
table_def+="trade_type char(16) not null default '',"									#交易类型
table_def+="product_id char(64) not null default '',"									#商品ID
table_def+="limit_pay char(64) not null default '',"									#指定支付方式
table_def+="appid char(64) not null default '',"										#公众号appid
table_def+="mch_id char(64) not null default '',"										#商户号
table_def+="device_info char(64) not null default '',"									#设备号
table_def+="pay_state tinyint unsigned not null default 0,"								#支付状态(0:等待支付 1:成功 2:失败)
table_def+="pay_reason tinyint unsigned not null default 0,"							#支付原因(1:下单 2:打赏)
table_def+="primary key (openid, out_trade_no)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#七夕活动页面分享统计表，单表
table_name="t_qixi_share_page_statistics"
table_def="("
table_def+="create_ts bigint unsigned not null default 0,"								#统计日期
table_def+="share_time bigint unsigned not null default 0,"								#七夕活动页面通知总数
table_def+="primary key (create_ts)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#点赞表
table_name="t_order_favor"
table_def="("
table_def+="orderid bigint unsigned not null default 0,"								#订单号
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="create_ts bigint unsigned not null default 0,"								#点赞时间
table_def+="primary key(orderid, openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#跟帖表
#注意：跟帖分两种：评论和回复，type字段注明两种的区别
table_name="t_order_follow"
table_def="("
table_def+="follow_id bigint unsigned not null default 0,"                          	#跟帖id
table_def+="content blob not null default '',"                          				#跟帖内容
table_def+="orderid bigint unsigned not null default 0,"                          		#订单号
table_def+="type tinyint unsigned not null default 0,"                          		#类型(1:评论， 2：回复)
table_def+="origin_comment_id bigint unsigned not null default 0,"						#如果类型为2， 这个字段表示回复哪条评论
table_def+="openid_from char(128) not null default '',"                        			#主动者openid
table_def+="openid_to char(128) not null default '',"                        			#被动者openid
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="del_ts bigint unsigned not null default 0,"									#删除时间
table_def+="primary key (follow_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#评论 
#注意，此表只保存评论和order的关系，回复和评论的关系不需要插入到此index表
table_name="t_comment_index_on_orderid"
table_def="("
table_def+="orderid bigint unsigned not null default 0,"                          		#订单号
table_def+="comment_id bigint unsigned not null default 0,"                          	#评论id
table_def+="primary key (orderid, comment_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#回复 
#注意，此表只保存回复和原始评论的关系，feed和评论的关系不需要插入到此index表
table_name="t_reply_index_on_origin_comment"
table_def="("
table_def+="origin_comment_id bigint unsigned not null default 0,"                      #评论id
table_def+="reply_id bigint unsigned not null default 0,"                          		#回复id
table_def+="primary key (origin_comment_id, reply_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


############################################################### MSG & NOTICE #####################################################################

#消息内容表
table_name="t_msg_content"
table_def="("
table_def+="msg_id bigint unsigned not null default 0,"                          		#消息Id
table_def+="content blob not null default '',"									 		#消息内容
table_def+="openid_from char(128) not null default '',"									#谁发
table_def+="openid_to char(128) not null default '',"									#发给谁
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (msg_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created"

#会话-消息 表
table_name="t_session_msg"
table_def="("
table_def+="session_id bigint unsigned not null default 0,"								#会话Id
table_def+="msg_id bigint unsigned not null default 0,"                          		#消息Id
table_def+="del_status tinyint unsigned not null default 0,"							#删除状态，第0位表示openid小者删除，第1位表示大者
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (session_id, msg_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created"

#用户-会话 表，两个用户只有唯一的一张表
table_name="t_user_session"
table_def="("
table_def+="session_id bigint unsigned not null default 0,"                         	#会话Id
table_def+="openid_a char(128) not null default '',"									#用户A openid (A < B)
table_def+="openid_b char(128) not null default '',"									#用户B openid
table_def+="content blob not null default '',"											#最新的一条消息内容
table_def+="newmsg_status tinyint unsigned not null default 0,"							#是否有新的消息, 第0位表示openid_a，第1位openid_b
table_def+="del_status tinyint unsigned not null default 0,"							#删除状态，第0位表示openid_a，第1位openid_b
table_def+="create_ts bigint unsigned not null default 0,"								#最后更新时间
table_def+="primary key (session_id)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created"

#红点 表
table_name="t_red_point"
table_def="("
table_def+="openid char(128) not null default '',"										#用户openid
table_def+="type bigint unsigned not null default 0,"									#红点类型 [1:点赞 2:评论 3:私信 4:信任]
table_def+="value bigint unsigned not null default 0,"									#新增数量
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (openid, type)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created"

#用户通知 表
table_name="t_user_notice"
table_def="("
table_def+="table_id bigint unsigned not null default 0,"								#表id
table_def+="openid char(128) not null default '',"                             			#用户openid 
table_def+="type tinyint unsigned not null default 0,"                           		#个人通知类型 [1.点赞 2.评论]
table_def+="status tinyint unsigned not null default 0,"                           		#状态0:未读，1：已读
table_def+="extra_data_0 blob not null default '',"                             		#扩展字段0 [type=1/2 表示被点赞或评论的order_id]
table_def+="extra_data_1 blob not null default '',"                             		#扩展字段1 [type=1 表示点赞用户openid, type=2 表示origin_comment_id]
table_def+="extra_data_2 blob not null default '',"                             		#扩展字段2 [type=2 表示follow_id]
table_def+="create_ts bigint unsigned not null default 0,"                              #创建时间
table_def+="primary key (table_id),"
table_def+="index(openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created"


############################################################### 中秋活动 #####################################################################

#彩票池
table_name="t_lottery_pool"
table_def="("
table_def+="lottery_num char(128) not null default '',"									#彩票号码
table_def+="date char(128) not null default '',"										#发布日期
table_def+="status tinyint unsigned not null default 0,"								#赠送状态[0:未赠送 1:已经赠送]
table_def+="openid char(128) not null default '',"										#获得此彩票的openid
table_def+="create_ts bigint unsigned not null default 0,"								#赠送具体时间
table_def+="primary key (lottery_num), index(date)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=ut8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created"

#抽奖总数
table_name="t_lottery_count"
table_def="("
table_def+="date char(128) not null default '',"										#日期
table_def+="prize tinyint unsigned not null default 0,"							        #奖品等级
table_def+="count bigint unsigned not null default 0,"									#总数
table_def+="primary key (date, prize)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created"


#用户抽奖记录
table_name="t_user_lottery"
table_def="("
table_def+="table_id bigint unsigned not null default 0,"								#表id
table_def+="openid char(128) not null default '',"										#用户openid
table_def+="prize tinyint unsigned not null default 0,"									#获奖奖品 [0.未抽中 1.奖品a ...]
table_def+="prize_desc blob not null default '',"										#奖品描述 [如彩票号码]
table_def+="create_ts bigint unsigned not null default 0,"								#抽奖时间
table_def+="primary key (table_id), index(openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created"


############################################################### 各种活动相关 #####################################################################

#用户扩展信息
table_name="t_user_extra_info"
table_def="("
table_def+="openid char(128) not null default '',"										#用户openid
table_def+="available_lottery_time tinyint unsigned not null default 0,"				#可用抽奖次数 [中秋活动]
table_def+="primary key (openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created"


#用户活动信息 
table_name="t_user_activity_info"
table_def="("
table_def+="openid char(128) not null default '',"										#用户openid
table_def+="info_id char(128) not null default '', "									#信息id
table_def+="activity_type int unsigned not null default 0,"								#活动类型
table_def+="activity_info blob not null default ''"										#活动信息
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created"


