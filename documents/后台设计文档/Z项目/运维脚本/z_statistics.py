#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import MySQLdb
import time
import argparse

import smtplib
from email.mime.text import MIMEText
from email.header import Header

import commands

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

DAY_FORMAT_1='%Y%m%d'
DAY_FORMAT_2='%m/%d'				#为了和/tmp/*.log 日历文件中的格式统一
TIME_FORMAT='%Y/%m/%d %H:%M:%S'

total_user = 0						#累计用户总数

# 所有
dictOrder = {
	'total_create': 0,				#点击下单
	'total_payed': 0,				#实际支付订单
	'total_payed_fee': 0,			#支付金额
	'total_fetch': 0				#总抢单
}

# 七夕
dictOrderQixi = {
	'total_payed': 0,				#七夕实际支付订单
	'total_fetch': 0,				#七夕应租抢单
	'total_share': 0,				#七夕活动页分享总数
	'total_view': 0					#七夕活动分享页浏览总数
}


def get_cur_date_yyyymmdd():
	cur_time = int(time.time())	
	cur_date =  time.strftime(DAY_FORMAT_1, time.gmtime(cur_time))

	return cur_date

def get_cur_beijing_date_mmdd():
	cur_time = int(time.time())	
	cur_beijing_date =  time.strftime(DAY_FORMAT_2, time.gmtime(cur_time + 8 * 60 * 60))

	return cur_beijing_date

def get_beijing_endline_ts_240000():
	cur_time = int(time.time())

	one_day = 24 * 60 * 60
	#格林威治时间比北京时间慢8小时
	#因为此脚本在北京时间 02:00:00 执行，相对于格林威治时间为前一天的 18:00:00
	#最终得到beijing_endline_ts 为前一天北京时间 24:00:00
	beijing_endline_ts = cur_time / one_day * one_day + 16 * 60 * 60

	return beijing_endline_ts

def send_notice_mail_by_proxy(_msg_content, subject):
	smtpServer = 'smtp.163.com'
	username = 'hooshoServer@163.com'
	password = 'hoosho9942'

	sender = 'hooshoServer@163.com'
	'''
	receivers = ['douyu@hoosho.com', 'xuebao@hoosho.com', 'qingzhi@hoosho.com', 'wolong@hoosho.com', 
				 'zijian@hoosho.com', 'longmao@hoosho.com', 'mohu@hoosho.com', 'bailing@hoosho.com', 
				 'zhaocaimao@hoosho.com', 'haimao@hoosho.com', 'feiyu@hoosho.com', 'shouma@hoosho.com',
				 'hanlu@hoosho.com']
	'''
	receivers = ['douyu@hoosho.com']

	message = MIMEText(_msg_content, 'html', 'utf-8')
	message['From'] = sender
	message['To'] = ';'.join(receivers)

	message['Subject'] = Header(subject, 'utf-8')

	try:
		smtp = smtplib.SMTP()  
		smtp.connect(smtpServer)  
		smtp.login(username, password)  
		smtp.sendmail(sender, receivers, message.as_string())  
		print 'send mail success'
	except smtplib.SMTPException, e:
		print str(e)
		print 'Error: send mail failed'
	
	smtp.quit()  

def send_notice_mail(_msg_content, subject):

	sender = 'hooshoServer@163.com'
	'''
	receivers = ['douyu@hoosho.com', 'xuebao@hoosho.com', 'qingzhi@hoosho.com', 'wolong@hoosho.com', 
				 'zijian@hoosho.com', 'longmao@hoosho.com', 'mohu@hoosho.com', 'bailing@hoosho.com', 
				 'zhaocaimao@hoosho.com', 'haimao@hoosho.com', 'feiyu@hoosho.com', 'shouma@hoosho.com',
				 'hanlu@hoosho.com']
	'''
	receivers = ['douyu@hoosho.com']

	message = MIMEText(_msg_content, 'html', 'utf-8')
	message['From'] = sender
	message['To'] = ';'.join(receivers)

	#subject = 'PA_UV_STATISTIC_' + str_day
	message['Subject'] = Header(subject, 'utf-8')

	try:
		smtpObj = smtplib.SMTP('localhost')
		smtpObj.sendmail(sender, receivers, message.as_string())	
		print 'send mail success'	
	except smtplib.SMTPException, e:
		print str(e)
		print 'Error: send mail failed'

	smtpObj.close()


