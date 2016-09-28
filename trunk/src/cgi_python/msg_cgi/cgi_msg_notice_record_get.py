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


def FetchFeedsInfo(listFeedId, iPaAppidMd5, iOpenidMd5):
	# 1.Build requestMsg protobuf
	stFeedDetailRequestMsg = _msg.Msg()

	stFeedDetailRequestMsgHead = stFeedDetailRequestMsg.head
	stFeedDetailRequestMsgHead.cmd = _msg.QUERY_FEED_DETAIL_REQ
	stFeedDetailRequestMsgHead.seq = int(time.time())

	stQueryFeedDetailReq = stFeedDetailRequestMsg.query_feed_detail_req
	stQueryFeedDetailReq.pa_appid_md5 = iPaAppidMd5
	stQueryFeedDetailReq.openid_md5 = iOpenidMd5

	for feedId in listFeedId:
		stQueryFeedDetailReq.feed_id_list.append(feedId)

	# 2.Send to server, and recv responseMsg protobuf	
	ioclient = _tcp_client.ProtoIOTcpClient(feedServerIP, feedServerPort)
	dictMsg = {'requestMsg':stFeedDetailRequestMsg, 'responseMsg':None, 'strErrMsg':None}

	ret = ioclient.io(dictMsg)

	if ret != 0:	
		logger.error('FetchFeedsInfo IO failed, errmsg = %s', dictMsg['strErrMsg'])
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return -1

	stFeedDetailResponseMsg = dictMsg['responseMsg']
	logger.debug('responseMsg: \n %s', stFeedDetailResponseMsg)

	if stFeedDetailResponseMsg.head.cmd != _msg.QUERY_FEED_DETAIL_RES:
		logger.error('FetchFeedsInfo response.head.cmd=%d, unkown, fuck!!!', stFeedDetailResponseMsg.head.cmd)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return -1

	if stFeedDetailResponseMsg.head.result != _msg.E_OK:
		logger.error('FetchFeedsInfo response.head.result=%d, not E_OK!', stFeedDetailResponseMsg.head.result)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return -1

	# 3.Parse
	listFeedId = [] #clear
	res = stFeedDetailResponseMsg.query_feed_detail_res
	dictFeedInfo = {}
	for i in range(len(res.feed_list)):
		stFeedInfo = res.feed_list[i]
		listFeedId.append(stFeedInfo.feed_id)

		dictFeedInfo[str(stFeedInfo.feed_id)] = FeedInfoPB2Dict(stFeedInfo)
	
	if len(dictFeedInfo) > 0:
		mycgi.addBody('feed_info', dictFeedInfo)

	return 0

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
	listOpenidMd5 = [] #clear
	res = stUserInfoResponseMsg.query_user_detail_info_res
	dictUserInfo = {}
	for i in range(len(res.user_detail_info_list)):
		stUserInfo = res.user_detail_info_list[i]
		listOpenidMd5.append(stUserInfo.openid_md5)

		dictUserInfo[str(stUserInfo.openid_md5)] = UserInfoPB2Dict(stUserInfo)
	
	if len(dictUserInfo) > 0:
		mycgi.addBody('user_info', dictUserInfo)

	return 0

