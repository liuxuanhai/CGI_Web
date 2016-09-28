#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os, sys, shutil, compileall

# Get current PATH : 
# os.getcwd() || os.path.abspath(os.curdir) || os.path.abspath('.')
curPath = os.getcwd() 
curPath_cgi = os.path.join(curPath, 'cgi')
curPath_proto = os.path.join(curPath, 'proto')
curPath_proto_io = os.path.join(curPath, 'proto_io')
curPath_util = os.path.join(curPath, 'util')
suffixs = ('.pyc')
destPath = '/home/dev/hoosho/cgi-bin/hp_python/common'
destPath_cgi = os.path.join(destPath, 'cgi')
destPath_proto =  os.path.join(destPath, 'proto')
destPath_proto_io = os.path.join(destPath, 'proto_io')
destPath_util = os.path.join(destPath, 'util')


# 过滤指定后缀文件
def endSuffix(s, *suffixs):
	ret = map(s.endswith, suffixs)
	if True in ret:
		return True
	else:
		return False

# 拷贝文件
def cpFile(srcPath, destPath):
	# 判断目录是否存在
	if not os.path.exists(destPath):
		os.makedirs(destPath)
	shutil.copy(srcPath, destPath)
	print 'cp %s -> %s' % (srcPath, destPath)

def installByPath(_curPath, _destPath):
	# 获取当前路径下所有文件
	files = os.listdir(_curPath)

	# 获取指定后缀文件列表
	suffiles = []
	for f in files:
		if endSuffix(f, suffixs):
			suffiles.append(f)

	# 补全文件绝对路径
	srcfiles = [os.path.join(_curPath, file) for file in suffiles]

	# 复制到指定路径
	for f in srcfiles:
		cpFile(f, _destPath)

def cleanByPath(_curPath):
	# 获取当前路径/lib下所有文件
	files = os.listdir(_curPath)

	# 获取指定后缀文件列表
	suffiles = []
	for f in files:
		if endSuffix(f, suffixs):
			suffiles.append(f)

	# 补全文件绝对路径
	srcfiles = [os.path.join(_curPath, file) for file in suffiles]

	# 清除子目录下的.pyc文件	
	for f in srcfiles:
		os.remove(f)

def build():	
	compileall.compile_dir(os.getcwd())
	os.remove(os.path.join(os.getcwd(), 'setup.pyc'))

	# 改变当前工作目录
	os.chdir('cgi/')
	compileall.compile_dir(os.getcwd())

	os.chdir('../proto/')
	compileall.compile_dir(os.getcwd())

	os.chdir('../proto_io/')
	compileall.compile_dir(os.getcwd())

	os.chdir('../util/')
	compileall.compile_dir(os.getcwd())

	os.chdir('../')

def install():
	installByPath(curPath, destPath)
	installByPath(curPath_cgi, destPath_cgi)
	installByPath(curPath_proto, destPath_proto)
	installByPath(curPath_proto_io, destPath_proto_io)
	installByPath(curPath_util, destPath_util)

def clean():
	cleanByPath(curPath)
	cleanByPath(curPath_cgi)
	cleanByPath(curPath_proto)
	cleanByPath(curPath_proto_io)
	cleanByPath(curPath_util)



if __name__ == '__main__':

	# 获取命令行参数 脚本名：sys.argv[0] 参数1：sys.argv[1] 参数2：sys.argv[2]
	if len(sys.argv) >= 2:		
		if sys.argv[1] == 'build':
			build()	
		elif sys.argv[1] == 'install':
			install()
		elif sys.argv[1] == 'clean':
			clean()
		else:
			print 'Usage: python setup.py <None | build | install | clean>'
		sys.exit(0)

	# 不带参数
	build()
	install()
	clean()
