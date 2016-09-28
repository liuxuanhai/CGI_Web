#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os, sys, shutil, compileall

# Get current PATH : 
# os.getcwd() || os.path.abspath(os.curdir) || os.path.abspath('.')
curPath = os.getcwd() 
destPath = '/home/dev/hoosho/cgi-bin/hp_python/msg'

exe_files = [
'logging.conf',
'cgi_msg_add.py',
'cgi_msg_get_msg_list_by_session.py',
'cgi_msg_get_newmsg_status.py',
'cgi_msg_get_session_list.py',
'cgi_msg_notice_record_get.py',
'cgi_msg_system_message_get.py',
'cgi_msg_system_red_point_get.py',
'cgi_msg_system_red_point_mod.py',
'cgi_msg_zombie_feedsnum_get.py'
]

# 拷贝文件
def cpFile(srcPath, destPath):
	# 判断目录是否存在
	if not os.path.exists(destPath):
		os.makedirs(destPath)
	shutil.copy(srcPath, destPath)
	print 'cp %s -> %s' % (srcPath, destPath)

def install():	
	# 补全文件绝对路径
	exe_srcfiles = [os.path.join(curPath, file) for file in exe_files]	

	# 复制到指定路径
	for f in exe_srcfiles:
		cpFile(f, destPath)	



if __name__ == '__main__':

	# 获取命令行参数 脚本名：sys.argv[0] 参数1：sys.argv[1] 参数2：sys.argv[2]
	if len(sys.argv) >= 2:				
		if sys.argv[1] == 'install':
			install()		
		else:
			print 'Usage: python setup.py <None | install>'
		sys.exit(0)

	# 不带参数	
	install()
