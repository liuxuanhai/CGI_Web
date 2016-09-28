#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import MySQLdb
import time


db_user="root"
db_password=".hoosho,9942"
db_name=""
table_name=""
table_prefix=""
table_def=""

##########################################################################################################
#######                               用户信息库 d_user		                                       #######
##########################################################################################################
db_name = 'd_user'

#用户信息表，按虎笑openid_md5%256, 分256表
table_name_prefix="t_user_info_"

#头像url前缀
portrait_pic_url_prefix="http://www.hoosho.com/portrait_pic/"	

#默认的主页封面图片id
main_page_cover_pic_id="0"

#僵尸用户(zombie)表，单表
zombie_table_name="t_user_zombie"

def insert2db(key, zombie):	
	#print key, zombie[0], zombie[1], zombie[2], zombie[3], zombie[4], zombie[5] 
	print zombie[0], zombie[5]
	uin = key	
	nick_wx = zombie[0]
	addr_country = zombie[1]
	addr_province = zombie[2]
	addr_city = zombie[3]
	sex = zombie[4]
	openid_md5 = int(zombie[5])
	portrait_pic_url = os.path.join(portrait_pic_url_prefix, zombie[6])
	curTime = int(time.time())

	index = openid_md5 % 256
	str_index = '%02x' % index
	table_name = table_name_prefix + str_index.replace('0x', '')
	#print table_name	

	insert_values_def="openid_md5=" + str(openid_md5);
	insert_values_def+=", portrait_pic_url_wx=\"" + portrait_pic_url + "\"";
	insert_values_def+=", nick_wx=\"" + nick_wx + "\"";
	insert_values_def+=", addr_country=\"" + addr_country + "\"";
	insert_values_def+=", addr_province=\"" + addr_province + "\"";
	insert_values_def+=", addr_city=\"" + addr_city + "\"";
	insert_values_def+=", sex=" + str(sex);
	insert_values_def+=", main_page_cover_pic_id=" + str(main_page_cover_pic_id);
	insert_values_def+=", create_ts=" + str(curTime);
	insert_values_def+=", last_update_ts=" + str(curTime);
	#print insert_values_def

	update_values_def="nick_wx=\"" + nick_wx + "\"";
	update_values_def+=", portrait_pic_url_wx=\"" + portrait_pic_url + "\"";
	update_values_def+=", addr_country=\"" + addr_country + "\"";
	update_values_def+=", addr_province=\"" + addr_province + "\"";
	update_values_def+=", addr_city=\"" + addr_city + "\"";
	update_values_def+=", sex=" + str(sex);
	update_values_def+=", main_page_cover_pic_id=" + str(main_page_cover_pic_id);
	update_values_def+=", last_update_ts=" + str(curTime);
	#print update_values_def

	sql1 = 'INSERT INTO ' + table_name + ' SET ' + insert_values_def + \
		   ' ON DUPLICATE KEY UPDATE ' + update_values_def

	#print sql1
	try:
		# 使用execute方法执行SQL语句
		cursor.execute(sql1)		
		# 提交到数据库执行
		db.commit()

	# Rollback in case there is any error
	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])
		db.rollback()
		

	zombie_insert_values_def = "uin=\"" + uin + "\"" + ", openid_md5=" + str(openid_md5)
	#zombie_update_values_def = ""

	sql2 = 'INSERT INTO ' + zombie_table_name + ' SET ' + zombie_insert_values_def

	#print sql2

	try:
		# 使用execute方法执行SQL语句		
		cursor.execute(sql2)
		# 提交到数据库执行
		db.commit()

	# Rollback in case there is any error	
	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s'% (e.args[0], e.args[1])
		db.rollback()


# 获取源文件夹
def getSrcDirs(curpath):
	l = []	
	for d in os.listdir(curpath):
		if os.path.isdir(d):
			l.append(os.path.join(curPath, d))	
	return l		

# 过滤指定后缀文件
def endSuffix(s, *suffixs):
	ret = map(s.endswith, suffixs)
	if True in ret:
		return True
	else:
		return False

def t_main():
	# 僵尸账户起始id	
	for s_dir in srcDirs:
			
		# 获取文件夹名字，如 “青雉”
		pos = s_dir.rfind('/') + 1
		uin = s_dir[pos:]
		#print uin

		# 获取当前路径下所有文件
		files = os.listdir(s_dir)

		# 获取指定后缀文件列表
		suffix_files = []
		for f in files:
			if endSuffix(f, suffixs):
				suffix_files.append(f)

		for sf in suffix_files:
			#分割字符串，先将字符串翻转，分割4次
			pre_sf = sf.replace('.png', '').replace('.jpg', '')
			s = pre_sf[::-1].split("_",5)
			s.reverse()
			i = 0
			zombie = []
			
			while i<len(s):
				zombie.append(s[i][::-1])
				#print zombie[i+1]			
				i += 1
			
			# append头像绝对路径
			#zombie.append(os.path.join(s_dir, sf))
			# append头像相对路径
			zombie.append(os.path.join(uin, sf))
			#print zombie[i+1]
			insert2db(uin, zombie)
			





if __name__ == '__main__':

	# 打开数据库连接, unix_socket默认是/var/lib/mysql/mysql/sock, 这里要根据/etc/my.cnf里面的 "socket=" 路径指定
	# 如阿里云： db = MySQLdb.connect("localhost", db_user, db_password, db_name, charset="utf8", unix_socket="/data/mysql/mysql.sock")
	db = MySQLdb.connect("localhost", db_user, db_password, db_name, charset="utf8", unix_socket="/data/mysql/mysql.sock")

	# 使用cursor()方法获取游标操作
	cursor = db.cursor()	

	# 获取当前路径
	curPath = os.getcwd()
	# 获取当前路径下的目录文件
	srcDirs = getSrcDirs(curPath)
	# 过滤后缀
	suffixs = ('.png', '.jpg')	

	i = 0
	while i<len(srcDirs):
		print srcDirs[i]
		i = i + 1

	# 主函数
	t_main()

	# 关闭数据库连接
	db.close()
