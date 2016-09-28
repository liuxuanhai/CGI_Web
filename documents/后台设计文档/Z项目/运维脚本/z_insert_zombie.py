#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import MySQLdb
import time


db_user="root"
db_password=".hoosho,9942"
db_name="d_z"
table_name="t_user_info"

#头像url前缀
portrait_pic_url_prefix="http://www.hoosho.com/portrait_pic/z/"	

def insert2db(_userinfo):		
	
	_userinfo[4] = portrait_pic_url_prefix + _userinfo[4]
	sql = 'INSERT INTO ' + table_name \
		 + ' SET openid=\'' + _userinfo[0] +'\'' \
		 + ', nickname=\'' + _userinfo[3] + '\'' \
		 + ', sex=' + _userinfo[1] \
		 + ', headimgurl=\'' + _userinfo[4] + '\'' \
		 + ', phone=\'' + _userinfo[2] + '\'' \
		 + ', user_type=2' \
		 + ' ON DUPLICATE KEY UPDATE' \
		 + ' nickname=\'' + _userinfo[3] + '\'' \
		 + ', sex=' + _userinfo[1] \
		 + ', headimgurl=\'' + _userinfo[4] + '\'' \
		 + ', phone=\'' + _userinfo[2] + '\'' 

	#print sql
	
	try:
		# 使用execute方法执行SQL语句
		cursor.execute(sql)		
		# 提交到数据库执行
		db.commit()

	# Rollback in case there is any error
	except MySQLdb.Warning, w:
		print 'Mysql Warning: %d:%s' % (w.args[0], w.args[1])
	except MySQLdb.Error, e:
		print 'Mysql Error: %d:%s' % (e.args[0], e.args[1])
		db.rollback()
	

# 获取源文件夹
def getSrcDirs(_curpath):
	l = []	
	for d in os.listdir(_curpath):
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

def t_main(_srcDirs):
	# 僵尸账户起始id	
	for s_dir in _srcDirs:
			
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
			#分割字符串，分割3次
			pre_sf = sf.replace('.png', '').replace('.jpg', '')
			s = pre_sf.split("_", 3)
			i = 0
			zombieinfo = []
			
			while i<len(s):
				zombieinfo.append(s[i])
				#print zombieinfo[i]			
				i += 1
			
			# append头像绝对路径
			#zombieinfo.append(os.path.join(s_dir, sf))
			# append头像相对路径
			zombieinfo.append(os.path.join(uin, sf))
			#print zombieinfo[i+1]
			insert2db(zombieinfo)

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
	t_main(srcDirs)

	# 关闭数据库连接
	db.close()
