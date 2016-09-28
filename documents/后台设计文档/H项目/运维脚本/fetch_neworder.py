#!/usr/bin/python
# -*- coding: UTF-8 -*-

import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.header import Header

import MySQLdb
import time
import demjson
import random
import string

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

db_user="root"
db_password=".hoosho,9942"
db_main="main"
db_d_user="d_user"
table_name="open_order"
db_d_pa="d_pa"

order_type={
'1':'开通公众号',
'2':'开通公众号+公众号认证',
'3':'开通公众号+公众号认证+开通支付功能',
'4':'开通粉丝空间后台管理端',
'5':'接入游戏'
}

order_status={
'0':'待审核',
'1':'审核通过，待沟通',
'2':'开通中',
'3':'已开通，待发送通知邮件',
'4':'完成',
'400':'已取消'
}

dictOrderInfo={
'1':{},
'2':{},
'3':{},
'4':{},
'5':{}
}

def md5(_str):
	import hashlib
	import types
	if type(_str) is types.StringType:
		m = hashlib.md5()
		m.update(_str)
		md5_str = m.hexdigest()
		val = 0
		for i in range(8, 24):
			val <<= 4
			val += int(md5_str[i], 16)
		return val
	else:
		return ''

def insertPaInfo():
	dictFansZoneOrder = dictOrderInfo['4'];
	if len(dictFansZoneOrder) == 0:
		return

	db = MySQLdb.connect("localhost", db_user, db_password, db_d_pa, charset="utf8", unix_socket="/data/mysql/mysql.sock")

	# 使用cursor()方法获取游标操作
	cursor = db.cursor()	

	for key in dictFansZoneOrder:
		tmp_appid = dictFansZoneOrder[key]['appid']
		tmp_appid_md5 = dictFansZoneOrder[key]['appid_md5']
		tmp_pa_name = dictFansZoneOrder[key]['name']

		sql = 'INSERT INTO t_pa_info SET' \
		    + ' appid_md5=' + str(tmp_appid_md5) \
		    + ', appid=\"' + str(tmp_appid) + "\"" \
		    + ', name=\"' + str(tmp_pa_name) + "\"" \
		    + ', create_ts=unix_timestamp()'

		print sql
		
		try:
			# 使用execute方法执行SQL语句
			cursor.execute(sql)
			# 提交到数据库执行
			db.commit()

		# Rollback in case there is any error
		except MySQLdb.Warning, w:
			print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
		except MySQLdb.Error, e:
			print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])
			db.rollback()
	
	# 关闭数据库连接
	db.close()

def send_notice_mail(_msg_html):

	sender = 'hooshoServer@163.com'
	
	receivers = ['douyu@hoosho.com', 'xuebao@hoosho.com', 
				 'qingzhi@hoosho.com', 'wolong@hoosho.com', 
				 'junyi@hoosho.com', 'zijian@hoosho.com', 
				 'longmao@hoosho.com', 'mohu@hoosho.com']
	
	#receivers = ['douyu@hoosho.com']

	message = MIMEText(_msg_html, 'html', 'utf-8')
	message['From'] = sender
	message['To'] = ';'.join(receivers)

	subject = '新订单通知'
	message['Subject'] = Header(subject, 'utf-8')

	try:
		smtpObj = smtplib.SMTP('localhost')
		smtpObj.sendmail(sender, receivers, message.as_string())
		print 'send notice mail success'
	except smtplib.SMTPException, e:
		print str(e)
		print 'Error: send notice mail failed'

	smtpObj.close()