def buildHtmlMsg(_endline_date):	

	# 开头
	_str_html = """
	<!DOCTYPE html>
	<html>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">

	<!-- CSS goes in the document HEAD or added to your external stylesheet -->
	<style type="text/css">
	table.gridtable {
		font-family: verdana,arial,sans-serif;
		font-size:11px;
		color:#333333;
		border-width: 1px;
		border-color: #666666;
		border-collapse: collapse;
		text-align: center;
	}
	table.gridtable th {
		border-width: 1px;
		padding: 8px;
		border-style: solid;
		border-color: #666666;
		background-color: #dedede;
		text-align: center;
	}
	table.gridtable td {
		border-width: 1px;
		padding: 8px;
		border-style: solid;
		border-color: #666666;
		background-color: #ffffff;
		text-align: center;
	}
	</style>

	<!-- Table goes in the document BODY -->
	"""

	# 表1
	_str_html += '<p>公众号统计</p>'

	_str_html += """
	<table class="gridtable">
	<tr>
		<th>用户总人数UV(七夕页面+公众号)</th><th>点击下单</th><th>实际下单</th><th>支付总额</th><th>抢单总数</th>
	</tr>
	"""

	_str_item = '<tr> \n' \
				+ '        <td>' + str(total_user) + '</td> \n' \
				+ '        <td>' + str(dictOrder['total_create']) + '</td> \n' \
				+ '        <td>' + str(dictOrder['total_payed']) + '</td> \n' \
				+ '        <td>' + str(dictOrder['total_payed_fee']) + '</td> \n' \
				+ '        <td>' + str(dictOrder['total_fetch']) + '</td> \n' \
				+ '    </tr> \n'

	_str_html += _str_item	

	_str_html += '    </table> \n'


	# 表2
	_str_html += '    <p>七夕主题统计</p>'

	_str_html += """
	<table class="gridtable">
	<tr>
		<th>浏览量PV</th><th>订单总数</th><th>应租总数</th><th>分享总数</th>
	</tr>
	"""

	# 加\n是为了打印日志好看一些 ^_^
	_str_item = '<tr> \n' \
				+ '        <td>' + str(dictOrderQixi['total_view']) + '</td> \n' \
				+ '        <td>' + str(dictOrderQixi['total_payed']) + '</td> \n' \
				+ '        <td>' + str(dictOrderQixi['total_fetch']) + '</td> \n' \
				+ '        <td>' + str(dictOrderQixi['total_share']) + '</td> \n' \
				+ '    </tr> \n'

	_str_html += _str_item	

	_str_html += '    </table> \n'

	# 说明
	_str_html += '    <p>统计截止时间：' + _endline_date + '</p>'
	_str_html += '    <p>说明：以上数据统计都是累计总数，不是按天统计</p>'	

	# 结尾
	_str_html += """
	</html>
	"""

	return _str_html

def FetchTotalUser(_cursor):	
	# SQL query
	sql = 'SELECT COUNT(*) FROM t_user_info'
	print sql

	_total_user = 0
	try:
		# 4.execute SQL
		_cursor.execute(sql)
		result = _cursor.fetchall()

		for r in result:
			_total_user = r[0]

	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])

	return _total_user


def FetchTotalOrder(_cursor, _beijing_endline_ts, _dictOrder):
	# SQL query
	# order_status 订单状态(0:默认状态即未完成支付 1:等待抢单 2:已被抢 3:已完成 4:已取消)

	# 1.点击下单
	sql = 'SELECT COUNT(*) FROM t_order_info where create_ts < ' + str(_beijing_endline_ts)
	print sql
	
	try:
		# 4.execute SQL
		_cursor.execute(sql)
		result = _cursor.fetchall()

		for r in result:
			_dictOrder['total_create'] = r[0]

	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])

	# 2.实际下单
	sql = 'SELECT COUNT(*) FROM t_order_info where order_status > 0 and create_ts < ' + str(_beijing_endline_ts)
	print sql
	
	try:
		# 4.execute SQL
		_cursor.execute(sql)
		result = _cursor.fetchall()

		for r in result:
			_dictOrder['total_payed'] = r[0]

	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])

	# 3.支付总额
	sql = 'SELECT SUM(price) FROM t_order_info where order_status > 0 and create_ts < ' + str(_beijing_endline_ts)
	print sql
	
	try:
		# 4.execute SQL
		_cursor.execute(sql)
		result = _cursor.fetchall()

		for r in result:
			_dictOrder['total_payed_fee'] = float(r[0]) / 100    #RMB: 分->元

	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])

	# 4.抢单总数
	sql = 'SELECT COUNT(*) FROM t_order_info where order_status > 1 and order_status < 4 and create_ts < ' + str(_beijing_endline_ts)
	print sql
	
	try:
		# 4.execute SQL
		_cursor.execute(sql)
		result = _cursor.fetchall()

		for r in result:
			_dictOrder['total_fetch'] = r[0]

	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])
	

