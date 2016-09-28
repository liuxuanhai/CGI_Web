#!/usr/bin/python
# -*- coding: UTF-8 -*-

import struct 

class ByteBuffer:
	
	def __init__(self):
		self.bytelist = []

class DatagramStringBuffer:
	
	def __init__(self):
		# magic为唯一标识码
		self.magic = 0x1f24ff88
		self.length = 0
		self.strbuffer = ''

		self.POS_MAGIC = 0
		self.POS_LENGTH = 4
		self.POS_STRING_BUFFER = 8

	def encode(self):
		self.length = 12+len(self.strbuffer)
		bytebuf = ByteBuffer()
		# uint32_t 主机字节序 -> 网络字节序
		bytebuf.bytelist.append(struct.pack('!I',self.magic))
		bytebuf.bytelist.append(struct.pack('!I',self.length))
		bytebuf.bytelist.append(struct.pack('!I',len(self.strbuffer)))

		bytebuf.bytelist.append(self.strbuffer)

		return bytebuf

	def frame(self, strbuffer):
		bytelist = list(strbuffer)

		# 注意，unpack返回的是tuple!!
		tmp_magic, = struct.unpack('!I', ''.join(bytelist[0:4]))
		# print 'tmp_magic: %d, self.magic: %d' % (tmp_magic, self.magic)
		if tmp_magic == self.magic:
			tmp_length, = struct.unpack('!I', ''.join(bytelist[4:8]))
			# print 'tmp_length: %s' % tmp_length
			if tmp_length == len(strbuffer):
				return True
			else:
				return True
		else:
			return False

	def decode(self, strbuffer):
		# string -> list
		bytelist = list(strbuffer)

		# 取list中的数据段，转为string
		str_data = ''.join(bytelist[12:])
		return str_data

