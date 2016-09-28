#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import MySQLdb
import time
import argparse

import smtplib
from email.mime.text import MIMEText
from email.header import Header

from pylsy import pylsytable

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

MONTH_FORMAT='%Y%m'
DAY_FORMAT='%Y%m%d'
s_MONTH_FORMAT='%Y/%m'
s_DAY_FORMAT='%Y/%m/%d'
TIME_FORMAT='%Y/%m/%d %H:%M:%S'

def init_dictPaItem(_id):
	dictItem = {}
	dictItem['id'] = _id
	dictItem['uv_statistic'] = 0

	return dictItem

def init_dictPaInfo():
	# 1.connect DB
    db = MySQLdb.connect('localhost', 'root', '.hoosho,9942', 'd_pa', charset='utf8', unix_socket='/data/mysql/mysql.sock')
    # 2.cursor
    cursor = db.cursor()
 
    sql = 'SELECT appid_md5, name FROM t_pa_info'
    print sql

    dictPaInfo = {}
    try:
        # execute SQL
        cursor.execute(sql)
        result = cursor.fetchall()
 
        for r in result:
            appid_md5 = int(r[0])
            name = str(r[1])
 
            dictPaInfo[name] = init_dictPaItem(appid_md5)
 
    except MySQLdb.Warning, w:
    	print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
    except MySQLdb.Error, e:
    	print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])

    #print dictPaInfo
 
    db.close()
    return dictPaInfo