def FetchCommentInfo(listFollowId, iPaAppidMd5, iOpenidMd5):
	# 1.Build requestMsg protobuf
	stRequestMsg = _msg.Msg()

	stRequestMsgHead = stRequestMsg.head
	stRequestMsgHead.cmd = _msg.QUERY_FOLLOW_DETAIL_REQ
	stRequestMsgHead.seq = int(time.time())

	stRequestMsgBody = stRequestMsg.query_follow_detail_req
	stRequestMsgBody.pa_appid_md5 = iPaAppidMd5
	stRequestMsgBody.openid_md5 = iOpenidMd5
	for followid in listFollowId:
		stRequestMsgBody.follow_id_list.append(followid)

	# 2.Send to server, and recv responseMsg protobuf	
	ioclient = _tcp_client.ProtoIOTcpClient(feedServerIP, feedServerPort)
	dictMsg = {'requestMsg':stRequestMsg, 'responseMsg':None, 'strErrMsg':None}

	ret = ioclient.io(dictMsg)

	if ret != 0:	
		logger.error('FetchCommentInfo IO failed, errmsg = %s', dictMsg['strErrMsg'])
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return -1

	stResponseMsg = dictMsg['responseMsg']
	logger.debug('responseMsg: \n %s', stResponseMsg)

	if stResponseMsg.head.cmd != _msg.QUERY_FOLLOW_DETAIL_RES:
		logger.error('FetchCommentInfo response.head.cmd=%d, unkown, fuck!!!', stResponseMsg.head.cmd)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return -1

	if stResponseMsg.head.result != _msg.E_OK:
		logger.error('FetchCommentInfo response.head.result=%d, not E_OK!', stResponseMsg.head.result)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return -1

	# 3.Parse
	listOpenidMd5 = []
	listFeedId = []
	stResponseMsgBody = stResponseMsg.query_follow_detail_res
	for follower in stResponseMsgBody.follow_list:
		listOpenidMd5.append(follower.openid_md5_from)
		listFeedId.append(follower.feed_id)
	# list去重
	# listOpenidMd5 = list(set(listOpenidMd5))

	# Get FeedsInfo
	if FetchFeedsInfo(listFeedId, iPaAppidMd5, iOpenidMd5) < 0:
		return -1

	# Get UserInfo
	if FetchUserInfo(listOpenidMd5) < 0:
		return -1

	# Gen FollowId set & FollowInfo list
	listFollowId = []
	listFollowInfo = []
	
	#倒序[::-1]	
	for i in range(len(stResponseMsgBody.follow_list))[::-1]:
		tmp_follow_feed_id = stResponseMsgBody.follow_list[i].feed_id
		tmp_follow_openid_md5 = stResponseMsgBody.follow_list[i].openid_md5_from
		if tmp_follow_feed_id in listFeedId and tmp_follow_openid_md5 in listOpenidMd5:
			listFollowId.append(stResponseMsgBody.follow_list[i].follow_id)
			listFollowInfo.append(FollowInfoPB2Dict(stResponseMsgBody.follow_list[i]))			

	mycgi.addBody('follow_list', listFollowInfo)
	return 0