def FetchTotalOrderQixi(_cursor, _beijing_endline_ts, _dictOrderQixi):
	# SQL query
	# order_status 订单状态(0:默认状态即未完成支付 1:等待抢单 2:已被抢 3:已完成 4:已取消)	

	# 1.七夕实际支付订单
	sql = 'SELECT COUNT(*) FROM t_order_info where order_desc like \'%七夕专属订单%\' and order_status > 0 and create_ts < ' + str(_beijing_endline_ts)
	print sql
	
	try:
		# 4.execute SQL
		_cursor.execute(sql)
		result = _cursor.fetchall()

		for r in result:
			_dictOrderQixi['total_payed'] = r[0]

	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])

	# 2.七夕应租抢单
	sql = 'SELECT COUNT(*) FROM t_order_info where order_desc like \'%七夕专属订单%\' and order_status > 1 and order_status < 4 and create_ts < ' + str(_beijing_endline_ts)
	print sql
	
	try:
		# 4.execute SQL
		_cursor.execute(sql)
		result = _cursor.fetchall()

		for r in result:
			_dictOrderQixi['total_fetch'] = r[0]

	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])

	# 3.七夕活动页分享总数
	sql = 'SELECT SUM(share_time) FROM t_qixi_share_page_statistics'
	print sql
	
	try:
		# 4.execute SQL
		_cursor.execute(sql)
		result = _cursor.fetchall()

		for r in result:
			_dictOrderQixi['total_share'] = r[0]

	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])

	# 4.七夕活动分享页浏览总数
	endline_date = get_cur_beijing_date_mmdd() + '/16'  	#08/10/16 即2016年8月10号
	sed_date = endline_date.replace("/", "\/")				#08/10/16 -> 08\/10\/16
	# sed命令，统计log文件从第一行开始到打印第一个"08/10/16"字符串，这个范围内的一个符合条件的关键字的总数，主要要用双引号
	# sed -n "1,/08\/10\/16/p" /tmp/cgi_z_order.log | grep order_detail_get | grep BEGIN | wc -l
	sed_str = "sed -n \"1,/" + sed_date + "/p\"" + " /tmp/cgi_z_order.log | grep order_detail_get | grep BEGIN | wc -l"
	print sed_str
	(status, _dictOrderQixi['total_view']) = commands.getstatusoutput(sed_str)
	

if __name__ == '__main__':
	# 1.connect DB
	db = MySQLdb.connect('localhost', 'root', '.hoosho,9942', 'd_z', charset='utf8', unix_socket='/data/mysql/mysql.sock')
	
	# 2.cursor
	cursor = db.cursor()

	# 3.获取当前日期yyyymmdd，以及统计截止时间
	cur_date = get_cur_date_yyyymmdd()
	beijing_endline_ts = get_beijing_endline_ts_240000()
	show_date = time.strftime(TIME_FORMAT, time.gmtime(beijing_endline_ts + 8 * 60 * 60))

	# 总用户量
	total_user = FetchTotalUser(cursor)

	# 总订单
	FetchTotalOrder(cursor, beijing_endline_ts, dictOrder)

	# 七夕订单
	FetchTotalOrderQixi(cursor, beijing_endline_ts, dictOrderQixi)	

	# 生成html邮件内容
	msg_html = buildHtmlMsg(show_date)
	mail_subject = 'ZIGA_数据统计_' + cur_date

	# 不用代理发送邮件
	#send_notice_mail(msg_html, mail_subject)

	# 邮箱代理
	send_notice_mail_by_proxy(msg_html, mail_subject)

	# 将html邮件内容保存到本地 /tmp/
	file_name = '/tmp/' + mail_subject + '.html'
	pfile = open(file_name, 'w')
	pfile.write(msg_html)
	pfile.close()

	# 4.close db connect
	db.close()	