def check_open_order_table():
	
	# 打开数据库连接, unix_socket默认是/var/lib/mysql/mysql/sock, 这里要根据/etc/my.cnf里面的 "socket=" 路径指定
	# 如阿里云： db = MySQLdb.connect("localhost", db_user, db_password, db_name, charset="utf8", unix_socket="/data/mysql/mysql.sock")
	db = MySQLdb.connect("localhost", db_user, db_password, db_main, charset="utf8", unix_socket="/data/mysql/mysql.sock")

	# 使用cursor()方法获取游标操作
	cursor = db.cursor()

	sql = 'SELECT * FROM ' + table_name + ' WHERE status=0'
	print sql

	_msg_content = ''
	listCount = [0, 0, 0, 0, 0, 0]

	try:
		# 使用execute方法执行SQL语句
		cursor.execute(sql)
		result = cursor.fetchall()
		for i,r in enumerate(result):		
			
			m_type = r[2]
			dictOrderInfo[str(m_type)][listCount[m_type]] = {}			
			tmp_dict = dictOrderInfo[str(m_type)][listCount[m_type]]
			listCount[m_type] += 1  

			tmp_dict['order_id'] = r[0]
			tmp_dict['need_uin'] = r[1]
			tmp_dict['order_type'] = order_type[str(r[2])]

			extra_info = str(r[3])
			dictInfo = demjson.decode(extra_info)

			if m_type == 1:			#开通公众号
				tmp_dict['public_type'] = dictInfo['base_info']['public_type']
				tmp_dict['main_type'] = dictInfo['base_info']['main_type']
			
			elif m_type == 2:		#开通公众号+公众号认证
				pass

			elif m_type == 3:		#开通公众号+公众号认证+开通支付功能
				tmp_dict['ad_type'] = dictInfo['base_info']['ad_type']
				tmp_dict['ad_phone'] = dictInfo['base_info']['ad_phone']
				tmp_dict['ad_mail'] = dictInfo['base_info']['ad_mail']

			elif m_type == 4:		#开通粉丝空间后台管理端				
				tmp_dict['picId'] = dictInfo['picId']						#公众号LOGO
				tmp_dict['name'] = dictInfo['name']							#公众号名称
				tmp_dict['wx_account'] = dictInfo['wxh'] 					#微信号
				tmp_dict['appid'] = dictInfo['appid']						#公众号APP ID
				tmp_dict['appid_md5'] = str(md5(str(tmp_dict['appid'])))	#appid_md5
				tmp_dict['email'] = dictInfo['email']						#指引邮箱				

			elif m_type == 5:		#接入游戏						
				tmp_dict['name'] = dictInfo['name']					#游戏名称				
				tmp_dict['email'] = dictInfo['email']				#指引邮箱

			else:		
				pass		
				

			tmp_dict['order_status'] = order_status[str(r[4])]

			time_format = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(r[5]))
			tmp_dict['create_ts'] = time_format				

	# Rollback in case there is any error
	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])

	# 将订单状态置为 '开通中'
	
	update_sql = 'UPDATE ' + table_name + ' SET status=2 WHERE status=0'
	print update_sql

	try:
		# 使用execute方法执行SQL语句
		cursor.execute(update_sql)
		# 提交到数据库执行
		db.commit()

	# Rollback in case there is any error
	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])
		db.rollback()
	
	# 关闭数据库连接
	db.close()


