#coding=utf-8


import MySQLdb

db_user="root"
db_password=".hoosho,9942"
db_name="d_feeds"

min_real_openid=20000
save_file_dir="save_file"
save_file = open(save_file_dir, "w")

table_prefix_feed_index="t_feed_index_on_appid_"
table_prefix_feed="t_feed_"

'''
#
#根据公众号列表,统计各公众号下的feed数和用户数
#
'''
appid_list = [6353185739184837437, 1973794657378257811, 6572143484629934541, 10735465656521363209, 9205669606090411229, 9021482066981739660, 14790374237753402906, 11911923347215589852, 622427904226352406]
appname_list = ["虎笑联盟", "启奏陛下", "绽放", "吃喝玩乐在北京", "当时我就震惊了", "秘密Secret", "任真天", "虎游戏", "C实习小助手"]

print len(appid_list)," ",  len(appname_list)

conn = MySQLdb.connect(host='localhost',
		user=db_user,
		passwd=db_password,
		db=db_name,
		port=3306,
		charset='utf8',
		)
		#unix_socket="/data/mysql/mysql.sock")
cur = conn.cursor()

# appid -> num
feed_num_real = {}
feed_num_real_del = {}
feed_num_false = {}
feed_num_false_del = {}

# appid -> num
user_num_real = {}
user_num_false = {}

# uniq user openid,  for count user_num
real_openid_list = []
false_openid_list = []

for (__i, appid) in enumerate(appid_list):
	print "公众号: ", appname_list[__i]
	print "appid: ", appid
	feed_num_real[appid] = 0
	feed_num_real_del[appid] = 0
	feed_num_false[appid] = 0
	feed_num_false_del[appid] = 0
	feed_num_real[appid] = 0
	feed_num_false[appid] = 0

	user_num_real[appid] = 0
	user_num_false[appid] = 0

	real_openid_list = []
	false_openid_list = []

	for i in range(256):
		#index = "%02x" % i
		#print index
		sql = "SELECT feed_id, openid_md5, del_ts FROM " + table_prefix_feed + "%02x" % i + " WHERE pa_appid_md5 = %s" % appid + " AND feed_type & 5 = feed_type "	
		#print sql
		try:
			cur.execute(sql)
		except MySQLdb.Error, e:
			print 'Mysql Error %d:%s' % (e.args[0], e.args[1])
			conn.rollback()

		all = cur.fetchall()
		for one in all:
			#print one
			feedid = one[0]
			openid = one[1]
			del_ts = one[2]
			#print type(feedid), type(openid), type(del_ts)
			
			if openid > min_real_openid:
				if del_ts > 0:
					feed_num_real_del[appid] += 1
				else:
					feed_num_real[appid] += 1
				real_openid_list.append(openid)
			else:
				#save_file.write("%s\n" % openid)
				if del_ts > 0:
					feed_num_false_del[appid] += 1
				else:
					feed_num_false[appid] += 1
				false_openid_list.append(openid)

	print "真实feed数: ", feed_num_real[appid]
	print "真实feed数（已删除）: ", feed_num_real_del[appid]
	print "僵尸feed数: ", feed_num_false[appid]
	print "僵尸feed数（已删除）: ", feed_num_false_del[appid]

	real_openid_list.sort()
	false_openid_list.sort()
	#real_openid_list = [1, 1, 1, 2, 3, 3, 4, 5, 5]
	#false_openid_list = [1, 1, 1, 2, 3, 3, 4, 5]

	save_file.write("\n\nAAA:%s\n" % appid)
	save_file.write("REAL\n")
	for openid in real_openid_list:
		save_file.write("%s\n" % openid)
	save_file.write("\n")

	save_file.write("FALSE\n")
	for openid in false_openid_list:
		save_file.write("%s\n" % openid)
	save_file.write("\n")

	for i in range(len(real_openid_list)) :
		if i == 0 or real_openid_list[i] != real_openid_list[i-1]:
			user_num_real[appid] += 1	

	for i in range(len(false_openid_list)) :
		if i == 0 or false_openid_list[i] != false_openid_list[i-1]:
			user_num_false[appid] += 1	

	print "真实粉丝数: ", user_num_real[appid]
	print "僵尸粉丝数: ", user_num_false[appid]

	print "\n"

