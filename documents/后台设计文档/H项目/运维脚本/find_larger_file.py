import os
import sys

debug = False

def FindLargerFile(target_dir, limit_size):
		
	listfile = os.listdir(target_dir)
	
	if debug:
		print '\n\n'
		print target_dir
		print listfile

	for f in listfile:
		f = target_dir + os.path.sep + f
		if debug:
			print f
		if os.path.isfile(f):
			if debug:
				print 'FILE', f
			size = os.path.getsize(f)
			#print f, size
			if size >= limit_size:
				print '%s, %s' % (f, size)

		elif os.path.isdir(f):
			if debug:
				print 'DIR', f
			FindLargerFile(f, limit_size)
			


if __name__ == '__main__':
	if len(sys.argv) != 3:
		print 'Usage: python %s TARGET_DIR LIMIT_SIZE' % sys.argv[0]
		exit(0)
	
	target_dir = sys.argv[1]
	limit_size = int(sys.argv[2])
	
	if not os.path.exists(target_dir):
		print 'Path %s not exist' % target_dir
		exit(0)
	
	if not os.path.isdir(target_dir):
		print '%s isn\'t path' % target_dir
		exit(0)
	
	#listfile = os.listdir(target_dir)
	#print listfile
	
	FindLargerFile(target_dir, limit_size)
