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

# msgServerIP = '127.0.0.1'

# 一次拉取消息数量限制
getMsgAmountLimit = 20
# 时间间隔 8*60*60 = 8 hours
interval = 28800

def FetchUserInfo(listOpenidMd5):
	# 1.Build requestMsg protobuf
	stUserInfoRequestMsg = _msg.Msg()

	stUserInfoRequestMsgHead = stUserInfoRequestMsg.head
	stUserInfoRequestMsgHead.cmd = _msg.QUERY_USER_DETAIL_INFO_REQ
	stUserInfoRequestMsgHead.seq = int(time.time())

	stQueryUserDetailInfoReq = stUserInfoRequestMsg.query_user_detail_info_req
	for openidMd5 in listOpenidMd5:
		stQueryUserDetailInfoReq.openid_md5_list.append(openidMd5)

	# 2.Send to server, and recv responseMsg protobuf
	ioclient = _tcp_client.ProtoIOTcpClient(userServerIP, userServerPort)
	dictMsg = {'requestMsg':stUserInfoRequestMsg, 'responseMsg':None, 'strErrMsg':None}

	ret = ioclient.io(dictMsg)

	if ret != 0:	
		logger.error('FetchFeedsInfo IO failed, errmsg = %s', dictMsg['strErrMsg'])
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return -1

	stUserInfoResponseMsg = dictMsg['responseMsg']
	logger.debug('responseMsg: \n %s', stUserInfoResponseMsg)

	if stUserInfoResponseMsg.head.cmd != _msg.QUERY_USER_DETAIL_INFO_RES:
		logger.error('FetchUserInfo response.head.cmd=%d, unkown, fuck!!!', stUserInfoResponseMsg.head.cmd)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return -1

	if stUserInfoResponseMsg.head.result != _msg.E_OK:
		logger.error('FetchUserInfo response.head.result=%d, not E_OK!', stUserInfoResponseMsg.head.result)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return -1

	# 3.Parse	
	res = stUserInfoResponseMsg.query_user_detail_info_res
	for i in range(len(res.user_detail_info_list)):
		if listOpenidMd5[0] == res.user_detail_info_list[i].openid_md5:
			stHostUserInfo = UserInfoPB2Dict(res.user_detail_info_list[i])
			mycgi.addBody('userinfo_from', stHostUserInfo)
			break
		elif i == len(res.user_detail_info_list) - 1:
			return -1

	for i in range(len(res.user_detail_info_list)):
		if listOpenidMd5[1] == res.user_detail_info_list[i].openid_md5:
			stGuestUserInfo = UserInfoPB2Dict(res.user_detail_info_list[i])
			mycgi.addBody('userinfo_to', stGuestUserInfo)
			break
		elif i == len(res.user_detail_info_list) - 1:
			return -1	

	return 0

def InnerProcess():
	# 1.Get HTTP params	
	msg_id = int(mycgi.getEnvValue('msg_id', 0))
	amount = int(mycgi.getEnvValue('amount', 0))
	openid_md5_from = int(mycgi.getEnvValue('openid_md5_from', 0))
	openid_md5_to = int(mycgi.getEnvValue('openid_md5_to', 0))
	
	# 2.Params check
	if 0 == openid_md5_from or 0 == openid_md5_to:
		logger.error('INVALID OPENID MD5! openid_md5_from = %d, openid_md5_to = %d', openid_md5_from, openid_md5_to)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_INVALID_PARAM)		
		return True
	if amount > getMsgAmountLimit:
		logger.error('amount: %d > getMsgAmountLimit: %d', amount, getMsgAmountLimit)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_INVALID_PARAM)		
		return True	

	# 3.Build requestMsg protobuf
	stRequestMsg = _msg.Msg()

	stRequestMsgHeader = stRequestMsg.head
	stRequestMsgHeader.cmd = _msg.QUERY_MSG_REQ
	stRequestMsgHeader.seq = int(time.time())

	stRequestMsgBody = stRequestMsg.msg_query_req
	stRequestMsgBody.msg_id = msg_id
	stRequestMsgBody.amount = amount
	stRequestMsgBody.openid_md5_from = openid_md5_from
	stRequestMsgBody.openid_md5_to = openid_md5_to	

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

	if stResponseMsg.head.cmd != _msg.QUERY_MSG_RES:
		logger.error('response.head.cmd=%d, unkown, fuck!!!', stResponseMsg.head.cmd)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	if stResponseMsg.head.result != _msg.E_OK:
		logger.error('response.head.result=%d, not E_OK!', stResponseMsg.head.result)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	# 5.protobuf -> json, and do reply
	strMsgQueryRes = stResponseMsg.msg_query_res
	msgContentList = []
	preTime = 0
	for i in range(len(strMsgQueryRes.msg_content)):
		dictItem = {}
		msgContent = strMsgQueryRes.msg_content[i]
		dictItem['id'] = str(msgContent.id)
		dictItem['content'] = msgContent.content
		dictItem['openid_md5_from'] = str(msgContent.openid_md5_from)
		dictItem['openid_md5_to'] = str(msgContent.openid_md5_to)

		if i == 0:
			dictItem['create_ts'] = str(msgContent.create_ts)
		else:
			if msgContent.create_ts - preTime >= interval:
				dictItem['create_ts'] = str(msgContent.create_ts)
			else:
				dictItem['create_ts'] = '0'

		preTime = msgContent.create_ts

		msgContentList.append(dictItem)

	# 6.Get UserInfo
	listOpenidMd5 = []
	listOpenidMd5.append(openid_md5_from)
	listOpenidMd5.append(openid_md5_to)
	if FetchUserInfo(listOpenidMd5) < 0:
		logger.error('FetchUserInfo failed!!!')
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	# 7.Reply
	mycgi.addBody('msg_content_list', msgContentList)	
	mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_OK)
	return True



if __name__ == '__main__':
	# __file__ 获取模块所在路径
	mycgi = _cgi.myCgi(__file__, 'logging.conf')

	# logger	
	logger = mycgi.getLogger()

	InnerProcess()	