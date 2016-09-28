#coding=utf-8

import MySQLdb
import random
import time

db_user="root"
db_password=".hoosho,9942"
db_name="d_feeds"

min_real_openid=20000
save_file_dir="save_file"
save_file = open(save_file_dir, "w")

table_prefix_feed_index="t_feed_index_on_appid_"
table_prefix_feed="t_feed_"

debug_flag = False
debug_flag = True 

#appid_list = [6353185739184837437, 1973794657378257811, 6572143484629934541, 10735465656521363209, 9205669606090411229, 9021482066981739660, 14790374237753402906, 11911923347215589852, 622427904226352406]
#appname_list = ["虎笑联盟", "启奏陛下", "绽放", "吃喝玩乐在北京", "当时我就震惊了", "秘密Secret", "任真天", "虎游戏", "C实习小助手"]

#print len(appid_list)," ",  len(appname_list)

conn = MySQLdb.connect(host='localhost',
	user=db_user,
	passwd=db_password,
	db=db_name,
	port=3306,
	charset='utf8',
	)
	#unix_socket="/data/mysql/mysql.sock")
cur = conn.cursor()

		#百灵(方琳) 05-12 16:20:05
		#0：发贴后每小时自动加3-13个阅读
		#1-2个赞：每小时自动加9-22个阅读
		#有评论及3个或以上赞：每小时自动加15-28个阅读
		#自动添加时间段：8:00--23:00

#根据等级获得相应阅读数增加量
lower = [3, 9, 15]
upper = [13, 22, 28]

#只增加2天内发表的feed的阅读量
count_day = 2
count_time = 60 * 60 * 24 * 2
begin_ts = int(time.time()) - count_time
print begin_ts

#根据feed点赞数、评论数计算feed等级
def cal_rank(feed):

	rank = 0	
	num_favorite = feed[6]
	num_comment = feed[8]
	if debug_flag:
		print 'favorite:', num_favorite
		print 'comment:', num_comment
	
	if num_favorite <= 0:
		rank = 0
	elif num_favorite >= 1 and num_favorite <= 2:
		rank = 1
	else:
		rank = 2

	if num_comment > 0:
		rank = 2

	return rank

def cal_num(rank):
	num = 0
	if rank < 0 or rank >= len(lower) or rank >= len(upper):
		print 'fucking rank'
		return 0
	num = random.randint(lower[rank], upper[rank])	
	return num

if __name__ == '__main__':
	for i in range(256):
		sql = 'SELECT * FROM ' + table_prefix_feed + '%02x' % i + ' WHERE del_ts = 0'
		if debug_flag:
			print sql
		cur.execute(sql)
		all = cur.fetchall()
		for one in all:
			if debug_flag:
				print 'feed_id:', one[0]
				print one
				print ''
			#2天前的不增加
			if one[10] < begin_ts:
				continue
			rank = cal_rank(one)
			num = cal_num(rank)
			if debug_flag:
				print 'rank:', rank
				print 'add_num:', num 
				print 'read:', one[9]
			num_read = one[9] + num
			sql = 'UPDATE ' + table_prefix_feed + '%02x' % i + ' SET num_read = %s' % num_read + ' WHERE feed_id = %s' % one[0]
			cur.execute(sql)
			conn.commit()

			if debug_flag:
				print sql


