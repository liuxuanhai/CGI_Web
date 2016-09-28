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

def FetchUserInfo(_listOpenidMd5, _pbUserInfoList):
	# 1.Build requestMsg protobuf
	stUserInfoRequestMsg = _msg.Msg()

	stUserInfoRequestMsgHead = stUserInfoRequestMsg.head
	stUserInfoRequestMsgHead.cmd = _msg.QUERY_USER_DETAIL_INFO_REQ
	stUserInfoRequestMsgHead.seq = int(time.time())

	stQueryUserDetailInfoReq = stUserInfoRequestMsg.query_user_detail_info_req
	for openidMd5 in _listOpenidMd5:
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

	for pbUserInfo in res.user_detail_info_list:
		_pbUserInfoList.append(pbUserInfo)
	
	return 0

def InnerProcess():
	# 1.Get HTTP params
	openid_md5 = int(mycgi.getEnvValue('openid_md5', 0))

	# 2.Params check
	if 0 == openid_md5:
		logger.error('INVALID PARAM!')
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_INVALID_PARAM)		
		return True

	# 3.Build requestMsg protobuf
	stRequestMsg = _msg.Msg()

	stRequestMsgHeader = stRequestMsg.head
	stRequestMsgHeader.cmd = _msg.QUERY_MSG_SESSION_LIST_REQ
	stRequestMsgHeader.seq = int(time.time())

	stRequestMsgBody = stRequestMsg.msg_query_session_list_req
	stRequestMsgBody.openid_md5 = openid_md5

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

	if stResponseMsg.head.cmd != _msg.QUERY_MSG_SESSION_LIST_RES:
		logger.error('response.head.cmd=%d, unkown, fuck!!!', stResponseMsg.head.cmd)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	if stResponseMsg.head.result != _msg.E_OK:
		logger.error('response.head.result=%d, not E_OK!', stResponseMsg.head.result)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	# 5.Get UserInfo
	listOpenidMd5 = []
	listOpenidMd5.append(openid_md5)
	stMsgQueryRes = stResponseMsg.msg_query_session_list_res
	for session in stMsgQueryRes.session:
		if openid_md5 == session.openid_md5_from:
			listOpenidMd5.append(session.openid_md5_to)
		else:
			listOpenidMd5.append(session.openid_md5_from)

	pbUserInfoList = []
	if FetchUserInfo(listOpenidMd5, pbUserInfoList) < 0:
		logger.error('FetchUserInfo failed!!!')
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)
		return True

	stHostUserInfo = UserInfoPB2Dict(pbUserInfoList[0])

	session_list = []
	for i in range(len(stMsgQueryRes.session))[::-1]:
		tSession = stMsgQueryRes.session[i]
		flag = False

		for j in range(len(pbUserInfoList))[::-1]:
			stGuestUserInfo = None
			tUserInfo = pbUserInfoList[j]
			if openid_md5 == tSession.openid_md5_from:
				if tUserInfo.openid_md5 == tSession.openid_md5_to:
					continue
				else:
					stGuestUserInfo = UserInfoPB2Dict(tUserInfo)
					flag = True
					break
			elif openid_md5 == tSession.openid_md5_to:
				if tUserInfo.openid_md5 == tSession.openid_md5_from:
					continue
				else:
					stGuestUserInfo = UserInfoPB2Dict(tUserInfo)
					flag = True
					break

		if flag is not True:
			continue

		dictItem = {}
		dictItem['openid_md5_from'] = str(tSession.openid_md5_from)
		dictItem['openid_md5_to'] = str(tSession.openid_md5_to)

		if openid_md5 == tSession.openid_md5_from:
			dictItem['userinfo_from'] = stHostUserInfo
			dictItem['userinfo_to'] = stGuestUserInfo
		else:
			dictItem['userinfo_from'] = stGuestUserInfo
			dictItem['userinfo_to'] = stHostUserInfo

		dictItem['content'] = tSession.content
		dictItem['create_ts'] = str(tSession.create_ts)
		dictItem['newmsg_status'] = tSession.newmsg_status

		session_list.append(dictItem)

	mycgi.addBody('session_list', session_list)

	mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_OK)
	return True



if __name__ == '__main__':
	# __file__ 获取模块所在路径
	mycgi = _cgi.myCgi(__file__, 'logging.conf')

	# logger	
	logger = mycgi.getLogger()

	InnerProcess()	