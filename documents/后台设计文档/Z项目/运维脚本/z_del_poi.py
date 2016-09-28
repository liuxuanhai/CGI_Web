#!/usr/bin/python
# -*- coding: UTF-8 -*-

import MySQLdb
import time

import z_bd_api

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

debug = True
debug = False

db_name = 'd_z_test'
order_info_name = 't_order_info'
ORDER_STATUS_FINISH = 3
ORDER_STATUS_CANCEL = 4
ORDER_DEL_TIME = 24 * 60 * 60

bd_api_ak = 'AjzvyDwSIojgsYIm18VgQ0UttpN1Qrd2'
bd_api_sk = 'P2odeT2uIo0HE8z2NmGQFa7HQL196u4d'
geotable_id = 146282  #z_project
geotable_id = 149210  #z_project_test 


if __name__ == '__main__':
	# connect DB
	# 177
	db = MySQLdb.connect('localhost', 'root', '.hoosho,9942', db_name, charset='utf8')
	# aliyun
	#db = MySQLdb.connect('localhost', 'root', '.hoosho,9942', db_name, charset='utf8', unix_socket='/data/mysql/mysql.sock')
	
	cursor = db.cursor()
	del_poi_list = []
	del_orderid_list = []
	cur_time = time.time()

	# get poi from bd
	page_index = 0
	page_size = 200
	#page_size = 2
	while True:
		resp = z_bd_api.get_poi_list(bd_api_sk, bd_api_ak, geotable_id, page_index, page_size)
		if resp['status'] != 0 and resp['status'] != 21:
			print 'BDAPI get_poi_list failed, status = %s, msg = %s' % (resp['status'], resp['message'])
			exit
		
		poi_list = resp['pois']
		if type(poi_list) != list:
			print 'invalid type(poi_list) = %s' % type(poi_list)
			exit	
		
		if resp['size'] == 0 or len(poi_list) == 0:
			print 'return 0 pois'
			break
		
		for poi in poi_list:
			orderid = poi['orderid']

			# get order info from db
			sql = 'SELECT order_status, finish_ts FROM %s WHERE orderid = %s' % (order_info_name, orderid)
			if debug:
				print 'sql = %s' % sql
			row_count = cursor.execute(sql)
			if row_count != 1:
				print 'ERROR, orderid = %s not found, del poi' % orderid
				del_poi_list.append(poi['id'])
				del_orderid_list.append(orderid);
				continue
			result = cursor.fetchone()
			if debug:
				print result
			if result[0] == ORDER_STATUS_FINISH or result[0] == ORDER_STATUS_CANCEL:
				if result[1] + ORDER_DEL_TIME < cur_time:
					print 'FOUND, orderid = %s has finished, del poi' % orderid
					del_poi_list.append(poi['id'])
					del_orderid_list.append(orderid);

		if resp['size'] < page_size:
			print 'req_size:%s > resp_size:%s, not more data' % (page_size, resp['size'])
			break	

		page_index += 1

	
	timeArray = time.localtime(cur_time)
	otherStyleTime = time.strftime("%Y-%m-%d %H:%M:%S", timeArray)
	print ''
	print 'Time: %s' % otherStyleTime
	print 'Del Poi List: %s' % del_poi_list
	print 'Del Orderid List: %s' % del_orderid_list
	#z_bd_api.del_poi_by_ids(bd_api_sk, bd_api_ak, geotable_id, del_poi_list)