def InnerProcess():	
	# 1.Get HTTP params
	# string -> uint64_t, long or int? however both test OK!
	# "python 自带大数整数运算，整数不会溢出，只要内存足够, 3.x后不再区分int/long,去除了long,统称int"
	#'''
	pa_appid_md5 = int(mycgi.getEnvValue('pa_appid_md5', 0))
	openid_md5 = int(mycgi.getEnvValue('openid_md5', 0))
	iType = int(mycgi.getEnvValue('type', 0))
	limit_ts = int(mycgi.getEnvValue('limit_ts', 0))
	pagesize = int(mycgi.getEnvValue('pagesize', 0)) 
	'''
	pa_appid_md5 = 6353185739184837437
	openid_md5 = 8363909989775180263
	iType = 2
	limit_ts = 0
	pagesize = 10
	'''
	
	# 2.Params check
	if pa_appid_md5 == 0 or openid_md5 == 0:
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_INVALID_PARAM)
		logger.error('invalid pa_appid_md5=%d, or openid_md5=%d', pa_appid_md5, openid_md5)
		return True

	if iType < 1 or iType > 3:
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_INVALID_PARAM)
		logger.error('invalid type=%d', iType)
		return True
	
	# 3.Build requestMsg protobuf
	stRequestMsg = _msg.Msg()

	stRequestMsgHeader = stRequestMsg.head
	stRequestMsgHeader.cmd = _msg.QUERY_NOTICE_RECORD_REQ
	stRequestMsgHeader.seq = int(time.time())

	stRequestMsgBody = stRequestMsg.notice_record_query_req
	stRequestMsgBody.pa_appid_md5 = pa_appid_md5
	stRequestMsgBody.openid_md5 = openid_md5
	stRequestMsgBody.type = iType
	stRequestMsgBody.limit_ts = limit_ts
	stRequestMsgBody.pagesize = pagesize 

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

	if stResponseMsg.head.cmd != _msg.QUERY_NOTICE_RECORD_RES:
		logger.error('response.head.cmd=%d, unkown, fuck!!!', stResponseMsg.head.cmd)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	if stResponseMsg.head.result != _msg.E_OK:
		logger.error('response.head.result=%d, not E_OK!', stResponseMsg.head.result)
		mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_SERVER_BUSY)		
		return True

	# 5.protobuf -> json, and do reply
	if iType == _comm_enum.NOTICE_RECORD_TYPE_COMMENT:
		listFollowId = []
		res = stResponseMsg.notice_record_query_res
		for i in range(len(res.notice_record_list)):
			listFollowId.append(int(res.notice_record_list[i].extra_data_2))

		#Get CommentInfo
		if FetchCommentInfo(listFollowId, pa_appid_md5, openid_md5) < 0:
			logger.error('FetchCommentInfo failed!')				
			return True

		#Gen notice record list
		for i in range(len(res.notice_record_list)):
			tmp_follow_id = int(res.notice_record_list[i].extra_data_2)
			if tmp_follow_id not in listFollowId:
				continue
			dictNoticeRecord = {}
			dictNoticeRecord['pa_appid_md5'] = str(res.notice_record_list[i].pa_appid_md5)
			dictNoticeRecord['openid_md5'] = str(res.notice_record_list[i].openid_md5)
			dictNoticeRecord['create_ts'] = str(res.notice_record_list[i].create_ts)
			dictNoticeRecord['type'] = res.notice_record_list[i].type
			dictNoticeRecord['status'] = res.notice_record_list[i].status
			dictNoticeRecord['extra_data_0'] = res.notice_record_list[i].extra_data_0
			dictNoticeRecord['extra_data_1'] = res.notice_record_list[i].extra_data_1
			dictNoticeRecord['extra_data_2'] = res.notice_record_list[i].extra_data_2

			mycgi.addBody('notice_record_list', dictNoticeRecord)

	elif iType == _comm_enum.NOTICE_RECORD_TYPE_FAVORITE:
		listFeedId = []
		listOpenidMd5 = []
		res = stResponseMsg.notice_record_query_res
		for i in range(len(res.notice_record_list)):
			listFeedId.append(int(res.notice_record_list[i].extra_data_0))
			listOpenidMd5.append(int(res.notice_record_list[i].extra_data_1))

		#Get FeedInfo
		if FetchFeedsInfo(listFeedId, pa_appid_md5, openid_md5) < 0:
			logger.error('FetchFeedsInfo failed!')				
			return True

		#Get UserInfo
		if FetchUserInfo(listOpenidMd5) < 0:
			logger.error('FetchUserInfo failed!')
			return True

		#Gen notice record list
		for i in range(len(res.notice_record_list)):
			tmp_feed_id = int(res.notice_record_list[i].extra_data_0)
			tmp_openid_md5 = int(res.notice_record_list[i].extra_data_1)

			if tmp_feed_id not in listFeedId or tmp_openid_md5 not in listOpenidMd5:
				continue
			dictNoticeRecord = {}
			dictNoticeRecord['pa_appid_md5'] = str(res.notice_record_list[i].pa_appid_md5)
			dictNoticeRecord['openid_md5'] = str(res.notice_record_list[i].openid_md5)
			dictNoticeRecord['create_ts'] = str(res.notice_record_list[i].create_ts)
			dictNoticeRecord['type'] = res.notice_record_list[i].type
			dictNoticeRecord['status'] = res.notice_record_list[i].status
			dictNoticeRecord['extra_data_0'] = res.notice_record_list[i].extra_data_0
			dictNoticeRecord['extra_data_1'] = res.notice_record_list[i].extra_data_1
			dictNoticeRecord['extra_data_2'] = res.notice_record_list[i].extra_data_2

			mycgi.addBody('notice_record_list', dictNoticeRecord)

	elif iType == _comm_enum.NOTICE_RECORD_TYPE_SYSTEM_MSG:
		res = stResponseMsg.notice_record_query_res
		for i in range(len(res.notice_record_list)):
			dictNoticeRecord = {}
			dictNoticeRecord['pa_appid_md5'] = str(res.notice_record_list[i].pa_appid_md5)
			dictNoticeRecord['openid_md5'] = str(res.notice_record_list[i].openid_md5)
			dictNoticeRecord['create_ts'] = str(res.notice_record_list[i].create_ts)
			dictNoticeRecord['type'] = res.notice_record_list[i].type
			dictNoticeRecord['status'] = res.notice_record_list[i].status
			dictNoticeRecord['extra_data_0'] = res.notice_record_list[i].extra_data_0
			dictNoticeRecord['extra_data_1'] = res.notice_record_list[i].extra_data_1
			dictNoticeRecord['extra_data_2'] = res.notice_record_list[i].extra_data_2

			mycgi.addBody('notice_record_list', dictNoticeRecord)

	mycgi.DoReply(CGI_RET_CODE.CGI_RET_CODE_OK)
	return True




if __name__ == '__main__':
	# __file__ 获取模块所在路径
	mycgi = _cgi.myCgi(__file__, 'logging.conf')	

	# logger
	logger = mycgi.getLogger()

	InnerProcess()	
