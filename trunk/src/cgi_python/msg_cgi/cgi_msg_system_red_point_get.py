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

def InnerProcess():
	# 1.Get HTTP params	
	openid_md5 = int(mycgi.getEnvValue('openid_md5', 0))
	pa_appid_md5 = int(mycgi.getEnvValue('pa_appid_md5', 0))
	
	# 2.Params check
	if 0 == openid_md5 or 0 == pa_appid_md5:
		logger.error('INVALID PARAM: openid_md5=%d, pa_appid_md5=%d!', openid_md5, pa_appid_md5)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_INVALID_PARAM)		
		return True

	# 3.Build requestMsg protobuf
	stRequestMsg = _msg.Msg()

	stRequestMsgHeader = stRequestMsg.head
	stRequestMsgHeader.cmd = _msg.QUERY_SYSTEM_RED_POINT_REQ
	stRequestMsgHeader.seq = int(time.time())

	stRequestMsgBody = stRequestMsg.query_system_red_point_req
	stRequestMsgBody.openid_md5 = openid_md5
	stRequestMsgBody.pa_appid_md5 = pa_appid_md5

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

	if stResponseMsg.head.cmd != _msg.QUERY_SYSTEM_RED_POINT_RES:
		logger.error('response.head.cmd=%d, unkown, fuck!!!', stResponseMsg.head.cmd)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	if stResponseMsg.head.result != _msg.E_OK:
		logger.error('response.head.result=%d, not E_OK!', stResponseMsg.head.result)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	# 5.protobuf -> json, and do reply
	# init RED POINT LIST
	redinfo_list = {}
	for i in range(1, 6):
		redinfo_list[str(i)] = 0

	stMsgQueryRes = stResponseMsg.query_system_red_point_res
	for msg in stMsgQueryRes.red_point_info:
		redinfo_list[str(msg.type)] = msg.value
	
	mycgi.addBody('redinfo_list', redinfo_list)
	mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_OK)
	return True





if __name__ == '__main__':
	# __file__ 获取模块所在路径
	mycgi = _cgi.myCgi(__file__, 'logging.conf')

	# logger	
	logger = mycgi.getLogger()

	InnerProcess()