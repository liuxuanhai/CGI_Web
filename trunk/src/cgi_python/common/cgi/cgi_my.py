#!/usr/bin/python
# -*- coding: UTF-8 -*-

import cgi, cgitb
from cgi_http_header import CHttpHeader
import demjson

import logging
import logging.config

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

class myCgi:

	def __init__(self, cgi_name, loggingConf):
		logging.config.fileConfig(loggingConf)
		self.logger = logging.getLogger('main')

		self.m_tHeader = CHttpHeader()
		self.m_tBody = {}	
		# 创建 FieldStorage 的实例化
		self.m_form = cgi.FieldStorage()
		self.m_cgi_name = cgi_name		
		self.logger.debug('BEGIN CGI --------------------- %s -----------------------', self.m_cgi_name)

	def getLogger(self):
		return self.logger

	def getEnvValue(self, key, deValue):
		self.logger.debug('%s: %s', key, self.m_form.getvalue(key))
		if self.m_form.getvalue(key) is not None:
			return self.m_form.getvalue(key)
		else:
			return deValue
		
	def addBody(self, key, value):
		self.m_tBody[key] = value

	def bodyOutput(self):
		# 将字典数据类型转为json
		json = demjson.encode(self.m_tBody)
		# decode('unicode-escape'), 将反斜杠u类型 '\uXXXX'的字符串转换为对应的unicode字符，即中文显示
		#print json
		print json.decode('unicode-escape')
		

	def DoReply(self, iErrCode):		
		self.m_tHeader.output()
		
		self.addBody('ec', iErrCode)
		self.bodyOutput()
		self.logger.debug('END   CGI --------------------- %s -----------------------\n', self.m_cgi_name)
