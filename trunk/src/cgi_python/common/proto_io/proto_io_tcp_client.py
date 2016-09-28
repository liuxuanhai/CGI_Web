#!/usr/bin/python
# -*- coding: UTF-8 -*-

import socket
import DatagramStringBuffer as _dsb
import common.proto.msg_pb2 as _msg

TIMEOUT = 5 #5s 

class ProtoIOTcpClient:

	def __init__(self, serverIP, serverPort):
		self.m_server_ip = serverIP
		self.m_server_port = serverPort
		self.m_timeout = TIMEOUT 
		self.sock = 0

	def tcp_connect(self):
		# 创建TCP socket: SOCK_STREAM , UDP: SOCK_DGRAM
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		# 设置socket的超时时间，单位为秒
		self.sock.settimeout(self.m_timeout)
		# 连接, 如果连接出错，返回socket.error错误
		if self.sock.connect((self.m_server_ip, self.m_server_port)) == socket.error:
			return -1	

		return 0

	def tcp_send(self, bytedata):
		# 发送数据send / sendall
		# sendall : 尝试返回之前发送所有数据，成功返回None，失败抛出异常
		if not self.sock.sendall(bytedata):
			return 0	#success

		return -1

	def tcp_recv(self, dictByte):
		# 接受数据
		dictByte['responseByte'] = self.sock.recv(1024*1024)
		if not dictByte['responseByte']:
			return -1

		# 进行数据校验
		datagramfactory = _dsb.DatagramStringBuffer()
		if not datagramfactory.frame(dictByte['responseByte']):
			return -1

		return 0

	def tcp_io(self, dictByte):
		if self.tcp_connect() < 0:
			dictByte['strErrMsg'] = 'tcp_connect fail'
			return -1

		if self.tcp_send(dictByte['requestByte']) < 0:
			dictByte['strErrMsg'] = 'tcp_send fail'
			return -1

		if self.tcp_recv(dictByte) < 0:
			dictByte['strErrMsg'] = 'tcp_recv fail'
			return -1

		return 0


	def io(self, dictEelement):
		#在数据工厂里面对protobuf格式数据，进行如下操作
		datagramfactory = _dsb.DatagramStringBuffer()
		# 1.序列化
		datagramfactory.strbuffer = dictEelement['requestMsg'].SerializeToString()

		if not datagramfactory.strbuffer:
			dictEelement['strErrMsg'] = 'requestMsg.SerializeToString failed'
			return -1
		# 2.编码(转网络字节序) bytebuf是class ByteBuffer对象，包含一个list
		bytebuf = datagramfactory.encode()

		# 3.list转string
		requestByte = ''.join(bytebuf.bytelist)

		# 4.字典类型，用于传参
		dictByte = {'requestByte':requestByte, 'responseByte':None, 'strErrMsg':None}

		ret = self.tcp_io(dictByte)
		if ret != 0:
			dictEelement['strErrMsg'] = dictByte['strErrMsg']
			return -1		
		
		# 5.解码
		protoStrResponse = datagramfactory.decode(dictByte['responseByte'])

		stResponseMsg = _msg.Msg()
		# 6.反序列化 string -> protobuf
		stResponseMsg.ParseFromString(protoStrResponse)

		dictEelement['responseMsg'] = stResponseMsg
		return 0

