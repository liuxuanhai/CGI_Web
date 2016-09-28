#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import time

# 包含上级目录，即../common
sys.path.append('..')

# cgi, proto_io
from common.cgi.cgi_ret_code_def import CGI_RET_CODE
import common.cgi.cgi_my as _cgi
import common.proto_io.proto_io_tcp_client as _tcp_client

# proto
import common.proto.msg_pb2 as _msg
import common.proto.msg_sixin_pb2 as _msg_sixin
import common.proto.comm_struct_pb2 as _comm_struct
import common.proto.comm_enum_pb2 as _comm_enum

# util
from common.util.common_util import *

msgServerIP = '127.0.0.1'

def InnerProcess():
	# 1.Get HTTP params
	#'''
	strMsgContent = mycgi.getEnvValue('content', '')
	openid_md5_from = int(mycgi.getEnvValue('openid_md5_from', 0))
	openid_md5_to = int(mycgi.getEnvValue('openid_md5_to', 0))

	# for test
	'''
	strMsgContent = '你好！world'
	openid_md5_from = 123
	openid_md5_to = 456
	'''

	# 2.Params check
	if strMsgContent == '' or openid_md5_from == 0 or openid_md5_to == 0:
		logger.error('INVALID PARAM!')
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_INVALID_PARAM)		
		return True

	# 3.Build requestMsg protobuf
	stRequestMsg = _msg.Msg()

	stRequestMsgHeader = stRequestMsg.head
	stRequestMsgHeader.cmd = _msg.ADD_MSG_REQ
	stRequestMsgHeader.seq = int(time.time())

	stRequestMsgBody = stRequestMsg.msg_add_req
	msgcontent = stRequestMsgBody.msg_content
	msgcontent.content = strMsgContent.decode('utf-8')
	msgcontent.openid_md5_from = openid_md5_from
	msgcontent.openid_md5_to = openid_md5_to

	logger.debug('requestMsg: \n %s', stRequestMsg)

	# 4.Send to server, and recv responseMsg protobuf	
	ioclient = _tcp_client.ProtoIOTcpClient(msgServerIP, msgServerPort)

	dictMsg = {'requestMsg':stRequestMsg, 'responseMsg':None, 'strErrMsg':None}

	ret = ioclient.io(dictMsg)

	if ret != 0:	
		logger.error('ProtoIOTcpClient IO failed, errmsg = %s', dictMsg['strErrMsg'])
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	stResponseMsg = dictMsg['responseMsg']
	logger.debug('responseMsg: \n %s', stResponseMsg)

	if stResponseMsg.head.cmd != _msg.ADD_MSG_RES:
		logger.error('response.head.cmd=%d, unkown, fuck!!!', stResponseMsg.head.cmd)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	if stResponseMsg.head.result != _msg.E_OK:
		logger.error('response.head.result=%d, not E_OK!', stResponseMsg.head.result)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	# 5.protobuf -> json, and do reply
	msg_id = stResponseMsg.msg_add_res.msg_id
	create_ts = stResponseMsg.msg_add_res.create_ts

	mycgi.addBody('msg_id', msg_id)
	mycgi.addBody('create_ts', create_ts)

	mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_OK)
	return True





if __name__ == '__main__':
	# __file__ 获取模块所在路径
	mycgi = _cgi.myCgi(__file__, 'logging.conf')

	# logger	
	logger = mycgi.getLogger()

	InnerProcess()	