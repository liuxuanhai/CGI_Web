# -*- coding: utf-8 -*-
import urllib
import urllib2
import hashlib
import json
import time

debug = False
debug = True

def create_geotable(bd_api_sk, query_dict):
	global debug
	#http://api.map.baidu.com/geodata/v3/geotable/create  //POST请求
	url = 'http://api.map.baidu.com/geodata/v3/geotable/create'
	query_str = query_dict_2_str(query_dict)
	sn = cal_bd_api_sn(bd_api_sk, '/geodata/v3/geotable/create?', query_str)
	query_str += '&sn=' + sn
	if debug:
		print 'query_str: %s' % query_str
	req = urllib2.Request(url, query_str)
	resp = urllib2.urlopen(req)
	
	page = resp.read()
	if debug:
		print 'resp: %s' % page
	
	data = json.loads(page.decode('utf-8'))
	if debug:
		print json.dumps(data, indent=4)
	return data


	'''
title		poi名称	string(256)	可选
address		地址	string(256)	可选
tags		tags	string(256)	可选
latitude	用户上传的纬度	double	必选
longitude   用户上传的经度	double	必选
coord_type	用户上传的坐标的类型	uint32
geotable_id	记录关联的geotable的标识	string(50)	必选
ak			用户的访问权限key	string(50)	必选
sn			用户的权限签名	string(50)	可选
{column key}	用户在column定义的key/value对	开发者自定义的类型（string、int、double）	唯一索引字段必选，且需要保证唯一，否则会创建失败
	'''
def create_poi(bd_api_sk, bd_api_ak, geotable_id, latitude, longitude, coord_type, title = '', address = '', tags = '', custom_key = {}):
	global debug
	#http://api.map.baidu.com/geodata/v3/poi/create // POST请求
	url = 'http://api.map.baidu.com/geodata/v3/poi/create'
	query_dict = custom_key
	query_dict['geotable_id'] = geotable_id
	query_dict['ak'] = bd_api_ak
	query_dict['latitude'] = latitude
	query_dict['longitude'] = longitude
	query_dict['coord_type'] = coord_type
	query_dict['title'] = title
	query_dict['address'] = address
	query_dict['tags'] = tags

	query_str = query_dict_2_str(query_dict)
	sn = cal_bd_api_sn(bd_api_sk, '/geodata/v3/poi/create?', query_str)
	query_str += '&sn=' + sn
	if debug:
		print 'query_str: %s' % query_str

	req = urllib2.Request(url, query_str)
	resp = urllib2.urlopen(req)
	
	page = resp.read()
	if debug:
		print 'resp: %s' % page
	
	data = json.loads(page.decode('utf-8'))
	if debug:
		print json.dumps(data, indent=4)
	return data

def get_poi_list(bd_api_sk, bd_api_ak, geotable_id, page_index, page_size):
	global debug
	#http://api.map.baidu.com/geodata/v3/poi/list // GET请求
	url = 'http://api.map.baidu.com/geodata/v3/poi/list'
	query_dict = {}
	query_dict['geotable_id'] = geotable_id
	query_dict['ak'] = bd_api_ak
	query_dict['page_index'] = page_index
	query_dict['page_size'] = page_size 

	query_str = query_dict_2_str(query_dict)
	sn = cal_bd_api_sn(bd_api_sk, '/geodata/v3/poi/list?', query_str)
	query_str += '&sn=' + sn
	if debug:
		print 'query_str: %s' % query_str

	req = urllib2.Request(url + '?' + query_str)
	resp = urllib2.urlopen(req)
	
	page = resp.read()
	if debug:
		print 'resp: %s' % page
	
	data = json.loads(page.decode('utf-8'))
	if debug:
		print json.dumps(data, indent=4)
	return data

def del_poi_by_ids(bd_api_sk, bd_api_ak, geotable_id, ids):
	global debug
	
	if type(ids) == list:
		if len(ids) == 0:
			print 'invalid ids empty'
			return
		tmp = ''
		f = True
		for v in ids:
			if f:
				f = False
			else:
				tmp += ','
			tmp = '%s%s' % (tmp, v)
		ids = tmp

	elif type(ids) == str:
		if ids == '':
			print 'invalid ids empty'
			return
	else:
		print 'invalid type(ids) = %s' % type(ids)
		return 

	#http://api.map.baidu.com/geodata/v3/poi/delete // POST请求
	url = 'http://api.map.baidu.com/geodata/v3/poi/delete'
	query_dict = {}
	query_dict['ak'] = bd_api_ak
	query_dict['geotable_id'] = geotable_id
	query_dict['ids'] = ids
	query_dict['is_total_del'] = 1

	query_str = query_dict_2_str(query_dict)
	sn = cal_bd_api_sn(bd_api_sk, '/geodata/v3/poi/delete?', query_str)
	query_str += '&sn=' + sn
	if debug:
		print 'query_str: %s' % query_str

	req = urllib2.Request(url, query_str)
	resp = urllib2.urlopen(req)
	
	page = resp.read()
	if debug:
		print 'resp: %s' % page
	
	data = json.loads(page.decode('utf-8'))
	print json.dumps(data, indent=4)
	print data['message']
	return data

	
def query_dict_2_str(query_dict):
	global debug
	l = []
	for key in query_dict:
		l.append(key)
	l.sort()
	query_str = ''
	f = True
	for v in l:
		if type(query_dict[v]) == str and query_dict[v] == '':
			continue
		if f:
			f = False
		else:
			query_str += '&'
		if debug:
			print v, query_dict[v]
		query_str = '%s%s=%s' % (query_str, v, query_dict[v])
	return urllib.quote(query_str, safe = "/:=&?#+!$;'@()*[]")


def cal_bd_api_sn(bd_api_sk, api_str, query_str):
	global debug
	src = api_str + query_str + bd_api_sk
	if debug:
		print 'src: %s' % src
		print 'quote: %s' % urllib.quote_plus(src)
	sn = hashlib.md5(urllib.quote_plus(src)).hexdigest()
	return sn
	

if __name__ == '__main__':


	########
	geotable_id = 146860
	bd_api_ak = 'AjzvyDwSIojgsYIm18VgQ0UttpN1Qrd2'
	bd_api_sk = 'P2odeT2uIo0HE8z2NmGQFa7HQL196u4d'

	'''
	d = {}
	d['geotable_id'] = geotable_id
	d['ak'] = bd_api_ak
	d['geotype'] = 1
	d['is_published'] = 0
	d['name'] = 6789
	create_geotable(bd_api_sk, d)
	'''
	#resp = create_poi(bd_api_sk, bd_api_ak, geotable_id, 10, 20, 1, custom_key = {'new_col_0': int(time.time())})
	#resp = get_poi_list(bd_api_sk, bd_api_ak, geotable_id, 0, 200)
	#del_poi_by_ids(bd_api_sk, bd_api_ak, geotable_id, [1763541889,1769264448])