def buildHtmlMsg():
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
	}
	table.gridtable th {
		border-width: 1px;
		padding: 8px;
		border-style: solid;
		border-color: #666666;
		background-color: #dedede;
	}
	table.gridtable td {
		border-width: 1px;
		padding: 8px;
		border-style: solid;
		border-color: #666666;
		background-color: #ffffff;
	}
	</style>

	<!-- Table goes in the document BODY -->
	"""

	tmp_index = 0
	for key in dictOrderInfo.keys():				
		tmp_dict = dictOrderInfo[key]
		if key == '1' and len(tmp_dict) > 0:
			'''
			_str_html += '    <tr> \n' \
				 + '        <th colspan="8">开通公众号 订单列表</th> \n' \
				 + '    </tr>'
			'''
			tmp_index += 1
			_str_html += '    <p>' + str(tmp_index) + '. 开通公众号 订单列表</p>'

			_str_html += """
			<table class="gridtable">
			<tr>
				<th>No.</th><th>订单ID</th><th>需求方平台账号</th><th>订单类型</th><th>公众号类型</th><th>客户类型</th><th>订单状态</th><th>创建时间</th>
			</tr>
			"""

			for index in tmp_dict.keys():				
				_str_item = '    <tr> \n' \
						 + '        <td>' + str(index+1) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_id']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['need_uin']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_type']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['public_type']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['main_type']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_status']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['create_ts']) + '</td> \n' \
						 + '    </tr> \n'
				_str_html += _str_item

			_str_html += '</table>'
		
		elif key == '2' and len(tmp_dict) > 0:
			'''
			_str_html += '    <tr> \n' \
				 + '        <th colspan="6">开通公众号+公众号认证 订单列表</th> \n' \
				 + '    </tr>'
			'''
			tmp_index += 1
			_str_html += '    <p><br />' + str(tmp_index) + '. 开通公众号+公众号认证 订单列表</p>'

			_str_html += """
			<table class="gridtable">
			<tr>
				<th>No.</th><th>订单ID</th><th>需求方平台账号</th><th>订单类型</th><th>订单状态</th><th>创建时间</th>
			</tr>
			"""

			for index in tmp_dict.keys():				
				_str_item = '    <tr> \n' \
						 + '        <td>' + str(index+1) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_id']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['need_uin']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_type']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_status']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['create_ts']) + '</td> \n' \
						 + '    </tr> \n'
				_str_html += _str_item

			_str_html += '</table>'

		elif key == '3' and len(tmp_dict) > 0:
			'''
			_str_html += '    <tr> \n' \
				 + '        <th colspan="9">开通公众号+公众号认证+开通支付功能 订单列表</th> \n' \
				 + '    </tr>'
			'''
			tmp_index += 1
			_str_html += '    <p><br />' + str(tmp_index) + '. 开通公众号+公众号认证+开通支付功能 订单列表</p>'

			_str_html += """
			<table class="gridtable">
			<tr>
				<th>No.</th><th>订单ID</th><th>需求方平台账号</th><th>订单类型</th><th>广告类型</th><th>联系电话</th><th>联系邮箱</th><th>订单状态</th><th>创建时间</th>
			</tr>
			"""

			for index in tmp_dict.keys():				
				_str_item = '    <tr> \n' \
						 + '        <td>' + str(index+1) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_id']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['need_uin']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_type']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['ad_type']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['ad_phone']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['ad_mail']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_status']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['create_ts']) + '</td> \n' \
						 + '    </tr> \n'	
				_str_html += _str_item		

			_str_html += '</table>'			 

		elif key == '4' and len(tmp_dict) > 0:
			'''
			_str_html += '    <tr> \n' \
				 + '        <th colspan="12">开通粉丝空间后台管理端 订单列表</th> \n' \
				 + '    </tr>'
			'''
			tmp_index += 1
			_str_html += '    <p><br />' + str(tmp_index) + '. 开通粉丝空间后台管理端 订单列表</p>'

			_str_html += """
			<table class="gridtable">
			<tr>
				<th>No.</th><th>订单ID</th><th>需求方平台账号</th><th>订单类型</th><th>公众号LOGO</th><th>显示名称</th><th>微信号</th><th>APPID</th><th>APPID_MD5</th><th>联系邮箱</th><th>订单状态</th><th>创建时间</th>
			</tr>
			"""

			for index in tmp_dict.keys():				
				_str_item = '    <tr> \n' \
						 + '        <td>' + str(index+1) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_id']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['need_uin']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_type']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['picId']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['name']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['wx_account']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['appid']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['appid_md5']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['email']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_status']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['create_ts']) + '</td> \n' \
						 + '    </tr> \n'
				_str_html += _str_item

			_str_html += '</table>'

		elif key == '5' and len(tmp_dict) > 0:
			'''
			_str_html += '    <tr> \n' \
				 + '        <th colspan="8">接入游戏 订单列表</th> \n' \
				 + '    </tr>'
			'''
			tmp_index += 1
			_str_html += '    <p><br />' + str(tmp_index) + '. 接入游戏 订单列表</p>'

			_str_html += """
			<table class="gridtable">
			<tr>
				<th>No.</th><th>订单ID</th><th>需求方平台账号</th><th>订单类型</th><th>游戏名称</th><th>联系邮箱</th><th>订单状态</th><th>创建时间</th>
			</tr>
			"""

			for index in tmp_dict.keys():				
				_str_item = '    <tr> \n' \
						 + '        <td>' + str(index+1) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_id']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['need_uin']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_type']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['name']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['email']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['order_status']) + '</td> \n' \
						 + ' 		<td>' + str(tmp_dict[index]['create_ts']) + '</td> \n' \
						 + '    </tr> \n'
				_str_html += _str_item

			_str_html += '</table>'
		
		else:
			pass
		
	# 结尾
	_str_html += """	
	</html>
	"""

	return _str_html

def handleFansZoneReq():
	dictFansZoneReq = dictOrderInfo['4']
	if len(dictFansZoneReq) == 0:
		return

	# 打开数据库连接, unix_socket默认是/var/lib/mysql/mysql/sock, 这里要根据/etc/my.cnf里面的 "socket=" 路径指定
	# 如阿里云： db = MySQLdb.connect("localhost", db_user, db_password, db_name, charset="utf8", unix_socket="/data/mysql/mysql.sock")
	db = MySQLdb.connect("localhost", db_user, db_password, db_d_user, charset="utf8", unix_socket="/data/mysql/mysql.sock")

	# 使用cursor()方法获取游标操作
	cursor = db.cursor()	

	for key in dictFansZoneReq:
		tmp_appid = dictFansZoneReq[key]['appid']
		tmp_email = dictFansZoneReq[key]['email']
		pa_appid_md5 = md5(str(tmp_appid))
		dictFansZoneReq[key]['appid_md5'] = pa_appid_md5
		dictFansZoneReq[key]['url'] = 'https://open.weixin.qq.com/connect/oauth2/authorize?appid=wxfd93b50f7122f71b' \
							   + '&redirect_uri=http%3A%2F%2Fwww.huxiao.com%2FnewHproject%2Fhproject%2F&response_type=code&scope=snsapi_userinfo' \
							   + '&state=' + str(pa_appid_md5) + '#wechat_redirect'

		rand_str = string.join(random.sample('1234567890qwertyuiopasdfghjklzxcvbnm', 4)).replace(' ','')
		manager_login_account = tmp_email
		manager_login_passwd = tmp_email + str(rand_str)
		dictFansZoneReq[key]['manager_login_account'] = manager_login_account
		dictFansZoneReq[key]['manager_login_passwd'] = manager_login_passwd

		rand_num = random.randint(1, 999999)
		manager_openid_md5 = 118000000 + rand_num

		manager_create_ts = int(time.time())
		
		sql = 'INSERT INTO t_manager SET' \
		    + ' uin=\"' + str(manager_login_account) + "\""\
		    + ', passwd=\"' + str(manager_login_passwd) + "\""\
		    + ', pa_appid_md5=' + str(pa_appid_md5) \
		    + ', openid_md5=' + str(manager_openid_md5) \
		    + ', create_ts=' + str(manager_create_ts) \
		    + ', admin=1'

		print sql
		
		try:
			# 使用execute方法执行SQL语句
			cursor.execute(sql)
			# 提交到数据库执行
			db.commit()

		# Rollback in case there is any error
		except MySQLdb.Warning, w:
			print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
		except MySQLdb.Error, e:
			print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])
			db.rollback()
		
	# 关闭数据库连接
	db.close()
	'''
	for key in dictFansZoneReq:
		print dictFansZoneReq[key]
	'''

def handleFansZoneMail():
	dictFansZoneReq = dictOrderInfo['4']
	if len(dictFansZoneReq) == 0:
		return
	
	for key in dictFansZoneReq:
		msg_html = ''
		receiver = dictFansZoneReq[key]['email']	
		# 生成指引邮件content
		msg_html = buildGuardHtmlMsg(dictFansZoneReq[key])
		# 发送指引邮件给客户
		send_guard_mail(receiver, msg_html)
		#send_guard_mail('douyu@hoosho.com', msg_html)

		# 发送邮件给卧龙
		send_guard_mail('wolong@hoosho.com', msg_html)


def buildGuardHtmlMsg(_dictFansZoneItem):
	# 开头
	_str_html = """
	<!DOCTYPE html>
	<html>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
	"""
	# Body
	_str_html += '<p>亲爱的用户，您好:</p><br />' \
			   + '<p>您于' + _dictFansZoneItem['create_ts'] + '提交了粉丝互动社区接入申请。</p><br />' \
			   + '<p>您的粉丝互动社区链接如下：</p>' \
	           + '<p>' + _dictFansZoneItem['url'] + '</p><br />' \
	           + '<p>您的粉丝互动社区管理后台账号如下：</p>' \
	           + '<p>账号：' + _dictFansZoneItem['manager_login_account'] + '</p>' \
	           + '<p>密码：' + _dictFansZoneItem['manager_login_passwd'] + '</p>' \
	           + '<p>请下载邮件附件中的“粉丝空间接入指引”完成接入</p><br /><br />' \
	           + '<p>								虎笑客服中心</p>' \
	           + '<p>							    ' + time.strftime('%Y年%m月%d日') +'</p><br /><br /><br /><br /><br />' \
	           + '<p>本邮件由系统自动发送，请勿直接回复！如有任何疑问，请联系我们的负责人。</p>' \
	           + '<p>联系方式：微信：ljynever99</p>'


	# 结尾
	_str_html += """	
	</html>
	"""

	return _str_html

def send_guard_mail(_receiver, _msg_html):
	#sender = 'fanszone@hoosho.com'
	sender = 'hooshoServer@163.com'	
	receiver = _receiver

	# 创建一个带附件的实例
	message = MIMEMultipart()
	message.attach(MIMEText(_msg_html, 'html', 'utf-8'))

	# 构造附件1，传送当前目录下的 粉丝空间接入指引.docx 文件
	att1 = MIMEText(open('/home/dev/hoosho/op_sh/粉丝空间接入指引.docx', 'rb').read(), 'base64', 'utf-8')
	att1["Content-Type"] = 'application/octet-stream'
	# 这里的filename可以任意写，写什么名字，邮件中显示什么名字
	att1["Content-Disposition"] = 'attachment; filename="粉丝空间接入指引.docx"'
	message.attach(att1)


	message['From'] = sender
	message['To'] = receiver

	subject = '【虎笑】粉丝空间开通通知'
	message['Subject'] = Header(subject, 'utf-8')

	try:
		smtpObj = smtplib.SMTP('localhost')
		smtpObj.sendmail(sender, receiver, message.as_string())
		print 'send guard mail success'
	except smtplib.SMTPException, e:
		print str(e)
		print 'Error: send guard mail failed'

	smtpObj.close()



if __name__ == '__main__':
	
	check_open_order_table()

	flag = False
	for key in dictOrderInfo.keys():
		if len(dictOrderInfo[key]) != 0:
			flag = True

	if flag is not True:
		print 'DB empty, sys exit without send mail!'
		sys.exit(0)

	# 生成新订单邮件content
	msg_html = buildHtmlMsg()	
	
	# 发送新订单通知 给公司内部成员
	send_notice_mail(msg_html)

	# 将粉丝空间的处理订单 公众号信息插入到 t_pa_info
	insertPaInfo()

	# 处理 开通粉丝空间后台管理端 订单，生成appid_md5 及 公众号跳转 URL
	#handleFansZoneReq()

	# 发送处理邮件
	#handleFansZoneMail()	
