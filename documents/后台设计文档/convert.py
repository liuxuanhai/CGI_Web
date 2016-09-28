#encoding=utf8

src_filename = 'S项目/存储文档/create_d_s.sh'
dest_pb_filename = 'test.proto'
dest_db2pb_filename = 'test_db2pb.h'
dest_pb2any_filename = 'test_pb2any.h'
pb_head = 'hoosho::msg::s::'

src_file_obj = open(src_filename)
dest_pb_obj = open(dest_pb_filename, 'w')
dest_db2pb_obj = open(dest_db2pb_filename, 'w')
dest_pb2any_obj = open(dest_pb2any_filename, 'w')

def DealTableName(line):
	#print 'NAME: %s' % line
	table_name = line[len('table_name="t_'):-1]
	#print 'NAME: %s' % table_name
	dest = ''
	upper = True
	for c in table_name:
		if c == '_':
			upper = True
			continue
		if upper:
			upper = False
			c = c.upper()
		dest += c
	return dest

def DealTableDef(line):
	end_pos = line.find('#')
	if end_pos == 0:
		end_pos = len(line)
	line = line[len('table_def+="'): end_pos]
	#print 'DEF: %s' % line
	arr = line.split()
	#print arr

	valname = arr[0]
	
	if arr[1].find('bigint') != -1:
		if arr[2] == 'unsigned':
			valtype = 'uint64'
		else:
			valtype = 'int64'
	elif arr[1].find('int') != -1:
		if arr[2] == 'unsigned':
			valtype = 'uint32'
		else:
			valtype = 'int32'
	else:
		valtype = 'string'
	return (valname, valtype)

if __name__ == '__main__':

	src_file_content = src_file_obj.readlines()
	clsname = ''
	first = True
	idx = 0
	for line in src_file_content:
		line = line.strip()
	
		if len(line) == 0 or line[0] == '#':
			continue
	
		if line.startswith('table_name="t_'):
			if first:
				first = False
			else:
				dest_pb_obj.write('}\n\n')
				dest_db2pb_obj.write('}\n\n')
				dest_pb2any_obj.write('\treturn any;\n')
				dest_pb2any_obj.write('}\n\n')


			clsname = DealTableName(line)
			#print 'Name: %s' % clsname

			#PB
			dest_pb_obj.write('message %s\n' % clsname)
			dest_pb_obj.write('{\n')
			idx = 0

			#DB2PB
			dest_db2pb_obj.write('inline void %sDB2PB(lce::cgi::CMysql& mysql, %s%s& st%s)\n' %(clsname, pb_head, clsname, clsname) )
			dest_db2pb_obj.write('{\n')

			#PB2Any
			dest_pb2any_obj.write('inline lce::cgi::CAnyValue %sPB2Any(%s%s pb)\n' % (clsname, pb_head, clsname))
			dest_pb2any_obj.write('{\n')
			dest_pb2any_obj.write('\tlce::cgi::CAnyValue any;\n')

		elif line.startswith('table_def+="'):
			#skip 
			if line.find('primary key') != -1:
				continue
			if line.find(')ENGINE=InnoDB') != -1:
				continue

			(valname, valtype) = DealTableDef(line)
			#print valname, valtype

			#DB2PB
			dest_db2pb_obj.write('\tst%s.set_%s(' % (clsname, valname))
			if valtype == 'string':
				dest_db2pb_obj.write('mysql.GetRow(%s)' % (idx))
			elif valtype == 'uint64' or valtype == 'int64':
				dest_db2pb_obj.write('strtoul(mysql.GetRow(%s), NULL, 10)' % (idx))
			else:
				dest_db2pb_obj.write('atoi(mysql.GetRow(%s))' % (idx))
			dest_db2pb_obj.write(');\n')

			#PB2Any
			if (valtype == 'uint64' or valtype == 'int64') and (valname.find('ts') == -1):
				dest_pb2any_obj.write('\tany["%s"] = int_2_str(pb.%s());\n' % (valname, valname))
			else:
				dest_pb2any_obj.write('\tany["%s"] = pb.%s();\n' % (valname, valname))

			idx += 1
			#PB
			dest_pb_obj.write('\toptional %s %s = %s;\n' % (valtype, valname, idx))

	if not first:
		dest_pb_obj.write('}\n\n')
		dest_db2pb_obj.write('}\n\n')
		dest_pb2any_obj.write('\treturn any;\n')
		dest_pb2any_obj.write('}\n\n')
