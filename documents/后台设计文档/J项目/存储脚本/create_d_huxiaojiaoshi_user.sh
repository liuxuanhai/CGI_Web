#!/bin/sh

db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_prefix=""
table_def=""

##########################################################################################################
#######                               J项目库 d_huxiaojiaoshi		                               #######
##########################################################################################################
db_name="d_huxiaojiaoshi"
echo "create database if not exists $db_name" | mysql -u${db_user} -p${db_password}
echo "DB ${db_name} created!!!"

#用户信息表，单表
table_name="t_user_info"
table_def="("
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="subscribe bigint unsigned not null default 0,"								#是否关注了公众号(0:没有关注, 1:已经关注)
table_def+="subscribe_time bigint unsigned not null default 0,"							#关注时间戳
table_def+="nickname tinyblob not null default '',"										#微信昵称
table_def+="sex tinyint unsigned not null default 0,"									#性别(1:男,2:女,0:未知)
table_def+="country char(128) not null default '',"										#国家
table_def+="province char(128) not null default '',"									#省份
table_def+="city char(128) not null default '',"										#城市
table_def+="headimgurl blob not null default '',"										#微信头像url
table_def+="self_desc blob not null default '',"										#自我描述
table_def+="primary key (openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#用户现金表，单表
table_name="t_user_cash"
table_def="("
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="total_income bigint unsigned not null default 0,"							#总收入
table_def+="total_outcome bigint unsigned not null default 0,"							#总提现
table_def+="total_outcome_pending bigint unsigned not null default 0,"					#待处理的冻结资金
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
table_def+="out_trade_openid char(128) not null default '',"								#支付商户订单的openid
table_def+="amount bigint unsigned not null default 0,"									#交易额
table_def+="type bigint unsigned not null default 0,"									#交易类型(1:进, 2:出)
table_def+="reason bigint unsigned not null default 0,"									#交易明细(1:回答, 2:偷听, 3:偷听分成 4:提现)
table_def+="balance bigint unsigned not null default 0,"								#每次产生现金流水后，都统计一次账户可提现余额
table_def+="create_ts bigint not null default 0,"										#交易时间
table_def+="primary key (table_id),"
table_def+="index(openid)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#统一下单表，单表
table_name="t_unifiedorder"
table_def="("
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="out_trade_no char(64) not null default '',"									#商户订单号
table_def+="prepay_id char(128) not null default '',"									#预支付交易会话标示
table_def+="body char(128) not null default '',"										#商品描述
table_def+="detail blob not null default '',"											#商品详情
table_def+="total_fee bigint unsigned not null default 0,"								#总金额
table_def+="nonce_str char(64) not null default '',"									#随机字符串
table_def+="sign char(64) not null default '',"											#签名
table_def+="attach char(128) not null default '',"										#附加数据
table_def+="fee_type char(16) not null default '',"										#货币类型
table_def+="spbill_create_ip char(32) not null default '',"								#终端IP
table_def+="time_start char(32) not null default '',"									#交易起始时间
table_def+="time_expire char(32) not null default '',"									#交易结束时间
table_def+="goods_tag char(64) not null default '',"									#商品标记
table_def+="notify_url blob not null default '',"										#通知地址
table_def+="trade_type char(16) not null default '',"									#交易类型
table_def+="product_id char(64) not null default '',"									#商品ID
table_def+="limit_pay char(64) not null default '',"									#指定支付方式
table_def+="appid char(64) not null default '',"										#公众号appid
table_def+="mch_id char(64) not null default '',"										#商户号
table_def+="device_info char(64) not null default '',"									#设备号
table_def+="pay_success tinyint unsigned not null default 0,"							#支付状态(1:成功 2:失败)
table_def+="wx_result_code char(32) not null default '',"								#支付状态返回码
table_def+="wx_err_code char(64) not null default '',"									#支付错误代码
table_def+="wx_err_code_des char(255) not null default '',"								#支付错误代码描述
table_def+="business_type tinyint unsigned not null default 0,"							#支付业务类型(1:回答 2:偷听)
table_def+="business_id bigint unsigned not null default 0,"							#支付业务id(回答:followid 偷听:listen)
table_def+="create_ts bigint unsigned not null default 0,"								#创建时间
table_def+="primary key (openid, out_trade_no)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"


