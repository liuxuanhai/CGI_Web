#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import time

# 包含上级目录，即../common
sys.path.append('..')

import os
os.environ['PYTHON_EGG_CACHE'] = '/tmp/.python-eggs'

# cgi, proto_io
from common.cgi.cgi_ret_code_def import CGI_RET_CODE
import common.cgi.cgi_my as _cgi
#import common.proto_io.proto_io_tcp_client as _tcp_client

# proto
#import common.proto.msg_pb2 as _msg
#import common.proto.msg_sixin_pb2 as _msg_sixin
#import common.proto.comm_struct_pb2 as _comm_struct
#import common.proto.comm_enum_pb2 as _comm_enum

# Mysql
import MySQLdb

# util
from common.util.common_util import *

def getZombieListByUin(_uin):
	zombies = []
	# 1.connect DB
	db = MySQLdb.connect('localhost', 'root', '.hoosho,9942', 'd_user', charset="utf8", unix_socket="/data/mysql/mysql.sock")

	# 2.cursor
	cursor = db.cursor()

	# 3.SQL query
	sql = 'SELECT openid_md5 FROM t_user_zombie WHERE uin="' + _uin + '"'
	logger.debug('SQL: %s', sql)

	try:
		# excute SQL
		cursor.execute(sql)
		result = cursor.fetchall()

		for r in result:
			zombies.append(r[0])
			#logger.debug('%s', r[0])
	except MySQLdb.Warning, w:
		logger.debug('Mysql Warning: %d:%s', w.args[0], w.args[1])
	except MySQLdb.Error, e:
		logger.error('Mysql Error: %d:%s', e.args[0], e.args[1])
	
	# 4.close db connect
	db.close()

	return zombies

def getNickNameByOpenidMd5(_cursor, _openid_md5):
	table_name_prefix = 't_user_info_'
	index = _openid_md5 % 256
	str_index = '%02x' % index
	table_name = table_name_prefix + str_index.replace('0x', '')
	sql = 'SELECT nick_wx FROM ' + table_name + ' WHERE openid_md5=' + str(_openid_md5)

	nick_name = ''
	try:
		_cursor.execute(sql)
		result = _cursor.fetchall()
		for r in result:
			nick_name = r[0]
	except MySQLdb.Warning, w:
		logger.debug('Mysql Warning: %d:%s', w.args[0], w.args[1])
	except MySQLdb.Error, e:
		logger.error('Mysql Error: %d:%s', e.args[0], e.args[1])

	return nick_name	

def getFeedsnum(_cursor, _uin_zombies, _dictId2Num):
	table_name_prefix = 't_feed_index_on_appid_'
	for i in range(256):
		str_index = '%02x' % i
		table_name = table_name_prefix + str_index.replace('0x', '')
		sql = 'SELECT pa_appid_md5, openid_md5 FROM ' + table_name + \
			  ' WHERE openid_md5>=' + str(_uin_zombies[0]) + \
			  ' AND openid_md5<=' + str(_uin_zombies[-1])

		try:
			_cursor.execute(sql)
			result = _cursor.fetchall()
			for r in result:
				_dictId2Num[str(r[1])] += 1
		except MySQLdb.Warning, w:
			logger.debug('Mysql Warning: %d:%s', w.args[0], w.args[1])
		except MySQLdb.Error, e:
			logger.error('Mysql Error: %d:%s', e.args[0], e.args[1])

def InnerProcess():
	# 1.Get HTTP params
	uin = mycgi.getEnvValue('uin', '')
	#uin = '军医'

	# 2.Params check
	if uin is None:
		logger.error('INVALID PARAM!')
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_INVALID_PARAM)		
		return True

	
	uin_zombies = getZombieListByUin(uin)
	# 3.connect DB
	db_user = MySQLdb.connect('localhost', 'root', '.hoosho,9942', 'd_user', charset="utf8", unix_socket="/data/mysql/mysql.sock")
	db_feed = MySQLdb.connect('localhost', 'root', '.hoosho,9942', 'd_feeds', charset="utf8", unix_socket="/data/mysql/mysql.sock")

	# 使用cursor()方法获取操作游标 
	cursor_feed = db_feed.cursor()
	cursor_user = db_user.cursor()	

	dictId2Num = {}
	for openid_md5 in uin_zombies:
		dictId2Num[str(openid_md5)] = 0

	getFeedsnum(cursor_feed, uin_zombies, dictId2Num)
	
	dictResult = {}
	for openid_md5 in uin_zombies:
		nick_name = getNickNameByOpenidMd5(cursor_user, openid_md5)		
		#print nick_name		
		dictResult[nick_name.decode('utf-8')] = dictId2Num[str(openid_md5)]

	# 关闭数据库连接
	db_user.close()
	db_feed.close()

	#print dictResult
	mycgi.addBody('result', dictResult)
	mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_OK)
	return True


if __name__ == '__main__':
	# __file__ 获取模块所在路径
	mycgi = _cgi.myCgi(__file__, 'logging.conf')

	# logger	
	logger = mycgi.getLogger()

	InnerProcess()	