def send_notice_mail_by_proxy(_msg_content, subject):
	smtpServer = 'smtp.163.com'
	username = 'hooshoServer@163.com'
	password = 'hoosho9942'

	sender = 'hooshoServer@163.com'
	'''
	receivers = ['douyu@hoosho.com', 'xuebao@hoosho.com', 'qingzhi@hoosho.com', 'wolong@hoosho.com', 
				 'zijian@hoosho.com', 'longmao@hoosho.com']
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
				 'zijian@hoosho.com', 'longmao@hoosho.com']
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


def getArgs():
	parse = argparse.ArgumentParser()
	parse.add_argument('--enable_send_mail', type=bool, default=False)	
	parse.add_argument('--unix_timestamp', type=int)
	parse.add_argument('--statistic_mode', type=str, default='byDay')
	args=parse.parse_args()
	return vars(args)

def FetchUVStatisticInfo(_str_month, _time_begin, _time_end, _statistic_mode, dictPaInfo):	
	# 1.connect DB
	db = MySQLdb.connect('localhost', 'root', '.hoosho,9942', 'd_user', charset='utf8', unix_socket='/data/mysql/mysql.sock')
	#db = MySQLdb.connect('localhost', 'root', '.hoosho,9942', 'd_user', charset='utf8')
	# 2.cursor
	cursor = db.cursor()

	# 3.SQL query
	table_name = 't_pa_uv_' + _str_month 
		
	for pa_name in dictPaInfo.keys():
		if _statistic_mode == 'byMonth':
			sql = 'SELECT COUNT(*) FROM ' + table_name + ' WHERE pa_appid_md5=' + str(dictPaInfo[pa_name]['id'])
		else:
			sql = 'SELECT COUNT(*) FROM ' + table_name + ' WHERE pa_appid_md5=' + str(dictPaInfo[pa_name]['id']) + ' AND create_ts>=' + str(_time_begin) + ' AND create_ts<' + str(_time_end)
		#print sql
			
		try:
			# execute SQL
			cursor.execute(sql)
			result = cursor.fetchall()
		
			for r in result:
				dictPaInfo[pa_name]['uv_statistic'] += r[0]				

		except MySQLdb.Warning, w:
			print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
		except MySQLdb.Error, e:
			print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])
		
	# 4.close db connect
	db.close()	

def FetchFeedsStatisticInfo(_time_begin, _time_end, dictPaInfo):
	# 1.connect DB
	db = MySQLdb.connect('localhost', 'root', '.hoosho,9942', 'd_feeds', charset='utf8', unix_socket='/data/mysql/mysql.sock')
	#db = MySQLdb.connect('localhost', 'root', '.hoosho,9942', 'd_feeds', charset='utf8')
	# 2.cursor
	cursor = db.cursor()

	# 3.SQL query
	for pa_name in dictPaInfo.keys():
		real_feeds_total = 0
		#zombie_feeds_total = 0
		for i in range(256):			
			table_name = 't_feed_' + "%02x" % i
			real_sql = 'SELECT COUNT(*) FROM ' + table_name \
				  + ' WHERE pa_appid_md5=' + str(dictPaInfo[pa_name]['id']) \
				  + ' AND openid_md5>100000 AND create_ts>=' + str(_time_begin) \
				  + ' AND create_ts<' + str(_time_end) \
				  + ' AND feed_type & 5 = feed_type'

			#print real_sql

			try:
				# execute SQL
				cursor.execute(real_sql)
				result = cursor.fetchall()
			
				for r in result:
					real_feeds_total += r[0]								

			except MySQLdb.Warning, w:
				print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
			except MySQLdb.Error, e:
				print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])

			'''
			zombie_sql = 'SELECT COUNT(*) FROM ' + table_name \
				  + ' WHERE pa_appid_md5=' + str(dictPaInfo[pa_name]['id']) \
				  + ' AND openid_md5<100000 AND create_ts>' + str(_time_begin) \
				  + ' AND create_ts<' + str(_time_end) \
				  + ' AND feed_type & 5 = feed_type'

			#print zombie_sql

			try:
				# execute SQL
				cursor.execute(zombie_sql)
				result = cursor.fetchall()
			
				for r in result:
					zombie_feeds_total += r[0]								

			except MySQLdb.Warning, w:
				print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
			except MySQLdb.Error, e:
				print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])
			'''

		dictPaInfo[pa_name]['real_feeds_statistic'] = real_feeds_total
		#dictPaInfo[pa_name]['zombie_feeds_statistic'] = zombie_feeds_total

	# 4.close db connect
	db.close()	

def buildHtmlMsg(_table_title, dictPaInfo):
	'''
	attributes = ['公众号', 'UV统计', 'Feeds统计']
	record_table = pylsytable(attributes)

	for pa_name in dictPaInfo.keys():
		record_table.append_data(attributes[0].decode('utf-8'), pa_name)
		record_table.append_data(attributes[1].decode('utf-8'), dictPaInfo[pa_name]['uv_statistic'])
		record_table.append_data(attributes[2].decode('utf-8'), dictPaInfo[pa_name]['feeds_statistic'])

	return record_table.__str__()	
	'''

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
	<table class="gridtable">
	"""

	_str_html += '    <tr> \n' \
				 + '        <th colspan="3">' + _table_title + '</th> \n' \
				 + '    </tr>'


	_str_html += """
	<tr>
		<th>公众号</th><th>UV统计</th><th>真实Feeds</th>
	</tr>
	"""

	for pa_name in dictPaInfo.keys():
		_str_item = '    <tr> \n' \
					 + '        <td>' + pa_name + '</td> \n' \
					 + ' 		<td>' + str(dictPaInfo[pa_name]['uv_statistic']) + '</td> \n' \
					 + ' 		<td>' + str(dictPaInfo[pa_name]['real_feeds_statistic']) + '</td> \n' \
					 + '    </tr> \n'
		_str_html += _str_item



	# 结尾
	_str_html += """
	</table>

	</html>
	"""

	return _str_html

if __name__ == '__main__':

	dayList = []

	args=getArgs()
	enable_send_mail=args['enable_send_mail']	#bool
	unix_timestamp=args['unix_timestamp']		#int
	statistic_mode=args['statistic_mode']		#str

	if unix_timestamp is not None:
		cur_time = unix_timestamp
	else:
		cur_time = int(time.time())
	
	one = 24*60*60	
	cur_date = time.strftime(DAY_FORMAT, time.gmtime(cur_time))
	t_curBeijingZero = cur_time/one*one - 8*60*60
	t_end = t_curBeijingZero + one
	if statistic_mode == 'byDay':
		str_day = time.strftime(DAY_FORMAT, time.gmtime(cur_time))
		dayList.append(str_day)
		t_begin = t_curBeijingZero		
		table_title = '[日统计]统计日期: ' + time.strftime(s_DAY_FORMAT, time.gmtime(cur_time))
		mail_subject = '[Daily]PA_UV_STATISTIC_' + cur_date
	elif statistic_mode == 'byWeek':
		for i in range(7):
			str_day = time.strftime(DAY_FORMAT, time.gmtime(cur_time - i*one))
			dayList.append(str_day)
		t_begin = t_curBeijingZero - 6*one	
		table_title = '[周统计]统计日期: ' \
					+ time.strftime(s_DAY_FORMAT, time.gmtime(t_begin + 8*60*60)) \
					+ ' - ' \
					+ time.strftime(s_DAY_FORMAT, time.gmtime(t_end + 8*60*60 - 1))	
		mail_subject = '[Weekly]PA_UV_STATISTIC_' + cur_date
	elif statistic_mode == 'byMonth':
		str_day = time.strftime(DAY_FORMAT, time.gmtime(cur_time))
		dayList.append(str_day)
		t_begin = t_curBeijingZero - 30*one
		table_title = '[月统计]统计月份: ' + time.strftime(s_MONTH_FORMAT, time.gmtime(cur_time))
		mail_subject = '[Monthly]PA_UV_STATISTIC_' + cur_date
	else:
		print 'Error: --statistic_mode only support args: [byDay | byWeek | byMonth] '
		sys.exit(0)

	

	print 'dayList: ', dayList
	print 't_begin: %s, t_end: %s' % (time.strftime(TIME_FORMAT, time.gmtime(t_begin + 8*60*60)), time.strftime(TIME_FORMAT, time.gmtime(t_end + 8*60*60)))

	dictPaInfo = init_dictPaInfo()

	if statistic_mode == 'byWeek':
		str_month_1 = dayList[0][:6]
		str_month_2 = dayList[6][:6]
		if str_month_1 == str_month_2:
			FetchUVStatisticInfo(str_month_1, t_begin, t_end, statistic_mode, dictPaInfo)
		else:
			FetchUVStatisticInfo(str_month_1, t_begin, t_end, statistic_mode, dictPaInfo)
			FetchUVStatisticInfo(str_month_2, t_begin, t_end, statistic_mode, dictPaInfo)
	else:
		str_day = dayList[0]
		str_month = str_day[:6]	#取前6位
		FetchUVStatisticInfo(str_month, t_begin, t_end, statistic_mode, dictPaInfo)

	FetchFeedsStatisticInfo(t_begin, t_end, dictPaInfo)

	msg_html = buildHtmlMsg(table_title, dictPaInfo)

	# open file
	file_name = '/tmp/' + mail_subject + '.html'
	pfile = open(file_name, 'w')
	pfile.write(msg_html)
	pfile.close()

	# 5.send mail
	if enable_send_mail:		
		#send_notice_mail(msg_html, mail_subject)
		send_notice_mail_by_proxy(msg_html, mail_subject)		
