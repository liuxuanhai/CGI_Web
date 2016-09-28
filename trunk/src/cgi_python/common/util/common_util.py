#!/usr/bin/python
# -*- coding: UTF-8 -*-

# ip | port
msgServerIP = '127.0.0.1'
msgServerPort = 58999

feedServerIP = '127.0.0.1'
feedServerPort = 58998

userServerIP = '127.0.0.1'
userServerPort = 37858

def FollowInfoPB2Dict(pbFollow):
	dictFollow = {}
	dictFollow['follow_id'] = str(pbFollow.follow_id)
	dictFollow['content'] = pbFollow.content 
	dictFollow['feed_id'] = str(pbFollow.feed_id) 
	dictFollow['type'] = pbFollow.type 
	dictFollow['origin_comment_id'] = str(pbFollow.origin_comment_id) 
	dictFollow['openid_md5_from'] = str(pbFollow.openid_md5_from) 
	dictFollow['openid_md5_to'] = str(pbFollow.openid_md5_to) 
	dictFollow['create_ts'] = pbFollow.create_ts
	dictFollow['del_ts'] = pbFollow.del_ts

	return dictFollow

def FeedInfoPB2Dict(pbFeed):
	dictFeed = {}
	dictFeed["pa_appid_md5"] = str(pbFeed.pa_appid_md5)
	dictFeed["openid_md5"] = str(pbFeed.openid_md5)
	dictFeed["feed_id"] = str(pbFeed.feed_id)
	dictFeed["cover_pic_id"] = str(pbFeed.cover_pic_id)
	dictFeed["pic_id_list"] = pbFeed.pic_id_list
	dictFeed["content"] = pbFeed.content
	dictFeed["num_favorite"] = pbFeed.num_favorite
	dictFeed["num_share"] = pbFeed.num_share
	dictFeed["num_comment"] = pbFeed.num_comment
	dictFeed["num_read"] = pbFeed.num_read
	dictFeed["create_ts"] = pbFeed.create_ts
	dictFeed["del_ts"] = pbFeed.del_ts

	return dictFeed

def UserInfoPB2Dict(pbUser):
	dictUser = {}
	dictUser["openid_md5"] = str(pbUser.openid_md5)
	dictUser["portrait_pic_id"] = str(pbUser.portrait_pic_id)
	dictUser["wx_portrait_url"] = pbUser.portrait_pic_url_wx
	dictUser["nick"] = pbUser.nick
	dictUser["wx_nick"] = pbUser.nick_wx
	dictUser["addr_country"] = pbUser.addr_country
	dictUser["addr_city"] = pbUser.addr_city
	dictUser["sex"] = pbUser.sex
	dictUser["phone_no"] = str(pbUser.phone_no)
	dictUser["email"] = pbUser.email
	dictUser["self_desc"] = pbUser.self_desc
	dictUser["main_page_cover_pic_id"] = str(pbUser.main_page_cover_pic_id)
	dictUser["hoosho_no"] = str(pbUser.hoosho_no)
	dictUser["birthday"] = pbUser.birthday_ts

	return dictUser

