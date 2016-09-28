#!/usr/bin/python
# -*- coding: UTF-8 -*-

class CHttpHeader:

	def __init__(self):
		self.m_sContentType = 'text/html'
		self.m_iStatusCode = 200
		self.m_sStatusMsg = ''
		self.m_listCookies = []
		self.m_listHeaders = []

	def addHeader(self, sHead):
		self.m_listHeaders.append(sHead)
	
	def output(self):
		if self.m_iStatusCode != 200:
			print 'HTTP/1.1 %d %s\c\r\n' % (m_iStatusCode, m_sStatusMsg)
		for h in self.m_listHeaders:
			print '%s\r\n' % h
		
		print 'Content-Type: %s\r\n' % self.m_sContentType
		# print 自带多一个\r\n，所以下面这行不需要输出
		#print '\r\n'