#企业付款(提现)请求表，单表
table_name="t_transferorder_req"
table_def="("
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="partner_trade_no char(64) not null default '',"								#商户订单号
table_def+="appid char(64) not null default '',"										#公众号appid
table_def+="mch_id char(64) not null default '',"										#商户号
table_def+="device_info char(64) not null default '',"									#设备号
table_def+="nonce_str char(64) not null default '',"									#随机字符串
table_def+="sign char(64) not null default '',"											#签名
table_def+="check_name char(16) not null default '',"									#校验用户姓名选项
table_def+="re_user_name char(32) not null default '',"									#收款用户姓名
table_def+="amount bigint unsigned not null default 0,"									#总金额
table_def+="des blob not null default '',"												#企业付款描述信息
table_def+="spbill_create_ip char(32) not null default '',"								#终端IP
table_def+="primary key (openid, partner_trade_no)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#企业付款(提现)应答表，单表
table_name="t_transferorder_rsp"
table_def="("
table_def+="openid char(128) not null default '',"									 	#微信openid
table_def+="partner_trade_no char(64) not null default '',"								#商户订单号
table_def+="payment_no char(64) not null default '',"									#微信订单号
table_def+="return_code char(16) not null default '',"									#返回状态码
table_def+="return_msg char(128) not null default '',"									#返回信息
table_def+="appid char(64) not null default '',"										#公众号appid
table_def+="mch_id char(64) not null default '',"										#商户号
table_def+="device_info char(64) not null default '',"									#设备号
table_def+="nonce_str char(64) not null default '',"									#随机字符串
table_def+="result_code char(16) not null default '',"									#业务结果
table_def+="err_code char(32) not null default '',"										#错误代码
table_def+="err_code_des char(128) not null default '',"								#错误代码描述
table_def+="payment_time char(64) not null default '',"									#微信支付成功时间
table_def+="primary key (openid, partner_trade_no)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"	


#给用户发微信红包(提现)请求表，单表
table_name="t_sendredpack_req"
table_def="("
table_def+="re_openid char(128) not null default '',"									#微信openid
table_def+="mch_billno char(64) not null default '',"									#商户订单号
table_def+="wxappid char(64) not null default '',"										#公众号appid
table_def+="mch_id char(64) not null default '',"										#商户号
table_def+="nonce_str char(64) not null default '',"									#随机字符串
table_def+="sign char(64) not null default '',"											#签名
table_def+="send_name char(64) not null default '',"									#商户名称
table_def+="total_amount bigint unsigned not null default 0,"							#付款金额
table_def+="total_num bigint unsigned not null default 0,"								#红包发放总人数
table_def+="wishing blob not null default '',"										#红包祝福语
table_def+="client_ip char(32) not null default '',"									#终端IP
table_def+="act_name char(64) not null default '',"										#活动名称
table_def+="remark blob not null default '',"											#备注
table_def+="primary key (re_openid, mch_billno)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"

#给用户发微信红包(提现)应答表，单表
table_name="t_sendredpack_rsp"
table_def="("
table_def+="re_openid char(128) not null default '',"									#微信openid
table_def+="mch_billno char(64) not null default '',"									#商户订单号
table_def+="send_listid char(64) not null default '',"									#微信订单号
table_def+="return_code char(16) not null default '',"									#返回状态码
table_def+="return_msg char(128) not null default '',"									#返回信息
table_def+="wxappid char(64) not null default '',"										#公众号appid
table_def+="mch_id char(64) not null default '',"										#商户号
table_def+="sign char(64) not null default '',"											#签名
table_def+="result_code char(16) not null default '',"									#业务结果
table_def+="err_code char(32) not null default '',"										#错误代码
table_def+="err_code_des char(128) not null default '',"								#错误代码描述
table_def+="total_amount bigint unsigned not null default 0,"							#付款金额
table_def+="send_time char(64) not null default '',"									#发放成功时间
table_def+="primary key (re_openid, mch_billno)"
table_def+=")ENGINE=InnoDB DEFAULT CHARSET=utf8"
echo "create table if not exists $table_name $table_def" | mysql -uroot -p${db_password} $db_name
echo "TABLE $table_name created!"
